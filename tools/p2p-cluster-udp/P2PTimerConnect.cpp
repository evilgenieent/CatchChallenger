#include "P2PTimerConnect.h"
#include "P2PServerUDP.h"
#include <iostream>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "../../server/epoll/Epoll.h"
#include "../../general/base/PortableEndian.h"

using namespace CatchChallenger;

//[8(sequence number)+4(size)+1(request type)+8(random)+ED25519_SIGNATURE_SIZE+ED25519_KEY_SIZE+ED25519_SIGNATURE_SIZE]
char P2PTimerConnect::handShake1[];

P2PTimerConnect::P2PTimerConnect()
{
    setInterval(100);
    startTime=std::chrono::steady_clock::now();

    //[8(sequence number)+4(size)+1(request type)+8(random)+ED25519_SIGNATURE_SIZE+ED25519_KEY_SIZE+ED25519_SIGNATURE_SIZE]
    uint64_t sequencenumber=0;
    memcpy(handShake1,&sequencenumber,sizeof(sequencenumber));
    const uint32_t size=htole16(1+8);
    memcpy(handShake1+8,&size,sizeof(size));
    const uint8_t requestType=1;
    memcpy(handShake1+8+4,&requestType,sizeof(requestType));
    memset(handShake1+8+4+1,0,8+ED25519_SIGNATURE_SIZE);
    memcpy(handShake1+8+4+1+8+ED25519_SIGNATURE_SIZE,P2PServerUDP::p2pserver->getPublicKey(),ED25519_KEY_SIZE);
    memcpy(handShake1+8+4+1+8+ED25519_SIGNATURE_SIZE+ED25519_KEY_SIZE,P2PServerUDP::p2pserver->getCaSignature(),ED25519_SIGNATURE_SIZE);
}

void P2PTimerConnect::exec()
{
    if(P2PServerUDP::p2pserver->hostToConnect.empty())
        return;
    {
        const std::chrono::time_point<std::chrono::steady_clock> end=std::chrono::steady_clock::now();
        if(end<startTime)
            startTime=end;
        else if(std::chrono::duration_cast<std::chrono::milliseconds>(end - startTime).count()<5000 &&
                P2PServerUDP::p2pserver->hostToConnectIndex>=P2PServerUDP::p2pserver->hostToConnect.size())
            return;
    }
    if(P2PServerUDP::p2pserver->hostToConnectIndex>=P2PServerUDP::p2pserver->hostToConnect.size())
    {
        P2PServerUDP::p2pserver->hostToConnectIndex=0;
        startTime=std::chrono::steady_clock::now();
    }
    size_t lastScannedIndex=P2PServerUDP::p2pserver->hostToConnectIndex;
    do
    {
        P2PServerUDP::HostToConnect &peerToConnect=P2PServerUDP::p2pserver->hostToConnect.at(lastScannedIndex);
        lastScannedIndex++;
        if(lastScannedIndex>=P2PServerUDP::p2pserver->hostToConnect.size())
            lastScannedIndex=0;

        peerToConnect.round++;
        if(peerToConnect.round<3 || peerToConnect.round==13)
        {
            if(peerToConnect.round==13)
                peerToConnect.round=3;

            //[8(sequence number)+4(size)+1(request type)+8(random)+ED25519_SIGNATURE_SIZE+ED25519_KEY_SIZE+ED25519_SIGNATURE_SIZE]
            const int readSize=fread(handShake1+8+4+1,1,8,P2PServerUDP::p2pserver->ptr_random);
            if(readSize != 8)
                abort();
            P2PServerUDP::p2pserver->sign(8+4+1+8,reinterpret_cast<uint8_t *>(handShake1));
            P2PServerUDP::p2pserver->write(handShake1,sizeof(handShake1),peerToConnect.serv_addr);

            P2PServerUDP::p2pserver->hostToConnectIndex=lastScannedIndex;
            std::cout << "P2PTimerConnect::exec() try co" << std::endl;
            return;
        }
    } while(lastScannedIndex!=P2PServerUDP::p2pserver->hostToConnectIndex);
    P2PServerUDP::p2pserver->hostToConnectIndex=lastScannedIndex;

    std::cout << "P2PTimerConnect::exec()" << std::endl;
}