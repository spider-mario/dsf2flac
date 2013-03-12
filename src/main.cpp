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

#include <boost/timer/timer.hpp>
#include <boost/filesystem.hpp>
#include <dsd_decimator.h>
#include <dsf_file_reader.h>
#include <dsdiff_file_reader.h>
#include "FLAC++/metadata.h"
#include "FLAC++/encoder.h"
#include "math.h"
#include "cmdline.h"

#define flacBlockLen 4192;

using boost::timer::cpu_timer;
using boost::timer::cpu_times;
using boost::timer::nanosecond_type;

static nanosecond_type reportInterval(500000000LL);
static cpu_timer timer;
static double lastPos;

/**
 * void setupTimer(double currPos)
 *
 * called by main to setup the boost timer for reporting progress to the user
 */
void setupTimer(double currPos)
{
	timer = cpu_timer();
	lastPos = currPos;
}

/**
 * void checkTimer(double currPos, double percent)
 *
 * called by main to report progress to the user.
 */
void checkTimer(double currPos, double percent)
{
	cpu_times const elapsed_times(timer.elapsed());
	nanosecond_type const elapsed(elapsed_times.system + elapsed_times.user);
	if (elapsed >= reportInterval) {
		printf("\33[2K\r");
		printf("Rate: %4.2fx\t",(currPos-lastPos)/elapsed*1000000000);
		printf("Progress: %3.1f%%",percent);
		fflush(stdout);
		lastPos = currPos;
		timer = cpu_timer();
	}
}

/**
 * int main(int argc, char **argv)
 *
 * Main
 */
