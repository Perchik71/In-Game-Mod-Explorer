#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace HotKey {
	INT WINAPI TextToVirtualKey(LPCSTR sKey, INT def);
}
