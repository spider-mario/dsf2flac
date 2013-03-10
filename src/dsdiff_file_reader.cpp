#include "dsdiff_file_reader.h"
#include "iostream"

static bool chanIdentsAllocated = false;
static bool sampleBufferAllocated = false;


dsdiffFileReader::dsdiffFileReader(char* filePath) : dsdSampleReader()
{
	
	// set some defaults
	ast_hours = 0;
	ast_mins = 0;
	ast_secs = 0;
	ast_samples = 0;
	samplesPerChar = 8; // always 8 samples per char (dsd wide not supported by dsdiff).
	errorMsg = "";
	
	// first let's open the file
	file.open(filePath, fstreamPlus::in | fstreamPlus::binary);
	// throw exception if that did not work.
	if (!file.is_open()) {
		errorMsg = "could not open file";
		valid = false;
		return;
	}
	// try and read the header data
	if (!(valid = readHeaders()))
		return;
		
	// we don't support compressed samples
	if (!checkIdent(compressionType,const_cast<char*>("DSD "))) {
		errorMsg = "Sorry, compressed sample types are not yet supported";
		valid = false;
		return;
	}
	
	allocateSampleBuffer();
	rewind(); // set the position to the start of the dsd
	// allocate the circular buffer (this will cause rewind() to be called)
	allocateBuffer();	
}

dsdiffFileReader::~dsdiffFileReader()
{
	// close the file
	file.close();
	
	// free mem in the chanIdents
	if (chanIdentsAllocated) {
		for (int i=0; i<chanNum; i++)
			delete[] chanIdents[i];
		delete[] chanIdents;
	}
	if (sampleBufferAllocated)
		delete[] sampleBuffer;
	
}


/**
 * void dsdiffFileReader::allocateSampleBuffer()
 *
 * Reset the position back to the start of the file
 *
 */
void dsdiffFileReader::allocateSampleBuffer() {
		sampleBuffer = new unsigned char[chanNum];
		sampleBufferAllocated = true;
}

/**
 * void dsdiffFileReader::rewind()
 *
 * Reset the position back to the start of the file
 *
 */
void dsdiffFileReader::rewind()
{
	// position the file at the start of the data chunk
	if (file.seekg(sampleDataPointer)) {
		errorMsg = "dsfFileReader::readFirstBlock:file seek error";
		//return false;
	}
	posMarker = 0;
}

/**
 * void dsdiffFileReader::step()
 *
 * Increments the position in the file by 8 dsd samples (1 byte of data).
 * The block buffers are updated with the new samples.
 *
 */
bool dsdiffFileReader::step()
{
	bool ok = true;
	
	if (isEOF())
		ok = false;
	else if (file.read_uchar(sampleBuffer,chanNum)) {
		errorMsg = "dsfFileReader::step:file read error";
		ok = false;
	}
	

	if (ok) {
		for (long unsigned int i=0; i<chanNum; i++)
			circularBuffers[i].push_front(sampleBuffer[i]);
	} else {
		for (long unsigned int i=0; i<chanNum; i++)
			circularBuffers[i].push_front(getIdleSample());
	}

	posMarker++;
	return ok;
}


/**
 * void dsdiffFileReader::readHeaders()
 *
 * Private function, called on create. Reads lots of info from the file.
 *
 */
bool dsdiffFileReader::readHeaders()
{	
	// look for the FRM8 chunk (probably at the start of the data).
	char ident[5];ident[4]='\0';
	long long unsigned int chunkStart;
	long long unsigned int chunkSz;
	bool frm8read = false;
	// assum first chunk is at the start of the file.
	chunkStart = 0;
	// stop once one frm8 chunk is read
	while (!frm8read) {
		// read the chunk header
		if (!readChunkHeader(ident,chunkStart,&chunkSz))
			 return false;
		// did we find a FRM8 chunk??
		if ( checkIdent(ident,const_cast<char*>("FRM8")) )
		{
			frm8read = readChunk_FRM8(chunkStart);
		}
		// otherwise, move to the next chunk
		chunkStart +=chunkSz;
	}
	return frm8read;
}

