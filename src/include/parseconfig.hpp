//
// Created by jaken on 17-5-22.
//
#include <map>
#include <fstream>
#include <cstring>
#include <regex>
#include <iostream>

#ifndef SIMPLEDNS_PARSECONFIG_HPP
#define SIMPLEDNS_PARSECONFIG_HPP

namespace dns {
    class parseconfig {
    public:
        parseconfig(const std::string &config_file_path);
        parseconfig(const std::string &&config_file_path);

        const std::string &operator[](std::string &index);

        const std::string &operator[](std::string &&index);

    private:
        const unsigned int MAX_SIZE = 128;

        std::map<std::string,std::string> configdata;
    };
}

dns::parseconfig::parseconfig(const std::string &config_file_path) {
    std::ifstream file_config(config_file_path,std::ios_base::in);
    char *buf = new char[MAX_SIZE];

    std::regex r("^[[:cntrl:]]*(\\w+)\\s?\\:\\s?([\\w/\\:\\.]+)[[:cntrl:]]*$");


    while(file_config.good()) {
        memset(buf,0,MAX_SIZE);

        file_config.getline(buf, MAX_SIZE - 1);
        if(buf[0]=='#')
            continue;
        std::smatch m;
        std::string buf_string(buf);
        if(!std::regex_match(buf_string,m,r))
            continue;
        configdata[std::string(std::move(m[1]))]=std::string(std::move(m[2]));
        //std::cout<<m[1]<<" "<<m[2]<<std::endl;
    }
}

dns::parseconfig::parseconfig(const std::string &&config_file_path):parseconfig(config_file_path) {
}

const std::string& dns::parseconfig::operator[](std::string &index){

    return configdata[index];
}

const std::string& dns::parseconfig::operator[](std::string &&index){

    return configdata[index];
}

dns::parseconfig config_("/etc/config.conf");



#endif //SIMPLEDNS_PARSECONFIG_HPP

extern dns::parseconfig config_;