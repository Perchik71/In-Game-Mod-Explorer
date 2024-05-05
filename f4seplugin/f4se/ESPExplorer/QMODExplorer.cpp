#include "QMODExplorer.h"
#include <string>
#include "f4se\GameRTTI.h"
#include "f4se\GameInput.h"
#include "common/IDebugLog.h"
#include <regex>
#include "f4se\GameMenus.h"

#include "QModManager.h"
#include "QMCM.h"

#include <future>
#include <thread>

#pragma warning (disable : 4477)

IDebugLog					gLog;
F4SEScaleformInterface*		g_scaleform		= nullptr;
F4SEPapyrusInterface*		g_papyrus		= nullptr;
F4SEMessagingInterface*		g_message		= nullptr;
PluginHandle				g_pluginHandle	= kPluginHandle_Invalid;

#define MASK_LIGHTMOD LIGHTMOD 

QINIReader g_tempIniFile;
QINIReader g_ignoreIniFile;

CHAR g_translationFile[MAX_PATH + 1];

static QModManager g_ModManager;

BGSListForm* g_formList = nullptr;

enum FormListID : size_t
{
	flArmor = 0,
	flAmmo,
	flWeapon,
	flBook,
	flNote,
	flMod,
	flKey,
	flAlchemy,
	flMiscItem,
	flMAX = flMiscItem
};

BGSListForm* g_List[FormListID::flMAX];

TESGlobalExt* glob_loop = nullptr;
TESGlobalExt* glob_itemQuantity = nullptr;
TESGlobalExt* glob_itemLimit = nullptr;
TESGlobalExt* glob_container = nullptr;

TESGlobalExt* glob_launchFlag = nullptr;

TESObjectCONT* g_ESPContainer = nullptr;
GFxMovieView* console_view = nullptr;

using namespace std;

typedef unordered_map<string, string> TranslationCache;
static TranslationCache s_TranslationCache;
string s_Text_SEARCH("Search");
string s_Text_RESULT("Result");

bool g_EnableClipboard = false;
bool g_AZERTYKeyboard = false;

extern USHORT g_espHotKey;

bool IsValidItem(TESForm * pItem)
{
	if (!pItem) return false;
	TESFullName * pName = DYNAMIC_CAST(pItem, TESForm, TESFullName);
	if (strlen(pName->name.c_str()) == 0 ) return false;
	
	BGSKeywordForm* pKeyword = DYNAMIC_CAST(pItem, TESForm, BGSKeywordForm);
	if (pKeyword) {
		for(UInt32 x = 0; x < pKeyword->numKeywords; x++) {	
			// FeaturedItem [KYWD:001B3FAC], quest item
			if(pKeyword->keywords[x]->formID == 0x183FAC)
				return false;
		}
	}

	return true;
};

bool HasKeyword(TESForm * pForm, UInt32 keywordID)
{
	BGSKeywordForm* pKeyword = DYNAMIC_CAST(pForm, TESForm, BGSKeywordForm);
	//_MESSAGE("pKeyword: %x formid:%08x keyword:%08x", pKeyword, pForm->formID, keywordID);
	if (pKeyword) {
		for(UInt32 x = 0; x < pKeyword->numKeywords; x++) {	
			// FeaturedItem [KYWD:001B3FAC], quest item
			if(pKeyword->keywords[x]->formID == keywordID)
				return true;
		}
	}
	return false;
};

ICriticalSection s_ItemCacheLock;

typedef unordered_map<UInt32, TESForm*> ItemCache;
static ItemCache s_ItemCache;

//namespace Parallel
//{
//	// https://stackoverflow.com/questions/40805197/parallel-for-each-more-than-two-times-slower-than-stdfor-each
//	// fast than tbb::parallel_for_each
//
//	class join_threads {
//	public:
//		explicit join_threads(std::vector<std::thread>& threads)
//			: threads_(threads) {}
//
//		~join_threads() {
//			for (size_t i = 0; i < threads_.size(); ++i) {
//				if (threads_[i].joinable())
//					threads_[i].join();
//			}
//		}
//
//	private:
//		std::vector<std::thread>& threads_;
//	};
//
//	template<typename Iterator, typename Func>
//	void for_each(Iterator first, Iterator last, Func func) {
//		const auto length = std::distance(first, last);
//		if (0 == length) return;
//
//		const auto min_per_thread = 25u;
//		const unsigned max_threads = (length + min_per_thread - 1) / min_per_thread;
//		const auto hardware_threads = std::thread::hardware_concurrency();
//		const auto num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2u, max_threads);
//		const auto block_size = length / num_threads;
//
//		std::vector<std::future<void>> futures(num_threads - 1);
//		std::vector<std::thread> threads(num_threads - 1);
//		join_threads joiner(threads);
//
//		auto block_start = first;
//		for (unsigned i = 0; i < num_threads - 1; ++i) {
//			auto block_end = block_start;
//			std::advance(block_end, block_size);
//			std::packaged_task<void(void)> task([block_start, block_end, func]()
//				{
//					std::for_each(block_start, block_end, func);
//				});
//			futures[i] = task.get_future();
//			threads[i] = std::thread(std::move(task));
//			block_start = block_end;
//		}
//
//		std::for_each(block_start, last, func);
//
//		for (size_t i = 0; i < num_threads - 1; ++i)
//			futures[i].get();
//	}
//}

