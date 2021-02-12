#include "SystemStatus.h"
#include <vector>
#include <string>
#include <utility>
#include <atomic>
#include <mutex>

bool SystemStatus::displayCode = true;
bool SystemStatus::displayDisk = false;
bool SystemStatus::displayFileSystemCue = true;
bool SystemStatus::displayFATTable = false;
bool SystemStatus::displayTrack = false;

std::string SystemStatus::trackAlgorithm = "SSTF";

int SystemStatus::sleepMiliSeconds = 200;
int SystemStatus::maxTimeChunck = 1000;

bool SystemStatus::isSystemRunning = true;
const std::vector<std::string> SystemStatus::supportedInstructions(
		{"MOV", "REQ", "DEF", "APP", 
		"CMP", "JMP", "JEQ", "JNE", 
		"DIE", "DEL", "NOP", "SNP"}
		);
long int SystemStatus::PIDCount = 1;
int SystemStatus::supportedResourceCount = 0;

bool SystemStatus::isRunningEmpty = true;
bool SystemStatus::isNoJobLeft = false;
bool SystemStatus::isNoReadyLeft = false;
bool SystemStatus::isRevivingBlockedProcess = false;



std::atomic_int SystemStatus::appendingReadyCount = {0};


std::vector<int> SystemStatus::makeVectorInt(){
	std::vector<int> ret;
	return ret;
}

std::vector<int> SystemStatus::makeAvailableVector(){
	std::vector<int> ret;
	/*
	for( int i=0; i<SystemStatus::supportedResourceCount; i++ ){
		ret.push_back(160);
		// auto fill for testing purpose
	}
	*/
	return ret;
}

std::vector<std::pair<int, std::vector<int>>> SystemStatus::makeVectorIntInt(){
	std::vector<std::pair<int,std::vector<int>>> ret;
	return ret;
}

std::mutex * SystemStatus::tableOperation = new std::mutex();

std::vector< std::pair<int,std::vector<int>> > SystemStatus::Max 	= makeVectorIntInt();
std::vector< std::pair<int,std::vector<int>> > SystemStatus::allocation = makeVectorIntInt();
std::vector< std::pair<int,std::vector<int>> > SystemStatus::need	= makeVectorIntInt();
std::vector<int> 			SystemStatus::available 	= makeAvailableVector();
std::vector<int> 			SystemStatus::max_available 	= makeAvailableVector();

