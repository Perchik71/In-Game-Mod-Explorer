#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "QConvertStringStd17.h"

QConvertString convert_string;

std::string QConvertString::to_bytes(const std::wstring _w) {
	int chars_num = WideCharToMultiByte(CP_UTF8, 0, _w.c_str(), -1, NULL, 0, NULL, NULL);
	char* str = new char[chars_num + 1];
	WideCharToMultiByte(CP_UTF8, 0, _w.c_str(), -1, str, chars_num, NULL, NULL);
	std::string res = str;
	delete[] str;
	return res;
}

std::wstring QConvertString::from_bytes(const std::string _s) {
	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, _s.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[wchars_num + 1];
	MultiByteToWideChar(CP_UTF8, 0, _s.c_str(), -1, wstr, wchars_num);
	std::wstring res = wstr;
	delete[] wstr;
	return res;
}