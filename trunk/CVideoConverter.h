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

using namespace std;

class CVideoConverter {
private:
	CVideoConverter();
public:
	virtual ~CVideoConverter();
	static CVideoConverter *instance();

	string getStatus();

private:
	vector<string> getInfo(string file);
	string converInfoString(string x);
};

#endif /* CVIDEOCONVERTER_H_ */