void ReadItem(FormType itemType, DataHandler* pDataHandler)
{
	std::string keyName;
	UnkFormArray *pItems;

	// Getting data from a shared array in the game, depending on the data type

	switch (itemType)
	{
	case kFormType_ARMO:
		pItems = reinterpret_cast<UnkFormArray*>(&pDataHandler->arrARMO);
		keyName = "Armor";
		break;
	case kFormType_BOOK:
		pItems = reinterpret_cast<UnkFormArray*>(&pDataHandler->arrBOOK);
		keyName = "Book";
		break;
	case kFormType_MISC:
		pItems = reinterpret_cast<UnkFormArray*>(&pDataHandler->arrMISC);
		keyName = "MiscItem";
		break;
	case kFormType_WEAP:
		pItems = reinterpret_cast<UnkFormArray*>(&pDataHandler->arrWEAP);
		keyName = "Weapon";
		break;
	case kFormType_AMMO:
		pItems = reinterpret_cast<UnkFormArray*>(&pDataHandler->arrAMMO);
		keyName = "Ammo";
		break;
	case kFormType_KEYM:
		pItems = reinterpret_cast<UnkFormArray*>(&pDataHandler->arrKEYM);
		keyName = "Key";
		break;
	case kFormType_ALCH:
		pItems = reinterpret_cast<UnkFormArray*>(&pDataHandler->arrALCH);
		keyName = "Alchemy";
		break;
	case kFormType_NOTE:
		pItems = reinterpret_cast<UnkFormArray*>(&pDataHandler->arrNOTE);
		keyName = "Note";
		break;
	case kFormType_OMOD:
		pItems = reinterpret_cast<UnkFormArray*>(&pDataHandler->arrOMOD);
		keyName = "Mod";
		break;
	default:
		break;
	}

	// Unlikely errors, if any, the game will get CTD sooner, but still...

	if (!pItems)
		_FATALERROR("An error has occurred, the array with items is nullptr");

	if (!pItems->entries && (pItems->count > 0))
		_FATALERROR("An error has occurred, the pointer to the first item in the array is equal to nullptr");

	// If there is any data, we try to read it

	if (pItems->count > 0)
	{
		auto pCurrent = pItems->entries;
		auto pEndPrefix = pItems->entries + pItems->count;
		UInt32 uModIndex = 0;
		TESForm* pForm = nullptr;

		while (pCurrent < pEndPrefix)
		{
			pForm = *pCurrent;
			if (pForm && IsValidItem(pForm))
			{
				s_ItemCache.emplace(pForm->formID, pForm);
				
				//_MESSAGE("Form: %08X %s", pForm->formID, pForm->GetFullName());

				uModIndex = pForm->formID >> 24;
				if (uModIndex == 0xFE)
				{
					// light mods

					UInt16 uLightModIndex = (UInt16)((pForm->formID >> 12) & 0xFFF);
					QModInfoCache::iterator info = g_ModManager.LightModInfoCache->find(uLightModIndex | MASK_LIGHTMOD);

					if (info != g_ModManager.LightModInfoCache->end()) 
					{
						//_MESSAGE("Form: %08X %03X %s", pForm->formID, uLightModIndex, pForm->GetFullName());

						switch (itemType) 
						{
						case kFormType_ARMO: (info->second)->m_armor++;		break;
						case kFormType_BOOK: (info->second)->m_book++;		break;
						case kFormType_MISC: (info->second)->m_miscItem++;	break;
						case kFormType_WEAP: (info->second)->m_weapon++;	break;
						case kFormType_AMMO: (info->second)->m_ammo++;		break;
						case kFormType_KEYM: (info->second)->m_key++;		break;
						case kFormType_ALCH: (info->second)->m_alchemy++;	break;
						case kFormType_NOTE: (info->second)->m_note++;		break;
						case kFormType_OMOD: (info->second)->m_mod++;		break;
						}

						info->second->m_num++;
					}
				}
				else
				{
					// default mods

					QModInfoCache::iterator info = g_ModManager.ModInfoCache->find(uModIndex);
					if (info != g_ModManager.ModInfoCache->end()) 
					{
						switch (itemType)
						{
						case kFormType_ARMO: (info->second)->m_armor++;		break;
						case kFormType_BOOK: (info->second)->m_book++;		break;
						case kFormType_MISC: (info->second)->m_miscItem++;	break;
						case kFormType_WEAP: (info->second)->m_weapon++;	break;
						case kFormType_AMMO: (info->second)->m_ammo++;		break;
						case kFormType_KEYM: (info->second)->m_key++;		break;
						case kFormType_ALCH: (info->second)->m_alchemy++;	break;
						case kFormType_NOTE: (info->second)->m_note++;		break;
						case kFormType_OMOD: (info->second)->m_mod++;		break;
						}

						info->second->m_num++;
					}
				}
			}

			// next item
			pCurrent++;
		}
	}
}

