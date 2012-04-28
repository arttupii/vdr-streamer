
#ifndef _FILE__CONVERSION
#define _FILE__CONVERSION

#include <string>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

class Conversion
{
    public:
    static string intToString(int x)
    {
        char tmp[20];
        sprintf (tmp, "%d",x);
        return tmp;
    }

    static string floatToString(float x)
    {
        char tmp[20];
        sprintf (tmp, "%f",x);
        return tmp;
    }

    static int stringToInt(string x)
    {
        return atoi(x.c_str());
    }

    static float stringTofloat(string x)
    {
        float d=0;
        sscanf(x.c_str(),"%f", &d);
        return (float)d;
    }
};
#endif
