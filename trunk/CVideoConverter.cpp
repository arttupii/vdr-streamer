/*
 * CVideoConverter.cpp
 *
 *  Created on: 30.4.2012
 *      Author: arttu
 */

#include "CVideoConverter.h"
#include <sstream>
#include "CCommon.h"
#include "configfile/configfile.h"

CVideoConverter::CVideoConverter() {
	// TODO Auto-generated constructor stub
	vdr_video_folder = "/home/video";
	ConfigFile::instance()->get_value("vdr_video_folder", vdr_video_folder);
}

CVideoConverter::~CVideoConverter() {
	// TODO Auto-generated destructor stub
}

CVideoConverter *CVideoConverter::instance()
{
	static CVideoConverter *p = 0;
	if(p==0)
	{
		p = new CVideoConverter();
	}
	return p;
}

void CVideoConverter::startVideoConverting(string folder)
{

}

string CVideoConverter::getStatus()
{
	stringstream data;
	vector<string> filelist;
	CCommon::get_file_list(filelist, vdr_video_folder.c_str());

	const string infofile = "/info";
	for(int i=0;i<filelist.size();i++)
	{
		string file = filelist[i];
		ssize_t info_index = file.find(infofile);

		if( (info_index + infofile.length()) == file.length() )
		{
			string folder = file.substr(0,info_index);
			vector<string> videoInfo = getInfo(vdr_video_folder + file);
			if(!videoInfo.empty())
			{
				string channel = videoInfo[0];
				string info = videoInfo[1];
				string name = videoInfo[2];
				string description = videoInfo[3];
				string status = "IDLE";
				data<<channel<<";"<<info<<";"<<name<<";"<<description<<";"<<folder<<";"<<status<<"\n";
			}
		}
	}

	return data.str();
}

vector<string> CVideoConverter::getInfo(string file)
{
	string tmp="";
	ifstream in;
	in.open(file.c_str());

	if (in.is_open())
	{
		 while (!in.eof()) {
			string x;
			getline(in,x);
		    tmp+=x;
		 }
	}
	in.close();

	int x,y;
	vector<string> v;

	x=tmp.find(" ",3)+1;
	y=tmp.find("E ");
	if(x==string::npos || y==string::npos) return v;
	string channel = tmp.substr(x, y-x);

	x=tmp.find("E ")+2;
	y=tmp.find("T ");
	if(x==string::npos || y==string::npos) return v;
	string info = tmp.substr(x, y-x);


	x=tmp.find("T ")+2;
	y=tmp.find("D ");
	if(x==string::npos || y==string::npos) return v;
	string name = tmp.substr(x, y-x);

	x=tmp.find("D ")+2;
	y=tmp.find("X ");
	if(x==string::npos || y==string::npos) return v;
	string description = tmp.substr(x, y-x);

	v.push_back(converInfoString(channel));
	v.push_back(converInfoString(info));
	v.push_back(converInfoString(name));
	v.push_back(converInfoString(description));

	return v;
}

string CVideoConverter::converInfoString(string x)
{
	for(size_t i=0;i<x.length();i++)
	{
		switch(x[i])
		{
		case '\n':
		case ';':
		case ':':
		case '\r':
			x[i]=' ';
		break;
		default:
			break;
		}
	}
	return x;
}