int main(int argc, char **argv)
{
	// use the cmdline processor
	gengetopt_args_info args_info;
	if (cmdline_parser (argc, argv, &args_info) != 0)
		exit(1) ;

	// if help or version given then exit now.
	if (args_info.help_given || args_info.version_given)
		exit(1);

	// collect the options
	int fs = args_info.samplerate_arg;
	int bits = args_info.bits_arg;
	bool dither = !args_info.nodither_flag;
	double userScaleDB = (double) args_info.scale_arg;
	double userScale = pow(10.0d,userScaleDB/20);
	boost::filesystem::path inpath(args_info.infile_arg);
	boost::filesystem::path outpath;
	if (args_info.outfile_given)
		outpath = args_info.outfile_arg;
	else {
		outpath = inpath;
		outpath.replace_extension(".flac");
	}
	// calc real scale and dither amplitude
	double scale = userScale * pow(2.0d,bits-1); // increase scale by factor of 2^23 (24bit).
	double tpdfDitherPeakAmplitude;
	if (dither)
		tpdfDitherPeakAmplitude = 1;
	else
		tpdfDitherPeakAmplitude = 0;
		
	//
	printf("%s ",CMDLINE_PARSER_PACKAGE_NAME);
	printf("%s\n\n",CMDLINE_PARSER_VERSION);

	// pointer to the dsdSampleReader (could be any valid type).
	dsdSampleReader* dsr;

	// create either a reder for dsf or dsd
	if (inpath.extension() == ".dsf" || inpath.extension() == ".DSF")
		dsr = new dsfFileReader((char*)inpath.c_str());
	else if (inpath.extension() == ".dff" || inpath.extension() == ".DFF")
		dsr = new dsdiffFileReader((char*)inpath.c_str());
	else {
		printf("Sorry, only .dff or .dff input files are supported\n");
		return 0;
	}

	// check reader is valid.
	if (!dsr->isValid()) {
		printf("Error opening DSDFF file!\n");
		printf("%s\n",dsr->getErrorMsg().c_str());
		return 0;
	}

	// create decimator
	dsdDecimator dec(dsr,fs);
	if (!dec.isValid()) {
		printf("%s\n",dec.getErrorMsg().c_str());
		return 0;
	}

	// feedback some info to the user
	printf("Input file\n\t%s\n\n",inpath.c_str());
	printf("Output file\n\t%s\n\n",outpath.c_str());
	printf("Output format\n\tSampleRate: %dHz\n\tDepth: %dbit\n\tDither: %s\n\tScale: %1.1fdB\n",fs, bits, (dither)?"true":"false",userScaleDB);
	printf("\tIdleSample: 0x%02x\n\n",dsr->getIdleSample());

	setupTimer(dsr->getPositionInSeconds());

	// flac vars
	bool ok = true;
	FLAC::Encoder::File encoder;
	FLAC__StreamEncoderInitStatus init_status;
	FLAC__StreamMetadata *metadata[2];
	FLAC__StreamMetadata_VorbisComment_Entry entry;

	// setup the encoder
	if(!encoder) {
		fprintf(stderr, "ERROR: allocating encoder\n");
		return 1;
	}
	ok &= encoder.set_verify(true);
	ok &= encoder.set_compression_level(5);
	ok &= encoder.set_channels(dec.getNumChannels());
	ok &= encoder.set_bits_per_sample(bits);
	ok &= encoder.set_sample_rate(dec.getOutputSampleRate());
	ok &= encoder.set_total_samples_estimate(dec.getLength());

	// add tags and a padding block
	if(ok) {

		bool err = false;
		err |= (metadata[0] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT)) == NULL;
		err |= (metadata[1] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING)) == NULL;
		char* tag = dsr->getArtist();
		if (tag != NULL) {
			bool err1 = !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ARTIST", tag);
			if (!err1)
				err |= !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false); // copy=false: let metadata object take control of entry's allocated string
			err |= err1;
		}
		tag = dsr->getAlbum();
		if (tag != NULL) {
			bool err1 = !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ALBUM", tag);
			if (!err1)
				err |= !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false); // copy=false: let metadata object take control of entry's allocated string
			err |= err1;
		}
		tag = dsr->getTitle();
		if (tag != NULL) {
			bool err1 = !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "TITLE", tag);
			if (!err1)
				err |= !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false); // copy=false: let metadata object take control of entry's allocated string
			err |= err1;
		}
		tag = dsr->getTrack();
		if (tag != NULL) {
			bool err1 = !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "TRACKNUMBER", tag);
			if (!err1)
				err |= !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false); // copy=false: let metadata object take control of entry's allocated string
			err |= err1;
		}
		tag = dsr->getYear();
		if (tag != NULL) {
			bool err1 = !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "DATE", tag);
			if (!err1)
				err |= !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false); // copy=false: let metadata object take control of entry's allocated string
			err |= err1;
		}
		if (err) {
			fprintf(stderr, "ERROR: error writing tags to flac, some might be missing.\n");
			//ok = false;
		} 
		metadata[1]->length = 2048; /* set the padding length */
		ok = encoder.set_metadata(metadata, 2);
	}

	// initialize encoder
	if(ok) {
		init_status = encoder.init(outpath.c_str());
		if(init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
			fprintf(stderr, "ERROR: initializing encoder: %s\n", FLAC__StreamEncoderInitStatusString[init_status]);
			ok = false;
		}
	}

	// create a FLAC__int32 buffer to hold the samples as they are converted
	unsigned int bufferLen = dec.getNumChannels()*flacBlockLen;
	FLAC__int32* buffer = new FLAC__int32[bufferLen];

	// CONVERSION LOOP //
	while (!dsr->isEOF()) {
	//while (dsr->getPositionInSeconds()<2) {
		// get a block of pcm samples from the deocder
		dec.getSamples(buffer,bufferLen,scale,tpdfDitherPeakAmplitude);
		// pass samples to encoder
		if(!(ok = encoder.process_interleaved(buffer, bufferLen/dec.getNumChannels())))
			fprintf(stderr, "   state: %s\n", encoder.get_state().resolved_as_cstring(encoder));
		// check the timer
		checkTimer(dsr->getPositionInSeconds(),dsr->getPositionAsPercent());
	}
	// close the flac file
	ok &= encoder.finish();
	// report back to the user
	printf("\33[2K\r");
	if (ok) {
		printf("Conversion completed sucessfully.\n");
	} else {
		printf("Error during conversion.\n");
		fprintf(stderr, "encoding: %s\n", ok? "succeeded" : "FAILED");
		fprintf(stderr, "   state: %s\n", encoder.get_state().resolved_as_cstring(encoder));
	}

	// free things
	FLAC__metadata_object_delete(metadata[0]);
	FLAC__metadata_object_delete(metadata[1]);
	delete[] buffer;

	return 0;
}
