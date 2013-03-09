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
  * dsf_file_reader.h
  * 
  * Header file for the class dsfFileReader.
  * 
  * This class extends dsdSampleReader providing acces to dsd samples and other info
  * from dsf files.
  * 
  * Some of the rarer features of dsf are not well tested due to a lack of files:
  * dsd64
  * 8bit dsd
  * 
  */

#ifndef DSFFILEREADER_H
#define DSFFILEREADER_H

#include "dsd_sample_reader.h" // Base class: dsdSampleReader
#include "id3/tag.h"
#include "id3/misc_support.h"

class dsfFileReader : public dsdSampleReader
{
public:
	// constructor and destructor
	dsfFileReader(char* filePath);
	virtual ~dsfFileReader();

public:
	// public methods required by dsdSampleReader
	long long unsigned int getLength();
	unsigned int getNumChannels();
	long long unsigned int getPosition();
	unsigned int getSamplingFreq();
	void step();
	void rewind();
	// overridden methods from dsdSampleReader
	unsigned char getIdleSample();
public:
	// other public methods
	void dispFileInfo();
	// warning these will return NULL if metadata does not exist
	char* getArtist() {return ID3_GetArtist ( &metadata );}
	char* getAlbum() {return ID3_GetAlbum ( &metadata );}
	char* getTitle() {return ID3_GetTitle ( &metadata );}
	char* getTrack() {return ID3_GetTrack ( &metadata );}
	char* getYear() {return ID3_GetYear ( &metadata );}
	
private:
	// private variables
	FILE *fid;
	// below store file info
	long long unsigned int fileSz;
	long long unsigned int metaChunkPointer;
	long long unsigned int sampleDataPointer;
	long long unsigned int dataChunkSz;
	long unsigned int formatVer;
	long unsigned int formatID;
	long unsigned int chanType;
	long unsigned int chanNum;
	long unsigned int samplingFreq;
	long unsigned int samplesPerChar;
	long long unsigned int sampleCount;
	long unsigned int blockSzPerChan;
	ID3_Tag metadata;
	// vars to hold the data and mark position
	unsigned char** blockBuffer; // used to store blocks of raw data from the file
	long unsigned int blockCounter; // stores the index to the current blockBuffer
	long unsigned int blockMarker; // stores the current position in the blockBuffer
	// other fields
	unsigned char idleSample; // the idle sample to for this file.
	
	// private methods
	void allocateBlockBuffer();
	void readHeaders();
	void readMetadata();
	bool readFirstBlock();
	bool readNextBlock();
};

#endif // DSFFILEREADER_H
