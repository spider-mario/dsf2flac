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
  * dsf_sample_reader.h
  * 
  * Header file for the class dsfSampleReader.
  * 
  * Abstract class defining anything which reads dsd samples from something.
  * If someone feels like it they could write an implementation of this class
  * and allow this convertor to work with other dsd sources (e.g. dsdiff files).
  * 
  */


#ifndef DSDSAMPLEREADER_H
#define DSDSAMPLEREADER_H

#include "stdio.h"
#include <boost/circular_buffer.hpp>

static const unsigned int defaultBufferLength = 1024;

class dsdSampleReader
{
public:
	// constructor and destructor
	dsdSampleReader();
	virtual ~dsdSampleReader();
	// abstract methods -> must be implemented by inheritors!
	virtual void step() = 0; // move buffer forward by 1byte (8bits) of sample data.
	virtual void rewind() = 0; // resets the dsd reader to the start of the dsd data.
	virtual unsigned int getNumChannels() = 0; // number of channels in the file
	virtual unsigned int getSamplingFreq() = 0; // the sampling freq of the dsd data
	virtual long long unsigned int getPosition() = 0; // current position in the file in samples
	virtual long long unsigned int getLength() = 0; // length of file in samples
	// public methods
	boost::circular_buffer<unsigned char>* getBuffer(); // get the char sample buffers (1 per channel) by default filled with getIdleSample()
	void setBufferLength(unsigned int bufferLength); // you can use this to set the length of the buffer WARNING: causes the file to be rewound!
	unsigned int getBufferLength(); // return the length of the circular buffers
	unsigned char getIdleSample() { return 0x69; }; // returns the idle tone used by this reader.
	double getPositionInSeconds(); // current position in the file in seconds
	double getPositionAsPercent();
	double getLengthInSeconds(); // length of file in seconds
	bool isEOF();
protected:
	// protected properties
	boost::circular_buffer<unsigned char>* circularBuffers;
protected:
	// protected methods, implementors need to call this!
	void allocateBuffer();
private:
	// private properties
	unsigned int bufferLength;
};

#endif // DSDSAMPLEREADER_H
