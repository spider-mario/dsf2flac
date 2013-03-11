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
  * dsd_decimator.h
  * 
  * Header file for the class dsdDecimator.
  * 
  * The dsdDecimator class does the actual conversion from dsd to pcm. Pass in a dsdSampleReader
  * to the create function along with the desired pcm sample rate (must be multiple of 44.1k).
  * Then you can simply read pcm samples into a int or float buffer using getSamples.
  * 
  */
  
#define calc_type double // you can change the type used to do the filtering... but there is barely any change in calc speed between float and double

#ifndef DSDDECIMATOR_H
#define DSDDECIMATOR_H

#include <dsd_sample_reader.h>

class dsdDecimator
{
public:
	dsdDecimator(dsdSampleReader *reader, unsigned int outputSampleRate);
	virtual ~dsdDecimator();
	long long unsigned int getLength(); // return total length in samples of decimated data
	unsigned int getOutputSampleRate();
	template <typename sampleType> void getSamples(sampleType *buffer, unsigned int bufferLen, double scale, double tpdfDitherPeakAmplitude = 0);
	// some helpful wrappers around the reader
	unsigned int getNumChannels() { return reader->getNumChannels(); };
	double getPositionInSeconds() { return reader->getPositionInSeconds(); };
	double getPositionAsPercent() { return reader->getPositionAsPercent(); };
	double getLengthInSeconds() { return reader->getLengthInSeconds(); };
	bool isEOF() { return reader->isEOF(); };
	bool isValid(); // return false if the decimator is invalid
	std::string getErrorMsg(); // returns a human readable error message
private:
	dsdSampleReader *reader;
	unsigned int outputSampleRate;
	unsigned int nLookupTable;
	calc_type** lookupTable;
	unsigned int skip;
	bool valid;
	std::string errorMsg;
	// private methods
	void initLookupTable(const double nCoefs,const double* coefs);
	template <typename sampleType> void getSamplesInternal(sampleType *buffer, unsigned int bufferLen, double scale, double tpdfDitherPeakAmplitude, bool roundToInt);
	
};

#endif // DSDDECIMATOR_H