bool dsdiffFileReader::readChunk_FRM8(long long unsigned int chunkStart)
{
	// read the header so that we are certain we are in the correct place.
	char ident[5];ident[4]='\0';
	long long unsigned int chunkSz;
	if (!readChunkHeader(ident,chunkStart,&chunkSz))
		return false;
	// check the ident
	if ( !checkIdent(ident,const_cast<char*>("FRM8")) ) {
		errorMsg = "dsdiffFileReader::readChunk_FRM8:chunk ident error";
		return false;
	}
	// another ident which MUST be "DSD "
	if (file.read_char(ident,4)) {
		errorMsg = "dsdiffFileReader::readChunk_FRM8:file read error";
		return false;
	}
	if ( !checkIdent(ident,const_cast<char*>("DSD ")) ) {
		errorMsg = "dsdiffFileReader::readChunk_FRM8:chunk ident error";
		return false;
	}
	// read all sub chunks in the FRM8 chunk
	long long unsigned int subChunkStart = file.tellg();
	long long unsigned int subChunkSz;
	// keep track of the non-optional chunks
	bool found_fver = false;
	bool found_prop = false;
	bool found_dsdt = false; // either dsd or dst
	//
	while (subChunkStart < chunkStart + chunkSz) {
		// read the header
		if (!readChunkHeader(ident,subChunkStart,&subChunkSz))
			return false;
		// see if we know how to read this chunk
		if ( !found_fver && checkIdent(ident,const_cast<char*>("FVER")) ) {
			found_fver = readChunk_FVER(subChunkStart);
		} else if ( !found_prop && checkIdent(ident,const_cast<char*>("PROP")) ) {
			found_prop = readChunk_PROP(subChunkStart);
		} else if ( !found_dsdt && checkIdent(ident,const_cast<char*>("DSD ")) ) {
			found_dsdt = readChunk_DSD(subChunkStart);
		} else
			printf("WARNING: unknown chunk type: %s\n",ident);
		// move to the next chunk
		subChunkStart = subChunkStart + subChunkSz;
	}
	// return true if all required chunks are ok.
	if (!found_fver) {
		errorMsg = "dsdiffFileReader::readChunk_FRM8:No valid FVER chunk found";
		return false;
	} else if (!found_prop) {
		errorMsg = "dsdiffFileReader::readChunk_FRM8:No valid PROP chunk found";
		return false;
	} else if (!found_prop) {
		errorMsg = "dsdiffFileReader::readChunk_FRM8:No valid DSD or DST chunk found";
		return false;
	} else
		return true;
}

bool dsdiffFileReader::readChunk_FVER(long long unsigned int chunkStart)
{
	// read the header so that we are certain we are in the correct place.
	char ident[5];ident[4]='\0';
	if (!readChunkHeader(ident,chunkStart))
		return false;
	// check the ident
	if ( !checkIdent(ident,const_cast<char*>("FVER")) ) {
		errorMsg = "dsdiffFileReader::readChunk_FVER:chunk ident error";
		return false;
	}
	// read the file version
	if (file.read_ui_rev(&dsdiffVersion,1)) {
		errorMsg = "dsdiffFileReader::readChunk_FVER:file read error";
		return false;
	};
	return true;
}

