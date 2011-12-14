#include "dllmain.h"

IMAGE_IMPORT_DESCRIPTOR* GetImportTable(HMODULE module)
{	
	IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)module;

	if (dosHeader->e_magic != 0x5A4D)
		return NULL;

	IMAGE_OPTIONAL_HEADER* optionalHeader = (IMAGE_OPTIONAL_HEADER*)((BYTE*)module + dosHeader->e_lfanew + 24);

	if (optionalHeader->Magic != 0x10B)
		return NULL;

	if (optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0 ||
		optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress == 0)
		return NULL;

	return (IMAGE_IMPORT_DESCRIPTOR*)((BYTE*)module + optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
}

DWORD PlaceIATHook(char* functionName, DWORD newFunctionAddress)
{
	HMODULE module = GetModuleHandle(0);
	IMAGE_IMPORT_DESCRIPTOR* importDescriptor = GetImportTable(module);

	if (!importDescriptor)
		return 0;

	while (importDescriptor->FirstThunk)
	{
		int n = 0;
		IMAGE_THUNK_DATA* thunkData = (IMAGE_THUNK_DATA*)((BYTE*)module + importDescriptor->OriginalFirstThunk);
		while (thunkData->u1.Function)
		{
			char* importFunctionName = (char*)((BYTE*)module + (DWORD)thunkData->u1.AddressOfData + 2);
			
			if(strcmp(importFunctionName, functionName) == 0)
			{
				PDWORD lpAddr = (DWORD*)((BYTE*)module + importDescriptor->FirstThunk) + n;

				DWORD oldProtection;
				VirtualProtect(lpAddr, sizeof(DWORD), PAGE_READWRITE, &oldProtection);
				
				DWORD original = *lpAddr;
				*(DWORD*)lpAddr = newFunctionAddress;

				VirtualProtect(lpAddr, sizeof(DWORD), oldProtection, &oldProtection);
				return original;
			}

			n++;
			thunkData++;
		}
		importDescriptor++;
	}

	return 0;
}

DWORD WINAPI MyGetLastError()
{
	SetLastError(987654321);
	return OrigGetLastError();
}

DWORD WINAPI Initialization(LPVOID lpParam)
{
	OrigGetLastError = (_GetLastError)PlaceIATHook("GetLastError", (DWORD)MyGetLastError);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if(dwReason == DLL_PROCESS_ATTACH)
	{
		HANDLE RunThread = CreateThread(NULL, 0, Initialization, NULL, 0, NULL);
		CloseHandle(RunThread);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		//clean up here
	}
	return TRUE;
}