#include <iostream>
#include <string>
#include <Windows.h>


int dll_injector_using_console(std::string dllName,std::string processName) {
	//first find the process using the window name[FindWindow]
	//then get the pid of the process(GetWindowThreadProcessId)
	//Use OpenProcess to open the handle to the process.
	//VirtualAllocEx for creating a virtual memory into the remote process.
	//LoadLibrary to load the dll into the memory of the injector
	//CreateRemoteThread for create a thread in the game.
	//also we can use CreateTool32Snapshot function in order to get the pid of the process.
	//the dll will be injected then also we can add the feature of hotkeys inside this.

	//opening the window of the game
	HWND game_wnd = FindWindowA(0, processName.c_str());

	//getting the pid of the process
	DWORD pid;
	GetWindowThreadProcessId(game_wnd, &pid);

	//open the handle to the process
	HANDLE game_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);

	//loading the dll into the memory
	LoadLibraryA(dllName.c_str());

	//writing the dll file into the memory of the remote process and getting the path of the loaded library inside the game
	LPVOID dllPathAddress = VirtualAllocEx(
		game_handle,
		NULL,
		(dllName.size())*sizeof(wchar_t),
		MEM_COMMIT,
		PAGE_READWRITE
	);

	HMODULE hKernel32 = GetModuleHandleA("kernel32");
	FARPROC pdllRoutine = GetProcAddress(hKernel32, "LoadLibraryW"); //A pointer type given more memory than the simple pointer type.



	//This will give us te handle to the thread that executes our dll file
	HANDLE hRemoteThread = CreateRemoteThread(
		game_handle,
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)pdllRoutine,
		dllPathAddress, //the path of the dll inside the game's memory
		0,
		NULL
	);

	//Now the thread should work in sync with the game main thread.
	if (!hRemoteThread){
		std::cout << "Could not create the remote thread!!" << std::endl;
		return 0;
	}
	DWORD exitCode;
	GetExitCodeThread(hRemoteThread, &exitCode); //This will tell us did the thread exited successfully.
	if (!exitCode) {
		std::cerr << "The thread could not be executed" << std::endl;
	}
	VirtualFreeEx(game_handle, dllPathAddress,dllName.size()*sizeof(wchar_t), MEM_RELEASE); 
	//When using the virtualfreeex function for the base address of the injected memory function then use release instead of decommit.
	return 1;
}
