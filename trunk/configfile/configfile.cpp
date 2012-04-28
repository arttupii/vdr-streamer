#include "configfile.h"

#include "Conversion.h"

#include <stdio.h>
#include <cstdio>

#include <stdlib.h>
#include <iostream>


ConfigFile::ConfigFile()
{
    extensions = false;
}


ConfigFile::~ConfigFile()
{
    //dtor
}

ConfigFile *p_instance=NULL;
ConfigFile* ConfigFile::instance()
{
	if(p_instance==NULL)
	{
		p_instance=new ConfigFile();
	}
	return p_instance;
}

string ConfigFile::remove_extra_marks_from_begin(string x)
{
    string tmp = "";
    for (unsigned int i=0;i<x.length();i++)
    {
        if (x[i]>32&&x[i]<127)
        {
            return x.substr(i);
        }
    }
    return tmp;
}
string ConfigFile::remove_extra_marks_from_end(string x)
{
    string tmp = "";
    for (int i=x.length();i>=0;i--)
    {
        if (x[i]>32&&x[i]<127)
        {
            return x.substr(0,i+1);
        }
    }
    return tmp;
}
string ConfigFile::get_class_name(string x)
{
    unsigned int s = x.find("[");
    unsigned int e = x.find_last_of("]");

    if (s!=string::npos && e!=string::npos)
    {
        return x.substr(s+1,e-1);
    }
    else return "";
}
string ConfigFile::get_value_name(string x)
{
    x = remove_extra_marks_from_begin(x);

    unsigned int s = x.find("=");

    if (s!=string::npos)
    {
        string tmp = x.substr(0,s-1);

        return tmp;
    }
    else return "";
}
string ConfigFile::get_value(string x)
{
    unsigned int s = x.find("=");

    if (s!=string::npos)
    {
        return remove_extra_marks_from_begin(remove_extra_marks_from_end(x.substr(s+1)));
    }
    else return "";
}
bool ConfigFile::open_configFile(string file)
{

    FILE * pFile;
    pFile = fopen (file.c_str(),"r");

    if (pFile!=NULL)
    {
        char c_tmp[1000];
        string tmp;
        string class_name = "";
        while (fgets(c_tmp,1000,pFile)!=NULL)
        {
            tmp = remove_extra_marks_from_begin(c_tmp);
            if (tmp.length()<1) continue; //ei mitään luettavaa
            if (tmp[0]=='#') continue; //kommentti
            if (tmp[0]=='[') //new class begins
            {
                class_name = get_class_name(tmp);

            }
            else
            {
                //class's items
                if (class_name.length()>0)
                {
                    string name = get_value_name(tmp);
                    string value = get_value(tmp);

                    if (name.length()>0)
                    {
                        if (value.length()>0)
                        {
                            set_value(class_name,name,value);
                        }
                    }
                }
            }
        }
        fclose(pFile);

        return true;
    }
    else
    {

    }
    return false;
}
bool ConfigFile::save_configFile(string file)
{
    FILE * pFile;
    pFile = fopen (file.c_str(),"w");
    string tmp;

    if (pFile!=NULL)
    {
        map<string,Class>::iterator it_class = db.begin();
        for (;it_class!=db.end();it_class++)
        {
            tmp = "[" + it_class->first + "]\n";
            fputs(tmp.c_str(),pFile);

            map<string,string>::iterator it_value_name = it_class->second.begin();
            for (;it_value_name!=it_class->second.end();it_value_name++)
            {
                tmp = "\t"+it_value_name->first+" = "+it_value_name->second+"\n";
                fputs(tmp.c_str(),pFile);
            }
            fputs("\n",pFile);
        }
        fclose(pFile);

        return true;
    }
    return false;
}

void ConfigFile::print_configFile()
{
    map<string,Class>::iterator it_class = db.begin();
    for (;it_class!=db.end();it_class++)
    {
        cout<<"["<<it_class->first<<"]\n";
        map<string,string>::iterator it_value_name = it_class->second.begin();
        for (;it_value_name!=it_class->second.end();it_value_name++)
        {
            cout<<"\t'"<<it_value_name->first<<"' = '"<<it_value_name->second<<"'\n";
        }
    }

}

bool ConfigFile::get_value(string _class,string name,string &value)
{
    map<string,Class>::iterator it_class = db.find(_class);
    if (it_class!=db.end())
    {
        map<string,string>::iterator it_value_name = it_class->second.find(name);
        if (it_value_name!=it_class->second.end())
        {
            value = it_value_name->second;
            return true;
        }
    }
    return false;
}
bool ConfigFile::get_value(string _class,string name,int &value)
{
    string tmp;
    if (get_value(_class,name,tmp))
    {
        value = Conversion::stringToInt(tmp);
        return true;
    }
    return false;
}
bool ConfigFile::get_value(string _class,string name,float &value)
{
    string tmp;
    if (get_value(_class,name,tmp))
    {
        value = Conversion::stringTofloat(tmp);
        return true;
    }
    return false;
}

void ConfigFile::set_value(string _class,string name,string value)
{
    map<string,Class>::iterator it_class = db.find(_class);
    if (it_class!=db.end())
    {
        (it_class->second)[name]=value;
    }
    else
    {
        Class tmp;
        tmp[name] = value;
        db[_class] = tmp;
    }
}

void ConfigFile::set_value(string _class,string name,int value)
{
    set_value(_class,name,Conversion::intToString(value));
}
void ConfigFile::set_value(string _class,string name,float value)
{
    set_value(_class,name,Conversion::floatToString(value));
}

bool ConfigFile::get_value(string name,string &value)
{
	return get_value("Default", name, value);
}
bool ConfigFile::get_value(string name,int &value)
{
	return get_value("Default", name, value);
}
bool ConfigFile::get_value(string name,float &value)
{
	return get_value("Default", name, value);
}

void ConfigFile::set_value(string name,string value)
{
	set_value("Default", name, value);
}
void ConfigFile::set_value(string name,int value)
{
	set_value("Default", name, value);
}
void ConfigFile::set_value(string name,float value)
{
	set_value("Default", name, value);
}
