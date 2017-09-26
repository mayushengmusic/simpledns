//
// Created by jaken on 3/20/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <functional>
#include <regex>
#ifndef SIMPLEDNS_UDP_H
#define SIMPLEDNS_UDP_H

//#include <sys/epoll.h>


namespace dns {
    class udp_server {//simple udp server using epoll
    public:
        udp_server();

        udp_server(const std::string & AddressAndPort);

        void loopwork(std::function<void(char *,char *,unsigned int ,unsigned int *)>) throw();

        //callback function first client->server message,second message handled by the callback function
        ~udp_server();
        static const int BUF_SIZE = 512;
    private:

        int serv_sock;

        char *MessageCome=new char[BUF_SIZE];
        char *MessageBack=new char[BUF_SIZE];
        ssize_t str_len;
        socklen_t clnt_adr_sz;
        struct sockaddr_in serv_adr, clnt_adr;
    };
}

dns::udp_server::udp_server() {

    serv_sock = socket(PF_INET,SOCK_DGRAM,0);
    if(serv_sock == -1)
    {
        std::cerr<<"socket() error\n";
        exit(-1);
    }
    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(10000);
    if(bind(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1)
    {
        close(serv_sock);
        std::cerr<<"bind() error\n";
        exit(-2);
    }
    clnt_adr_sz=sizeof(clnt_adr);



}

dns::udp_server::udp_server(const std::string & AddressAndPort) {

    std::regex r("^(.+?):(.+)$");
    std::smatch m;
    if(!std::regex_match(AddressAndPort,m,r))
    {
        std::cerr<<"please check your listen address and port"<<std::endl;
        exit(-3);
    }

    std::string address=m[1];
    std::string port = m[2];


    serv_sock = socket(PF_INET,SOCK_DGRAM,0);
    if(serv_sock == -1)
    {
        std::cerr<<"socket() error\n";
        exit(-1);
    }
    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=inet_addr(address.c_str());
    serv_adr.sin_port=htons((uint16_t)atoi(port.c_str()));
    if(bind(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1)
    {
        close(serv_sock);
        std::cerr<<"bind() error\n";
        exit(-2);
    }
    clnt_adr_sz=sizeof(clnt_adr);


}

void dns::udp_server::loopwork(std::function<void(char *,char *,unsigned int,unsigned int *)> callback) throw() {


    while(1) {
        memset(MessageCome,0,BUF_SIZE);
        str_len = recvfrom(serv_sock, MessageCome, BUF_SIZE, 0, (struct sockaddr *) &clnt_adr, &clnt_adr_sz);
        if(str_len==-1)
        {
            close(serv_sock);
            std::cerr<<"recvfrom() error\n";
            break;
        }
        unsigned int sendlen=0;
        memset(MessageBack,0,BUF_SIZE);
        callback(MessageCome,MessageBack,(unsigned int)str_len,&sendlen);
        sendto(serv_sock, MessageBack, sendlen, 0, (struct sockaddr *) &clnt_adr, clnt_adr_sz);
    }
}

dns::udp_server::~udp_server() {
    close(serv_sock);
    delete[] MessageCome;
    delete[] MessageBack;
}

#endif //SIMPLEDNS_UDP_H
