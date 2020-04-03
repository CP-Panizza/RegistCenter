//
// Created by cmj on 20-3-26.
//

#include "util.h"
#include <dirent.h>
#include <sys/stat.h>



std::vector<std::string> split(std::string str, std::string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    str+=pattern;//扩展字符串以方便操作
    int size=str.size();

    for(int i=0; i<size; i++)
    {
        pos=str.find(pattern,i);
        if(pos<size)
        {
            std::string s=str.substr(i,pos-i);
            result.push_back(s);
            i=pos+pattern.size()-1;
        }
    }
    return result;
}

bool contain(std::string str, std::string target){
    if(str == target)
        return true;
    if(str == "")
        return false;
    if(target == "")
        return true;
    std::string::size_type pos = str.find(target);
    if(pos == std::string::npos){
        return false;
    } else {
        return true;
    }
}


bool file_exists (const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}


bool dir_exists(std::string path){
    DIR *dir;
    if((dir = opendir(path.c_str())) == NULL){
        return false;
    }
    closedir(dir);
    return true;
}

long file_size(const char *filepath){
    struct stat info;
    stat(filepath, &info);
    int size = info.st_size;
    return size;
}

void trim_space(std::string &s)
 {
    int index = 0;
    if( !s.empty())
    {
        while( (index = s.find(' ',index)) != std::string::npos)
        {
            s.erase(index,1);
        }
    }
 }