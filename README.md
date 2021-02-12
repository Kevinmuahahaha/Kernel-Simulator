## Description:
This project simulates the functionalities of a simplified kernel.

	z.B. 
	- handling read/write, aka. talking with the harddrive.
	- handling resource requests
	- task scheduling
	- read codes(sudo machine instructions)
	- usw.
	
Tasks are pre-configed in a config file. These tasks will then go through the entire process of scheduling till they're finished.
This project provides it's own code-parser. See demo to get a sense of how these sudo-machine-codes are written.
Scheduling algorithm: Time-shared, dynamic priority.

## How it looks:
Running the project:

![running](https://github.com/Kevinmuahahaha/Kernel-Simulator/blob/main/images/demo1.png)
	
This project can parse sudo-machine-codes:

![codes](https://github.com/Kevinmuahahaha/Kernel-Simulator/blob/main/images/demo_codes.png)


## Cross-platform building:
	edit utils.cpp
	to build on linux, make sure the first line contains:
	#define LINUX

	to build on windows, make sure the first line contains:
	#define WINDOWS


## Using this project's compiler:
	build and run: 'samples/get version.cpp'
	
		199711 for C++98
		201103 for C++11
		201402 for C++14   <--- used by this project
		201703 for C++17

	compiler used by this project:
		g++ (GCC) 10.1.0
		Thread model: posix
		Target: x86_64-pc-linux-gnu


## Compiling on Linux (requires pthread)
	g++ -pthread  main.cpp utils.cpp MemorySpace.cpp DataStructures.cpp  SystemStatus.cpp HighLevelScheduler.cpp ProcessScheduler.cpp ModuleBanker.cpp CodeExecutor.cpp FileSystem.cpp Module_DiskAccess.cpp

## Compiling on Windows
	Your compiler must support pthread
	e.g Cygwin
	see https://stackoverflow.com/questions/2150938/can-i-get-unixs-pthread-h-to-compile-in-windows


