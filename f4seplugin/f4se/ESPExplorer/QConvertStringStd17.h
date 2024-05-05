#pragma once

#include <string.h>

class QConvertString {
public:
	std::string to_bytes(const std::wstring _w);
	std::wstring from_bytes(const std::string _s);
public:
	QConvertString() = default;
	~QConvertString() = default;
};

extern QConvertString convert_string;