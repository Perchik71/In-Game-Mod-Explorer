#include "QMODExplorer.h"
#include "QModManager.h"
#include "QMCM.h"

#include <algorithm>

#include "common/IErrors.h"
#include "common/IDebugLog.h"

extern QINIReader g_ignoreIniFile;

QModInfo::QModInfo(const char* FileName):
	m_name(FileName), m_num(0), m_armor(0), m_weapon(0), m_book(0), m_key(0), m_miscItem(0), 
	m_note(0), m_mod(0), m_ammo(0), m_alchemy(0) 
{
	m_title = FileName;
#if 0
	// Remove the file extension and make the name uppercase

	auto it = m_title.find_last_of('.');
	if (it != std::string::npos)
		m_title = m_title.substr(0, it);

	std::for_each(m_title.begin(), m_title.end(), [](char& c) { c = ::toupper(c); });
#endif
}

void QModManager::ReadMod() {
	for (int i = 0; i < dataHandler->modList.loadedMods.count; i++) 
	{
		ModInfo* modInfo = dataHandler->modList.loadedMods[i];

		if (g_ignoreIniFile.ReadBool("Ignored Files", modInfo->name, false))
		{
			_MESSAGE("i: %d index: %X name: %s (IGNORED)", i, modInfo->modIndex, modInfo->name);
			continue;
		}

		_MESSAGE("i: %d index: %X name: %s", i, modInfo->modIndex, modInfo->name);

		if (modInfoCache.find(modInfo->modIndex) == modInfoCache.end())
			modInfoCache.insert(QModInfoCache::value_type(modInfo->modIndex, new QModInfo(modInfo->name)));

		if (!_stricmp(modInfo->name, ESP_NAME))
			_ESPExplorerIndex = modInfo->modIndex;
	}
}

void QModManager::ReadLightMod() {
	for (int i = 0; i < dataHandler->modList.lightMods.count; i++) 
	{
		ModInfo* modInfo = dataHandler->modList.lightMods[i];
		UInt32 lightIndex = modInfo->lightIndex | LIGHTMOD;

		if (g_ignoreIniFile.ReadBool("Ignored Files", modInfo->name, false))
		{
			_MESSAGE("i: %d index: %X name: %s (IGNORED)", i, lightIndex, modInfo->name);
			continue;
		}

		_MESSAGE("i: %d index: %X name: %s", i, lightIndex, modInfo->name);

		if (lightModInfoCache.find(lightIndex) == lightModInfoCache.end())
			lightModInfoCache.insert(QModInfoCache::value_type(lightIndex, new QModInfo(modInfo->name)));
	}
}
//| MASK_LIGHTMOD
void QModManager::Initialize() {
	dataHandler = *(g_dataHandler.GetPtr());
	ASSERT(dataHandler);

	_MESSAGE("=========================== Read Mods List ==========================");

	// lightModInfoCache no condition (may not be any)
	if (modInfoCache.empty()) {

		_MESSAGE("pDataHandler:%p loadedMods.capacity:%d loadedMods.count:%d lightMods.capacity:%d lightMods.count:%d", \
			dataHandler, dataHandler->modList.loadedMods.capacity, dataHandler->modList.loadedMods.count,
			dataHandler->modList.lightMods.capacity, dataHandler->modList.lightMods.count);

		_MESSAGE("ESM/ESP files:");
		ReadMod();

		_MESSAGE("ESL files:");
		ReadLightMod();
	}
}