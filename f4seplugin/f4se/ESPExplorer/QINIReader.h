#pragma once

#include <string.h>

class QINIReader {
private:
	std::wstring m_name;
public:
	bool ReadBool(const char* section, const char* key, bool defvalue);
	int ReadInt(const char* section, const char* key, int defvalue);
	unsigned int ReadUInt(const char* section, const char* key, unsigned int defvalue);
	std::string ReadString(const char* section, const char* key, const char* defvalue);
	void WriteBool(const char* section, const char* key, bool value);
	void WriteInt(const char* section, const char* key, int value);
	void WriteUInt(const char* section, const char* key, unsigned int value);
	void WriteString(const char* section, const char* key, const char* value);
public:
	void Initialize(const wchar_t* name);
public:
	QINIReader();
};

class QINIReaderUTF8 {
private:
	std::string m_name;
public:
	bool ReadBool(const char* section, const char* key, bool defvalue);
	int ReadInt(const char* section, const char* key, int defvalue);
	unsigned int ReadUInt(const char* section, const char* key, unsigned int defvalue);
	std::string ReadString(const char* section, const char* key, const char* defvalue);
	void WriteBool(const char* section, const char* key, bool value);
	void WriteInt(const char* section, const char* key, int value);
	void WriteUInt(const char* section, const char* key, unsigned int value);
	void WriteString(const char* section, const char* key, const char* value);
public:
	void Initialize(const char* name);
public:
	QINIReaderUTF8();
};
