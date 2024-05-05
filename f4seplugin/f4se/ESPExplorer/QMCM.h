#pragma once

#include "QINIReader.h"

extern QINIReaderUTF8 g_userIniFile;
extern QINIReaderUTF8 g_defaultIniFile;

extern CHAR g_keybindsFile[MAX_PATH + 1];

namespace MCM {

	bool ReadBool(const char* section, const char* key, bool defvalue);
	int ReadInt(const char* section, const char* key, int defvalue);
	unsigned int ReadUInt(const char* section, const char* key, unsigned int defvalue);
	std::string ReadString(const char* section, const char* key, const char* defvalue);

	namespace Keybinds {
		bool IsKeybinds();
		UInt32 GetHotKey();
	}
}