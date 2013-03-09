/**
 * dsf2flac - http://code.google.com/p/dsf2flac/
 * 
 * A file conversion tool for translating dsf dsd audio files into
 * flac pcm audio files.
 * 
 *
 * Copyright (c) 2013 by respective authors.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * 
 * Acknowledgements
 * 
 * Many thanks to the following authors and projects whose work has greatly
 * helped the development of this tool.
 * 
 * 
 * Sebastian Gesemann - dsd2pcm (http://code.google.com/p/dsd2pcm/)
 * SACD Ripper (http://code.google.com/p/sacd-ripper/)
 * Maxim V.Anisiutkin - foo_input_sacd (http://sourceforge.net/projects/sacddecoder/files/)
 * Vladislav Goncharov - foo_input_sacd_hq (http://vladgsound.wordpress.com)
 * 
 */
 
 /**
  * dsf_file_reader.cpp
  * 
  * Implementation of class dsfFileReader.
  * 
  * This class extends dsdSampleReader providing acces to dsd samples and other info
  * from dsf files.
  * 
  * Some of the rarer features of dsf are not well tested due to a lack of files:
  * dsd64
  * 8bit dsd
  */

#include "dsf_file_reader.h"

/**
 * dsfFileReader::dsfFileReader(char* filePath)
 * 
 * Constructor, pass in the location of a dsd file!
 */
dsfFileReader::dsfFileReader(char* filePath) : dsdSampleReader()
{
	// first let's open the file
	fid = fopen(filePath,"r");
	// throw exception if that did not work.
	if (fid==NULL) {
		fputs ("File error\n",stderr);
		exit (1);
	}
	// read the header data
	readHeaders();
	
	// this code only works with single bit data (could be upgraded later on)
	if (samplesPerChar!=8) {
		fputs ("Sorry, only one bit data is supported\n",stderr);
		exit (1);
	}
	// only 2822400Hz sampling rate is allowed (could be upgraded later on)
	if (getSamplingFreq()!=2822400) {
		fputs ("Sorry, only sampling rate 2822400Hz is supported\n",stderr);
		exit (1);
	}
	// read the metadata
	readMetadata();
	// allocate the blockBuffer
	allocateBlockBuffer();
	readFirstBlock(); // call here so that idle sample is set.... not great really.
	// allocate the circular buffer (this will cause rewind() to be called)
	allocateBuffer();
}

/**
 * dsfFileReader::~dsfFileReader()
 * 
 * Destructor, close the file and free the block buffers
 * 
 */
dsfFileReader::~dsfFileReader()
{
	// close the file
	fclose(fid);
	// free the mem in the block buffers
	for (long unsigned int i = 0; i<chanNum; i++) {
		delete[] blockBuffer[i];
	}
	delete[] blockBuffer;
}

/**
 * unsigned int dsfFileReader::getNumChannels()
 * 
 * Return the number of channels of audio in the dsd file.
 * 
 */
unsigned int dsfFileReader::getNumChannels()
{
	return chanNum;
}

/**
 * unsigned int dsfFileReader::getSamplingFreq()
 * 
 * Return the dsd sampling frequency.
 * Normally either 2822400Hz or 5644800Hz.
 * 
 */
unsigned int dsfFileReader::getSamplingFreq()
{
	return samplingFreq;
}

/**
 * long long unsigned int dsfFileReader::getPosition()
 * 
 * Returns the current location in the file in dsd samples.
 * 
 */
long long unsigned int dsfFileReader::getPosition()
{
	return (blockCounter*blockSzPerChan + blockMarker)*samplesPerChar;
}

/**
 * long long unsigned int dsfFileReader::getLength()
 * 
 * Returns the length of the file in samples.
 * i.e. each channel has getLength() dsd samples.
 * 
 */
long long unsigned int dsfFileReader::getLength()
{
	return sampleCount;
}

/**
 * void dsfFileReader::step()
 * 
 * Increments the position in the file by 8 dsd samples (1 byte of data).
 * The block buffers are updated with the new samples.
 * 
 */
void dsfFileReader::step()
{	
	bool ok = true;
	if (blockMarker>=blockSzPerChan) {
		ok = readNextBlock();
	}

	if (ok) {
		for (long unsigned int i=0; i<chanNum; i++)
			circularBuffers[i].push_front(blockBuffer[i][blockMarker]);
	} else {
		for (long unsigned int i=0; i<chanNum; i++)
			circularBuffers[i].push_front(getIdleSample());
	}

	blockMarker++;
}

/**
 * void dsfFileReader::rewind()
 * 
 * Rewind to the start of the dsd data.
 * 
 */
void dsfFileReader::rewind()
{
	readFirstBlock();
}

/**
 * unsigned char dsfFileReader::getIdleSample()
 * 
 * Return the idle sample used for this file.
 * 
 */
unsigned char dsfFileReader::getIdleSample()
{
	return idleSample;
}