bool dsdiffFileReader::readChunk_PROP(long long unsigned int chunkStart)
{
	// read the header so that we are certain we are in the correct place.
	char ident[5];ident[4]='\0';
	long long unsigned int chunkSz;
	if (!readChunkHeader(ident,chunkStart,&chunkSz))
		return false;
	// check the ident
	if ( !checkIdent(ident,const_cast<char*>("PROP")) ) {
		errorMsg = "dsdiffFileReader::readChunk_PROP:chunk ident error";
		return false;
	}
	// another ident which MUST be "SND "
	if (file.read_char(ident,4)) {
		errorMsg = "dsdiffFileReader::readChunk_PROP:file read error";
		return false;
	}
	if ( !checkIdent(ident,const_cast<char*>("SND ")) ) {
		errorMsg = "dsdiffFileReader::readChunk_PROP:PROP chunk format error";
		return false;
	}
	
	// read all sub chunks in the FRM8 chunk
	long long unsigned int subChunkStart = file.tellg();
	long long unsigned int subChunkSz;
	// keep track of the chunks we've read
	bool found_fs   = false;
	bool found_chnl = false;
	bool found_cmpr = false;
	bool found_abss = false;
	//
	while (subChunkStart < chunkStart + chunkSz) {
		// read the header
		if (!readChunkHeader(ident,subChunkStart,&subChunkSz))
			return false;
		// see if we know how to read this chunk
		if ( !found_fs && checkIdent(ident,const_cast<char*>("FS  ")) ) {
			found_fs = readChunk_FS(subChunkStart);
		} else if ( !found_chnl && checkIdent(ident,const_cast<char*>("CHNL")) ) {
			found_chnl = readChunk_CHNL(subChunkStart);
		} else if ( !found_cmpr && checkIdent(ident,const_cast<char*>("CMPR")) ) {
			found_cmpr = readChunk_CMPR(subChunkStart);
		} else if ( !found_abss && checkIdent(ident,const_cast<char*>("ABSS")) ) {
			found_abss = readChunk_ABSS(subChunkStart);
		} else
			printf("WARNING: unknown or repeated chunk type: %s\n",ident);
		// move to the next chunk
		subChunkStart = subChunkStart + subChunkSz;
	}
	// check that all the required chunks were read
	if (!found_fs) {
		errorMsg = "dsdiffFileReader::readChunk_PROP: no valid FS chunk";
		return false;
	} else if (!found_chnl) {
		errorMsg = "dsdiffFileReader::readChunk_PROP: no valid CHNL chunk";
		return false;
	} else if (!found_cmpr) {
		errorMsg = "dsdiffFileReader::readChunk_PROP: no valid CMPR chunk";
		return false;
	}
	else 
		return true;
}

bool dsdiffFileReader::readChunk_FS(long long unsigned int chunkStart)
{
	// read the header so that we are certain we are in the correct place.
	char ident[5];ident[4]='\0';
	if (!readChunkHeader(ident,chunkStart))
		return false;
	// check the ident
	if ( !checkIdent(ident,const_cast<char*>("FS  ")) ) {
		errorMsg = "dsdiffFileReader::readChunk_FS:chunk ident error";
		return false;
	}
	if (file.read_ui_rev(&samplingFreq,1)) {
		errorMsg = "dsdiffFileReader::readChunk_FS:File read error";
		return false;
	}
	return true;
}

bool dsdiffFileReader::readChunk_CHNL(long long unsigned int chunkStart)
{
	// read the header so that we are certain we are in the correct place.
	char ident[5];ident[4]='\0';
	if (!readChunkHeader(ident,chunkStart))
		return false;
	// check the ident
	if ( !checkIdent(ident,const_cast<char*>("CHNL")) ) {
		errorMsg = "dsdiffFileReader::readChunk_CHNL:chunk ident error";
		return false;
	}
	// read number of channels
	if (file.read_sui_rev(&chanNum,1)) {
		errorMsg = "dsdiffFileReader::readChunk_CHNL:file read error";
		return false;
	}
	// read channel identifiers
	chanIdents = new char*[chanNum];
	for (int i=0;i<chanNum;i++) {
		chanIdents[i] = new char[5];
		if (file.read_char(chanIdents[i],4)) {
			errorMsg = "dsdiffFileReader::readChunk_CHNL:file read error";
			return false;
		};
		chanIdents[i][4] = '\0';
	}
	return true;
}

bool dsdiffFileReader::readChunk_CMPR(long long unsigned int chunkStart)
{
	// read the header so that we are certain we are in the correct place.
	char ident[5];ident[4]='\0';
	if (!readChunkHeader(ident,chunkStart))
		return false;
	// check the ident
	if ( !checkIdent(ident,const_cast<char*>("CMPR")) ) {
		errorMsg = "dsdiffFileReader::readChunk_CMPR:chunk ident error";
		return false;
	}
	// type of compression
	compressionType[4] = '\0';
	if (file.read_char(compressionType,4)) {
		errorMsg = "dsdiffFileReader::readChunk_CMPR:file read error";
		return false;
	}
	// read length of next entry
	unsigned char n;
	if (file.read_uchar(&n,1)) {
		errorMsg = "dsdiffFileReader::readChunk_CMPR:file read error";
		return false;
	}
	compressionName = new char[n+1];
	compressionName[n] = '\0';
	if (file.read_char(compressionName,n)) {
		errorMsg = "dsdiffFileReader::readChunk_CMPR:file read error";
		return false;
	}
	return true;
}

