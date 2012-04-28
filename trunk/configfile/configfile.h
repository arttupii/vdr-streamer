#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <map>

#include<string>
using namespace std;

class ConfigFile
{
    public:
        ConfigFile();
        ~ConfigFile();

        static ConfigFile* instance();

        bool get_value(string _class,string name,string &value);
        bool get_value(string _class,string name,int &value);
        bool get_value(string _class,string name,float &value);

        void set_value(string _class,string name,string value);
        void set_value(string _class,string name,int value);
        void set_value(string _class,string name,float value);

        bool get_value(string name,string &value);
        bool get_value(string name,int &value);
        bool get_value(string name,float &value);

        void set_value(string name,string value);
        void set_value(string name,int value);
        void set_value(string name,float value);

        bool save_configFile(string file);
        bool open_configFile(string file);
        void print_configFile();
        void allowe_extensions(bool set)
        {
            extensions = set;
        }
    protected:
    private:
        typedef map<string,string> Class;
        map<string,Class> db;

        string filename;

        string remove_extra_marks_from_begin(string x);
        string remove_extra_marks_from_end(string x);
        string get_class_name(string x);
        string get_value_name(string x);
        string get_value(string x);

        bool extensions;

};

#endif // CONFIGFILE_H
