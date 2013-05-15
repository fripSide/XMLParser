#pragma once

//-----------------------------------------------
//
//-----------------------------------------------

#include <memory>
#include <fstream>

enum Encoding {
	ANSI = 0,
	UTF_8,			//multy char
	UTF8_NO_BOM,	//multy char
	UTF_16,        //wild char
	UTF_16_BIGEND, //wild char
	DEFAULT = UTF_8,
};

class xmlDoc
{
public:
	xmlDoc();
	bool LoadFile(const char* filename);
	bool LoadFile(std::istream&);
	bool LoadStream(const char*,size_t); // load from memeory


private:
	std::unique_ptr<char[]> m_buffer;
	size_t beg;
	size_t end; //the end of buffer
	size_t cur; //
};


//ignore utf16 utf32
namespace Encode {

	//dectect utf8 utf8_BOM ansi 
	Encoding DectecEncoding(const char* str,size_t st);
}