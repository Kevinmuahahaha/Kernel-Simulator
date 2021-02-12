#include "Module_DiskAccess.h"
#include <string> 
#include <stdio.h> 
#include <sstream>
#include <malloc.h> 
#include <stdlib.h>
#include <iostream>
#include <map>


int  Module_DiskAccess::a[BITROW][BITCOLUMN];
char Module_DiskAccess::data[DATAONE][DATATWO][DATATHREE];
int  Module_DiskAccess::c[BITROW*BITCOLUMN][2];

int Module_DiskAccess::diskHeadPosition = 0;
int Module_DiskAccess::updateDiskHeadPosition = 0;


//int * Module_DiskAccess::dataPosition;
int Module_DiskAccess::y = -1;
map<int,int> Module_DiskAccess::Module_DiskAccess::fat = Module_DiskAccess::makeIntIntMap();
map<int,int>::iterator Module_DiskAccess::Module_DiskAccess::Module_DiskAccess::fat_Iter = Module_DiskAccess::makeIntIntMapIter();

Module_DiskAccess::Module_DiskAccess(){

	for(int index_row=0; index_row<BITROW; index_row++){
		for(int index_col=0; index_col<BITCOLUMN; index_col++){
			Module_DiskAccess::a[index_row][index_col] = 0;
		}
	}
	
	for(int index_one=0; index_one<DATAONE; index_one++) {
		for(int index_two=0; index_two<DATATWO; index_two++) {
			for(int index_three=0; index_three<DATATHREE; index_three++) {
				Module_DiskAccess::data[index_one][index_two][index_three] = 0;
			}
		}
	}


	int block_count = 0;
	for (int i = 0; i < BITROW; i++){
		for (int j = 0; j < BITCOLUMN; j++){
			if (Module_DiskAccess::a[i][j])
				continue;
			else{
				Module_DiskAccess::c[block_count][0] = block_count + 1;
				Module_DiskAccess::c[block_count][1] = i * BITCOLUMN + j;	// 得到相对块号
				block_count++;
			}
		}
	}
	Module_DiskAccess::y = block_count;

}

map<int,int> Module_DiskAccess::makeIntIntMap(){
	map <int,int> ret;
	for(int i = 0; i < BITROW * BITCOLUMN; i++) {
		ret.insert(pair <int, int>  ( i, -1 ) );
	}
	return ret;
}

map<int,int>::iterator Module_DiskAccess::makeIntIntMapIter(){
	map<int,int>::iterator ret;
	return ret;
}




int Module_DiskAccess::store(string str, bool enable_log){

	//k=1，默认从第一块开始分配 
	int i, j, k = 1, p, m, n, q,count,l,beforePosition = 0,result = -1;
	map<int,int>::iterator iter;
	count = str.length();
	if (k + count > Module_DiskAccess::y) {
		Module_DiskAccess::log("bad","磁盘空间不足！！！\n ");
		return -1;
	}
	else {
		//存储数据的位置  
//		Module_DiskAccess::dataPosition = (int *) malloc(count);
		for ( i = k,l = k,j=0; i < k + count; ) {
			q = c[l - 1][1];
			m = q / (DATATWO * DATATHREE);		// 柱面号
			n = (q % (DATATWO * DATATHREE)) / DATATHREE; //磁道 
			p = (q % (DATATWO * DATATHREE))% DATATHREE;  //扇区 
			if(Module_DiskAccess::a[q/BITCOLUMN][q%BITCOLUMN] != 1) {
				//把申请的磁盘块置为一，表示占用 
				Module_DiskAccess::a[q/BITCOLUMN][q%BITCOLUMN] = 1; 
				//存储数据到磁盘中 
				Module_DiskAccess::data[m][n][p] = str[j]; 
				if(i == k) {
					beforePosition = q; 
					result = q;
				} else {
					iter = Module_DiskAccess::fat.find(beforePosition);
					iter->second = q;
					beforePosition = q;
				}			
				i++;
				j++;
				l++;
			} else {
				l++;	
			}
		}
		
		//最后指向-1表示结束
		iter = Module_DiskAccess::fat.find(beforePosition);
		iter->second = -1;
		Module_DiskAccess::y = Module_DiskAccess::y - count;
		

		stringstream stream;
		if( enable_log ){
			stream << "存储数据后的位视图为(column:row)：\n";
			for ( int _sub_display_index = 0; _sub_display_index < BITCOLUMN; _sub_display_index++){
				for (int _display_index = 0; _display_index < BITROW; _display_index++) {
					stream << a[_display_index][_sub_display_index];
				}
				stream << endl;
			}
			stream << "\n\n";
			Module_DiskAccess::print( stream.str() );
		}
		
	}
	return result;
}

