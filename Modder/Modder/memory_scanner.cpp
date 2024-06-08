#include <iostream>
#include <Windows.h>
#include <string>
#include <fstream>

int filter(HANDLE pHandle) {
	std::fstream file("filters.txt",std::ios::in | std::ios::out | std::ios::app);
	//opens the filters.txt in both write and read mode
	unsigned char* buffer = (unsigned char*)calloc(1, 2048);
	SIZE_T bytesRead;
	
	//First write everything inside the file names filters.txt
	for (DWORD i = 0x0; i < 0x7fffffff; i+=2048) {
		ReadProcessMemory(pHandle, (void*)i, buffer, 2048, &bytesRead);
		file.write((const char*)buffer, sizeof(buffer));
	}
	file.close();

	//Then we have to see whether decrease or increase is happening
	while (true) {
		std::string sub_command;
		std::cout << ":>> ";
		std::cin >> sub_command;

		//Open a new file read the process memory into that file and then compare the values of the two files to get the chages.
		if (sub_command == "dec") {
			//decrease by the value given
			DWORD value;
			std::cout << "Value: ";
			std::cin >> value;
			unsigned char* buffer = (unsigned char*)calloc(1,2048);
			SIZE_T bytesRead = 0;
			//write a new file with the next content that is obtained after the changes
			std::fstream file2("filters_changed.txt", std::ios::in | std::ios::out | std::ios::app);
			for (DWORD i = 0; i < 0x7fffffff; i += 2048) {
				ReadProcessMemory(pHandle, (void*)i, buffer, 2048, &bytesRead);
				file2.write((const char*)buffer, sizeof(buffer));
			}
			file2.close();
			//we open the different handles of the same files again
			//Then we compare the values in the same way they were compared as equal dwords

		}
		else if (sub_command == "inc") {
			//increase by the value given

		}
		else if (sub_command == "change") {
			//change the value by the given value will take absolute mod
		}
		else if (sub_command == "unchanged") {
			//this is the value that will be unchanged thus this will comapre the buffer with the previous values
		}	
		
	}

}
int search(HANDLE pHandle,DWORD value) {
	//for searching we will need to write to the file only once when we actually found the value equal to the desired value.
	FILE* fptr = NULL;
	fopen_s(&fptr, "search_result.txt", "w");
	SIZE_T bytes_read = 0;
	unsigned char* buffer = (unsigned char*)calloc(1, 2048);
	//A block of 2048 byts each will be read.
	for (DWORD i = 0; i < 0x7fffffff; i += 2048) {
		//the loop will read all the memory in the user's 32 bit process that can be stored.
		ReadProcessMemory(pHandle, (void*)i, buffer, 2048, &bytes_read);

		//now we will loop into the read  2048 bytes long buffer and check if we have our value in it or not
		for (int j = 0; j < 2044; j += 4) {
			DWORD val = 0;
			memcpy(&val, &buffer[j], 4);//This is also making blocks of 4 bytes each that are being copied into the destination from the source address.
			if (value == val) {
				fprintf(fptr, "%x\n", i + j);
				//Writes the address of the value in the hex format
			}
		}
	}
	return 1;
}
int mem_scan(std::string processWindowName) {
	//we have to get all the values of the game in a buffer
	//if the user types dec:30
	//Then we will see which addresses have decreased its values by 30 and not display them but only display their count
	//We have to take inputs after each iteration
	//We will write to a file instead of a buffer
	//If there are no entries left then give out error if trying to filter further.
	//In many games the values can be of different types like bytes or DWORDs thus we will scan for DWORDs for now.

	while (true) {
		HWND game_wnd = FindWindowA(0,processWindowName.c_str());
		DWORD pid = 0;
		GetWindowThreadProcessId(game_wnd, &pid);
		HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, NULL, pid);
		std::cout << ":>> ";
		std::string command;
		std::cin >> command;
		if (command == "search") {
			DWORD value;
			std::cin >> value;
			search(pHandle,value);
		}
		else if (command == "filter") {
			DWORD value;
			std::string sub_command;
			std::cin >> value;
			filter(pHandle);
		}
	}
}