/**
 * bool dsfFileReader::readFirstBlock()
 * 
 * The dsd file is arranged in blocks. This private function is called
 * by the constructor to read the first block into the block buffer.
 * 
 */
bool dsfFileReader::readFirstBlock()
{
	// position the file at the start of the data chunk
	fseek(fid,sampleDataPointer,SEEK_SET);
	blockCounter = 0;
	blockMarker = 0;
	bool r = readNextBlock();
	blockCounter = 0;
	idleSample = blockBuffer[0][0];
	return r;
}

/**
 * bool dsfFileReader::readNextBlock()
 * 
 * The dsd file is arranged in blocks. This private function is called whenever
 * new data from the file is needed for the buffer.
 * 
 */
bool dsfFileReader::readNextBlock()
{
	// return -1 if this is the end of the file
	if (isEOF()) {
		return false;
	}

	size_t res;
	for (long unsigned int i=0; i<chanNum; i++) {
		res = fread(blockBuffer[i],sizeof(unsigned char),blockSzPerChan,fid);
		if (res != (size_t)blockSzPerChan) {
			fputs ("Read error\n",stderr);
			exit (3);
		}
	}

	blockCounter++;
	blockMarker=0;

	return true;
}

/**
 * void dsfFileReader::readHeaders()
 * 
 * Private function, called on create. Reads lots of info from the file.
 * 
 */
void dsfFileReader::readHeaders()
{
	size_t res;
	long long unsigned int chunkStart;
	long long unsigned int chunkSz;
	char ident[4];

	// blank current variables (seems to be necessary??)
	fileSz = 0;
	sampleDataPointer = 0;
	metaChunkPointer = 0;
	dataChunkSz = 0;
	formatVer = 0;
	formatID = 0;
	chanType = 0;
	chanNum = 0;
	samplingFreq = 0;
	samplesPerChar = 1;
	sampleCount = 0;
	blockSzPerChan = 0;

	// double check that this is the start of the file.
	fseek(fid,0,SEEK_SET);

	// DSD CHUNK //
	chunkStart = ftell(fid);
	// 4 bytes which should be "DSD "
	res = fread(ident,1,4,fid);
	if (res!=4) {
		fputs ("Read error\n",stderr);
		exit (1);
	}
	if ( ident[0] != 'D' || ident[1] != 'S' || ident[2] != 'D' || ident[3] != ' ') {
		fputs ("File format error\n",stderr);
		exit (EXIT_FAILURE);
	}
	// 8 bytes chunk size
	res = fread(&chunkSz,8,1,fid);
	if (res!=1) {
		fputs ("Read error\n",stderr);
		exit (EXIT_FAILURE);
	}
	// 8 bytes file size
	res = fread(&fileSz,8,1,fid);
	if (res!=1) {
		fputs ("Read error\n",stderr);
		exit (EXIT_FAILURE);
	}
	// 8 bytes metadata pointer
	res = fread(&metaChunkPointer,8,1,fid);
	if (res!=1) {
		fputs ("Read error\n",stderr);
		exit (EXIT_FAILURE);
	}
	// we should be at the end of the DSD chunk now
	if ( chunkStart + chunkSz != (long unsigned int) ftell(fid) ) {
		fseek(fid,chunkStart + chunkSz,SEEK_SET);
	}

	// FMT CHUNK //
	chunkStart = ftell(fid);
	// 4 bytes which should be "fmt "
	res = fread(ident,1,4,fid);
	if (res!=4) {
		fputs ("Read error\n",stderr);
		exit (EXIT_FAILURE);
	}
	if ( ident[0] != 'f' || ident[1] != 'm' || ident[2] != 't' || ident[3] != ' ') {
		fputs ("File format error\n",stderr);
		exit (EXIT_FAILURE);
	}
	// 8 bytes chunk size
	res = fread(&chunkSz,8,1,fid);
	if (res!=1) {
		fputs ("Read error\n",stderr);
		exit (EXIT_FAILURE);
	}
	// 4 bytes format version
	res = fread(&formatVer,4,1,fid);
	if (res!=1) {
		fputs ("Read error\n",stderr);
		exit (EXIT_FAILURE);
	}
	// 4 bytes format id
	res = fread(&formatID,4,1,fid);
	if (res!=1) {
		fputs ("Read error\n",stderr);
		exit (EXIT_FAILURE);
	}
	// 4 bytes channel type
	res = fread(&chanType,4,1,fid);
	if (res!=1) {
		fputs ("Read error\n",stderr);
		exit (EXIT_FAILURE);
	}
	// 4 bytes channel num
	res = fread(&chanNum,4,1,fid);
	if (res!=1) {
		fputs ("Read error\n",stderr);
		exit (EXIT_FAILURE);
	}
	// 4 bytes samplingFreq
	res = fread(&samplingFreq,4,1,fid);
	if (res!=1) {
		fputs ("Read error\n",stderr);
		exit (EXIT_FAILURE);
	}
	// 4 bytes bitsPerSample
	long unsigned int bitsPerSample = 0;
	res = fread(&bitsPerSample,4,1,fid);
	if (res!=1) {
		fputs ("Read error\n",stderr);
		exit (EXIT_FAILURE);
	}
	if (bitsPerSample==1) {
		samplesPerChar = 8;
	} else if (bitsPerSample==8) {
		samplesPerChar = 1;
	}
	// 4 bytes sampleCount
	res = fread(&sampleCount,8,1,fid);
	if (res!=1) {
		fputs ("Read error\n",stderr);
		exit (EXIT_FAILURE);
	}
	// 4 bytes blockSzPerChan
	res = fread(&blockSzPerChan,4,1,fid);
	if (res!=1) {
		fputs ("Read error\n",stderr);
		exit (EXIT_FAILURE);
	}
	// 4 bytes ununsed
	fseek(fid,4,SEEK_CUR);
	// we are now at the end of the fmt chunk
	if ( chunkStart + chunkSz != (long unsigned int) ftell(fid) ) {
		fseek(fid,chunkStart + chunkSz,SEEK_SET);
	}

	// DATA CHUNK //
	// 4 bytes which should be "data"
	res = fread(ident,1,4,fid);
	if (res!=4) {
		fputs ("Read error\n",stderr);
		exit (EXIT_FAILURE);
	}
	if ( ident[0] != 'd' || ident[1] != 'a' || ident[2] != 't' || ident[3] != 'a') {
		fputs ("File format error\n",stderr);
		exit (EXIT_FAILURE);
	}

	// 8 bytes chunk size
	res = fread(&dataChunkSz,8,1,fid);
	if (res!=1) {
		fputs ("Read error\n",stderr);
		exit (EXIT_FAILURE);
	}

	// store the location of the data
	sampleDataPointer = ftell(fid);

	return;
}

