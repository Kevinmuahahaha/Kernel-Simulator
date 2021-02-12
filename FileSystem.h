#ifndef INCLUDE_FILE_SYSTEM
#define INCLUDE_FILE_SYSTEM

#include <vector>
#include <utility>
#include <string>

class FileSystem{
	public:
		/*
		 * nullptr when failed to read.
		 * */
		static std::string* readFile(std::string filename);
		static bool writeFile(std::string filename, std::string content);
		static bool deleteFile(std::string filename);
	private:
		static std::vector<int> findTracksByBlockStartingIndex(int starting_index);
		static int findNextBlockByKey( int starting_index );

		static std::vector< std::pair<std::string, int> > bindTable;
		/* 
		 * called when writeFile = ture 
		 * -1 --> failed to store.
		 *
		 * */
		static void bindFileNameAndBlock(std::string filename, int block);
};

#endif
