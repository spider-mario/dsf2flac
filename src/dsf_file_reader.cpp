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
 * Jesus R - www.sonore.us
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

static bool blockBufferAllocated = false;

/**
 * dsfFileReader::dsfFileReader(char* filePath)
 *
 * Constructor, pass in the location of a dsd file!
 */
dsfFileReader::dsfFileReader(char* filePath) : dsdSampleReader()
{
	
	// first let's open the file
	file.open(filePath, fstreamPlus::in | fstreamPlus::binary);
	// throw exception if that did not work.
	if (!file.is_open()) {
		errorMsg = "could not open file";
		valid = false;
		return;
	}
	// read the header data
	if (!(valid = readHeaders()))
		return;

	// this code only works with single bit data (could be upgraded later on)
	if (samplesPerChar!=8) {
		errorMsg = "Sorry, only one bit data is supported";
		valid = false;
		return;
	}
	// read the metadata
	readMetadata();
	// allocate the blockBuffer
	allocateBlockBuffer();
	readFirstBlock();
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
	file.close();
	// free the mem in the block buffers
	if (blockBufferAllocated) {
		for (long unsigned int i = 0; i<chanNum; i++)
		{
			delete[] blockBuffer[i];
		}
		delete[] blockBuffer;
	}
}

/**
 * void dsfFileReader::step()
 *
 * Increments the position in the file by 8 dsd samples (1 byte of data).
 * The block buffers are updated with the new samples.
 *
 */
