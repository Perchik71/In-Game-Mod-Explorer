#pragma once

#include <string>

#include "f4se\GameData.h"

#define ESP_NAME "ESPExplorerFO4.esp"

struct QModInfo {
	QModInfo(const char* FileName);

	std::string m_name;
	std::string m_title;
	size_t m_num;
	size_t m_armor, m_weapon, m_book, m_key;
	size_t m_miscItem, m_note, m_mod, m_ammo, m_alchemy;
};

typedef std::map<UInt16, QModInfo*> QModInfoCache;

class QModManager {
private:
	int	_ESPExplorerIndex;
	DataHandler* dataHandler;
	QModInfoCache modInfoCache;
	QModInfoCache lightModInfoCache;
private:
	void ReadMod();
	void ReadLightMod();
public:
	QModManager() = default;
	~QModManager() = default;
public:
	void Initialize();
public:
	__forceinline QModInfoCache* GetModInfoCache() { return &modInfoCache; }
	__forceinline QModInfoCache* GetLightModInfoCache() { return &lightModInfoCache; }
	__forceinline int GetESPExplorerIndex() const { return _ESPExplorerIndex; }

	__declspec(property(get = GetModInfoCache)) QModInfoCache* ModInfoCache;
	__declspec(property(get = GetLightModInfoCache)) QModInfoCache* LightModInfoCache;
	__declspec(property(get = GetESPExplorerIndex)) int ESPExplorerIndex;
};