string Module_DiskAccess::readData(int position){
	int one = 0, two = 0, three = 0;
	string result = "";
	fat_Iter = fat.find(position);
	if(fat_Iter == fat.end()) {
		Module_DiskAccess::log("bad","没有该数据"); 
	} else {
		one = (fat_Iter->first) / (DATATWO * DATATHREE);
		two = ((fat_Iter->first) % (DATATWO * DATATHREE)) / DATATHREE;
		three = ((fat_Iter->first) % (DATATWO * DATATHREE)) % DATATHREE;
		result += data[one][two][three];
		position = fat_Iter->second;
		while(position != -1) {
			fat_Iter = fat.find(position);
			one = (fat_Iter->first) / (DATATWO * DATATHREE);
			two = ((fat_Iter->first) % (DATATWO * DATATHREE)) / DATATHREE;
			three = ((fat_Iter->first) % (DATATWO * DATATHREE)) % DATATHREE;
			result += data[one][two][three];
			position = fat_Iter->second;
		}	
	}
	return result;

}

void Module_DiskAccess::recycleAll(){

	int i, j;
	//位视图所有都置为0 
	for (i = 0; i < BITROW; i++) {
		for (j = 0; j < BITCOLUMN; j++) {
			if (!a[i][j])
				continue;
			else
			{
				a[i][j] = 0;
			}
		}
	}
}

void Module_DiskAccess::recycleFromBlock(int position, bool enable_log){

	int i = 0,j = 0,row = 0,column = 0,deleteLen = 0;
	
	Module_DiskAccess::fat_Iter = Module_DiskAccess::fat.find(position);
	if(Module_DiskAccess::fat_Iter == Module_DiskAccess::fat.end()) {
		Module_DiskAccess::log("bad","没有该数据,无法删除"); 
		return; 
	} else {
		row = (Module_DiskAccess::fat_Iter->first) / BITCOLUMN;
		column = (Module_DiskAccess::fat_Iter->first) % BITCOLUMN;
		Module_DiskAccess::a[row][column] = 0; 
		position = Module_DiskAccess::fat_Iter->second;
		deleteLen++;
		while(position != -1) {
			fat_Iter = fat.find(position);
			row = (fat_Iter->first) / BITCOLUMN;
		    column = (fat_Iter->first) % BITCOLUMN;
			a[row][column] = 0; 
			position = fat_Iter->second;
			deleteLen++;
		}	
		Module_DiskAccess::y = Module_DiskAccess::y + deleteLen;
	}
	
	if( enable_log ){
		stringstream stream;
		stream << "删除数据后的位视图为(column:row)：\n";
		for (j = 0; j < BITCOLUMN; j++){
			for (i = 0; i < BITROW; i++) {
				stream << a[i][j];
			}
			stream << endl;
		}
		stream << "\n\n";
		Module_DiskAccess::print( stream.str() );
	}
}



