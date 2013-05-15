#include "XMLDocument.h"
#include <cassert>
#include <iostream>
#include <string>
#include <algorithm>

xmlDoc::xmlDoc():m_buffer(),beg(),end(),cur(),m_size()
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
	m_size = st;
	in.seekg(0,std::ios::beg);    //<--------------WTF!!!
	

	std::cout<<st<<std::endl;	//<<<<<<<<<debug : this line should be delete 

	std::unique_ptr<char[]> buf(new char[st + 1]);
	
	//memset(buf.get(),0,st + 1);

	if (buf.get() == nullptr) {
		return false;
	}

	in.read(buf.get(),st);
	buf[st] = 0;

	if (m_buffer.get() != nullptr) {
		m_buffer.release();
	}
	m_buffer = std::move(buf);
	
	std::cout<<m_buffer.get()<<std::endl; //<<<<<<<<<debug : this line should be delete 

	Encoding ec = Encode::DectecEncoding(m_buffer.get(),st,true);
	std::cout << ec <<std::endl; //<<<<<<<<<debug : this line should be delete
	return true;
}

bool xmlDoc::LoadStream(const char* str, size_t st)
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

//just writingt . don't care about the encoding
bool xmlDoc::SaveFile(const char* filename, Encoding ec)
{
	assert(filename != NULL);
	std::string head;
	if (ec == UTF8_NO_BOM) {
		head = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\r\n";
	} else {
		head = "<?xml version=\"1.0\"?>\r\n";
	}

	std::ofstream out(filename,std::ios::binary);
	if (!out) {
		return false;
	}

	out.write(head.c_str(),head.length());
	out.write(m_buffer.get(),m_size);
	out.close();

	return true;
	
}

//suspend to support unicode
Encoding Encode::DectecEncoding(const char* str, size_t st, bool multibuf)
{
	if (st < 3) {
		return DEFAULT;
	}

	//bom
	unsigned char bom[3] = {0};
	bom[0] = str[0] & 0xff;
	bom[1] = str[1] & 0xff;
	bom[2] = str[2] & 0xff;

	//utf-8 0xef 0xbb 0xbf
	if (bom[0] == 0xef && bom[1] == 0xbb && bom[2] == 0xbf) {
		return UTF_8;
	}
	
	if (multibuf) {
		return UTF8_NO_BOM;
	} else {
		return ANSI;
	}
	
}