#include <iostream>
#include <Windows.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <tlhelp32.h>
#include <stdio.h>


std::vector<int> convertByteStringToHexVector(int length) {
    std::string input;
    std::getline(std::cin, input);
    std::istringstream iss(input);
    std::vector<int> hexArray;
    std::string hexValue;
    while (iss >> hexValue) {
        int number;
        std::stringstream converter;
        converter << std::hex << hexValue;
        converter >> number;
        hexArray.push_back(number);
    }
    return hexArray;
}
//(const wchar_t*)processName.c_str()
int pattern_scan(std::string processName, int length) {
    std::vector<int> instrn_bytes = convertByteStringToHexVector(length);
    HANDLE snapshot = 0;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    Process32First(snapshot, &pe32);
    do {
        if (wcscmp(pe32.szExeFile, L"wesnoth.exe") == 0) {
            HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, true,
                pe32.th32ProcessID);


            //Handling each module inside the process.
            HANDLE module_snapshot = 0;
            MODULEENTRY32 me32;
            me32.dwSize = sizeof(MODULEENTRY32);
            module_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,
                pe32.th32ProcessID);
            Module32First(module_snapshot, &me32);
            do {
                if (wcscmp(me32.szModule, L"wesnoth.exe") == 0) {

                    break;
                }
            } while (Module32Next(module_snapshot, &me32));
            int* buffer = (int*)calloc(1, me32.modBaseSize);
            SIZE_T bytes_read = 0;
            ReadProcessMemory(process, (void*)me32.modBaseAddr, buffer, me32.modBaseSize,
                &bytes_read);
           /* for (unsigned int i = 0; i < me32.modBaseSize - sizeof(bytes_read); i++) {
                for (int j = 0; j < length; j++) {
                    if (instrn_bytes[j] != buffer[i + j]) {
                        break;
                    }
                    if (j + 1 == sizeof(instrn_bytes)) {
                        printf("%x\n", i + (DWORD)me32.modBaseAddr);
                    }
                }
            }*/
            for (int i = 0; i < sizeof(buffer)/sizeof(buffer[0]); i++) {
                std::cout << buffer[i] << std::endl;
            }
            CloseHandle(process);
                free(buffer);
                break;
            }
        } while (Process32Next(snapshot, &pe32));

        return 1;
    
}