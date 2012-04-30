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
