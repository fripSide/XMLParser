#include "XMLDocument.h"
#include <cassert>
#include <iostream>
#include <string>
#include <algorithm>

xmlDoc::xmlDoc():m_buffer(),beg(),end(),cur(),m_size(),errormsg(),root(new xmlElement)
{
	memset(errorstr,0,15);
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

bool xmlDoc::Parser()
{
	while (true) {
		if (cur == m_size) {
			break;
		}

		std::shared_ptr<xmlElement> cnode(new xmlElement);
		if (!getNode(cnode)) {
			return false;
		}
		root->chilarens.push_back(cnode);
		cnode->parent = root.get();
	}

	return true;
}

void xmlDoc::Traverse(std::shared_ptr<xmlElement> rt)
{
	if (rt.get() != nullptr) {
		std::cout<<rt->Print()<<std::endl;
		for (auto tit = rt->chilarens.begin(); tit != rt->chilarens.end(); ++tit) {
			Traverse(*tit);
			std::cout<<(*tit)->Print()<<std::endl;
		}
	}
}

std::shared_ptr<xmlElement> xmlDoc::GetRoot() const
{
	return root;
}

void xmlDoc::skipWhiteSpace()
{
	while (m_buffer[cur] == ' ' && cur != m_size) {
		++cur;
	}
}

std::string xmlDoc::GetEororStr()
{
	if (errormsg.empty()) {
		return "";
	}
	errormsg += "/n";
	errormsg += errorstr;

	return errormsg;
}

void xmlDoc::setErrorStr(const char* str)
{
	errormsg = str;
	if (m_size - cur > 14) {
		strncpy(errorstr,&m_buffer[cur],14);
	} else {
		strncpy(errorstr,&m_buffer[cur],m_size - cur);
	}
}

//-----------------------------------------------
//parser node
//-----------------------------------------------
bool xmlDoc::getNode(std::shared_ptr<xmlElement> xe)
{
	assert(xe.get() != nullptr && "xmlElement不能为nullptr");
	if (!errormsg.empty()) {
		return false;
	}

	skipCommentAndHead();
	
	if (!getStartTag(xe)) { //获取标签失败或者遇到< />标签直接结束
		return false;
	}

	while (nextIsStartTag()) { //add child node
		std::shared_ptr<xmlElement> cnode(new xmlElement);
		cnode->parent = xe.get();
		xe->chilarens.push_back(cnode);
		getNode(cnode);
	}

	getText(xe);
	skipCommentAndHead();
	if (getEndTag(xe) && errormsg.empty()) {
		return false;
	}

	return true;
}

bool xmlDoc::skipCommentAndHead() // skip coment and head <? ？>
{
	if (match("<?")) {
		while (m_buffer[cur + 2] != '>' && m_buffer[cur] != '>' && cur != m_size - 2) {
			++cur;
		}
		if (!match("?>")) {
			setErrorStr("此应该出现 ?>");
			return false;
		}
	}
	
	//skipcomment
	while (match("<!--")) {
		while (m_buffer[cur] != '>' && m_buffer[cur + 3] != '>' && cur != m_size - 3) {
			++cur;
		}
		if (!match("-->")) {
			setErrorStr("此处注释格式错误应为 -->");
			return false;
		}
	}
	return true;
}

bool xmlDoc::nextIsStartTag()
{
	skipWhiteSpace();
	int t = cur + 1;
	if (m_buffer[t] == '<' && m_buffer[t + 1] != '/' && m_buffer[t + 1] != '!') {
		return true;
	}

	return false;
}

bool xmlDoc::match(const char* text)
{
	skipWhiteSpace();
	int len = strlen(text);
	if (strncmp(text,&m_buffer[cur],len) == 0) {
		cur += len;
		return true;
	}
	return false;
}

bool xmlDoc::getText(std::shared_ptr<xmlElement> xe)
{
	assert(xe.get() != nullptr && "xmlElement不能为nullptr");
	skipWhiteSpace();
	skipCommentAndHead();
	std::string text = "";
	while (m_buffer[cur] != '<' && cur != m_size) {
		text += m_buffer[cur];
		++cur;
	}

	if (!text.empty()) {
		xe->tagvalue = text;
		return true;
	}
	return false;
}

bool xmlDoc::getStartTag(std::shared_ptr<xmlElement> xe)
{
	assert(xe.get() != nullptr && "xmlElement不能为nullptr");
	skipWhiteSpace();

	
	 
	if (match("<")) {
		std::string tag = "";
		skipWhiteSpace();
		while (m_buffer[cur] != ' ' && m_buffer[cur] != '>') {
			tag += m_buffer[cur];
			++cur;
		}
		
		if (!tag.empty()) {
			xe->tagname = tag;
		}

		getAttribute(xe);

		if (match("/>")) { //不需要end tag
			return false;
		}
		
		if (match(">")) {
			return true;
		} else {
			setErrorStr("此处应该出现 '>' ");
			return false;
		}
	} else {
		setErrorStr("此处应该出现 '<' ");
		return false;
	}
	return true;
}

bool xmlDoc::getAttribute(std::shared_ptr<xmlElement> xe)
{
	assert(xe.get() != nullptr && "xmlElement不能为nullptr");
	skipWhiteSpace();
	if (m_buffer[cur] == '>' || !errormsg.empty()) {
		return false;
	}
	std::string attname = "";
	std::string attval = "";

	while (m_buffer[cur] != ' ' && m_buffer[cur] != '=') {
		attname += m_buffer[cur];
		++cur;
	}

	if (match("=")) {
		if (match("\"")) { //"
			skipWhiteSpace();
			while (m_buffer[cur] != ' ' && m_buffer[cur] != '"') {
				attval += m_buffer[cur];
				++cur;
			}
		} else {
			setErrorStr("此处应该出现 “");
			return false;
		}

		if (match("\"")) {
		} else {
			setErrorStr("此处应该出现 ”");
			return false;
		}
	} else {
		setErrorStr("此处应该出现 =");
		return false;
	}

	if (!attname.empty()) {
		xe->attributes.push_back(std::make_pair(attname,attval));
	} else {
		return false;
	}
	
	while(getAttribute(xe));

	return false;
}

bool xmlDoc::getEndTag(std::shared_ptr<xmlElement> xe)
{
	assert(xe.get() != nullptr && "xmlElement不能为nullptr");
	if (match("</")) {
		std::string endtag;
		while (m_buffer[cur] != ' ' && m_buffer[cur] != '>') {
			endtag += m_buffer[cur];
			++cur;
		}

		if (endtag != xe->tagname) {
			endtag = "结束标签应该为： " + xe->tagname;
			setErrorStr(endtag.c_str());
			return false;
		}
		skipWhiteSpace();

		if (m_buffer[cur] != '>') {
			setErrorStr("此处缺少反标签 >");
			return false;
		}
		
		return true;
	} else {
		setErrorStr("此处缺少结束标签 </>");
		return false;
	}

	return false;
}

//-----------------------------------------------
//xmlElement
//-----------------------------------------------
std::string xmlElement::Print()
{
	return tagname;
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