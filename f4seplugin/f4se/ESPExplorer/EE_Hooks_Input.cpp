#include "f4se_common/Utilities.h"
#include "f4se_common/SafeWrite.h"

#define LOG_INPUT_HOOK 0

typedef BOOL (WINAPI * _RegisterRawInputDevices)(RAWINPUTDEVICE * devices, UINT numDevices, UINT structSize);
_RegisterRawInputDevices Original_RegisterRawInputDevices = nullptr;

typedef UINT (WINAPI * _GetRawInputData)(HRAWINPUT rawinput, UINT cmd, void * data, UINT * dataSize, UINT headerSize);
_GetRawInputData Original_GetRawInputData = nullptr;

USHORT g_espHotKey;
extern void HotKeyProc();
extern bool g_EnableClipboard;
extern void GetClipboard();
//extern bool g_AZERTYKeyboard;

BOOL WINAPI Hook_RegisterRawInputDevices(RAWINPUTDEVICE * devices, UINT numDevices, UINT structSize)
{
#if LOG_INPUT_HOOK
	_MESSAGE("RegisterRawInputDevices %08X %08X", numDevices, structSize);
	for(UINT i = 0; i < numDevices; i++)
	{
		RAWINPUTDEVICE * dev = &devices[i];

		_MESSAGE("%04X %04X %08X %08X", dev->usUsagePage, dev->usUsage, dev->dwFlags, dev->hwndTarget);
	}
#endif

	BOOL result = Original_RegisterRawInputDevices(devices, numDevices, structSize);

	return result;
}

UINT WINAPI Hook_GetRawInputData(HRAWINPUT rawinput, UINT cmd, void * data, UINT * dataSize, UINT headerSize)
{
	UINT result = Original_GetRawInputData(rawinput, cmd, data, dataSize, headerSize);

#if LOG_INPUT_HOOK

	_MESSAGE("GetRawInputData %08X %08X %08X", cmd, *dataSize, headerSize);
#endif

	if(data)
	{
		RAWINPUT * input = (RAWINPUT *)data;

#if LOG_INPUT_HOOK
		_MESSAGE("hdr: %08X %08X %08X %08X", input->header.dwType, input->header.dwSize, input->header.hDevice, input->header.wParam);
#endif

		if(cmd == RID_INPUT)
		{
			switch(input->header.dwType)
			{
				case RIM_TYPEHID:
				{
#if LOG_INPUT_HOOK
					_MESSAGE("hid: %08X %08X", input->data.hid.dwSizeHid, input->data.hid.dwCount);
#endif
				}
				break;

				case RIM_TYPEKEYBOARD:
				{
					RAWKEYBOARD * kbd = &input->data.keyboard;
					if (kbd->VKey == g_espHotKey && kbd->Flags == RI_KEY_BREAK)
						HotKeyProc();
					else if (g_EnableClipboard) 
					{
						if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
						{
							if (kbd->VKey == 0x43)
								// Ctrl + C
								GetClipboard();
						}
					}
#if LOG_INPUT_HOOK
					_MESSAGE("kbd: %04X Flags:%04X %04X VKey:%04X %08X %08X",
						kbd->MakeCode, kbd->Flags, kbd->Reserved, kbd->VKey, kbd->Message, kbd->ExtraInformation);
#endif
				}
				break;

				case RIM_TYPEMOUSE:
				{
					RAWMOUSE * mse = &input->data.mouse;

#if LOG_INPUT_HOOK
					_MESSAGE("mse: %04X %08X %08X %d %d %08X",
						mse->usFlags, mse->ulButtons, mse->ulRawButtons, mse->lLastX, mse->lLastY, mse->ulExtraInformation);
#endif
				}
				break;
			}
		}
	}


	return result;
}

void Hooks_Input_Init()
{
	//
}

void Hooks_Input_Commit(const USHORT HotKey)
{
	//if (HotKey > 0){

		void ** iat = (void **)GetIATAddr(GetModuleHandle(NULL), "user32.dll", "RegisterRawInputDevices");
		Original_RegisterRawInputDevices = (_RegisterRawInputDevices)*iat;
		SafeWrite64((uintptr_t)iat, (UInt64)Hook_RegisterRawInputDevices);

		iat = (void **)GetIATAddr(GetModuleHandle(NULL), "user32.dll", "GetRawInputData");
		Original_GetRawInputData = (_GetRawInputData)*iat;
		SafeWrite64((uintptr_t)iat, (UInt64)Hook_GetRawInputData);

		g_espHotKey = HotKey;
	//}
}