bool dsfFileReader::step()
{
	bool ok = true;
	
	if (isEOF())
		ok = false;
	else if (blockMarker>=blockSzPerChan)
		ok = readNextBlock();

	if (ok) {
		for (long unsigned int i=0; i<chanNum; i++)
			circularBuffers[i].push_front(blockBuffer[i][blockMarker]);
	} else {
		for (long unsigned int i=0; i<chanNum; i++)
			circularBuffers[i].push_front(getIdleSample());
	}

	blockMarker++;
	return ok;
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
	if (file.seekg(sampleDataPointer)) {
		errorMsg = "dsfFileReader::readFirstBlock:file seek error";
		return false;
	}
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
		// fill the blockBuffer with the idle sample
		unsigned char idle = getIdleSample();
		for (unsigned int i=0; i<chanNum; i++)
			for (unsigned int j=0; j<blockSzPerChan; j++)
				blockBuffer[i][j] = idle;
		return false;
	}

	for (unsigned int i=0; i<chanNum; i++) {
		if (file.read_uchar(blockBuffer[i],blockSzPerChan)) {
			// if read failed fill the blockBuffer with the idle sample
			unsigned char idle = getIdleSample();
			for (unsigned int i=0; i<chanNum; i++)
				for (unsigned int j=0; j<blockSzPerChan; j++)
					blockBuffer[i][j] = idle;
			return false;
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
bool dsfFileReader::readHeaders()
{
	long long unsigned int chunkStart;
	long long unsigned int chunkSz;
	char ident[4];

	// double check that this is the start of the file.
	if (file.seekg(0)) {
		errorMsg = "dsfFileReader::readHeaders:file seek error";
		return false;
	}

	// DSD CHUNK //
	chunkStart = file.tellg();
	// 4 bytes which should be "DSD "
	if (file.read_char(ident,4)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	if ( !checkIdent(ident,const_cast<char*>("DSD ")) ) {
		errorMsg = "dsfFileReader::readHeaders:DSD ident error";
		return false;
	}
	// 8 bytes chunk size
	if (file.read_llui(&chunkSz,1)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	// 8 bytes file size
	if (file.read_llui(&fileSz,1)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	// 8 bytes metadata pointer
	if (file.read_llui(&metaChunkPointer,1)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	// we should be at the end of the DSD chunk now
	if ( chunkStart + chunkSz != (long long unsigned int) file.tellg() ) {
		if(file.seekg(chunkStart + chunkSz)) {
			errorMsg = "dsfFileReader::readHeaders:file seek error";
			return false;
		}
	}

	// FMT CHUNK //
	chunkStart = file.tellg();
	// 4 bytes which should be "fmt "
	if (file.read_char(ident,4)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	if ( !checkIdent(ident,const_cast<char*>("fmt ")) ) {
		errorMsg = "dsfFileReader::readHeaders:file ident error";
		return false;
	}
	// 8 bytes chunk size
	if (file.read_llui(&chunkSz,1)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	// 4 bytes format version
	if (file.read_ui(&formatVer,1)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	// 4 bytes format id
	if (file.read_ui(&formatID,1)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	// 4 bytes channel type
	if (file.read_ui(&chanType,1)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	// 4 bytes channel num
	if (file.read_ui(&chanNum,1)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	// 4 bytes samplingFreq
	if (file.read_ui(&samplingFreq,1)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	// 4 bytes bitsPerSample
	unsigned int bitsPerSample = 0;
	if (file.read_ui(&bitsPerSample,1)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	if (bitsPerSample==1) {
		samplesPerChar = 8;
	} else if (bitsPerSample==8) {
		samplesPerChar = 1;
	}
	// 8 bytes sampleCount
	if (file.read_llui(&sampleCount,1)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	// 4 bytes blockSzPerChan
	if (file.read_ui(&blockSzPerChan,1)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	// 4 bytes ununsed
	if (file.seekg(4,fstreamPlus::cur)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	// we are now at the end of the fmt chunk
	if ( chunkStart + chunkSz != (long long unsigned int) file.tellg() ) {
		if (file.seekg(chunkStart + chunkSz)) {
			errorMsg = "dsfFileReader::readHeaders:file seek error";
			return false;
		}
	}

	// DATA CHUNK //
	// 4 bytes which should be "data"
	if (file.read_char(ident,4)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	if ( !checkIdent(ident,const_cast<char*>("data")) ) {
		errorMsg = "dsfFileReader::readHeaders:file ident error";
		return false;
	}
	// 8 bytes chunk size
	if (file.read_llui(&dataChunkSz,1)) {
		errorMsg = "dsfFileReader::readHeaders:file read error";
		return false;
	}
	// store the location of the data
	sampleDataPointer = file.tellg();

	return true;
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
	blockBuffer = new unsigned char*[chanNum];
	for (long unsigned int i = 0; i<chanNum; i++)
		blockBuffer[i] = new unsigned char[blockSzPerChan];
	blockBufferAllocated = true;
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

	if (file.seekg(metaChunkPointer)) {
		// if we failed then let's not worry too much
		file.clear();
		return;
	}
	
	// read the first ID3_TAGHEADERSIZE bytes of the metadata (which should be the header).
	unsigned char id3header[ID3_TAGHEADERSIZE];
	if (file.read_uchar(id3header,ID3_TAGHEADERSIZE)) {
		return;
	}
	// check this is actually an id3 header
	long long unsigned int id3tagLen;
	if ( (id3tagLen = ID3_IsTagHeader(id3header)) > -1 )
		return;
	// read the tag
	unsigned char* id3tag = new unsigned char[ id3tagLen ];
	if (file.read_uchar(id3tag,id3tagLen)) {
		return;
	}
	
	metadata.Parse (id3header, id3tag);
	
	delete[] id3tag;
}

/**
 * bool dsfFileReader::checkIdent(char* a, char* b)
 *
 * private method, pretty handy for cheking idents
 *
 */
bool dsfFileReader::checkIdent(char* a, char* b)
{
	return ( a[0]==b[0] && a[1]==b[1] && a[2]==b[2] && a[3]==b[3] );
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
	printf("formatVer: %u\n",formatVer);
	printf("formatID: %u\n",formatID);
	printf("chanType: %u\n",chanType);
	printf("chanNum: %u\n",chanNum);
	printf("samplingFreq: %u\n",samplingFreq);
	printf("samplesPerChar: %u\n",samplesPerChar);
	printf("sampleCount: %llu\n",sampleCount);
	printf("blockSzPerChan: %u\n",blockSzPerChan);

	return;
}
