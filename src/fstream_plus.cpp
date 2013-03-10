#include "fstream_plus.h"

fstreamPlus::fstreamPlus() : std::fstream()
{
}

fstreamPlus::~fstreamPlus()
{
}

/** Overload seekg methods to return true on fail **/
bool fstreamPlus::seekg(streampos pos)
{
	std::fstream::seekg(pos);
	return !good();
}
bool fstreamPlus::seekg(streamoff pos, ios_base::seekdir way)
{
	std::fstream::seekg(pos,way);
	return !good();
}

/** Additional read methods - native bit order **/
bool fstreamPlus::read_char(char* b,stream_size n) {return read_helper(b,n);}
bool fstreamPlus::read_uchar(unsigned char* b,stream_size n) {return read_helper(b,n);}
bool fstreamPlus::read_ui(unsigned int* b,stream_size n) {return read_helper(b,n);}
bool fstreamPlus::read_lui(long unsigned int* b,stream_size n) {return read_helper(b,n);}
bool fstreamPlus::read_llui(long long unsigned int* b,stream_size n) {return read_helper(b,n);}

/** Additional read methods - reverse byte order **/
bool fstreamPlus::read_sui_rev(short unsigned int* b,stream_size n) {return read_helper_rev(b,n);}
bool fstreamPlus::read_ui_rev(unsigned int* b,stream_size n) {return read_helper_rev(b,n);}
bool fstreamPlus::read_lui_rev(long unsigned int* b,stream_size n) {return read_helper_rev(b,n);}
bool fstreamPlus::read_llui_rev(long long unsigned int* b,stream_size n) {return read_helper_rev(b,n);}

/** templates for the readers **/
template<typename rType> bool fstreamPlus::read_helper(rType* b, stream_size n) {
	read( reinterpret_cast<char*>(b),sizeof(rType)*n);
	return bad();
}
template<typename rType> bool fstreamPlus::read_helper_rev(rType* b, stream_size n) {
	read( reinterpret_cast<char*>(b),sizeof(rType)*n);
	reverseByteOrder(b,n);
	return bad();
}


/** Extra things **/
template<typename rType> void fstreamPlus::reverseByteOrder(rType* b,long long unsigned int n)
{
	for (long long unsigned int i=0; i<n; i++)
		b[i] = reverseByteOrder(b[i]);
}
template<typename rType> rType fstreamPlus::reverseByteOrder(rType b)
{
	int n = sizeof(rType);
	rType b2;
	unsigned char* dst = reinterpret_cast<unsigned char*>(&b2);
	unsigned char* src = reinterpret_cast<unsigned char*>(&b);
	for (int i=0;i<n;i++)
		dst[i] = src[n-1-i];
	return b2;
}
