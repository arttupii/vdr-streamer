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
#include<map>
#include"CMutex.h"
using namespace std;
class CVideoConverter;


typedef struct{
	string channel;
	string name;
	string info;
	string description;
	string folder;


	string task_id;
	bool task_isRunning;
	string task_status;
	string task_pid;
	pid_t task_pid_child;
	string task_source_folder;
	string task_target_folder;
	string task_target_file_name;
} TaskInfo;



class CVideoConverter {
private:
	CVideoConverter();
public:
	virtual ~CVideoConverter();

	static string startVideoConverting(string id);
	static string stopVideoConverting(string id);

	static void startTask();

	static void updateVideos();
	static void checkTaskStatus();
private:
	map<int, string> split(string text, char c);
	void runTask();
	void write_status_to_disk();

	void setVideoConvertingStatus(string id, string status);
	string getVideoConvertingStatus(string id);

	static int writeToPosixQueue(string text);

	void updateVideoInfoFromVdrDir();

	int getOngoingTaskCount();
	void startNewTask(string id);
	list<TaskInfo>::iterator findTaskInfo(string id);

	vector<string> getInfo(string file);
	string converInfoString(string x);
	string vdr_video_folder;
	string video_output_folder;

	int id_counter;
	int max_count_video_tasks;

	list<TaskInfo> tasks;
	friend class Task;
	friend void *runUpdateVdrFilesTask(void*);
	CMutex mutex;

	list<string> waiting_list;

};

#endif /* CVIDEOCONVERTER_H_ */
