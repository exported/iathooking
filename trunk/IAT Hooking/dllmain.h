#include <windows.h>


typedef DWORD (WINAPI *_GetLastError)();
_GetLastError OrigGetLastError;

IMAGE_IMPORT_DESCRIPTOR* GetImportTable(HMODULE module);
DWORD PlaceIATHook(char* functionName, DWORD newFunctionAddress);

DWORD WINAPI MyGetLastError();

DWORD WINAPI Initialization(LPVOID lpParam);