vector<int> Module_DiskAccess::display(vector<int> list_of_tracks){

	vector<int> list_of_tracks_back;
	//将list_of_tracks中的内容赋值给list_of_tracks_back
	list_of_tracks_back.assign(list_of_tracks.begin(), list_of_tracks.end());
	//对list_of_tracks_back中的内容排序
	sort(list_of_tracks_back.begin(),list_of_tracks_back.end());
	//对list_of_tracks_back中重复的元素进行清除
	list_of_tracks_back.erase( unique(list_of_tracks_back.begin(),list_of_tracks_back.end()),list_of_tracks_back.end());
	//输出list_of_tracks_back中的元素
	for(int i=0;i<list_of_tracks_back.size();i++) printf("%-5d",list_of_tracks_back[i]);
	printf("\n");

	return list_of_tracks_back;
}
void Module_DiskAccess::SSTF(vector<int> list_of_tracks_back, int * track_number, int size,int cp){


	int i = 0;
	//找到第一个小于cp的磁道号
	for(; i < size-1; i++) if(track_number[i+1] >= cp) break; 

	int Min_Left = i;
	int Max_Right = Min_Left+1;
	//left,right用于标志当前磁头走的是大于cp的那个方向还是小于cp的方向
	bool left = false,right = false;
	/*
		distance：跨越的磁道总数
		last : 磁头的上一个位置
	*/
	int distance = 0;
	vector<int>::iterator location_index,last_index;

	while(Min_Left >= 0 || Max_Right <= size-1){
		Module_DiskAccess::updateDiskHeadPosition = cp;
	
		if(Max_Right == size || track_number[Max_Right]-cp > cp-track_number[Min_Left] && Min_Left>=0) {
			left =true;
			cp = track_number[Min_Left--];
			
			location_index=find(list_of_tracks_back.begin(),list_of_tracks_back.end(),cp); 
			last_index=find(list_of_tracks_back.begin(),list_of_tracks_back.end(),Module_DiskAccess::updateDiskHeadPosition);

			for(int i= 0;i<location_index-list_of_tracks_back.begin();i++) printf("%-5s"," ");
			//printf("%-5d", cp);
			printf("%-5s","*" );

			if(right) for(int j =0; j<(last_index-list_of_tracks_back.begin())-(location_index-list_of_tracks_back.begin())-1;j++) printf("%-5s",".");
			right = false;
		}

		else {
			right =true;
			cp = track_number[Max_Right++];

			location_index=find(list_of_tracks_back.begin(),list_of_tracks_back.end(),cp); 
			last_index=find(list_of_tracks_back.begin(),list_of_tracks_back.end(),Module_DiskAccess::updateDiskHeadPosition);

			if(left) {
				printf("%-5s"," ");
				for(int j =0; j<(location_index-list_of_tracks_back.begin())-(last_index-list_of_tracks_back.begin())-1;j++) printf("%-5s",".");
			}
			else for(int i= 0;i<location_index-list_of_tracks_back.begin();i++)printf("%-5s"," ");
			//printf("%-5d", cp);
			printf("%-5s","*" );
			left = false;
			
		}
		
		
		printf("\n");
		distance += abs(cp-Module_DiskAccess::updateDiskHeadPosition);
		Module_DiskAccess::updateDiskHeadPosition = cp ;
	}
	printf("\n跨越的磁道总数:%d\n",distance);
	Module_DiskAccess::updateDiskHeadPosition = Module_DiskAccess::updateDiskHeadPosition;
	printf("最后一次访问的磁道为:%d\n",Module_DiskAccess::updateDiskHeadPosition);

}
void Module_DiskAccess::SCAN(vector<int> list_of_tracks_back, int * track_number, int size,int cp){


	/*
		distance：跨越的磁道总数
		mini :记录最小的大于当前磁头的磁道号下标
	*/
	int distance = 0,mini = 0;
	vector<int>::iterator location_index;
	//寻找最小的大于当前磁头的磁道号下标
	for(int i = 0; i < size; i++){
		if(track_number[i]>=cp){mini = i;break;}
	}

	if(mini == 0){
		Module_DiskAccess::updateDiskHeadPosition = track_number[size-1];
	}else{
		Module_DiskAccess::updateDiskHeadPosition = track_number[0];
	}

	for(int j =mini; j<size; j++){

		location_index=find(list_of_tracks_back.begin(),list_of_tracks_back.end(),track_number[j]); 

		for(int i= 0;i<location_index-list_of_tracks_back.begin();i++)printf("%-5s"," ");
		printf("%-5s\n","*");

		//记录磁道号
		if(j== mini){distance += abs(cp-track_number[j]);}
		else
			distance += abs(track_number[j]-track_number[j-1]);
	}

	for(int j =mini-1; j>=0; j--){
		
		location_index=find(list_of_tracks_back.begin(),list_of_tracks_back.end(),track_number[j]); 

		for(int i= 0;i<location_index-list_of_tracks_back.begin();i++)printf("%-5s"," ");
		printf("%-5s","*");

		if(j == mini-1){
			for(int i= 0;i<list_of_tracks_back.size()-2-(location_index-list_of_tracks_back.begin());i++)printf("%-5s",".");
			distance += abs(track_number[size-1]-track_number[j]);
		}else{
			distance += abs(track_number[j]-track_number[j+1]);
		}
		printf("\n");
	}
	
		printf("\n跨越的磁道总数:%d\n",distance);
		Module_DiskAccess::updateDiskHeadPosition = Module_DiskAccess::updateDiskHeadPosition;
		printf("最后一次访问的磁道为:%d\n",Module_DiskAccess::updateDiskHeadPosition);

}

