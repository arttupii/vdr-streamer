/*
 * CVideoConverter.cpp
 *
 *  Created on: 30.4.2012
 *      Author: arttu
 */

#include "CVideoConverter.h"
#include <sstream>

CVideoConverter::CVideoConverter() {
	// TODO Auto-generated constructor stub

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
string CVideoConverter::getStatus()
{
	stringstream data;

	return data.str();
}

vector<string> CVideoConverter::getInfo(string file)
{
	string tmp="";
	ifstream in;
	in.open("info");

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

	x=tmp.find("T ")+2;
	y=tmp.find("D ");
	if(x==string::npos || y==string::npos) return v;
	string name = tmp.substr(x, y-x);

	x=tmp.find("D ")+2;
	y=tmp.find("X ");
	if(x==string::npos || y==string::npos) return v;
	string description = tmp.substr(x, y-x);

	v.push_back(converInfoString(channel));
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
