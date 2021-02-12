#ifndef INCLUDE_HEADER_SYSTEM_STATUS
#define INCLUDE_HEADER_SYSTEM_STATUS
#include <string>
#include <vector>
#include <utility>
#include <atomic>
#include <mutex>
class SystemStatus{
	public:
		static bool displayCode;
		static bool displayDisk;
		static bool displayFileSystemCue;
		static bool displayFATTable;
		static bool displayTrack;
		
		static std::string trackAlgorithm;


		static int sleepMiliSeconds;

		static int maxTimeChunck; // miliseconds
		static bool isSystemRunning;
		static bool isRevivingBlockedProcess;
		static const std::vector<std::string> supportedInstructions;
		static long int PIDCount;
		static int supportedResourceCount; /* A,B,C,D,E = 5*/

		static bool isRunningEmpty;
		static bool isNoJobLeft;
		static bool isNoReadyLeft;
		static std::atomic_int appendingReadyCount; 
		/*
		 * whenever there's a thread appending readyQueue( block -> ready ) 
		 * 	appendingReadyCount ++
		 *
		 * */

		static std::mutex * tableOperation;

		static std::vector< std::pair<int,std::vector<int>> > Max;
		static std::vector< std::pair<int,std::vector<int>> > allocation;
		static std::vector< std::pair<int,std::vector<int>> > need;
		static std::vector<int> available;
		static std::vector<int> max_available; // shouldn't be changed

		static bool isTimeUp;

	private:
		static std::vector<int> makeVectorInt();
		static std::vector<int> makeAvailableVector();
		static std::vector<std::pair<int,std::vector<int>>> makeVectorIntInt();
};
#endif
