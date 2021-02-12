#ifndef INCLUDE_MODULE_DISK_ACCESS
#define INCLUDE_MODULE_DISK_ACCESS
#include <string> 
#include <stdio.h> 
#include <malloc.h> 
#include <stdlib.h>
#include <iostream>
#include <map>
#include "utils.h"

#define BITROW 50
#define BITCOLUMN 6

#define DATAONE 50
#define DATATWO 2
#define DATATHREE 3

using namespace std;

class Module_DiskAccess{
	public:

		static void printTracks( vector<int> list_of_tracks, int &start, char choice='A' );
		/* Rejects any list_of_tracks size < 1 */

		static const int TracksPerCylinder = DATATWO;
		static const int SectorsPerTrack = DATATHREE;

		static int diskHeadPosition; // which track
		static int updateDiskHeadPosition; // last accessed track


		Module_DiskAccess();
		static int  a[BITROW][BITCOLUMN];
		static char data[DATAONE][DATATWO][DATATHREE];
		static int * dataPosition;
		static int c[BITROW*BITCOLUMN][2];
		static int y;
		static map <int, int> fat;
		static map <int, int>::iterator fat_Iter;

		static int store(string str, bool enable_log = true);
		static void recycleFromBlock(int position, bool enable_log = true);

		static void recycleAll();
		static string readData(int position);

	private:
		static map<int,int> makeIntIntMap();
		static map<int,int>::iterator makeIntIntMapIter();

		static inline void log(string status, string msg){
			utils::log(status,"DskAcs" ,msg);
		}
		static inline void print(const string stream){
			cout << stream << flush;
		}

		static vector<int> display(vector<int> list_of_tracks);
		static void SSTF(vector<int> list_of_tracks_back, int * track_number, int size,int cp);
		static void SCAN(vector<int> list_of_tracks_back, int * track_number, int size,int cp);
		static void FCFS(vector<int> list_of_tracks_back, int * track_number, int size,int cp);

};

#endif