bool dsdiffFileReader::readChunk_ABSS(long long unsigned int chunkStart)
{
	// read the header so that we are certain we are in the correct place.
	char ident[5];ident[4]='\0';
	if (!readChunkHeader(ident,chunkStart))
		return false;
	// check the ident
	if ( !checkIdent(ident,const_cast<char*>("ABSS")) ) {
		errorMsg = "dsdiffFileReader::readChunk_ABSS:chunk ident error";
		return false;
	}
	// hours
	if (file.read_sui_rev(&ast_hours,1)) {
		errorMsg = "dsdiffFileReader::readChunk_ABSS:file read error";
		return false;
	}
	// mins
	if (file.read_uchar(&ast_mins,1)) {
		errorMsg = "dsdiffFileReader::readChunk_ABSS:file read error";
		return false;
	}
	// secs
	if (file.read_uchar(&ast_secs,1)) {
		errorMsg = "dsdiffFileReader::readChunk_ABSS:file read error";
		return false;
	}
	// samples
	if (file.read_ui_rev(&ast_samples,1)) {
		errorMsg = "dsdiffFileReader::readChunk_ABSS:file read error";
		return false;
	}
	return true;
}

bool dsdiffFileReader::readChunk_DSD(long long unsigned int chunkStart)
{
	// read the header so that we are certain we are in the correct place.
	char ident[5];ident[4]='\0';
	long long unsigned int chunkLen;
	if (!readChunkHeader(ident,chunkStart,&chunkLen))
		return false;
	// check the ident
	if ( !checkIdent(ident,const_cast<char*>("DSD ")) ) {
		errorMsg = "dsdiffFileReader::readChunk_DSD:chunk ident error";
		return false;
	}
	// we are now at the start of the dsd data, record the position
	sampleDataPointer = file.tellg();
	// chunkLen contains the number of samples
	sampleCount = (chunkLen - 12)/chanNum*samplesPerChar;
	// guess the idle sample
	if (file.read_uchar(&idleSample,1)) {
		errorMsg = "dsdiffFileReader::readChunk_DSD:file read error";
		return false;
	}
	
	return true;
}

/**
 * long long unsigned int dsdiffFileReader::readChunkHeader(char* ident, long long unsigned int chunkStart)
 * 
 * Private method: a little helper to get rid of repetitive code!
 * 
 */
bool dsdiffFileReader::readChunkHeader(char* ident, long long unsigned int chunkStart) {
	long long unsigned int chunkSz;
	return readChunkHeader(ident,chunkStart,&chunkSz);
}
bool dsdiffFileReader::readChunkHeader(char* ident, long long unsigned int chunkStart, long long unsigned int* chunkSz) {
	// make sure we are at the start of the chunk
	if (file.seekg(chunkStart)) {
		errorMsg = "dsdiffFileReader::readChunkHeader:File seek error";
		return false;
	}
	// read the ident
	if (file.read_char(ident,4)) {
		errorMsg = "dsdiffFileReader::readChunkHeader:File read error";
		return false;
	}
	// 8 bytes chunk size
	if (file.read_llui_rev(chunkSz,1)) {
		errorMsg = "dsdiffFileReader::readChunkHeader:File read error";
		return false;
	}
	// if the chunk size is odd then add one... because for some stupid reason you have to do this....
	// also add the header length (12bytes).
	if ( chunkSz[0] & 1 )
		chunkSz[0] += 13;
	else
		chunkSz[0] +=12;
	
	return true;
}

bool dsdiffFileReader::checkIdent(char* a, char* b)
{
	return ( a[0]==b[0] && a[1]==b[1] && a[2]==b[2] && a[3]==b[3] );
}

void dsdiffFileReader::dispFileInfo() {
	printf("dsdiffVersion: %08x\n",dsdiffVersion);
	printf("samplingRate: %u\n",samplingFreq);
	printf("chanNum: %u\n",chanNum);
	for (int i=0;i<chanNum;i++)
		printf("chanIdent%u: %s\n",i,chanIdents[i]);
	printf("compressionType: %s\n",compressionType);
	printf("compressionName: %s\n",compressionName);
	printf("ast_hours: %u\n",ast_hours);
	printf("ast_mins: %d\n",ast_mins);
	printf("ast_secs: %d\n",ast_secs);
	printf("ast_samples: %u\n",ast_samples);
	printf("sampleDataPointer: %llu\n",sampleDataPointer);
	printf("sampleCount: %llu\n",sampleCount);
	
}



