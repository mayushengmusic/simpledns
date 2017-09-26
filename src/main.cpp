#include "include/parseconfig.hpp"
#include "include/udp_server.hpp"
#include "include/message.h"
#include "include/callbackmysql.hpp"






int main() {



    dns::udp_server test_server(config_["ipandport"]);//config ip+port
    test_server.loopwork(translatequerywithmysql);





    return 0;
}