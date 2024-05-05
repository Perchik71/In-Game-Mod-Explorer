#pragma once

#define MODNAME "MODExplorer"
#define LIGHTMOD 0xE000

#include "common\IDebugLog.h"
#include "common\ICriticalSection.h"
#include "f4se\PluginAPI.h"
#include "f4se_common\f4se_version.h"
#include <shlobj.h>
#include "f4se\GameData.h"
#include "f4se\ScaleformCallbacks.h"
#include "f4se\ScaleformValue.h"
#include "f4se\GameTypes.h"
#include "f4se\PapyrusNativeFunctions.h"
#include "f4se\PapyrusVM.h"
#include "f4se\PapyrusArgs.h"
#include <conio.h>
#include <libloaderapi.h>
#include <tchar.h>
#include <shlwapi.h>
#include <io.h>
#include "f4se_common\Utilities.h"
#include "EE_Hooks_Input.h"

class TESGlobalExt : public TESForm
{
public:
	enum { kTypeID = kFormType_GLOB };

	BSString	unk20;	// 20
	Float32		m_value;	// 30
};

#define PATH_CURRENT	0
#define PATH_DLLPATH	1
#define PATH_MYDOCUMENT	2

//#define ESP_NAME (TEXT("ESPExplorerFO4.esp"))

bool IsValidItem(TESForm * pItem);
void ReadItem(FormType itemType, DataHandler* pDataHandler);
void InitPapyrusGlobal(const PCHAR sectionName, const PCHAR keyName, TESGlobalExt * global, int defaultValue );
void ReadTranslation();

TESForm * GetNthFormFromESP(StaticFunctionTag*, UInt32 , UInt32 );

