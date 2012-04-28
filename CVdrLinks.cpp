/*
 * CVdrLinks.cpp
 *
 *  Created on: Apr 24, 2012
 *      Author: arttu
 */

#include "CVdrLinks.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include "configfile/configfile.h"
#include <stdlib.h>
#include <sstream>
CVdrLinks::CVdrLinks() {
	// TODO Auto-generated constructor stub

}

CVdrLinks::~CVdrLinks() {
	// TODO Auto-generated destructor stub
}

string removeExtraMarks(string line)
{
	string tmp="";
	for(int i=0;i<line.length();i++)
	{
		switch(line[i])
		{
		case '\n':
		case '\r':
		break;
		default:
			tmp+=line[i];
			break;
		}
	}
	return tmp;
}

CVdrLinks* CVdrLinks::instance()
{
	static CVdrLinks* _instance=NULL;
	if(_instance==NULL)
	{
		_instance=new CVdrLinks();
	}
	return _instance;
}
string CVdrLinks::getLink(string fileName)
{
	string ret="";
	list<VdrLink>::iterator it = links.begin();
	for(;it!=links.end();it++)
	{
		if((*it).fileName==fileName)
		{
			return (*it).link;
		}
	}
	return ret;
}

list<VdrLink> CVdrLinks::get()
{
	return links;
}
void CVdrLinks::update()
{
	string channels_m3u_link = "http://192.168.11.6:3000/channels.m3u";
	ConfigFile::instance()->get_value("channels.m3u link", channels_m3u_link);
	string cmd = "wget ";
	cmd += channels_m3u_link + " --output-document=channels.m3u";
	printf("channels.m3u-link --> %s\n", cmd.c_str());
	system(cmd.c_str());

	ifstream infile;
	infile.open("channels.m3u"); // open file
	if(infile) {
	  string s="";
	  int line=0;
	  string name="";
	  string link="";
	  int c=1;
	  while(getline(infile, s)) {
	    line++;
	    if(line==1) continue;
	    if(line&1)
	    {
	    	link = removeExtraMarks(s);
	    	printf("--> !%s! --> !%s!\n", name.c_str(), link.c_str());
	    	VdrLink l;
	    	l.name = name;
	    	l.link = link;
		stringstream tmp;
		tmp<<"channel_"<<c<<".stream";
		l.fileName = tmp.str();
	    	links.push_back(l);
		c++;
	    }
	    else
	    {
	    	name = removeExtraMarks(s);
	    	int index = name.find(" ");
	    	if(index!=string::npos)
	    	{
	    		name = name.substr(index+1);
	    	}
	    }
	  }
	}
	else
	{
		printf("Cannot open channels.m3u\n");
		exit(1);
	}
}
