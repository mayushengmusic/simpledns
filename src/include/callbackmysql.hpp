//
// Created by jaken on 17-5-1.
//

#include <set>
#include "udp_client.hpp"
#include "mysqlconnectorc.hpp"
#include "parseconfig.hpp"
#ifndef SIMPLEDNS_CALLBACKMYSQL_HPP
#define SIMPLEDNS_CALLBACKMYSQL_HPP

namespace dns{
    class connectwithmysql{
    public:
        connectwithmysql();
        bool checkdata(const std::string & domin) ;
        std::string getipv4(const std::string & domin) ;
        ~connectwithmysql();

    private:
        dns::mysqlconnector *connectorptr;
        std::set<std::string> dominset;
        static std::string mysqlurl;
        static std::string mysqluser;
        static std::string mysqlpasswd;
        static std::string basedomin;
    };
}

std::string dns::connectwithmysql::mysqlurl=config_["mysqlurl"];
std::string dns::connectwithmysql::mysqluser=config_["mysqluser"];
std::string dns::connectwithmysql::mysqlpasswd=config_["mysqlpasswd"];
std::string dns::connectwithmysql::basedomin=config_["base"];


dns::connectwithmysql::connectwithmysql():dominset() {
    connectorptr=new dns::mysqlconnector(mysqlurl,mysqluser,mysqlpasswd);
    const std::list<std::string> *listptr=connectorptr->excSQLAndGetData("select username from radcheck","username");
    if(listptr!=NULL)
    for(auto & x: *listptr)
    {
        static const std::string zero(".");
        dominset.insert(x+zero+basedomin);
    }
    delete connectorptr;
    connectorptr=NULL;

}

dns::connectwithmysql::~connectwithmysql() {
    if(connectorptr!=NULL)
        delete connectorptr;
}

bool dns::connectwithmysql::checkdata(const std::string &domin)  {



    if(dominset.find(domin)==dominset.end()) {

        return false;
    }

    int sub_len;
    for(int i=0;i<domin.length();++i)
    {
        if(domin[i]=='.') {
            sub_len = i;
            break;
        }
    }




    connectorptr=new dns::mysqlconnector(mysqlurl,mysqluser,mysqlpasswd);
    std::string SQL = "select acctstoptime from radacct where username=\"";
    SQL+=domin.substr(0,sub_len);
    SQL+="\"";
    SQL+=" order by acctstarttime desc limit 1;";

    const std::list<std::string> * listptr=connectorptr->excSQLAndGetData(SQL,"acctstoptime");


    while(listptr==NULL)
    {
        sleep(3);
        delete connectorptr;
        std::cerr<<"database connect break try to connect again\n";
        connectorptr=new dns::mysqlconnector(mysqlurl,mysqluser,mysqlpasswd);

        listptr=connectorptr->excSQLAndGetData(SQL,"acctstoptime");
    }

    delete connectorptr;
    connectorptr=NULL;


    if(listptr->size()!=1)
    {
        return true;
    }



    return false;
}

std::string dns::connectwithmysql::getipv4(const std::string &domin) {
    int sub_len;
    for(int i=0;i<domin.length();++i)
    {
        if(domin[i]=='.') {
            sub_len = i;
            break;
        }
    }

    connectorptr=new dns::mysqlconnector(mysqlurl,mysqluser,mysqlpasswd);
    std::string SQL = "select callingstationip  from radacct where username = \"";
    SQL+=domin.substr(0,sub_len);
    SQL+="\"";
    SQL+=" order by acctstarttime desc limit 1";
    const std::list<std::string> * listptr=connectorptr->excSQLAndGetData(SQL,"callingstationip");
    while(listptr==NULL)
    {
        sleep(3);
        delete connectorptr;
        std::cerr<<"database connect break try to connect again\n";
        connectorptr=new dns::mysqlconnector(mysqlurl,mysqluser,mysqlpasswd);

        listptr=connectorptr->excSQLAndGetData(SQL,"callingstationip");
    }

    delete connectorptr;
    connectorptr=NULL;

    if(listptr->size()!=1)
    {
        std::cerr<<"please check the database\n";
        return "";
    }

    return *(listptr->begin());

}


void translatequerywithmysql(char *MessageCome,char *MessageBack,unsigned int MessageCome_lenth,unsigned int *MessageBack_lenth)
{
    dns::Message querymessage;
    static dns::connectwithmysql mysqlworker;
    static std::string ipaddress;
    bool ipaddressnotempty=false;

    querymessage.decode(MessageCome,(const unsigned int)MessageCome_lenth);

    /*
    std::cout<<"*******************************************\n";
    std::cout<<querymessage.asString()<<std::endl;
    std::cout<<"*******************************************\n";
    */

    if(querymessage.getQdCount()==1)
    {
        std::vector<dns::QuerySection*>::const_iterator it = querymessage.getmQueries().begin();
        static std::string question;
        question=(*it)->getName();


       ipaddressnotempty=mysqlworker.checkdata(question);//
        if(ipaddressnotempty)
            ipaddress=mysqlworker.getipv4(question);//

    }


    if(ipaddressnotempty)
    {

        querymessage.setQr(dns::Message::typeResponse);
        dns::ResourceRecord *rrA = new dns::ResourceRecord();
        rrA->setType(dns::RDATA_A);
        rrA->setClass(dns::CLASS_IN);
        rrA->setTtl(60);
        dns::RDataA *rdataA = new dns::RDataA();
        dns::uchar ip4[4] ={};
        std::stringstream oss;
        oss.str("");
        oss.clear();
        static std::regex r("([\\d]+)\\.([\\d]+)\\.([\\d]+)\\.([\\d]+)");
        static std::smatch m;

        std::regex_match(ipaddress,m,r);

        unsigned int temp=0;

        oss<<m[1]<<" "<<m[2]<<" "<<m[3]<<" "<<m[4];


        oss>>temp;
        ip4[0]=(dns::uchar)temp;

        oss>>temp;
        ip4[1]=(dns::uchar)temp;

        oss>>temp;
        ip4[2]=(dns::uchar)temp;

        oss>>temp;
        ip4[3]=(dns::uchar)temp;

        rdataA->setAddress(ip4);
        rrA->setRData(rdataA);
        rrA->setTtl(0);
        querymessage.addAnswer(rrA);
        querymessage.encode(MessageBack,dns::udp_server::BUF_SIZE,*MessageBack_lenth);
    }
    else {

        static dns::udp_client queryclient(config_["forwarddns"].c_str());
        ssize_t length = queryclient.querydns(MessageCome, MessageCome_lenth, MessageBack);
        *MessageBack_lenth=(unsigned int)length;
    }

    dns::Message response;
    response.decode(MessageBack,*MessageBack_lenth);
    /*
    std::cout<<"*******************************************\n";
    std::cout<<response.asString()<<std::endl;
    std::cout<<"*******************************************\n";
    */

}



#endif //SIMPLEDNS_CALLBACKMYSQL_HPP