void Module_DiskAccess::FCFS(vector<int> list_of_tracks_back, int * track_number, int size,int cp){

	bool flag = false;
	int distance = 0,last_number = 0;
	vector<int>::iterator location_index;
	for(int i=0;i<size;i++){
		location_index=find(list_of_tracks_back.begin(),list_of_tracks_back.end(),track_number[i]); 

		if(i==0){
			for(int j = 0;j<location_index-list_of_tracks_back.begin();j++)
			 printf("%-5s"," ");
			
			//printf("%-5d\n",track_number[i]);
			 printf("%-5s\n","*");

			 Module_DiskAccess::updateDiskHeadPosition = location_index-list_of_tracks_back.begin();
			 distance += abs( track_number[i]-cp);

		}else{

			if(Module_DiskAccess::updateDiskHeadPosition < location_index-list_of_tracks_back.begin()){
			flag =false;
			for(int j = 0;j<=Module_DiskAccess::updateDiskHeadPosition;j++)
			 printf("%-5s"," ");

			for(int j = Module_DiskAccess::updateDiskHeadPosition ;j<location_index-list_of_tracks_back.begin()-1;j++)
			 printf("%-5s",".");
			//printf("%-5d\n",track_number[i]);
			printf("%-5s\n","*");

			}else{
				flag =true;
				for(int j = 0;j<location_index-list_of_tracks_back.begin();j++)
				printf("%-5s"," ");
				//printf("%-5d",track_number[i]);
				printf("%-5s","*");

				for(int j = location_index-list_of_tracks_back.begin()+1;j<Module_DiskAccess::updateDiskHeadPosition;j++)  printf("%-5s",".");
				printf("\n\n");
			}

			Module_DiskAccess::updateDiskHeadPosition = location_index-list_of_tracks_back.begin();
			distance += abs( track_number[i]-track_number[i-1]);
		}
		
	}
	Module_DiskAccess::updateDiskHeadPosition = track_number[size-1];
	printf("\n跨越的磁道总数:%d\n",distance);
	Module_DiskAccess::updateDiskHeadPosition = Module_DiskAccess::updateDiskHeadPosition;
	printf("最后一次访问的磁道为:%d\n",Module_DiskAccess::updateDiskHeadPosition);

}



void Module_DiskAccess::printTracks( vector<int> list_of_tracks, int &start, char choice ){


	printf("磁道的访问顺序：");
	for(int i=0;i<list_of_tracks.size();i++) printf("%-5d",list_of_tracks[i]);
	printf("\n");

	vector<int> list_of_tracks_back,list_of_fcfs;
	list_of_fcfs.assign(list_of_tracks.begin(), list_of_tracks.end());//复制


	switch (choice)
	{
		case 'A' :
			printf("SSTF\n");
			sort(list_of_tracks.begin(),list_of_tracks.end());//排序
			list_of_tracks_back = display(list_of_tracks);
			SSTF(list_of_tracks_back,&list_of_tracks[0],list_of_tracks.size(),start);
			break;
		case 'B' :
			printf("SCAN\n");
			sort(list_of_tracks.begin(),list_of_tracks.end());//排序
			list_of_tracks_back = display(list_of_tracks);
			SCAN(list_of_tracks_back,&list_of_tracks[0],list_of_tracks.size(),start);
			break;
		case 'C' :
			printf("FCFS\n");
			list_of_tracks_back = display(list_of_tracks);
			FCFS(list_of_tracks_back,&list_of_fcfs[0],list_of_fcfs.size(),start);
			break;
		default:
			cout << "You did not enter A, B, or C!\n";
	}


}
