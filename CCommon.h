/*
 * CCommon.h
 *
 *  Created on: 30.4.2012
 *      Author: arttu
 */

#ifndef CCOMMON_H_
#define CCOMMON_H_
#include <vector>
#include <string>
using namespace std;

class CCommon {
public:
	CCommon();
	virtual ~CCommon();
	static pid_t popen2(const char *command, int *infp, int *outfp);
	static void get_file_list(vector<string> &fileList, const char *dir_path, int DoNotUse_subsrt=0);
};

#endif /* CCOMMON_H_ */
