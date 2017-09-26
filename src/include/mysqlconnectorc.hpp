//
// Created by jaken on 17-6-5.
//
#include <iostream>
#include <string>
#include <list>
#include <regex>
#include <sstream>
#include "parseconfig.hpp"
#include <mysql/mysql.h>

#ifndef CAPIMYSQL_MYSQLCONNECTORC_HPP
#define CAPIMYSQL_MYSQLCONNECTORC_HPP
namespace dns {
    class mysqlconnector {
    public:
        mysqlconnector(std::string url, std::string user, std::string pass);

        const std::list<std::string> * excSQLAndGetData(const std::string &SQL, const std::string &index);

        bool excSQLwithoutReturn(const std::string &SQL);


    private:

        static const std::string database;
         std::string ip;
         std::string user;
         std::string password;
         int port;

        std::list<std::string> excdata;

    };
}


const std::string dns::mysqlconnector::database = config_["database"] ;

dns::mysqlconnector::mysqlconnector(std::string url, std::string user, std::string pass):user(user),password(pass),excdata()
{
    //tcp://10.0.3.2:3306

    std::regex r("tcp\\://([\\d\\.]+)\\:([\\d]+)");
    std::smatch m;
    std::regex_match(url,m,r);
    ip = m[1];
    std::stringstream ss;
    ss<<m[2];
    ss>>port;

}


bool dns::mysqlconnector::excSQLwithoutReturn(const std::string &SQL) {

    MYSQL *con = mysql_init(NULL);

    if (con == NULL)
    {
        std::cerr<<"connect error"<<std::endl;
        return false;
    }

    if (mysql_real_connect(con, ip.c_str(), user.c_str(), password.c_str(),
                           database.c_str(), port, NULL, 0) == NULL)
    {
       std::cerr<<"connect error"<<std::endl;
        mysql_close(con);
        return false;
    }

    if (mysql_query(con,SQL.c_str()))
    {
        std::cerr<<"exc SQL error"<<std::endl;
        mysql_close(con);
        return false;
    }
    mysql_close(con);

    return true;
}



const std::list<std::string>* dns::mysqlconnector::excSQLAndGetData(const std::string &SQL, const std::string &index) {
    MYSQL *con = mysql_init(NULL);

    if (con == NULL)
    {
        std::cerr<<"connect error"<<std::endl;
        return NULL;
    }

    if (mysql_real_connect(con, ip.c_str(), user.c_str(), password.c_str(),
                           database.c_str(), port, NULL, 0) == NULL)
    {
        std::cerr<<"connect error"<<std::endl;
        mysql_close(con);
        return NULL;
    }

    if (mysql_query(con,SQL.c_str()))
    {
        std::cerr<<"exc SQL error"<<std::endl;
        mysql_close(con);
        return NULL;
    }

    MYSQL_RES *result = mysql_store_result(con);

    if (result == NULL)
    {
        std::cerr<<"get result error"<<std::endl;
        mysql_close(con);
        return NULL;
    }

    int num_fields = mysql_num_fields(result);

    MYSQL_ROW row;


    excdata.clear();
    while ((row = mysql_fetch_row(result)))
    {
        for(int i = 0; i < num_fields; i++)
        {
            if(row[i])
                excdata.push_back(std::string(row[i]));
        }

    }

    mysql_free_result(result);
    mysql_close(con);
    return &excdata;
}

#endif //CAPIMYSQL_MYSQLCONNECTORC_HPP
