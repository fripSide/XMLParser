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
	std::unique_ptr<wchar_t[]> m_wbuffer;
	size_t beg;
	size_t end; //the end of buffer
	size_t cur; //
};

namespace Encode {

}