TESForm* GetFormFromESP(UInt32 formID, UnkFormArray* pItems)
{
	if (g_ModManager.ESPExplorerIndex > 0){
		UInt32 _formid = g_ModManager.ESPExplorerIndex * 0x1000000 + formID;
		TESForm* pItem;
		for (UInt32 n = 0; n < pItems->count; n++) {
			//pItem = nullptr;
			pItems->GetNthItem(n, pItem);
			if (pItem && pItem->formID == _formid) {
#ifdef EEDEBUG
				Console_Print("fIND FORM ID:%8X", _formid);
#endif // EEDEBUG
				return pItem;
				break;
				//s_ItemCache.insert(ItemCache::value_type(pitem->formID, pitem));
				//_MESSAGE("%s %08X", keyName, pitem->formID);
			}
		}
	}
	return nullptr;
}

void InitPapyrusGlobal(const PCHAR sectionName, const PCHAR keyName, TESGlobalExt * global, int defaultValue = 0)
{
	if (global)
		global->m_value = MCM::ReadInt(sectionName, keyName, defaultValue);
}

void ReadToBuffer()
{
#if EEDEBUG
	_MESSAGE("ReadToBuffer() start");
#endif // EEDEBUG
	if (g_ModManager.ESPExplorerIndex < 0) return;
#if EEDEBUG
	_MESSAGE("ReadToBuffer() g_ESPExplorerIndex < 0");
#endif // EEDEBUG

	s_ItemCacheLock.Enter();

	if (s_ItemCache.empty()) 
	{
		// Initializing Mod Manager objects
		g_ModManager.Initialize();

		DataHandler* pDataHandler = *(g_dataHandler.GetPtr());
		// Only for test
		if (g_ModManager.ESPExplorerIndex > 0){
			TESForm *pForm;
			// ============================== FormList ===========================================
			pForm = GetFormFromESP(0xf99, reinterpret_cast<UnkFormArray*>(&pDataHandler->arrFLST));
			ASSERT_STR(g_formList = static_cast<BGSListForm*>(pForm), "Can't init g_formList");

			pForm = GetFormFromESP(0x3d52, reinterpret_cast<UnkFormArray*>(&pDataHandler->arrFLST));
			ASSERT_STR(g_List[FormListID::flAlchemy] = static_cast<BGSListForm*>(pForm), "Can't init g_ListAlchemy");
			
			pForm = GetFormFromESP(0x3d53, reinterpret_cast<UnkFormArray*>(&pDataHandler->arrFLST));
			ASSERT_STR(g_List[FormListID::flAmmo] = static_cast<BGSListForm*>(pForm), "Can't init g_ListAmmo");

			pForm = GetFormFromESP(0x3d54, reinterpret_cast<UnkFormArray*>(&pDataHandler->arrFLST));
			ASSERT_STR(g_List[FormListID::flWeapon] = static_cast<BGSListForm*>(pForm), "Can't init g_ListWeapon");

			pForm = GetFormFromESP(0x3d55, reinterpret_cast<UnkFormArray*>(&pDataHandler->arrFLST));
			ASSERT_STR(g_List[FormListID::flArmor] = static_cast<BGSListForm*>(pForm), "Can't init g_ListArmor");

			pForm = GetFormFromESP(0x3d56, reinterpret_cast<UnkFormArray*>(&pDataHandler->arrFLST));
			ASSERT_STR(g_List[FormListID::flBook] = static_cast<BGSListForm*>(pForm), "Can't init g_ListBook");

			pForm = GetFormFromESP(0x3d57, reinterpret_cast<UnkFormArray*>(&pDataHandler->arrFLST));
			ASSERT_STR(g_List[FormListID::flNote] = static_cast<BGSListForm*>(pForm), "Can't init g_ListNote");

			pForm = GetFormFromESP(0x3d58, reinterpret_cast<UnkFormArray*>(&pDataHandler->arrFLST));
			ASSERT_STR(g_List[FormListID::flMod] = static_cast<BGSListForm*>(pForm), "Can't init g_ListMod");

			pForm = GetFormFromESP(0x3d59, reinterpret_cast<UnkFormArray*>(&pDataHandler->arrFLST));
			ASSERT_STR(g_List[FormListID::flKey] = static_cast<BGSListForm*>(pForm), "Can't init g_ListKey");

			pForm = GetFormFromESP(0x3d5a, reinterpret_cast<UnkFormArray*>(&pDataHandler->arrFLST));
			ASSERT_STR(g_List[FormListID::flMiscItem] = static_cast<BGSListForm*>(pForm), "Can't init g_ListMiscItem");
			// =================================== Glob ========================================
			pForm = GetFormFromESP(0xf9A, reinterpret_cast<UnkFormArray*>(&pDataHandler->arrGLOB));
			ASSERT_STR(glob_loop = static_cast<TESGlobalExt *>(pForm), "Can't init glob_loop");

			pForm = GetFormFromESP(0xf9B, reinterpret_cast<UnkFormArray*>(&pDataHandler->arrGLOB));
			ASSERT_STR(glob_itemQuantity = static_cast<TESGlobalExt *>(pForm), "Can't init glob_itemQuantity");

			pForm = GetFormFromESP(0x1742, reinterpret_cast<UnkFormArray*>(&pDataHandler->arrCONT));
			ASSERT_STR(g_ESPContainer = static_cast<TESObjectCONT *>(pForm), "Can't init g_ESPContainer");

			if (MCM::ReadBool(MODNAME, "bArmor", true))
				ReadItem(kFormType_ARMO, pDataHandler);
			if (MCM::ReadBool(MODNAME, "bBook", true))
				ReadItem(kFormType_BOOK, pDataHandler);
			if (MCM::ReadBool(MODNAME, "bMiscItem", true))
				ReadItem(kFormType_MISC, pDataHandler);
			if (MCM::ReadBool(MODNAME, "bWeapon", true))
				ReadItem(kFormType_WEAP, pDataHandler);
			if (MCM::ReadBool(MODNAME, "bAmmo", true))
				ReadItem(kFormType_AMMO, pDataHandler);
			if (MCM::ReadBool(MODNAME, "bKey", true))
				ReadItem(kFormType_KEYM, pDataHandler);
			if (MCM::ReadBool(MODNAME, "bAlchemy", true))
				ReadItem(kFormType_ALCH, pDataHandler);
			if (MCM::ReadBool(MODNAME, "bNote", true))
				ReadItem(kFormType_NOTE, pDataHandler);
			if (MCM::ReadBool(MODNAME, "bMod", false))
				ReadItem(kFormType_OMOD, pDataHandler);
		}

		// end of testing
	}

	ReadTranslation();

#ifdef EEDEBUG
	_MESSAGE("End of Translation data.");
#endif // EEDEBUG

	InitPapyrusGlobal(MODNAME, "iItemQuantity", glob_itemQuantity, 5);
	InitPapyrusGlobal(MODNAME, "bLoopExplorer", glob_loop, 1);

	s_ItemCacheLock.Leave();
}

