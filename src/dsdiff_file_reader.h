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
 


#ifndef DSDIFFFILEREADER_H
#define DSDIFFFILEREADER_H

#include "dsd_sample_reader.h" // Base class: dsdSampleReader
#include "fstream_plus.h"

class dsdiffFileReader : public dsdSampleReader
{
public:
	// constructor and destructor
	dsdiffFileReader(char* filePath);
	virtual ~dsdiffFileReader();

public:
	// public methods required by dsdSampleReader
	long long unsigned int getLength() {return sampleCount;};
	unsigned int getNumChannels() {return chanNum;};
	long long unsigned int getPosition() {return posMarker*samplesPerChar;};
	unsigned int getSamplingFreq() {return samplingFreq;};
	bool step();
	void rewind();
	bool msbIsYoungest() { return false;}
	// overridden methods from dsdSampleReader
	// unsigned char getIdleSample() {return 0;};
	unsigned char getIdleSample() {return idleSample;};
	bool isEOF() {return file.eof() || dsdSampleReader::isEOF();};
	
public:
	// other public methods
	void dispFileInfo();
	
private:
	// private variables
	fstreamPlus file;
	// read from the file
	unsigned int dsdiffVersion;
	unsigned int samplingFreq;
	unsigned short int chanNum;
	char** chanIdents;
	char  compressionType[5];
	char* compressionName;
	unsigned short ast_hours;
	unsigned char  ast_mins;
	unsigned char  ast_secs;
	unsigned int   ast_samples;
	unsigned int samplesPerChar;
	long long unsigned int sampleDataPointer;
	long long unsigned int sampleCount; //per channel
	
	// vars to hold the data and mark position
	long long unsigned int posMarker;
	unsigned char* sampleBuffer;
	// other fields
	unsigned char idleSample; // the idle sample to for this file.
	
	// private methods
	void allocateSampleBuffer();
	// all below here are for reading the headers
	bool readHeaders();
	static bool checkIdent(char* a, char* b); // MUST be used with the char[4]s or you'll get segfaults!
	bool readChunkHeader(char* ident, long long unsigned int chunkStart);
	bool readChunkHeader(char* ident, long long unsigned int chunkStart, long long unsigned int *chunkSz);
	// readers for specific types of chunk (return true if loaded ok)
	bool readChunk_FRM8(long long unsigned int chunkStart);
	bool readChunk_FVER(long long unsigned int chunkStart);
	bool readChunk_PROP(long long unsigned int chunkStart);
	bool readChunk_FS(long long unsigned int chunkStart);
	bool readChunk_CHNL(long long unsigned int chunkStart);
	bool readChunk_CMPR(long long unsigned int chunkStart);
	bool readChunk_ABSS(long long unsigned int chunkStart);
	bool readChunk_DSD(long long unsigned int chunkStart);
	
	
	
};

#endif // DSDIFFFILEREADER_H
