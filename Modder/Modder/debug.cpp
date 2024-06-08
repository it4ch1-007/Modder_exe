#include <iostream>
#include <Windows.h>
#include <string>
#include <tlhelp32.h>


int debug() {
	//We will first try setting a breakpoint at the instruction where the ammo is being subtracted or simply we will break when the player shoots.
	//the breakpoint should be set and the continue should work and the registers must be shown.

	HANDLE snapshot = NULL;
	HANDLE thread_handle = NULL;
	HANDLE process_handle = NULL;

	PROCESSENTRY32 pe32 = { 0 };
	DWORD pid=0;
	DWORD continueStatus = DBG_CONTINUE; //this tells us whether to continue or not.
	SIZE_T bytes_written = 0;

	BYTE break_instrn = 0xcc;
	BYTE original_instrn;

	DEBUG_EVENT debugEvent = {0};

	CONTEXT context = {0};

	bool flag	= false;

	pe32.dwSize = sizeof(PROCESSENTRY32);
	int cnt = 0;
	snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	Process32First(snapshot, &pe32);
	HWND game_wnd = FindWindowA(0, "The Battle for Wesnoth - 1.14.9");
	GetWindowThreadProcessId(game_wnd, &pid);
	do {
		if (pe32.th32ProcessID == pid) {
			pid = pe32.th32ProcessID;
			std::cout << "Found" << std::endl;
			process_handle = OpenProcess(PROCESS_ALL_ACCESS, true, pe32.th32ProcessID);
			WriteProcessMemory(process_handle, (void*)0x007ccd9e, &break_instrn, 1, &bytes_written);
			WriteProcessMemory(process_handle, (void*)0x0593E7E4, &break_instrn, 1, &bytes_written);
			//This writes the break instrn at this address and thus it will break now when the gold decreses.
		}
		//std::cout << "Finding" << std::endl;
	} while (Process32Next(snapshot, &pe32));

	DebugActiveProcess(pid); //this attaches the debugger to the main program
	std::cout << "Debugger attached" << std::endl;
	while (true) {
		continueStatus = DBG_CONTINUE;
		if (!WaitForDebugEvent(&debugEvent, INFINITE)) //This makes the debugger wait for the program to give a debug event.
			return 0;

		switch (debugEvent.dwDebugEventCode) {
		case EXCEPTION_DEBUG_EVENT:
			switch(debugEvent.u.Exception.ExceptionRecord.ExceptionCode){
			case EXCEPTION_BREAKPOINT:
				std::cout << "Breakpoint Hit" << std::endl; //This will be first printed when the debugger is attached as it is a debug event inside the program.
				//then it will print when the address of the instruction is reached.
				continueStatus = DBG_CONTINUE;
				break;
			default:
				continueStatus = DBG_EXCEPTION_NOT_HANDLED;
				break;

			}
			break;
		default:
			continueStatus = DBG_EXCEPTION_NOT_HANDLED;
			break;
		}
		ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, continueStatus);
		
	}
	CloseHandle(process_handle);
	return 1;

}