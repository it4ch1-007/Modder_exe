#include <iostream>
#include <stdlib.h>
#include "DLLInjector.h"
#include "calllogger.h"
#include "patternscan.h"
#include "debug.h"


void print_help() {
	std::cout << "Usage: modder.exe <option> <files> \n\n\n \
-dll_inject: calls dll injector in remote process  \
 \t Usage: modder.exe -dll_inject dllPath processWindowName\
\n\n -pattern: search for any instruction inside the memory\
\t Usage: modder.exe -pattern <assembly instruction>\
\n\n -calllogger: search and log the call instructions\
\t Usage: modder.exe -calllogger <process_window_name>";
}
int main(int argc,char* argv[]) {

	switch (argc) {
	case 1:
			print_help();
			exit(0);
	case 5:
	
		
		break;

		//std::cout << argc << std::endl;
	case 4:
		if (std::string(argv[1]) == "-pattern") {
			std::cout << "Pattern Scanner Running ..." << std::endl;
			int length = atoi(argv[3]);
			std::cout << "\nEnter the Instruction {In hex format separated with spaces: }";
			std::string processWindowName = static_cast<std::string>(argv[2]);
			if (!pattern_scan(processWindowName, length)) {
				std::cerr << "Could not find this pattern!!" << std::endl;
			}
		}
		if (std::string(argv[1]) == "-dll_inject") {
			std::cout << "Dll-injector Running ..." << std::endl;
			std::string dllName = static_cast<std::string>(argv[2]);
			std::string processWindowName = static_cast<std::string>(argv[3]);
			if (!dll_injector_using_console(dllName, processWindowName)) {
				std::cout << "Dll injection Failed" << std::endl;
			}
		}
		
		break;

	case 3:
		
		if (std::string(argv[1]) == "-calllogger") {
			std::cout << "Call Logger Running ...." << std::endl;
			std::string game_window_name = static_cast<std::string>(argv[2]);
			call_log(game_window_name);
		}
		if (std::string(argv[1]) == "-debug") {
			std::cout << "Debugger Running ...." << std::endl;
			std::string processName = static_cast<std::string>(argv[2]);
			debug();
		}
		if (std::string(argv[1]) == "-dis") {
			std::cout << "Disassembly:" << std::endl;
			std::string processName = static_cast<std::string>(argv[2]);
			std::string address = static_cast<std::string>(argv[3]);
			disassemble(processName, address);
		}
		break;
	case 2:
		if (std::string(argv[1]) == "-help") {
			print_help();
		}
		break;
	}
	



}