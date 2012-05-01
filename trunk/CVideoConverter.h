/*
 * CVideoConverter.h
 *
 *  Created on: 30.4.2012
 *      Author: arttu
 */

#ifndef CVIDEOCONVERTER_H_
#define CVIDEOCONVERTER_H_
#include<string>
#include<fstream>
#include<vector>
#include<sstream>
#include<list>

#include"CMutex.h"
using namespace std;
class CVideoConverter;


typedef struct{
	string channel;
	string name;
	string info;
	string description;
	string folder;
	string id;
	string status;
	bool taskRunning;
} TaskInfo;



class CVideoConverter {
private:
	CVideoConverter();
public:
	virtual ~CVideoConverter();
	static CVideoConverter *instance();

	string getStatus();

	void startVideoConverting(string id);
	void stopVideoConverting(string id);
	void videoConvertingReady(string id, string status);
private:
	void updateVideoInfoFromVdrDir();

	list<TaskInfo>::iterator findTaskInfo(string id);

	vector<string> getInfo(string file);
	string converInfoString(string x);
	string vdr_video_folder;
	int id_counter;

	list<TaskInfo> tasks;
	friend class Task;
	CMutex mutex;
};

#endif /* CVIDEOCONVERTER_H_ */
