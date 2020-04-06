//
// Created by Administrator on 2020/4/1.
//

#include "util.h"
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <map>

std::vector<std::string> split(std::string str, std::string pattern) {
    std::string::size_type pos;
    std::vector<std::string> result;
    str += pattern;//扩展字符串以方便操作
    int size = str.size();

    for (int i = 0; i < size; i++) {
        pos = str.find(pattern, i);
        if (pos < size) {
            std::string s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}

bool contain(std::string str, std::string target) {
    if (str == target)
        return true;
    if (str == "")
        return false;
    if (target == "")
        return true;
    std::string::size_type pos = str.find(target);
    if (pos == std::string::npos) {
        return false;
    } else {
        return true;
    }
}


bool file_exists(const std::string &name) {
    std::ifstream f(name.c_str());
    return f.good();
}


bool dir_exists(std::string path) {
    DIR *dir;
    if ((dir = opendir(path.c_str())) == NULL) {
        return false;
    }
    closedir(dir);
    return true;
}

long file_size(const char *filepath) {
    struct stat info;
    stat(filepath, &info);
    int size = info.st_size;
    return size;
}

void trim_space(std::string &s) {
    int index = 0;
    if (!s.empty()) {
        while ((index = s.find(' ', index)) != std::string::npos) {
            s.erase(index, 1);
        }
    }
}

std::string &replace_all(std::string &str, const std::string &old_value, const std::string &new_value) {
    while (true) {
        std::string::size_type pos(0);
        if ((pos = str.find(old_value)) != std::string::npos) {
            str.replace(pos, old_value.length(), new_value);
        } else { break; }
    }
    return str;
}

std::string read_file(std::string file) {
    std::ifstream fin(file);
    std::stringstream buffer;
    buffer << fin.rdbuf();
    std::string str(buffer.str());
    return str;
}

std::map<std::string, std::string> getConf(std::string file){
    std::map<std::string, std::string> conf;
    std::string data = read_file(file);
    auto v = split(data, "\n");
    for(auto x : v){
        auto str = replace_all(x, "\n", "");
        trim_space(str);
        auto k_v = split(str, "=");
        if(k_v.size() == 2){
            conf[k_v[0]] = k_v[1];
        }
    }
    return conf;
}