void ReadTranslation()
{
	if (s_TranslationCache.empty())
	{
		CHAR buff[2048];
		CHAR value[2048];
		size_t len, pointer;
		std::string keyName;
		len = GetPrivateProfileStringA("Translation", nullptr, "", buff, sizeof(buff), g_translationFile) + 1;
		if (len != 1)
		{
			buff[len - 1] = '\0';
			pointer = 0;
			for (int i = 0; i != len; i++) 
			{
				if (buff[i] == '\0' && pointer != i)
				{
					GetPrivateProfileStringA("Translation", &buff[pointer], "", value, sizeof(value), g_translationFile);
					
					if (!_stricmp(&buff[pointer], "SEARCH") && value[0] != '\0') 
						s_Text_SEARCH = value;
					else if (!_stricmp(&buff[pointer], "RESULT") && value[0] != '\0')
						s_Text_RESULT = value;

					keyName = std::string("T_") + &buff[pointer];
					s_TranslationCache.insert(TranslationCache::value_type(keyName, value));
					pointer = i + 1;
				}
			}
		}
	}
};

TESForm* GetNthFormFromESP(StaticFunctionTag*, UInt32 espID, UInt32 n)
{
#ifdef EEDEBUG
	Console_Print("hello world from example plugin");
#endif // EEDEBUG

	return nullptr;
	ReadToBuffer();
	auto it = s_ItemCache.find(espID * 0x1000000);
	if (n < distance(it, s_ItemCache.end()))
	{
		advance(it, n);
//		if ((it->first >> 24) == espID)
//			return it->second;
	}

	return nullptr;
}

UInt32 GetINIInt(StaticFunctionTag* base, BSFixedString section, BSFixedString key, UInt32 defaultvalue)
{
	if (!_stricmp(key.c_str(), "HotKey"))
		return MCM::Keybinds::GetHotKey();
		//return HotKey::TextToVirtualKey(MCM::ReadString(MODNAME, "sHotkey", "F10").c_str(), VK_F10);
	/*else if (!_stricmp(key.c_str(), "AZERTY"))
		return MCM::ReadBool(MODNAME, "bAZERTY", FALSE);*/

	return g_tempIniFile.ReadInt(section.c_str(), key.c_str(), defaultvalue);
}

UInt32 ResetHotKey(StaticFunctionTag* base, UInt32 newHotkey)
{
	_MESSAGE("Reinitialisation Hotkey:0x%X", newHotkey);
	
	g_espHotKey = newHotkey;
	g_AZERTYKeyboard = false;//MCM::ReadBool(MODNAME, "bAZERTY", false);

	return g_espHotKey;
}

bool GetShiftPress(StaticFunctionTag* base)
{
	return (GetAsyncKeyState(VK_SHIFT) & 0x8000);
}

void CleanAllFormList()
{
	for (size_t i = 0; i <= FormListID::flMAX; i++)
		g_List[i]->forms.Clear();
}

void AddToFormList(TESForm* tForm)
{
	switch (tForm->formType)
	{	
	case kFormType_AMMO: g_List[FormListID::flAmmo]->forms.Push(tForm); break;
	case kFormType_ARMO: g_List[FormListID::flArmor]->forms.Push(tForm); break;
	case kFormType_WEAP: g_List[FormListID::flWeapon]->forms.Push(tForm); break;
	case kFormType_BOOK: g_List[FormListID::flBook]->forms.Push(tForm); break;
	case kFormType_KEYM: g_List[FormListID::flKey]->forms.Push(tForm); break;
	case kFormType_OMOD: g_List[FormListID::flMod]->forms.Push(tForm); break;
	case kFormType_NOTE: g_List[FormListID::flNote]->forms.Push(tForm); break;
	case kFormType_ALCH: g_List[FormListID::flAlchemy]->forms.Push(tForm); break;
	case kFormType_MISC: 
		if (HasKeyword(tForm, 0x00135C17))
			g_List[FormListID::flMod]->forms.Push(tForm);
		else 
			g_List[FormListID::flMiscItem]->forms.Push(tForm);
		break;
	default:
		break;
	}
}

