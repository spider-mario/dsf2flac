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
  * dsd_decimator.cpp
  * 
  * Implementation of the class dsdDecimator.
  * 
  * The dsdDecimator class does the actual conversion from dsd to pcm. Pass in a dsdSampleReader
  * to the create function along with the desired pcm sample rate (must be multiple of 44.1k).
  * Then you can simply read pcm samples into a int or float buffer using getSamples.
  * 
  */

#include "dsd_decimator.h"
#include <math.h>
#include "filters.cpp"

static bool lookupTableAllocated = false;

/**
 * dsdDecimator::dsdDecimator(dsdSampleReader *r, unsigned int rate)
 * 
 * Constructor!
 * 
 * Pass in a dsdSampleReader and the desired output sample rate.
 * 
 */
dsdDecimator::dsdDecimator(dsdSampleReader *r, unsigned int rate)
{
	reader = r;
	outputSampleRate = rate;
	valid = true;;
	errorMsg = "";
	
	// ratio of out to in sampling rates
	unsigned int ratio = r->getSamplingFreq() / outputSampleRate;
	
	// load the required filter into the lookuptable based on in and out sample rate
	if (ratio == 8)
		initLookupTable(nCoefs_352,coefs_352);
	else if (ratio == 16)
		initLookupTable(nCoefs_176,coefs_176);
	else if (ratio == 32)
		initLookupTable(nCoefs_88,coefs_88);
	else
	{
		valid = false;
		errorMsg = "Sorry, incompatible sample rate combination";
		return;
	}
	
	// set the buffer to the length of the table if not long enough
	if (nLookupTable > reader->getBufferLength())
		reader->setBufferLength(nLookupTable);
		
	// how many bytes to skip after each output sample is calculated.
	skip = ratio/8 - 1; 
}

/**
 * dsdDecimator::~dsdDecimator
 * 
 * Destructor! Free the lookuptable
 * 
 */
dsdDecimator::~dsdDecimator()
{
	if (lookupTableAllocated) {
		for (unsigned int n=0; n<nLookupTable; n++)
		{
			delete[] lookupTable[n];
		}
		delete[] lookupTable;
	}
}

/**
 * unsigned long long int dsdDecimator::dsdDecimator::getLength()
 * 
 * Return the reader length in output samples
 * 
 */
unsigned long long int dsdDecimator::getLength()
{
	return (*reader).getLength()*outputSampleRate/(*reader).getSamplingFreq();
}

/**
 * unsigned int dsdDecimator::getOutputSampleRate()
 * 
 * Return the output sample rate
 * 
 */
unsigned int dsdDecimator::getOutputSampleRate()
{
	return outputSampleRate;
}

/**
 * bool dsdDecimator::isValid()
 * 
 * Return false if the reader is invalid (format/file error for example).
 * 
 */
bool dsdDecimator::isValid()
{
	return valid;
}

/**
 * bool dsdDecimator::getErrorMsg()
 * 
 * Returns a message describing the last error which caused the reader to become invalid.
 * 
 */
std::string dsdDecimator::getErrorMsg() {
	return errorMsg;
}

/**
 * void dsdDecimator::initLookupTable(const double nCoefs,const double* coefs)
 * 
 * private method. Initialises the lookup table used for very fast filtering.
 * 
 */
void dsdDecimator::initLookupTable(const double nCoefs,const double* coefs)
{
	// calc how big the lookup table is.
	nLookupTable = (nCoefs+7)/8;
	// allocate the table
	lookupTable = new calc_type*[nLookupTable];
	for (unsigned int n=0; n<nLookupTable; n++)
	{
		lookupTable[n] = new calc_type[256];
		for (int m=0; m<256; m++)
			lookupTable[n][m] = 0;
	}
	// loop over each entry in the lookup table
	for (unsigned int t=0; t<nLookupTable; t++) {
		// how many samples from the filter are spanned in this entry
		int k = nCoefs - t*8;
		if (k>8) k=8;
		// loop over all possible 8bit dsd sequences
		for (int dsdSeq=0; dsdSeq<256; ++dsdSeq) {
			double acc = 0.0;
			for (int bit=0; bit<k; bit++) {
				double val;
				if (reader->msbIsYoungest())
					val = -1 + 2*(double) !!( dsdSeq & (1<<(7-bit)) );
				else
					val = -1 + 2*(double) !!( dsdSeq & (1<<(bit)) );
				acc += val * coefs[t*8+bit];
			}
			lookupTable[t][dsdSeq] = (calc_type) acc;
		}
	}
	lookupTableAllocated = true;
}


