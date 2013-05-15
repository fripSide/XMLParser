#pragma once

//-----------------------------------------------
//
//-----------------------------------------------

#include <windows.h>
#include <iostream>
#include <string>

#include "XMLDocument.h"

std::string GetCurPath();

int main(int argc,char* argv[]) {

	std::string str = GetCurPath();
    //str = str.substr(0,str.find_last_of("\\/"));
	//str = str.substr(0,str.find_last_of("\\/"));
	str += "\\test\\xml.txt";

	std::cout << str <<std::endl; //<<<<<<<debug

	xmlDoc doc;
	doc.LoadFile(str.data());
	
	getchar();
	return 0;
}

std::string GetCurPath() {
	wchar_t path[MAX_PATH];
	HMODULE hModule = GetModuleHandle(NULL);
	GetModuleFileName(hModule,path,MAX_PATH);
	std::wstring wstr(path);
	std::string str(wstr.begin(),wstr.end());

	return str.substr(0,str.find_last_of("\\/"));
}