#define ISNUMBER(value) ((value.GetType()==GFxValue::Type::kType_Int) || (value.GetType()==GFxValue::Type::kType_Number) || (value.GetType()==GFxValue::Type::kType_UInt))

class GFxESPTransfer: public GFxFunctionHandler
{
public:
	virtual void Invoke(Args * args)
	{
		GFxMovieRoot * movieRoot = (args->movie)->movieRoot;
		GFxValue root;

		ASSERT_STR(movieRoot->GetVariable(&root, "root"), "GFxESPTransfer ERROR: couldn't get root");
		ASSERT_STR(ISNUMBER(args->args[0]) || args->args[0].IsString(), "GFxESPTransfer ERROR: Param 0 is not a number or a string.");

		CleanAllFormList();

		auto buffer = std::make_unique<char[]>(2048);

		if (args->args[0].IsString())
		{
			string keywordBuff = args->args[0].GetString();

			keywordBuff.erase(0, keywordBuff.find_first_not_of(" "));
			keywordBuff.erase(keywordBuff.find_last_not_of(" ") + 1);

			string keyword("");
			for each (auto c in keywordBuff) {
				switch (c)
				{
				case '(':
				case ')':
				case '[':
				case ']':
				case '^':
				case '$':
				case '.':
				case '|':
				case '+':
					keyword.append(1, '\\'); keyword.append(1, c); break;
				case '?': keyword.append(".{1}"); break;
				case '*': keyword.append(".*"); break;
				case '\\': keyword.append("\\\\"); break;
				default: keyword.append(1, c); break;
				}
			}

			size_t _num = 0;
			TESFullName* pName;
			string findStr;
			
			//Console_Print("keyword: %s", keyword.c_str());
			for each (auto item in s_ItemCache)
			{
				pName = DYNAMIC_CAST(item.second, TESForm, TESFullName);
				sprintf(buffer.get(), "%08X %s %s", item.second->formID, item.second->GetEditorID(), pName->name.c_str());

				std::smatch result;
				std::string sbuf = buffer.get();

				const std::regex pattern(keyword, std::regex_constants::icase);
				if (std::regex_search(sbuf, result, pattern))
				{
					AddToFormList(item.second);
					_num++;
				}
			}

			if (g_ESPContainer)
			{
				sprintf(buffer.get(), "%s:%s(%d)", s_Text_RESULT.c_str(), keywordBuff, _num);
				CALL_MEMBER_FN(&(g_ESPContainer->fullName.name), Set)(buffer.get());
			}

			sprintf(buffer.get(), "%d", _num);
			movieRoot->CreateString(args->result, buffer.get());
		}
		else
		{
			// The user has selected a plugin
			UInt32 modIndex = args->args[0].GetInt(), itemTotal = 0;
			bool modIsLight = (modIndex & MASK_LIGHTMOD) == MASK_LIGHTMOD;

			if (g_ESPContainer)
			{
				const char* modTitle;

				if (modIsLight)
					modTitle = (*g_ModManager.LightModInfoCache)[modIndex]->m_title.c_str();
				else
					modTitle = (*g_ModManager.ModInfoCache)[modIndex]->m_title.c_str();

				CALL_MEMBER_FN(&(g_ESPContainer->fullName.name), Set)(modTitle);

				_MESSAGE("Index of the selected plugin: %08X %s", modIndex, modTitle);
			}

			if (modIsLight)
			{
				for each (auto item in s_ItemCache)
				{
					if (((item.first >> 24) == 0xFE) && (((item.first >> 12) & 0xFFF) == (modIndex & 0xFFF)))
					{
						AddToFormList(item.second);
						itemTotal++;
					}
				}
			}
			else
			{
				for each (auto item in s_ItemCache)
				{
					if ((item.first >> 24) == modIndex)
					{
						AddToFormList(item.second);
						itemTotal++;
					}
				}
			}

			sprintf(buffer.get(), "%d", itemTotal);
			movieRoot->CreateString(args->result, buffer.get());
		}
	}
};

class GFxESPGetIni: public GFxFunctionHandler
{
public:
	virtual void Invoke(Args * args)
	{
		ASSERT_STR(args->numArgs == 3 && args->args[0].IsString() && args->args[1].IsString(), "GetIni parameters error");

		bool isGetString = args->args[2].IsString();
		
		int value;
		string sValue;
		if (isGetString)
			sValue = g_tempIniFile.ReadString(args->args[0].GetString(), args->args[1].GetString(), args->args[2].GetString());
		else
			value = g_tempIniFile.ReadInt(args->args[0].GetString(), args->args[1].GetString(), args->args[2].GetInt());

		GFxMovieRoot* movieRoot = (args->movie)->movieRoot;
		GFxValue root;
		if(!movieRoot->GetVariable(&root, "root"))
		{
			_FATALERROR("Couldn't get root");
			return;
		} 

		CHAR buffer[MAX_PATH + 1];
		if (isGetString)
			sprintf(buffer, "%s", sValue);
		else
			sprintf(buffer, "%d", value);

		movieRoot->CreateString(args->result, buffer);
	}
};

