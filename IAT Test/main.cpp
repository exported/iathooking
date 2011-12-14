#include <tchar.h>
#include <windows.h>
#include <iostream>

using namespace std;


int _tmain(int argc, _TCHAR* argv[])
{
	HMODULE lib = LoadLibrary(L".\\Hook.dll");

	if (!lib)
	{
		printf("Failed to load Hook.dll. Error code %u.\n", GetLastError());
	}
	else
	{
		printf("Setting current ErrorCode to 0 ...\n");
		SetLastError(0);
		printf("Current ErrorCode: %u\n", GetLastError());
	}

	system("pause");
	return 0;
}