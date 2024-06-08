#include <iostream>
#include <string>
#include <Windows.h>
#include <psapi.h>


//PROCESSENTRY pe32;

int call_log(std::string processName) {

	DWORD pid;
	//Finding the main module of the game
	HWND game_wnd = FindWindowA(0, processName.c_str());


	//USE THIS ONLY WHEN THE PROCESS EXECUTABLE NAME IS GIVEN 
	//gets info about all processes running on the system.
	//HANDLE hProcessSnap =  CreateToolhelp32Snapshot(
	//	TH32CS_SNAPPROCESS,
	//	0
	//);

	//pe32.dwsize = sizeof(PROCESSENTRY32);
	////This sets the size of the structure we are going to use to store the info of the pe

	//Process32First(hProcessSnap, &pe32);

	//do {
	//	if(wcscmp(pe32.))
	//} while (Process32Next(hProcessSnap, &pe32));

	//HANDLE pHandle = OpenProcess()

	GetWindowThreadProcessId(game_wnd, &pid); //gets us the pid of the game

	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);


	char main_module[MAX_PATH];

	GetProcessImageFileNameA(
		pHandle,
		main_module,
		sizeof(main_module)
	);

	DWORD bytes_read;
	HMODULE modules[128] = {0};
	
	EnumProcessModules(
		pHandle,
		modules,
		sizeof(modules),
		&bytes_read
	);

	MODULEINFO module_info;
	SIZE_T num_bytes;
	GetModuleInformation(
		pHandle,
		modules[0],
		&module_info,
		sizeof(module_info)
	);

	unsigned char instructions[4096];
	DWORD block_size = 4096;
	BYTE call_instrn = 0xe8;
	//Now reading the memory bytes into a buffer
	//We will have all the bytes of the memory into the instructions array where we have all the characters.
	for (DWORD i = 0; i < module_info.SizeOfImage; i += 4096) {
		ReadProcessMemory(pHandle, LPVOID((DWORD)module_info.lpBaseOfDll+i), &instructions, block_size, &num_bytes);
		for (DWORD j = 0; j < bytes_read; j++) {
			if (instructions[i] == call_instrn) {
				std::cout << "Found call" << std::endl;
			}
		}
	}
	return 1;
	//We created blocks of 4096 bytes to make sure that in any block if the access for bytes reading is not given to the program then we will not nullify all the bytes we can actually read.
	//This is because ReadProcessMemory stores null bytes if for any section it is not able to read the bytes from the memory of the game duye to access constraint.


}