/**
 * template <typename sampleType> void getSamples(sampleType *buffer, unsigned int bufferLen, double scale, double tpdfDitherPeakAmplitude);
 * 
 * Read pcm output samples in format sampleType into a buffer of length bufferLen.
 * bufferLen must be a multiple of getNumChannels(). Channels are interleaved into the buffer.
 * 
 * You also need to provide a scaling factor. This is particularly important for int sample types. The raw dsd data has peak amplitude +-1.
 * 
 * If you wish to add TPDF dither to the data before quantization then please also provide the peak amplitude.
 * 
 * These sample types are supported:
 * 
 * short int
 * int
 * long int
 * float
 * double
 * 
 * Others should be very simple to add (just take a look at the templates below).
 * 
 */
template<> void dsdDecimator::getSamples(short int *buffer, unsigned int bufferLen, double scale, double tpdfDitherPeakAmplitude)
{
	getSamplesInternal(buffer,bufferLen,scale,tpdfDitherPeakAmplitude,true);
}
template<> void dsdDecimator::getSamples(int *buffer, unsigned int bufferLen, double scale, double tpdfDitherPeakAmplitude)
{
	getSamplesInternal(buffer,bufferLen,scale,tpdfDitherPeakAmplitude,true);
}
template<> void dsdDecimator::getSamples(long int *buffer, unsigned int bufferLen, double scale, double tpdfDitherPeakAmplitude)
{
	getSamplesInternal(buffer,bufferLen,scale,tpdfDitherPeakAmplitude,true);
}
template<> void dsdDecimator::getSamples(float *buffer, unsigned int bufferLen, double scale, double tpdfDitherPeakAmplitude)
{
	getSamplesInternal(buffer,bufferLen,scale,tpdfDitherPeakAmplitude,false);
}
template<> void dsdDecimator::getSamples(double *buffer, unsigned int bufferLen, double scale, double tpdfDitherPeakAmplitude)
{
	getSamplesInternal(buffer,bufferLen,scale,tpdfDitherPeakAmplitude,false);
}

/**
 * template <typename sampleType> void dsdDecimator::getSamplesInternal(sampleType *buffer, unsigned int bufferLen, double scale, double tpdfDitherPeakAmplitude, bool roundToInt)
 *  
 * private method. Does the actual calculation for the getSamples method. Using the lookup tables FIR calculation is a pretty simple summing operation.
 * 
 */

template <typename sampleType> void dsdDecimator::getSamplesInternal(sampleType *buffer, unsigned int bufferLen, double scale, double tpdfDitherPeakAmplitude, bool roundToInt)
{
	// check the buffer seems sensible
	div_t d = div(bufferLen,getNumChannels());
	if (d.rem) {
		fputs("Buffer length is not a multiple of getNumChannels()",stderr);
		exit(EXIT_FAILURE);
	}
	// get the sample buffer
	boost::circular_buffer<unsigned char>* buff = (*reader).getBuffer();
	unsigned int s = bufferLen/getNumChannels();
	for (unsigned int i=0; i<s; i++) {
		(*reader).step(); // step the buffer
		// filter each chan in turn
		for (unsigned int c=0; c<getNumChannels(); c++) {
			calc_type sum = 0.0;
			for (unsigned int t=0; t<nLookupTable; t++) {
				unsigned int byte = (unsigned int) buff[c][t] & 0xFF;
				sum += lookupTable[t][byte];
			}
			sum = sum*scale;
			// dither before rounding/truncating
			if (tpdfDitherPeakAmplitude > 0) {
				// TPDF dither
				calc_type rand1 = (calc_type)(rand()) / (calc_type)(RAND_MAX); // rand value between 0 and 1
				calc_type rand2 = (calc_type)(rand()) / (calc_type)(RAND_MAX); // rand value between 0 and 1
				sum = sum + (rand1-rand2)*tpdfDitherPeakAmplitude;
			}
			if (roundToInt)
				buffer[i*getNumChannels()+c] = static_cast<sampleType>(round(sum));
			else
				buffer[i*getNumChannels()+c] = static_cast<sampleType>(sum);
		}
		for (unsigned int m=0; m<skip; m++)
			(*reader).step(); // step the buffer
	}
}