/**
 * void dsfFileReader::allocateBlockBuffer()
 * 
 * Private function, called on create. Allocates the block buffer which
 * holds the dsd data read from the file for when it is required by the buffer.
 * 
 */
void dsfFileReader::allocateBlockBuffer()
{
	//blockBuffer = (unsigned char**) malloc(chanNum * sizeof(unsigned char *));
	blockBuffer = new unsigned char*[chanNum];
	if (blockBuffer == NULL) {
		fputs ("Memory error assigning blockBuffer\n",stderr);
		exit (EXIT_FAILURE);
	}
	for (long unsigned int i = 0; i<chanNum; i++) {
		//blockBuffer[i] = (unsigned char*) malloc(blockSzPerChan * sizeof(unsigned char));
		blockBuffer[i] = new unsigned char[blockSzPerChan];
		if (blockBuffer[i] == NULL) {
			fputs ("Memory error assigning blockBuffer\n",stderr);
			exit (EXIT_FAILURE);
		}
	}
}

/**
 * void dsfFileReader::readMetadata()
 * 
 * Private function, called on create. Attempts to read the metadata from the
 * end of the dsf file.
 * 
 */
void dsfFileReader::readMetadata()
{
	// zero if no metadata
	if (metaChunkPointer == 0) {
		return;
	}

	fseek(fid,metaChunkPointer,SEEK_SET);
	// read the first ID3_TAGHEADERSIZE bytes of the metadata (which should be the header).
	unsigned char id3header[ID3_TAGHEADERSIZE];
	size_t res = fread(id3header,1,ID3_TAGHEADERSIZE,fid);
	if (res != (size_t) ID3_TAGHEADERSIZE) {
		return;
	}
	// check this is actually an id3 header
	long long unsigned int id3tagLen;
	if ( (id3tagLen = ID3_IsTagHeader(id3header)) > -1 )
		return;
	// read the tag
	unsigned char* id3tag = new unsigned char[ id3tagLen ];
	res = fread(id3tag,1,id3tagLen,fid);
	if (res != (size_t) id3tagLen) {
		return;
	}
		
	metadata.Parse (id3header, id3tag);
	
	delete[] id3tag;
}



/**
 * void dsfFileReader::dispFileInfo()
 *
 * Can be called to display some useful info to stdout.
 * 
 */
void dsfFileReader::dispFileInfo()
{
	printf("filesize: %llu\n",fileSz);
	printf("metaChunkPointer: %llu\n",metaChunkPointer);
	printf("sampleDataPointer: %llu\n",sampleDataPointer);
	printf("dataChunkSz: %llu\n",dataChunkSz);
	printf("formatVer: %lu\n",formatVer);
	printf("formatID: %lu\n",formatID);
	printf("chanType: %lu\n",chanType);
	printf("chanNum: %lu\n",chanNum);
	printf("samplingFreq: %lu\n",samplingFreq);
	printf("samplesPerChar: %lu\n",samplesPerChar);
	printf("sampleCount: %llu\n",sampleCount);
	printf("blockSzPerChan: %lu\n",blockSzPerChan);

	return;
}
