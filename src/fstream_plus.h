#ifndef FILEPLUS_H
#define FILEPLUS_H

#include <fstream>

typedef long long unsigned int stream_size;

class fstreamPlus : public std::fstream
{
public:
	fstreamPlus();
	virtual ~fstreamPlus();
	
	/** Overload seekg methods to return true on fail **/
	bool seekg(streampos pos);
	bool seekg(streamoff pos, ios_base::seekdir way);
	
	/** Additional read methods - native bit order **/
	// All return true on error. All read "n" numbers (not n chars/bytes!)
	bool read_char (char*                   b,stream_size n);
	bool read_uchar(unsigned char*			b,stream_size n);
	bool read_sui  (unsigned int*      		b,stream_size n);
	bool read_ui   (unsigned int*      		b,stream_size n);
	bool read_lui  (long unsigned int*      b,stream_size n);
	bool read_llui (long long unsigned int* b,stream_size n);
	
	
	/** Additional read methods - reverse byte order **/
	// All return true on error. All read "n" numbers (not n chars/bytes!)
	bool read_char_rev (char*                   b,stream_size n) { return read_char(b,n); };
	bool read_uchar_rev(unsigned char*			b,stream_size n) { return read_uchar(b,n); };
	bool read_sui_rev  (short unsigned int*     b,stream_size n);
	bool read_ui_rev   (unsigned int*      		b,stream_size n);
	bool read_lui_rev  (long unsigned int*      b,stream_size n);
	bool read_llui_rev (long long unsigned int* b,stream_size n);
	
	
	/** Extra things **/
	template<typename rType> static void reverseByteOrder(rType* b,long long unsigned int n);
	template<typename rType> static rType reverseByteOrder(rType b);
	
private:

	/** templates for the readers **/
	template<typename rType> bool read_helper(rType* b, stream_size n);
	template<typename rType> bool read_helper_rev(rType* b, stream_size n);
	

};

#endif // FILEPLUS_H
