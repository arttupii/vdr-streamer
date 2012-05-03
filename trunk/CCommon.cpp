/*
 * CCommon.cpp
 *
 *  Created on: 30.4.2012
 *      Author: arttu
 */

#include "CCommon.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>

CCommon::CCommon() {
	// TODO Auto-generated constructor stub

}

CCommon::~CCommon() {
	// TODO Auto-generated destructor stub
}




void CCommon::get_file_list(vector<string> &fileList, const char *dir_path, int DoNotUse_subsrt)
{
	struct dirent *dp;
	        // enter existing path to directory below
	if(DoNotUse_subsrt==0)
	{
		DoNotUse_subsrt=strlen(dir_path);
		while(dir_path[DoNotUse_subsrt]=='/')
		{
			DoNotUse_subsrt++;
		}
	}

	DIR *dir = opendir(dir_path);
	if(dir==NULL)
	{
		printf(" CCommon::get_file_list, invalid folder %s\n", dir_path);
		exit(1);
	}
	while ((dp=readdir(dir)) != NULL)
	{
		if( !((strcmp(dp->d_name,".")==0) || (strcmp(dp->d_name,"..")==0)) )
		{
			switch(dp->d_type)
			{
				case DT_DIR:
				{
					char buffer[2048];
					snprintf(buffer, sizeof(buffer), "%s/%s", dir_path, dp->d_name);
					get_file_list(fileList, buffer, DoNotUse_subsrt);
					break;
				}
				case DT_REG:
				{
					char buffer[2048];
					snprintf(buffer, sizeof(buffer), "%s/%s", dir_path, dp->d_name);
					fileList.push_back( string(buffer).substr(DoNotUse_subsrt));
					break;
				}
				case DT_SOCK:
				case DT_FIFO:
				case DT_LNK:
				case DT_BLK:
				case DT_CHR:
				case DT_UNKNOWN:
					break;
			}
		}
	}
	closedir(dir);
	return ;
}
