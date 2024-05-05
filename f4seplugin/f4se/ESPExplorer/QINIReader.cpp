#include "QINIReader.h"
#include "QConvertStringStd17.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

bool QINIReader::ReadBool(const char* section, const char* key, bool defvalue) {
	return GetPrivateProfileIntW(convert_string.from_bytes(section).c_str(), convert_string.from_bytes(key).c_str(), (UINT)defvalue, m_name.c_str()) != 0;
}

int QINIReader::ReadInt(const char* section, const char* key, int defvalue) {
	return (INT)GetPrivateProfileIntW(convert_string.from_bytes(section).c_str(), convert_string.from_bytes(key).c_str(), (UINT)defvalue, m_name.c_str());
}

unsigned int QINIReader::ReadUInt(const char* section, const char* key, unsigned int defvalue) {
	return GetPrivateProfileIntW(convert_string.from_bytes(section).c_str(), convert_string.from_bytes(key).c_str(), (UINT)defvalue, m_name.c_str());
}

std::string QINIReader::ReadString(const char* section, const char* key, const char* defvalue) {
	WCHAR sValue[4096];
	GetPrivateProfileStringW(convert_string.from_bytes(section).c_str(), convert_string.from_bytes(key).c_str(), convert_string.from_bytes(defvalue).c_str(), sValue, 4096, m_name.c_str());
	return convert_string.to_bytes(sValue);
}

void QINIReader::WriteBool(const char* section, const char* key, bool value) {
	WCHAR sValue[4096];
	_swprintf(sValue, L"%d", value ? 1 : 0);
	WritePrivateProfileStringW(convert_string.from_bytes(section).c_str(), convert_string.from_bytes(key).c_str(), sValue, m_name.c_str());
}

void QINIReader::WriteInt(const char* section, const char* key, int value) {
	WCHAR sValue[4096];
	_swprintf(sValue, L"%d", value);
	WritePrivateProfileStringW(convert_string.from_bytes(section).c_str(), convert_string.from_bytes(key).c_str(), sValue, m_name.c_str());
}

void QINIReader::WriteUInt(const char* section, const char* key, unsigned int value) {
	WCHAR sValue[4096];
	_swprintf(sValue, L"%d", value);
	WritePrivateProfileStringW(convert_string.from_bytes(section).c_str(), convert_string.from_bytes(key).c_str(), sValue, m_name.c_str());
}

void QINIReader::WriteString(const char* section, const char* key, const char* value) {
	WritePrivateProfileStringW(convert_string.from_bytes(section).c_str(), convert_string.from_bytes(key).c_str(), convert_string.from_bytes(value).c_str(), m_name.c_str());
}

void QINIReader::Initialize(const wchar_t* name) {
	m_name = name;
}

QINIReader::QINIReader()
{}

bool QINIReaderUTF8::ReadBool(const char* section, const char* key, bool defvalue) {
	return GetPrivateProfileIntA(section, key, (UINT)defvalue, m_name.c_str()) != 0;
}

int QINIReaderUTF8::ReadInt(const char* section, const char* key, int defvalue) {
	return (INT)GetPrivateProfileIntA(section, key, (UINT)defvalue, m_name.c_str());
}

unsigned int QINIReaderUTF8::ReadUInt(const char* section, const char* key, unsigned int defvalue) {
	return GetPrivateProfileIntA(section, key, (UINT)defvalue, m_name.c_str());
}

std::string QINIReaderUTF8::ReadString(const char* section, const char* key, const char* defvalue) {
	CHAR sValue[4096];
	GetPrivateProfileStringA(section, key, defvalue, sValue, 4096, m_name.c_str());
	return sValue;
}

void QINIReaderUTF8::WriteBool(const char* section, const char* key, bool value) {
	CHAR sValue[4096];
	sprintf(sValue, "%d", value ? 1 : 0);
	WritePrivateProfileStringA(section, key, sValue, m_name.c_str());
}

void QINIReaderUTF8::WriteInt(const char* section, const char* key, int value) {
	CHAR sValue[4096];
	sprintf(sValue, "%d", value);
	WritePrivateProfileStringA(section, key, sValue, m_name.c_str());
}

void QINIReaderUTF8::WriteUInt(const char* section, const char* key, unsigned int value) {
	CHAR sValue[4096];
	sprintf(sValue, "%d", value);
	WritePrivateProfileStringA(section, key, sValue, m_name.c_str());
}

void QINIReaderUTF8::WriteString(const char* section, const char* key, const char* value) {
	WritePrivateProfileStringA(section, key, value, m_name.c_str());
}

void QINIReaderUTF8::Initialize(const char* name) {
	m_name = name;
}

QINIReaderUTF8::QINIReaderUTF8()
{}