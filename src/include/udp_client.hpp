//
// Created by jaken on 3/27/17.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <regex>


#ifndef SIMPLEDNS_UDP_CLIENT_HPP
#define SIMPLEDNS_UDP_CLIENT_HPP
namespace dns{
    class udp_client{
    public:
        udp_client(const std::string &link);

        ssize_t querydns(char *query,size_t querysize,char *response);

        static const size_t BUF_SIZE = 512;

    private:
        int sock;

        struct sockaddr_in serv_adr;
    };
}

dns::udp_client::udp_client(const std::string &link) {
    std::regex r("^(.+?):(.+)$");
    std::smatch m;
    if(!std::regex_match(link,m,r))
    {
        std::cerr<<"please check your dns address and port"<<std::endl;
        exit(-3);
    }

    std::string address=m[1];
    std::string port = m[2];



    sock = socket(PF_INET,SOCK_DGRAM,0);
    if(sock==-1)
    {
        std::cerr<<"sock() error"<<std::endl;
        exit(-4);
    }

    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=inet_addr(address.c_str());
    serv_adr.sin_port=htons((uint16_t)atoi(port.c_str()));
    if(connect(sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1)
        exit(-5);
}


ssize_t dns::udp_client::querydns(char *query, size_t querysize, char *response) {
    ssize_t writelen=write(sock,query,querysize);

    if(writelen==-1)
        return -1;

    memset(response,0,BUF_SIZE);
    return read(sock,response,BUF_SIZE-1);

}


#endif //SIMPLEDNS_UDP_CLIENT_HPP