class GFxESPSetIni: public GFxFunctionHandler
{
public:
	virtual void Invoke(Args * args)
	{
		ASSERT_STR(args->numArgs == 3 && args->args[0].IsString() && args->args[1].IsString(), "SetIni parameters error");

		if (args->args[2].IsString())
			g_tempIniFile.WriteString(args->args[0].GetString(), args->args[1].GetString(), args->args[2].GetString());
		else 
			g_tempIniFile.WriteInt(args->args[0].GetString(), args->args[1].GetString(), args->args[2].GetInt());
	}
};

class GFxESPAllowInput: public GFxFunctionHandler
{
public:
	virtual void Invoke(Args * args)
	{
		ASSERT_STR((args->numArgs == 1) && args->args[0].IsBool(), "GFxESPAllowInput ERROR: Para0 is not a bool.");
		bool allowInput = args->args[0].GetBool();

		(*g_inputMgr)->AllowTextInput(allowInput);
	}
};

class GFxESPFunction : public GFxFunctionHandler
{
public:
	virtual void Invoke(Args* args)
	{
		GFxMovieRoot* movieRoot = (args->movie)->movieRoot;
		GFxValue root;
		if(!movieRoot->GetVariable(&root, "root"))
		{
			_ERROR("GFxESPFunction:couldn't get root");
			return;
		} 

		movieRoot->CreateObject(args->result);
		
		CHAR buffer[1024];
		CHAR nameBuffer[1024];
		DataHandler* pDataHandler = *(g_dataHandler.GetPtr());

		ReadToBuffer();

		// glob_launchFlag must exist it's important

		if (!glob_launchFlag)
			_FATALERROR("glob_launchFlag is nullptr");

		UInt32 counter = 0;
		UInt16 selfID = (UInt16)(glob_launchFlag->formID >> 24);

		// For some reason, the list is upside down, so first we will
		// work out light mods upside down, and then also the usual ones.

		for (auto itCurrent = g_ModManager.LightModInfoCache->rbegin();
			itCurrent != g_ModManager.LightModInfoCache->rend(); itCurrent++)
		{
			// Mod index
			sprintf(nameBuffer, "id%d", counter);
			sprintf(buffer, "%d", itCurrent->first);
			args->result->SetMember(nameBuffer, &GFxValue(buffer));

			// Mod name
			sprintf(nameBuffer, "name%d", counter);
			sprintf(buffer, "L: %s", itCurrent->second->m_title.c_str());
			args->result->SetMember(nameBuffer, &GFxValue(buffer));

			// Hide or show the light mod
			// If there is no items in the light mod hide mod
			sprintf(nameBuffer, "filter%d", counter);
			sprintf(buffer, "%d", itCurrent->second->m_num == 0);
			args->result->SetMember(nameBuffer, &GFxValue(buffer));

			// Show number of items in the light mod
			sprintf(nameBuffer, "num%d", counter);
			sprintf(buffer, "%d", itCurrent->second->m_num);
			args->result->SetMember(nameBuffer, &GFxValue(buffer));

			// Next
			counter++;
		}

		for (auto itCurrent = g_ModManager.ModInfoCache->rbegin(); 
			itCurrent != g_ModManager.ModInfoCache->rend(); itCurrent++)
		{
			// This mod needs to be spared
			if (selfID == itCurrent->first)
				continue;

			// Mod index
			sprintf(nameBuffer, "id%d", counter);
			sprintf(buffer, "%d", itCurrent->first);	
			args->result->SetMember(nameBuffer, &GFxValue(buffer));
			
			// Mod name
			sprintf(nameBuffer, "name%d", counter);
			sprintf(buffer, "%s", itCurrent->second->m_title.c_str());
			args->result->SetMember(nameBuffer, &GFxValue(buffer));

			// Hide or show the mod
			// If there is no items in the mod hide mod
			sprintf(nameBuffer, "filter%d", counter);
			sprintf(buffer, "%d", itCurrent->second->m_num == 0);
			args->result->SetMember(nameBuffer, &GFxValue(buffer));
			
			// Show number of items in the mod
			sprintf(nameBuffer, "num%d", counter);
			sprintf(buffer, "%d", itCurrent->second->m_num);
			args->result->SetMember(nameBuffer, &GFxValue(buffer));

			// Next
			counter++;
		}

		// Output the number of visible mods
		sprintf(buffer, "%d", counter);
		args->result->SetMember("num", &GFxValue(buffer));

		for each (auto info in s_TranslationCache)
			args->result->SetMember(info.first.c_str(), &GFxValue(info.second.c_str()));
	}
};

void GetClipboard()
{
	if (OpenClipboard(NULL)){
		HANDLE hGlobal;
		if (hGlobal = GetClipboardData(CF_TEXT)) {
			PTCHAR recstr = nullptr;
			PTCHAR pGlobal = (PTCHAR)GlobalLock(hGlobal); 
			_MESSAGE("Clipboard :%s", pGlobal);
			GlobalUnlock(hGlobal);
		}

		CloseClipboard(); 
	}
}

