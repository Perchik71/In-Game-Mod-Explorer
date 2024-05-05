#include "QMCM.h"
#include "QMODExplorer.h"

#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include <json/single_include/nlohmann/json.hpp>

#include <fstream>
#include <filesystem>

QINIReaderUTF8 g_userIniFile;
QINIReaderUTF8 g_defaultIniFile;

CHAR g_keybindsFile[MAX_PATH + 1];

namespace MCM {
	__forceinline bool HasUserConfigureValueInt(const char* section, const char* key) {
		return g_userIniFile.ReadInt(section, key, -1) != -1;
	}

	__forceinline bool HasUserConfigureValueString(const char* section, const char* key) {
		return g_userIniFile.ReadString(section, key, "<no user>").compare("<no user>") != 0;
	}

	bool ReadBool(const char* section, const char* key, bool defvalue) {
		if (HasUserConfigureValueInt(section, key))
			return g_userIniFile.ReadBool(section, key, defvalue);
		else
			return g_defaultIniFile.ReadBool(section, key, defvalue);
	}

	int ReadInt(const char* section, const char* key, int defvalue) {
		if (HasUserConfigureValueInt(section, key))
			return g_userIniFile.ReadInt(section, key, defvalue);
		else
			return g_defaultIniFile.ReadInt(section, key, defvalue);
	}

	unsigned int ReadUInt(const char* section, const char* key, unsigned int defvalue) {
		if (HasUserConfigureValueInt(section, key))
			return g_userIniFile.ReadUInt(section, key, defvalue);
		else
			return g_defaultIniFile.ReadUInt(section, key, defvalue);
	}

	std::string ReadString(const char* section, const char* key, const char* defvalue) {
		if (HasUserConfigureValueString(section, key))
			return g_userIniFile.ReadString(section, key, defvalue);
		else
			return g_defaultIniFile.ReadString(section, key, defvalue);
	}

	namespace Keybinds {
		bool IsKeybinds() {
			if (!std::filesystem::exists(g_keybindsFile))
				return false;

			std::ifstream ifs(g_keybindsFile);
			nlohmann::json j;
			ifs >> j;
			ifs.close();

			if (j.contains("keybinds") && j.at("keybinds").is_array()) {
				auto keybindsArray = j.at("keybinds");
				// iterate the array
				for (auto it = keybindsArray.begin(); it != keybindsArray.end(); ++it) {
					auto structIt = *it;
					if (structIt.contains("modName") && !_stricmp(MODNAME, structIt.at("modName").get<std::string>().c_str()))
						return true;
				}
			}

			return false;
		}

		UInt32 GetHotKey() {
			if (!IsKeybinds())
				return VK_F10;

			std::ifstream ifs(g_keybindsFile);
			nlohmann::json j;
			ifs >> j;
			ifs.close();

			if (j.contains("keybinds") && j.at("keybinds").is_array()) {
				auto keybindsArray = j.at("keybinds");
				// iterate the array
				for (auto it = keybindsArray.begin(); it != keybindsArray.end(); ++it) {
					auto structIt = *it;
					if (structIt.contains("modName") && !_stricmp(MODNAME, structIt.at("modName").get<std::string>().c_str()))
						return (UInt32)structIt.at("keycode").get<int>();
				}
			}

			// return bimbo for compile
			return VK_F10;
		}
	}
}