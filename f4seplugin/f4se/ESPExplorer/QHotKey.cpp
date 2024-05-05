#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>
#include <map>

namespace HotKey {

	std::map<INT, std::string> mapKey = {
		{'0', "0"},
		{'1', "1"},
		{'2', "2"},
		{'3', "3"},
		{'4', "4"},
		{'5', "5"},
		{'6', "6"},
		{'7', "7"},
		{'8', "8"},
		{'9', "9"},
		{'A', "A"},
		{'B', "B"},
		{'C', "C"},
		{'D', "D"},
		{'E', "E"},
		{'F', "F"},
		{'G', "G"},
		{'H', "H"},
		{'I', "I"},
		{'J', "J"},
		{'K', "K"},
		{'L', "L"},
		{'M', "M"},
		{'N', "N"},
		{'O', "O"},
		{'P', "P"},
		{'Q', "Q"},
		{'R', "R"},
		{'S', "S"},
		{'T', "T"},
		{'U', "U"},
		{'V', "V"},
		{'W', "W"},
		{'X', "X"},
		{'Y', "Y"},
		{'Z', "Z"},
		{VK_F1, "F1"},
		{VK_F2, "F2"},
		{VK_F3, "F3"},
		{VK_F4, "F4"},
		{VK_F5, "F5"},
		{VK_F6, "F6"},
		{VK_F7, "F7"},
		{VK_F8, "F8"},
		{VK_F9, "F9"},
		{VK_F10, "F10"},
		{VK_F11, "F11"},
		{VK_F12, "F12"},
		{VK_NUMPAD0, "NUM0"},
		{VK_NUMPAD1, "NUM1"},
		{VK_NUMPAD2, "NUM2"},
		{VK_NUMPAD3, "NUM3"},
		{VK_NUMPAD4, "NUM4"},
		{VK_NUMPAD5, "NUM5"},
		{VK_NUMPAD6, "NUM6"},
		{VK_NUMPAD7, "NUM7"},
		{VK_NUMPAD8, "NUM8"},
		{VK_NUMPAD9, "NUM9"},
	};

	INT WINAPI TextToVirtualKey(LPCSTR sKey, INT def) {
		for each (auto key in mapKey) {
			if (!_stricmp(key.second.c_str(), sKey))
				return key.first;
		}

		return def;
	}
}