bool GetFullNameA(CHAR* fullName, const UInt32 dirType, const CHAR* fileName)
{
	CHAR szPath[MAX_PATH] = "";

	switch (dirType)
	{
	case PATH_CURRENT:
		break;
	case PATH_DLLPATH: {
		GetModuleFileNameA(NULL, szPath, MAX_PATH);
		std::string str = szPath;
		size_t found = str.find_last_of("/\\");
		strcpy_s(szPath, MAX_PATH, str.substr(0, found).c_str());
		break;
	}
	case PATH_MYDOCUMENT:
		SHGetFolderPathA(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, szPath);
		break;
	default:
		return false;
		break;
	}

	sprintf(fullName, "%s%s", szPath, fileName);

	return TRUE;
}

bool GetFullNameW(WCHAR *fullName, const UInt32 dirType, const WCHAR *fileName)
{
	WCHAR szPath[MAX_PATH] = L"";

	switch (dirType)
	{
	case PATH_CURRENT:
		break;
	case PATH_DLLPATH: {
		GetModuleFileNameW(NULL, szPath, MAX_PATH);
		std::wstring str = szPath;
		size_t found = str.find_last_of(L"/\\");
		wcscpy_s(szPath, MAX_PATH, str.substr(0, found).c_str());
		break;
	}
	case PATH_MYDOCUMENT:
		SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, szPath);
		break;
	default:
		return false;
		break;
	}

	_swprintf(fullName, L"%s%s", szPath, fileName);

	return TRUE;
}

void CreateIniFile()
{
	GetFullNameA(g_translationFile, PATH_DLLPATH, "\\Data\\F4SE\\Plugins\\MODExplorer.translation");
	GetFullNameA(g_keybindsFile, PATH_DLLPATH, "\\Data\\MCM\\Settings\\Keybinds.json");

	WCHAR szBuffPath[MAX_PATH + 1];
	WCHAR templateFileName[MAX_PATH + 1];

	GetFullNameW(templateFileName, PATH_DLLPATH, L"\\Data\\F4SE\\Plugins\\MODExplorerTemp.template");
	GetFullNameW(szBuffPath, PATH_MYDOCUMENT, L"\\My Games\\Fallout4\\F4SE\\MODExplorerTemp.ini");
	CopyFileW(templateFileName, szBuffPath, TRUE);
	g_tempIniFile.Initialize(szBuffPath);

	GetFullNameW(templateFileName, PATH_DLLPATH, L"\\Data\\F4SE\\Plugins\\MODExplorerIgnoredFiles.template");
	GetFullNameW(szBuffPath, PATH_MYDOCUMENT, L"\\My Games\\Fallout4\\F4SE\\MODExplorerIgnoredFiles.ini");
	CopyFileW(templateFileName, szBuffPath, TRUE);
	g_ignoreIniFile.Initialize(szBuffPath);

	CHAR szBuffPathA[MAX_PATH + 1];

	GetFullNameA(szBuffPathA, PATH_DLLPATH, "\\Data\\MCM\\Config\\MODExplorer\\settings.ini");
	g_defaultIniFile.Initialize(szBuffPathA);
	GetFullNameA(szBuffPathA, PATH_DLLPATH, "\\Data\\MCM\\Settings\\MODExplorer.ini");
	g_userIniFile.Initialize(szBuffPathA);
}

BSFixedString GetTest(StaticFunctionTag* thisInput, BSFixedString s)
{
	BSFixedString str("This is a Papyrus!");
	return str;
};

void HotKeyProc()
{
	//if (g_ESPExplorerIndex < 0) return;

	_MESSAGE("Hotkey Pressed! GameReady: %d g_MODExplorerIndex: %d", *g_isGameDataReady, g_ModManager.ESPExplorerIndex);
	if (!glob_launchFlag && (*g_isGameDataReady))
	{
		DataHandler* pDataHandler = *(g_dataHandler.GetPtr());
		
		// Only for test
		if (g_ModManager.ESPExplorerIndex <= 0)
			g_ModManager.Initialize();

		if (g_ModManager.ESPExplorerIndex > 0)
		{
			TESForm *pForm;

			pForm = GetFormFromESP(0xfB0, reinterpret_cast<UnkFormArray*>(&pDataHandler->arrGLOB));
			glob_launchFlag = static_cast<TESGlobalExt*>(pForm);
			_MESSAGE("ESPExplorerFO4.esp found. glob_launchFlag: %d", glob_launchFlag->m_value);
		} 
		else
			_MESSAGE("Can't find ESPExplorerFO4.esp");
	}

	_MESSAGE("Prepare to send command. glob_launchFlag = %d g_MODExplorerIndex = %d", glob_launchFlag->m_value, 
		g_ModManager.ESPExplorerIndex);
	
	if (glob_launchFlag && (glob_launchFlag->m_value == 0) && g_ModManager.ESPExplorerIndex >= 0)
	{
		_MESSAGE("Command sent to papyrus." );
		glob_launchFlag->m_value = 1;
	} 
	else
		_MESSAGE("Cancel command.");
}

