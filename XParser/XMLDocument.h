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
	DEFAULT = UTF8_NO_BOM,
};

class xmlDoc
{
public:
	xmlDoc();

	//load buffer
	bool LoadFile(const char* filename);
	bool LoadFile(std::istream&);
	bool LoadStream(const char*, size_t); // load from memeory
	bool SaveFile(const char* filename, Encoding ec = DEFAULT);

	//

private:
	std::unique_ptr<char[]> m_buffer;
	size_t m_size; //the exact length of chars in buffer = buffer_size - 1
	size_t beg;
	size_t end; //the end of buffer
	size_t cur; //
};


//ignore utf16 utf32, I don't want to use TCHAR to replace char.
namespace Encode {

	//dectect utf8 utf8_BOM ansi 
	//set multibytes to true to distinguage utf-8_no_bom with ansi
	Encoding DectecEncoding(const char* str, size_t st, bool multibytes);

	size_t CoverUTF8ToAnsi(char* dest, char* source, size_t st);
}