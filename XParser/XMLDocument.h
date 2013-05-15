#pragma once

//-----------------------------------------------
//
//-----------------------------------------------

#include <memory>
#include <fstream>

enum Encoding {
	ANSI = 0,
	UTF_8,
	UTF8_NO_BOM,
	UTF_16,
	UTF_16_BIGEND,
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