void F4SEListener(F4SEMessagingInterface::Message* msg)
{
	if (msg->type == F4SEMessagingInterface::kMessage_GameDataReady)
	{
		_MESSAGE("Prepare buffer. Message sender: %s, type: %d", msg->sender, msg->type);
		ReadToBuffer();
	}
};

bool RegisterPapyrus(VirtualMachine * vm)
{
	vm->RegisterFunction(
		new NativeFunction3<StaticFunctionTag, UInt32, BSFixedString, BSFixedString, UInt32>("GetINIInt", "espE_f4seFuncs", GetINIInt, vm));
	vm->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, bool>("GetShiftPress", "espE_f4seFuncs", GetShiftPress, vm));
	vm->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, UInt32, UInt32>("ResetHotKey", "espE_f4seFuncs", ResetHotKey, vm));
	
	return true;
};

bool RegisterScaleform(GFxMovieView * view, GFxValue * root)
{
	RegisterFunction <GFxESPFunction>(root, view->movieRoot, "GetESPInfo");
	RegisterFunction <GFxESPGetIni>(root, view->movieRoot, "GetIni");
	RegisterFunction <GFxESPSetIni>(root, view->movieRoot, "SetIni");
	RegisterFunction <GFxESPTransfer>(root, view->movieRoot, "Transfer");
	RegisterFunction <GFxESPAllowInput>(root, view->movieRoot, "AllowInput");

	return true;
}

extern "C"
{
	__declspec(dllexport) F4SEPluginVersionData F4SEPlugin_Version =
	{
		F4SEPluginVersionData::kVersion,
		1,
		MODNAME,
		"perchik71",
		0,
		0,
		{ RUNTIME_VERSION_1_10_980, 0 },
		0,	// works with any version of the script extender. you probably do not need to put anything here
	};

	bool F4SEPlugin_Load(const F4SEInterface* f4se)
	{
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Fallout4\\F4SE\\" MODNAME ".log");
		_MESSAGE(MODNAME" F4SEPlugin_Loads");

		CreateIniFile();

		// store plugin handle so we can identify ourselves later
		g_pluginHandle = f4se->GetPluginHandle();

		_MESSAGE("F4SE version check. f4seVersion:0x%x,runtimeVersion:0x%x", f4se->f4seVersion, f4se->runtimeVersion);

		if (f4se->isEditor)
		{
			_MESSAGE("loaded in editor, marking as incompatible");

			return false;
		}
		else if (f4se->runtimeVersion != CURRENT_RELEASE_RUNTIME)
		{
			_FATALERROR("MODExplorer: unsupported runtime version %08X", f4se->runtimeVersion);

			return false;
		}

		// get the scaleform interface and query its version
		if (!(g_scaleform = (F4SEScaleformInterface*)f4se->QueryInterface(kInterface_Scaleform)))
		{
			_FATALERROR("couldn't get scaleform interface");

			return false;
		}
		else
		{
			if (g_scaleform->interfaceVersion < F4SEScaleformInterface::kInterfaceVersion)
			{
				_FATALERROR("scaleform interface too old (%d expected %d)", g_scaleform->interfaceVersion,
					F4SEScaleformInterface::kInterfaceVersion);

				return false;
			}
		}

		if (!(g_papyrus = (F4SEPapyrusInterface*)f4se->QueryInterface(kInterface_Papyrus)))
		{
			_FATALERROR("couldn't get papyrus interface");

			return false;
		}
		else
		{
			if (g_papyrus->interfaceVersion < F4SEPapyrusInterface::kInterfaceVersion)
			{
				_FATALERROR("papyrus interface too old (%d expected %d)", g_papyrus->interfaceVersion,
					F4SEPapyrusInterface::kInterfaceVersion);

				return false;
			}
		}

		// get the serialization interface and query its version
		if (!(g_message = (F4SEMessagingInterface*)f4se->QueryInterface(kInterface_Messaging)))
		{
			_FATALERROR("couldn't get messaging interface");

			return false;
		}
		else
		{
			if (g_message->interfaceVersion < F4SEMessagingInterface::kInterfaceVersion)
			{
				_FATALERROR("F4SEMessagingInterface interface too old (%d expected %d)",
					g_message->interfaceVersion, F4SEMessagingInterface::kInterfaceVersion);

				return true;
			}
		}

		// register scaleform callbacks
		if (g_scaleform){
			g_scaleform->Register("ESPPlugin", RegisterScaleform);
		}

		// register papyrus functions
		if (g_papyrus) {
			g_papyrus->Register(RegisterPapyrus);
		}
		// register callbacks and unique ID for serialization

		if (g_message){
			g_message->RegisterListener(1, "F4SE", F4SEListener);
		}

		// Read Hotkey
		int vKey = MCM::Keybinds::GetHotKey();
			//HotKey::TextToVirtualKey(MCM::ReadString(MODNAME, "sHotkey", "F10").c_str(), VK_F10);
		_MESSAGE("Initialisation Hotkey:0x%X", vKey);

		Hooks_Input_Init();
		Hooks_Input_Commit(vKey);

		g_AZERTYKeyboard = false;//MCM::ReadBool(MODNAME, "bAZERTY", false);

		return true;
	}
};
