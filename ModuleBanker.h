#ifndef INCLUDE_MODULE_BANKER
#define INCLUDE_MODULE_BANKER

#include <string.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <utility>
#include "DataStructures.h"
#include "SystemStatus.h"
#include "utils.h"
using namespace std;

class ModuleBanker{
	public:
		static bool solveRequest( const Item &item, bool enabled_log = true );

		/* 
		 * When a process dies, it releases its resource,
		 * the output looks messy if 
		 * releaseResource() prints all the time.
		 *
		 * One may call releaseResource( item, false )
		 * to avoid printing the whole table.
		 *
		 * */
		static void releaseResource( const Item &item, bool enabled_log = true );
		static void showdata();
		static int  findIndexById(int id);
	private:
		static inline void log(string status, string msg){
			utils::log(status,"Banker" ,msg);
		}
		static inline void print(const string stream){
			cout << stream << flush;
		}
		static int  chkerr(int s);

};

#endif
