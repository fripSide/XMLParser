#include "XMLDocument.h"
#include <cassert>
#include <iostream>
#include <string>
#include <algorithm>

xmlDoc::xmlDoc():m_buffer(),beg(),end(),cur()
{

}

bool xmlDoc::LoadFile(const char* filename)
{
	assert(filename != NULL);
	std::ifstream in;
	in.open(filename,std::ios::binary);
	if (!in.is_open()) {
		return false;
	}
	bool ret =  LoadFile(in);
	in.close();
	return ret;
}

bool xmlDoc::LoadFile(std::istream& in)
{

	in.seekg(0,std::ios::end);
	size_t st = static_cast<size_t>(in.tellg());
	end = st;
	in.seekg(0,std::ios::beg);    //<--------------WTF!!!
	

	std::cout<<st<<std::endl;	//<<<<<<<<<debug : this line should be delete 

	std::unique_ptr<char[]> buf(new char[st + 1]);
	
	//memset(buf.get(),0,st + 1);

	if (buf.get() == nullptr) {
		return false;
	}

	in.read(buf.get(),st);
	buf[st] = 0;
	m_buffer = std::move(buf);
	
	std::cout<<m_buffer.get()<<std::endl; //<<<<<<<<<debug : this line should be delete 

	LoadStream(m_buffer.get(),st);
	return true;
}

bool xmlDoc::LoadStream(const char* str,size_t st)
{
	if (m_buffer.get() != nullptr) {
		m_buffer.release();
	}

	std::unique_ptr<char[]> buf(new char[st + 1]);
	if (buf.get() == nullptr) {
		return false;
	}

	memcpy(buf.get(),str,st);
	buf[st] = 0;
	m_buffer = std::move(buf);

	return true;
}

Encoding Encode::DectecEncoding(const char* str,size_t st)
{

}