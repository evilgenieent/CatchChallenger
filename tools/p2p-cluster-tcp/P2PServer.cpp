#include "P2PServer.h"
#include <string>
#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "../../server/epoll/Epoll.h"
#include <cstring>

using namespace CatchChallenger;

P2PServer::P2PServer()
{
}

bool P2PServer::tryListen(const char * const host, const uint16_t &port)
{
    if(!tryListenInternal(host, std::to_string(port).c_str()))
        return false;
    return true;
}

bool P2PServer::tryListenInternal(const char* const ip,const char* const port)
{
    if(strlen(port)==0)
    {
        std::cout << "P2PServer::tryListenInternal() port can't be empty (abort)" << std::endl;
        abort();
    }

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;

    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* All interfaces */

    if(ip==NULL || ip[0]=='\0')
        s = getaddrinfo(NULL, port, &hints, &result);
    else
        s = getaddrinfo(ip, port, &hints, &result);
    if (s != 0)
    {
        std::cerr << "getaddrinfo:" << gai_strerror(s) << std::endl;
        return false;
    }

    close();
    rp = result;
    if(rp == NULL)
    {
        std::cerr << "rp == NULL, can't bind" << std::endl;
        return false;
    }
    unsigned int bindSuccess=0,bindFailed=0;
    while(rp != NULL)
    {
        const int sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sfd == -1)
        {
            std::cerr
                    << "unable to create the socket: familly: " << rp->ai_family
                    << ", rp->ai_socktype: " << rp->ai_socktype
                    << ", rp->ai_protocol: " << rp->ai_protocol
                    << ", rp->ai_addr: " << rp->ai_addr
                    << ", rp->ai_addrlen: " << rp->ai_addrlen
                    << std::endl;
            continue;
        }

        int one=1;
        if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one)!=0)
            std::cerr << "Unable to apply SO_REUSEADDR" << std::endl;
        one=1;
        if(setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof one)!=0)
            std::cerr << "Unable to apply SO_REUSEPORT" << std::endl;

        s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
        if(s!=0)
        {
            //unable to bind
            ::close(sfd);
            std::cerr
                    << "unable to bind: familly: " << rp->ai_family
                    << ", rp->ai_socktype: " << rp->ai_socktype
                    << ", rp->ai_protocol: " << rp->ai_protocol
                    << ", rp->ai_addr: " << rp->ai_addr
                    << ", rp->ai_addrlen: " << rp->ai_addrlen
                    << ", errno: " << std::to_string(errno)
                    << std::endl;
            bindFailed++;
        }
        else
        {
            if(sfd==-1)
            {
                std::cerr << "Leave without bind but s!=0" << std::endl;
                return false;
            }

            int flags = fcntl(sfd, F_GETFL, 0);
            if(flags == -1)
            {
                std::cerr << "fcntl get flags error" << std::endl;
                return false;
            }
            flags |= O_NONBLOCK;
            int s = fcntl(sfd, F_SETFL, flags);
            if(s == -1)
            {
                std::cerr << "fcntl set flags error" << std::endl;
                return false;
            }

            s = listen(sfd, SOMAXCONN);
            if(s == -1)
            {
                close();
                std::cerr << "Unable to listen" << std::endl;
                return false;
            }

            epoll_event event;
            event.data.ptr = this;
            event.events = EPOLLIN | EPOLLOUT | EPOLLET;
            s = Epoll::epoll.ctl(EPOLL_CTL_ADD, sfd, &event);
            if(s == -1)
            {
                close();
                std::cerr << "epoll_ctl error" << std::endl;
                return false;
            }
            sfd_list.push_back(sfd);

            std::cout
                    << "correctly bind: familly: " << rp->ai_family
                    << ", rp->ai_socktype: " << rp->ai_socktype
                    << ", rp->ai_protocol: " << rp->ai_protocol
                    << ", rp->ai_addr: " << rp->ai_addr
                    << ", rp->ai_addrlen: " << rp->ai_addrlen
                    << ", port: " << port
                    << ", rp->ai_flags: " << rp->ai_flags
                    //<< ", rp->ai_canonname: " << rp->ai_canonname-> corrupt the output
                    << std::endl;
            char hbuf[NI_MAXHOST];
            if(!getnameinfo(rp->ai_addr, rp->ai_addrlen, hbuf, sizeof(hbuf),NULL, 0, NI_NAMEREQD))
                std::cout << "getnameinfo: " << hbuf << std::endl;
            bindSuccess++;
        }
        rp = rp->ai_next;
    }
    freeaddrinfo (result);
    return bindSuccess>0;
}

void P2PServer::close()
{
    unsigned int index=0;
    while(index<sfd_list.size())
    {
        ::close(sfd_list.at(index));
        index++;
    }
    sfd_list.clear();
}

int P2PServer::accept(sockaddr *in_addr,socklen_t *in_len)
{
    unsigned int index=0;
    while(index<sfd_list.size())
    {
        const int &sfd=sfd_list.at(index);
        const int &returnValue=::accept(sfd, in_addr, in_len);
        if(returnValue!=-1)
            return returnValue;
        index++;
    }
    return -1;
}

P2PServer::EpollObjectType P2PServer::getType() const
{
    return P2PServer::EpollObjectType::ServerP2P;
}
