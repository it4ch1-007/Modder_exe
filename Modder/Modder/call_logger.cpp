#include <iostream>
#include <string>
#include <Windows.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <sstream>
#define READ_PAGE_SIZE 4096


std::wstring charToWChar(const char* charStr) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, charStr, -1, NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, charStr, -1, &wstrTo[0], size_needed);
	return wstrTo;
}

int call_log(std::string processName,std::string address) {
	std::stringstream ss;
	ss << std::hex << address;
	DWORD address_num;
	ss >> address_num;
	std::cout << "Starting call logger..\n" << std::endl;
	HANDLE process_snapshot = NULL;
	HANDLE thread_handle = NULL;
	HANDLE process_handle = NULL;
	PROCESSENTRY32 pe32 = { 0 };
	DWORD pid;
	DWORD continueStatus = DBG_CONTINUE;
	SIZE_T bytes_written = 0;

	BYTE break_instrn = 0xcc;
	BYTE call_instrn = 0xe8;

	DEBUG_EVENT debugEvent = { 0 };
	CONTEXT context = { 0 };
	bool first_break_occured = false;
	//Coz I donot want to stop on the first break call or the first debug event which is actually the start of the program code
	HMODULE modules[128] = { 0 };
	MODULEINFO module_info = { 0 };

	SIZE_T bytes_read = 0;
	DWORD offset = 0;
	DWORD call_location = 0;
	SIZE_T call_locn_bytes_read = 0;
	DWORD last_call_locn = 0;

	unsigned char instrns[READ_PAGE_SIZE] = { 0 };
	//To store theinstrns where the call instruction is used.

	int bpts_set = 0;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	//This is the way to obtain the processid of the given processName from all the processes we have running on the system right now
	process_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	Process32First(process_snapshot, &pe32);

	do {
		if (wcscmp(pe32.szExeFile, charToWChar(processName.c_str()).c_str()) == 0) {
			pid = pe32.th32ProcessID;
			//Getting the handle of the process given
			process_handle = OpenProcess(PROCESS_ALL_ACCESS, true, pid);
		}
	} while (Process32Next(process_snapshot, &pe32));
		//To iterate through all the processes we just have to use Process32Next()

	DebugActiveProcess(pid);
	//Attach the debugger

	for (;;) {
		continueStatus = DBG_CONTINUE;

		if (!WaitForDebugEvent(&debugEvent, INFINITE))
			return 0;
		switch (debugEvent.dwDebugEventCode) {
		case EXCEPTION_DEBUG_EVENT:
			switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode) {
			case EXCEPTION_BREAKPOINT:
				//If the breakpoint is hit
				if (!first_break_occured) {
					thread_handle = OpenThread(THREAD_ALL_ACCESS, true, debugEvent.dwThreadId);
					//To get the handle of the thread which has the debug event
					std::cout << "Attaching the desird bpts.." << std::endl; \
					EnumProcessModules(process_handle, modules, sizeof(modules), (unsigned long*) &bytes_read);
					GetModuleInformation(process_handle, modules[0], &module_info, sizeof(module_info));
					//This gets me the informatin of a specific module of the process 
					first_break_occured = 1;
					for (DWORD i = 0; i < module_info.SizeOfImage; i += READ_PAGE_SIZE) {
						//Here we will try to read the process memory 
						//It will fail and nullify the output if the permissions for the module or the area of memory is not given for all access
						//So we will read the memory in parts of buffers
						ReadProcessMemory(process_handle, (LPVOID((DWORD)module_info.lpBaseOfDll + i)), &instrns, READ_PAGE_SIZE,&bytes_read);
						for (DWORD c = 0;c < bytes_read; c++) {
							//if 0xe8 is found then determine if it is a call instruction
							if (instrns[c] == call_instrn) {
								offset = (DWORD)module_info.lpBaseOfDll + i + c;
							}
							ReadProcessMemory(process_handle, (LPVOID)(offset + 1), &call_location, 4, &call_locn_bytes_read);
							call_location += offset + 5;
							//If the call instruction takes us out of the whole image or take us to the invalid memory then its just 0xe8 and not a call instruction
							if (call_location < (DWORD)module_info.lpBaseOfDll || call_location>((DWORD)module_info.lpBaseOfDll + module_info.SizeOfImage))
								continue;
							//Then if the call instruction is valid then we write the break instrn just after the call instruction
							if (offset != address_num && bpts_set < 2000) {//The giben bpts cannot be set to the beginning of the program and the last of the program
								WriteProcessMemory(process_handle, (void*)offset, &break_instrn, 1, &bytes_written);
								FlushInstructionCache(process_handle, (LPVOID)offset, 1);
								bpts_set++;
								//We are limiting the set_bpts to 2000
							}
						}
					}
					std::cout << "Breakpoints attached :::: " << std::endl;
				}
				else {
					//If we hit the bpt then we have to get the eip to the prev instrn and then open the handle to the current thread.
					//Then we are going to write back the call_instrn that we made the break instrn.
					thread_handle = OpenThread(THREAD_ALL_ACCESS, true, debugEvent.dwThreadId);
					if (thread_handle != NULL) {
						context.ContextFlags = CONTEXT_ALL;
						GetThreadContext(thread_handle, &context);
						//The struct context of the tlhelp32.h can be used to modify any instrn or component of the program code
						//or the assembly code.
						context.Rip--;
						context.EFlags != 0x100;
						SetThreadContext(thread_handle, &context);
						CloseHandle(thread_handle);
						WriteProcessMemory(process_handle, (void*)context.Rip, &call_instrn, 1, &bytes_written);
						FlushInstructionCache(process_handle, (LPVOID)context.Rip, 1);
						//We have to flush the cache for the instrn as now the instrn itself changes
						last_call_locn = context.Rip;
					}
				}
				first_break_occured = true;
				continueStatus = DBG_CONTINUE;
				break;
			case EXCEPTION_SINGLE_STEP:
				//When the user is trying to do a single step using a debugger or the assembly instrns
				thread_handle = OpenThread(THREAD_ALL_ACCESS, true, debugEvent.dwThreadId);
				if (thread_handle != NULL) {
					context.ContextFlags = CONTEXT_ALL;
					GetThreadContext(thread_handle, &context);
					CloseHandle(thread_handle);
					WriteProcessMemory(process_handle, (void*)last_call_locn, &break_instrn, 1, &bytes_written);
					FlushInstructionCache(process_handle, (LPVOID)last_call_locn, 1);
					last_call_locn= 0;
				}
				continueStatus = DBG_CONTINUE;
				break;
			default:
				//If an unknown exception occurs
				continueStatus = DBG_EXCEPTION_NOT_HANDLED;
				break;
			}
			break;
		default:
			continueStatus = DBG_EXCEPTION_NOT_HANDLED;
			break;
		}
		ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, continueStatus);
		//This determines if a specific thread of a specific process should continue or not in the process of debugging.
	}
	CloseHandle(process_handle);
	return 0;
}
