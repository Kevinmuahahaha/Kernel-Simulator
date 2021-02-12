#include "FileSystem.h"
#include "Module_DiskAccess.h"
#include "SystemStatus.h"
#include "utils.h"
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <utility>
#include <string>

std::vector< std::pair<std::string, int> > FileSystem::bindTable;

// map<int,int> Module_DiskAccess::fat

int FileSystem::findNextBlockByKey( int starting_index ){
	// -1 indicates end of the file
	if( Module_DiskAccess::fat.count(starting_index) > 0 ){
		return Module_DiskAccess::fat.find(starting_index)->second;
	}
	else{
		return -1;
	}
}

std::vector<int> FileSystem::findTracksByBlockStartingIndex( int starting_index ){
	std::vector<int> ret;
	/*
	 * - Check the block sequence.
	 * - Calculate their tracks.
	 * - Return.
	 *
	 * */
	int next_block = starting_index;
	int before_next_block = -1;
	int _tmp_track = -1;
	while( next_block != -1 ){
		_tmp_track = next_block / ( Module_DiskAccess::TracksPerCylinder * Module_DiskAccess::SectorsPerTrack  );

		// utils::log("debug","FAT",std::to_string( next_block ) + " : " +std::to_string( _tmp_track ));
		
		/* No need for hand-made selection.
		 * Because there's a "vector --> set --> vector" conversion for cleaning the data.
		if( ret.size() > 0 ){
			if( _tmp_track != ret[ret.size()-1] ){
				ret.push_back( _tmp_track );
			}
		}
		else{
			ret.push_back( _tmp_track );
		}
		*/

		if( _tmp_track >= 0 ){
			ret.push_back( _tmp_track );
		}

		// next_block becomes it's own "next block"

		before_next_block = next_block;

		next_block = FileSystem::findNextBlockByKey( next_block );

		/* 
		 * note that the following (commented) piece of code 
		 * is a false implementation,
		 * it only works when the system uses "FCFS" disk scheduling.
		 * Update "last position" within each scheduling algorithm instead.
		 *
		if( next_block == -1 ){
			// disk head points to the last access track.
			Module_DiskAccess::updateDiskHeadPosition = next_block;
		}
		*/
	}

	std::set<int> s( ret.begin(), ret.end() );

	ret.assign( s.begin(), s.end() );

	return ret;
}

std::string* FileSystem::readFile(std::string filename){
	std::string* ret = nullptr;
	bool file_found = false;
	int file_found_index = -1;

	for(int i=0; i<FileSystem::bindTable.size(); i++){
		/*Try to find filename */
		if( FileSystem::bindTable[i].first == filename ){
			file_found = true;
			file_found_index = i;
			break;
		}
	}

	if( file_found ){
		int target_block = FileSystem::bindTable[file_found_index].second;
		ret = new std::string();
		*ret = Module_DiskAccess::readData( target_block );

		if( SystemStatus::displayFATTable ){
			for( std::map<int,int>::const_iterator it = Module_DiskAccess::fat.begin(); 
					it != Module_DiskAccess::fat.end(); ++it ) {
				int key = it->first;
				int value = it->second;
				utils::log("debug","FAT",std::to_string(key) + " : " + std::to_string(value));
			}
		}
	}

	if( file_found && SystemStatus::displayTrack ){
		vector<int> tracks;
		tracks = FileSystem::findTracksByBlockStartingIndex(file_found);

		//debug
		
		/*
		std::string p;
		for(int x=0; x<tracks.size(); x++){
			p = p + std::to_string(tracks[x]) + ",";
		}
		utils::log("test","track",p);
		*/
		

		char choice = 'A';
		if( SystemStatus::trackAlgorithm == "SSTF" ){
			choice = 'A';
		}
		else if( SystemStatus::trackAlgorithm == "SCAN" ){
			choice = 'B';
		}
		else if( SystemStatus::trackAlgorithm == "FCFS" ){
			choice = 'C';
		}
		Module_DiskAccess::printTracks( tracks, Module_DiskAccess::diskHeadPosition, choice );
		Module_DiskAccess::diskHeadPosition = Module_DiskAccess::updateDiskHeadPosition;
	}

	return ret;
}

bool FileSystem::writeFile(std::string filename, std::string content){
	/*
	 * if sucess, call bindFileNameAndBlock()
	 * Overwrites original data if file already exists
	 * if subsequent write fails, this file will be empty
	 */
	for(int i=0; i<FileSystem::bindTable.size(); i++){
		if( FileSystem::bindTable[i].first == filename ){
			// found filename, delete it.
			FileSystem::deleteFile( filename );
		}
	}

	int first_block = Module_DiskAccess::store( content, SystemStatus::displayDisk );
	if( first_block == -1 ){
		return false;
	}
	else{
		FileSystem::bindFileNameAndBlock( filename, first_block );
		if( SystemStatus::displayFATTable ){
			for( int i=0; i<FileSystem::bindTable.size(); i++ ){
				utils::log("FAT","F.S",FileSystem::bindTable[i].first + " : " + std::to_string(FileSystem::bindTable[i].second));
			}
		}
		return true;
	}
	return true;
}

bool FileSystem::deleteFile(std::string filename){
	bool file_found = false;
	for( int i=0; i<FileSystem::bindTable.size(); i++ ){
		if( FileSystem::bindTable[i].first == filename ){
			file_found = true;
			// delete the file first.
			int target_block = FileSystem::bindTable[i].second;
			FileSystem::bindTable.erase( FileSystem::bindTable.begin() + i );
			Module_DiskAccess::recycleFromBlock( target_block, SystemStatus::displayDisk );
			break;
		}
	}
	return file_found;
}

void FileSystem::bindFileNameAndBlock(std::string filename, int block){
	std::pair<std::string, int> _tmp_push;
	_tmp_push.first  = filename;
	_tmp_push.second = block;

	FileSystem::bindTable.push_back(_tmp_push);
}
