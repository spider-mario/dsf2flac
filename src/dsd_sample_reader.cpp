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
 * dsd_sample_reader.cpp
 *
 * Implementation of class dsdSampleReader.
 *
 * Abstract class defining anything which reads dsd samples from something.
 * If someone feels like it they could write an implementation of this class
 * and allow this convertor to work with other dsd sources (e.g. dsdiff files).
 *
 */

#include <dsd_sample_reader.h>

/**
 * dsdSampleReader::dsdSampleReader()
 *
 * Constructor!
 *
 */
dsdSampleReader::dsdSampleReader()
{
	bufferLength = defaultBufferLength;
	isBufferAllocated = false;
}

/**
 * dsdSampleReader::dsdSampleReader()
 *
 * Destructor, free the circular buffers.
 *
 */
dsdSampleReader::~dsdSampleReader()
{
	if (isBufferAllocated)
		delete[] circularBuffers;
	isBufferAllocated = false;
}

/**
 * boost::circular_buffer<unsigned char>* dsdSampleReader::getBuffer()
 *
 * Returns the circular buffers, one per channel in an array.
 * When step() is called a new 1byte (8 dsd bits) sample is added to the start
 * of the buffer and one is removed from the end. If you grab this buffer
 * you can easily do some fir filtering by multiplying the contents by an impulse
 * response, call step() to shift to the next dsd sample and do the same!
 *
 */
boost::circular_buffer<unsigned char>* dsdSampleReader::getBuffer()
{
	return circularBuffers;
}

/**
 * unsigned int dsdSampleReader::getBufferLength()
 *
 * Returns the current length of the buffers.
 *
 */
unsigned int dsdSampleReader::getBufferLength()
{
	return bufferLength;
}

/**
 * void dsdSampleReader::setBufferLength(unsigned int b)
 *
 * Sets the length of the buffers.
 * WARNING! calling this will cause the dsd reader to be rewound to the
 * beginning and the buffer will be filled by getIdleSample();
 *
 */
bool dsdSampleReader::setBufferLength(unsigned int b)
{
	if (b<1) {
		errorMsg = "dsdSampleReader::setBufferLength:buffer length must be >0";
		return false;
	}
	bufferLength=b;
	resizeBuffer();
	rewind();
	return true;
}

/**
 * double dsdSampleReader::getPositionInSeconds()
 *
 * Return the current position in seconds.
 *
 */
double dsdSampleReader::getPositionInSeconds()
{
	return getPosition() / (double) getSamplingFreq();
}

/**
 * double dsdSampleReader::getPositionAsPercent()
 *
 * Return the current position as a percent of the reader length.
 *
 */
double dsdSampleReader::getPositionAsPercent()
{
	return 100* getPosition() / (double) getLength();
}

/**
 * double dsdSampleReader::getLengthInSeconds()
 *
 * Return the total length of the reader in seconds
 *
 */
double dsdSampleReader::getLengthInSeconds()
{
	return getLength() / (double) getSamplingFreq();
}

/**
 * bool dsdSampleReader::isValid()
 *
 * Return false if the reader is invalid (format/file error for example).
 *
 */
bool dsdSampleReader::isValid()
{
	return valid;
}

/**
 * bool dsdSampleReader::getErrorMsg()
 *
 * Returns a message describing the last error which caused the reader to become invalid.
 *
 */
std::string dsdSampleReader::getErrorMsg()
{
	return errorMsg;
}

/**
 * void dsdSampleReader::allocateBuffer()
 *
 * Protected function, allocate the buffer.
 * MUST be called by implementors on construction.
 */
void dsdSampleReader::allocateBuffer()
{
	if (isBufferAllocated)
		return;
		
	circularBuffers = new boost::circular_buffer<unsigned char> [getNumChannels()];
	for (long unsigned int i = 0; i<getNumChannels(); i++) {
		boost::circular_buffer<unsigned char> cb(getBufferLength());
		circularBuffers[i] = cb;
	}
	isBufferAllocated = true;
	clearBuffer();
	return;
}

/**
 * void dsdSampleReader::clearBuffer()
 *
 * Protected function, clear the buffer - fill with idleSamples
 * MUST be called by implementors on construction.
 */
void dsdSampleReader::clearBuffer() 
{
	if (!isBufferAllocated) {
		allocateBuffer();
		return;
	}
	
	unsigned char c = getIdleSample();
	for (long unsigned int i = 0; i<getNumChannels(); i++)
		for (unsigned int j=0; j<getBufferLength(); j++)
			circularBuffers[i].push_front(c);

}

/**
 * void dsdSampleReader::resizeBuffer()
 *
 * Protected function, resize the buffer and then clear the buffer.
 * MUST be called by implementors on construction.
 */
void dsdSampleReader::resizeBuffer() {
	if (!isBufferAllocated) {
		allocateBuffer();
		return;
	}
	for (long unsigned int i = 0; i<getNumChannels(); i++)
		circularBuffers[i].set_capacity(getBufferLength());
	clearBuffer();
}



char* dsdSampleReader::latin1_to_utf8(char* latin1) {
	return reinterpret_cast<char*>(latin1_to_utf8( reinterpret_cast<unsigned char*>(latin1)));
}
unsigned char* dsdSampleReader::latin1_to_utf8(unsigned char* latin1)
{
	if (latin1==NULL)
		return NULL;
		
	// count latin1
	int n=0;
	while (latin1[n])
		n++;
	// make buffer for converted chars
	unsigned char* utf8 = new unsigned char[n*2+1];
	unsigned char* utf8_t = utf8;

	while (*latin1) {
		if (*latin1<128)
			*utf8++=*latin1++;
		else
			*utf8++=0xc2+(*latin1>0xbf), *utf8++=(*latin1++&0x3f)+0x80;
	}
	*utf8='\0';
	
	return utf8_t;
}
