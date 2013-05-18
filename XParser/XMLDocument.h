#pragma once

#pragma warning(disable:4996)
//-----------------------------------------------
//
//-----------------------------------------------
#include <vector>
#include <list>
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

class InputStream
{
public:
	void Andance();
	void PushBack();

private:
	std::unique_ptr<char[]> m_buffer;
};

class xmlElement;

class xmlDoc
{
public:
	xmlDoc();

	struct Location {
		Location(int b, int e) : beg_pos(b), end_pos(e) {}
		Location() : beg_pos(), end_pos() {}

		bool IsValid() const {
			return beg_pos >= 0 && end_pos >= beg_pos;
		}

		static Location Invalid() {
			return Location(-1,-1);
		}

		int beg_pos;
		int end_pos;
	};


	//load buffer
	bool LoadFile(const char* filename);
	bool LoadFile(std::istream&);
	bool LoadStream(const char*, size_t); // load from memeory
	bool SaveFile(const char* filename, Encoding ec = DEFAULT);

	//
	bool Parser();
	void Traverse(std::shared_ptr<xmlElement> rt);
	std::shared_ptr<xmlElement> GetRoot() const;
	std::string GetEororStr();
public:
	class XMLprocessHanlder
	{
	public:

	};
private:
	bool getAttribute(std::shared_ptr<xmlElement> xe);
	bool getStartTag(std::shared_ptr<xmlElement> xe);
	bool nextIsStartTag();
	bool getEndTag(std::shared_ptr<xmlElement> xe);
	bool getText(std::shared_ptr<xmlElement> xe);									//
	bool getNode(std::shared_ptr<xmlElement> xe);
	bool match(const char* text);
	bool matchnext(const char* text);
	void setErrorStr(const char* str);
	void skipWhiteSpace();
	bool skipCommentAndHead();
	

private:
	std::string             errormsg;
	char					errorstr[15];
	std::unique_ptr<char[]> m_buffer;
	size_t m_size; //the exact length of chars in buffer = buffer_size - 1 ,except 0 of the end
	size_t beg;
	size_t end; //the end of buffer
	int cur; //current index
	Location la; 

	std::shared_ptr<xmlElement> root;
	std::shared_ptr<xmlElement> node;
};

class xmlElement
{
public:
	friend class xmlDoc;
	xmlElement():tagvalue(),tagname(),parent(NULL),chilarens(),needEndTag(true) {}
	std::string  Print();

private:
	bool		   needEndTag;
	std::string    tagname;
	std::string    tagvalue;
	std::list<std::pair<std::string,std::string>> attributes;

	//std::shared_ptr<xmlElement>				 parent;
	// |if we use sharedptr here.child node and parent node will referent to each other
	// |then ....
	// |___________________________________________________

	xmlElement*								 parent;
	std::vector<std::shared_ptr<xmlElement>> chilarens;
};


//ignore utf16 utf32, I don't want to use TCHAR to replace char.
namespace Encode {

	//dectect utf8 utf8_BOM ansi 
	//set multibytes to true to distinguage utf-8_no_bom with ansi
	Encoding DectecEncoding(const char* str, size_t st, bool multibytes);

	size_t CoverUTF8ToAnsi(char* dest, char* source, size_t st);
}