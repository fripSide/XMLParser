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
	std::shared_ptr<xmlElement> cnode(new xmlElement);
	root->childrens.push_back(cnode);
	cnode->parent = root.get();
	if (!getNode(cnode)) {
		return false;
	}
	return true;
}

void xmlDoc::Traverse(std::shared_ptr<xmlElement> rt)
{
	std::ofstream out("testwrite.xml",std::ios::binary);
	if (!out.is_open()) {
		return;
	}
	
	if (rt.get() != nullptr) {
		for (auto tit = rt->childrens.begin(); tit != rt->childrens.end(); ++tit) {
			(*tit)->Print(out);
		}
	}

	out.close();
}

std::shared_ptr<xmlElement> xmlDoc::GetRoot() const
{
	return root;
}

void xmlDoc::skipWhiteSpace()
{
	while ((m_buffer[cur] == ' ' || 
		m_buffer[cur] == '\r'    || 
		m_buffer[cur] == '\t'    ||
		m_buffer[cur] == '\n' )  &&
		cur != m_size) {
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

	if (cur == m_size) {
		return true;
	}

	if (!matchnext("<") || matchnext("</") || !errormsg.empty() ) {
		return false;
	}
	
	if (!getStartTag(xe)) { //获取标签失败
		return false;
	}

	//遇到< />直接结束,就读下一行
	if (!xe->needEndTag) {
		std::shared_ptr<xmlElement> cnode(new xmlElement);
		xmlElement* parent = xe->parent;
		cnode->parent = parent;
		parent->childrens.push_back(cnode);
		getNode(cnode);
		return true;
	}

	bool hastext = true;
	while (nextIsStartTag()) { //add child node
		std::shared_ptr<xmlElement> cnode(new xmlElement);
		cnode->parent = xe.get();
		xe->childrens.push_back(cnode);
		getNode(cnode);
		hastext = false;
	}
	if (hastext) {
		getText(xe);
	}
	
	if (matchnext("</")  || !errormsg.empty()) {
		if (!getEndTag(xe)) {
			return false;
		}
	}
	
	return true;
}

bool xmlDoc::skipCommentAndHead() // skip coment and head <? ？>
{
	while (match("/r") || match("/n") || match("/t"));
	
	if (match("<?")) {
		while (m_buffer[cur + 1] != '>' && m_buffer[cur] != '>' && cur != m_size - 2) {
			++cur;
		}
		if (!match("?>")) {
			setErrorStr("此应该出现 ?>");
			return false;
		}
	}
	
	//skipcomment
	while (match("<!--")) {
		while (m_buffer[cur] != '>' && m_buffer[cur + 2] != '>' && cur != m_size - 3) {
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
	int t = cur;
	
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

bool xmlDoc::matchnext(const char* text)
{
	skipWhiteSpace();
	skipCommentAndHead();
	int len = strlen(text);
	if (strncmp(text,&m_buffer[cur],len) == 0) {
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
	if (match("<")) {
		std::string tag = "";
		skipWhiteSpace();
		while (m_buffer[cur] != ' ' 
			&& m_buffer[cur] != '\r'
			&& m_buffer[cur] != '\n'
			&& m_buffer[cur] != '\t'
			&& m_buffer[cur] != '>'
			&& m_buffer[cur] != '/') {
			tag += m_buffer[cur];
			++cur;
		}
		
		if (!tag.empty()) {
			xe->tagname = tag;
		}

		getAttribute(xe);

		if (match("/>")) { //不需要end tag
			xe->needEndTag = false;
			return true;
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
	if (m_buffer[cur] == '>' || m_buffer[cur] == '/' || !errormsg.empty()) {
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
	char b = m_buffer[cur -1];
	char c = m_buffer[cur];
	char d = m_buffer[cur + 1];
	char e = m_buffer[cur + 2];
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

		if (m_buffer[cur] == '>') {
			++cur;
			return true;
		} else {
			setErrorStr("此处缺少反标签 >");
			return false;
		}
		
	} else {
		setErrorStr("此处缺少结束标签 </>");
		return false;
	}

	return true;
}

//-----------------------------------------------
//xmlElement
//-----------------------------------------------
void xmlElement::Print(std::ostream& out)
{
	std::string reval = "<";
	reval += tagname;
	reval += " ";
	for (auto mit = attributes.begin(); mit != attributes.end(); ++mit) {
		reval += mit->first;
		reval += "=\"";
		reval += mit->second;
		reval += "\" ";
	}
	if (!needEndTag) {
		reval += "/>\r\n";
		out<<reval;
		return;
	} else {
		reval += ">";
	}
	
	out<<reval;

	for (auto citer = childrens.begin(); citer != childrens.end(); ++citer) {
		(*citer)->Print(out);		
	}
	
	reval = "";
	reval += tagvalue;
	reval += "</";
	reval += tagname;
	reval += ">\r\n";
	out<<reval;	
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