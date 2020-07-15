#ifndef MF_PIPEIMPL_H_
#define MF_PIPEIMPL_H_

#include <string>
#include <vector>
#include <memory>

#include "MFTypes.h"
#include "MFPipe.h"
#include "url.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <unistd.h>

inline int operator < (const sockaddr_in &a, const sockaddr_in& b)
{
    if(a.sin_len!=b.sin_len)
        return a.sin_len<b.sin_len;
    if(a.sin_port!=b.sin_port)
        return a.sin_port<b.sin_port;
    if(a.sin_family!=b.sin_family)
        return a.sin_family<b.sin_family;
    if(a.sin_addr.s_addr!=b.sin_addr.s_addr)
        return a.sin_addr.s_addr<b.sin_addr.s_addr;
    return 0;
}
class MFPipeImpl: public MFPipe
{
    int udp_socket;
    enum MODE
    {
        CLIENT,SERVER
    };
    MODE mode;

    enum PKT_TYPE
    {
        HELLO,
        ELLOH,
        PACKET
    };
//    std::thread receiver;
    struct _mx {
        std::mutex mutex;
        std::map<sockaddr_in,time_t> customers;
        std::map<std::string/*channel*/,std::deque<REF_getter<MF_BASE_TYPE> > > incoming_packets;
    };
    _mx mx;
    std::thread pth;
public:
    MFPipeImpl():udp_socket(-1) {}
    ~MFPipeImpl()
    {
        close(udp_socket);
        udp_socket=-1;
        pth.join();
//        delete  pth;
    }

    HRESULT PipeInfoGet(/*[out]*/ std::string *pStrPipeName, /*[in]*/ const std::string &strChannel, MF_PIPE_INFO* _pPipeInfo) override
    {
        return E_NOTIMPL;
    }

    static void server_thread(void *p)
    {
        MFPipeImpl * _this=(MFPipeImpl*)p;
        char buf[0x10000];
        struct sockaddr_in from;
        socklen_t from_socklen=sizeof (sockaddr_in);
        while(1)
        {
            if(_this->udp_socket==-1)
            {
                printf("server thread exited\n");
                return;
            }
            int res=recvfrom(_this->udp_socket,buf,sizeof(buf),0,(struct sockaddr*)& from, &from_socklen);
            if(res!=-1)
            {
                inBuffer in(buf,res);
                PKT_TYPE pktType=(PKT_TYPE)in.get_PN();
                switch(pktType)
                {
                case HELLO:
                {
                    std::lock_guard<std::mutex> g(_this->mx.mutex);
                    if(!_this->mx.customers.count(from))
                    {
                        _this->mx.customers[from]=time(NULL);
                    }
                }
                {
                    outBuffer o;
                    o<<ELLOH;
                    auto buf=o.asString();
                    sendto(_this->udp_socket,buf->buffer,buf->size,0,(struct sockaddr*)& from, sizeof (struct sockaddr_in));
                }
                break;
                case PACKET:
                {
                    std::string channel=in.get_PSTR();
                    std::string objectType=in.get_PSTR();
                    if(objectType==typeid (MF_FRAME).name())
                    {
                        REF_getter<MF_BASE_TYPE> ptr=new MF_FRAME;
                        ptr->unpack(in);
                        {
                            std::lock_guard<std::mutex> g(_this->mx.mutex);
                            _this->mx.incoming_packets[channel].push_back(ptr);
                        }
                    }
                    else if(objectType==typeid (MF_BUFFER).name())
                    {
                        REF_getter<MF_BASE_TYPE> ptr=new MF_BUFFER;
                        ptr->unpack(in);
                        {
                            std::lock_guard<std::mutex> g(_this->mx.mutex);
                            _this->mx.incoming_packets[channel].push_back(ptr);
                        }
                    }
                    else throw std::runtime_error("invalid objectType 1");


                }
                break;
                default:
                    throw std::runtime_error("invalid pkt type 2");
                }

            }
        }
    }
    static void client_thread(void *p)
    {
        MFPipeImpl * _this=(MFPipeImpl*)p;
        while(1)
        {
            if(_this->udp_socket==-1)
            {
                printf("client thread exited\n");
                return;
            }

            char buf[0x10000];
            struct sockaddr_in from;
            socklen_t from_socklen=sizeof (sockaddr_in);
            int res=recvfrom(_this->udp_socket,buf,sizeof(buf),0,(struct sockaddr*)& from, &from_socklen);
            if(res!=-1)
            {
                inBuffer in(buf,res);
                PKT_TYPE pktType=(PKT_TYPE)in.get_PN();
                switch(pktType)
                {
                case ELLOH:
                    printf("client connected successfully\n");
                    break;
                case PACKET:
                {
                    std::string channel=in.get_PSTR();
                    std::string objectType=in.get_PSTR();
                    if(objectType==typeid (MF_FRAME).name())
                    {
                        REF_getter<MF_BASE_TYPE> ptr=new MF_FRAME;
                        ptr->unpack(in);
                        {
                            std::lock_guard<std::mutex> g(_this->mx.mutex);
                            _this->mx.incoming_packets[channel].push_back(ptr);
                        }
                    }
                    else if(objectType==typeid (MF_BUFFER).name())
                    {
                        REF_getter<MF_BASE_TYPE> ptr=new MF_BUFFER;
                        ptr->unpack(in);
                        {
                            std::lock_guard<std::mutex> g(_this->mx.mutex);
                            _this->mx.incoming_packets[channel].push_back(ptr);
                        }
                    }
                    else throw std::runtime_error("invalid objectType");


                }
                break;
                default:
                    throw std::runtime_error("invalid pkt type");
                }

            }
        }

    }

    HRESULT PipeCreate( /*[in]*/ const std::string &strPipeID, /*[in]*/ const std::string &strHints) override
    {
        Url u;
        u.parse(strPipeID);
        if(u.protocol=="udp")
        {
            udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
            int optval = 1;
            setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR,
                       (const void *)&optval, sizeof(int));
            struct sockaddr_in serveraddr;
            bzero((char *) &serveraddr, sizeof(serveraddr));
            serveraddr.sin_family = AF_INET;
            serveraddr.sin_addr.s_addr = inet_addr(u.host.c_str());
            serveraddr.sin_port = htons((unsigned short)atoi(u.port.c_str()));
            if (bind(udp_socket, (struct sockaddr *) &serveraddr,
                     sizeof(serveraddr)) < 0)
            {
                printf("ERROR on binding\n");
                return S_FALSE;
            }
            mode=SERVER;


            pth=std::thread(server_thread,this);

        }
        return S_OK;
    }

    HRESULT PipeOpen( /*[in]*/ const std::string &strPipeID, /*[in]*/ int _nMaxBuffers, /*[in]*/ const std::string &strHints) override
    {
        Url u;
        u.parse(strPipeID);
        if(u.protocol=="udp")
        {
            udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
            int optval = 1;
            setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR,
                       (const void *)&optval, sizeof(int));
            struct sockaddr_in serveraddr;
            bzero((char *) &serveraddr, sizeof(serveraddr));
            serveraddr.sin_family = AF_INET;
            serveraddr.sin_addr.s_addr = inet_addr(u.host.c_str());
            serveraddr.sin_port = htons((unsigned short)atoi(u.port.c_str()));
            if(connect(udp_socket,(struct sockaddr*)&serveraddr,sizeof(serveraddr))<0)
            {
                printf("connect error %s\n",strerror(errno));
                return S_FALSE;
            }
            outBuffer o;
            o<<HELLO;
            auto buf=o.asString();
            send(udp_socket,buf->buffer,buf->size,0);
            pth=std::thread(client_thread,this);
        }
        return S_OK;

    }

    HRESULT PipePut( /*[in]*/ const std::string &strChannel, /*[in]*/ const REF_getter<MF_BASE_TYPE> &pBufferOrFrame, /*[in]*/ int _nMaxWaitMs, /*[in]*/ const std::string &strHints) override
    {
        outBuffer out;
        out<<PACKET;
        out<<strChannel;
        pBufferOrFrame->pack(out);

        auto buf=out.asString();
        std::map<sockaddr_in,time_t> customers;
        {
            std::lock_guard<std::mutex> g(mx.mutex);
            customers=mx.customers;
        }
        for(auto & z: customers)
        {
            sendto(udp_socket,buf->buffer,buf->size,0,(struct sockaddr*)&z.first,sizeof (struct sockaddr_in));
        }

        return S_OK;
    }

    HRESULT PipeGet( /*[in]*/ const std::string &strChannel, /*[out]*/ REF_getter<MF_BASE_TYPE> &pBufferOrFrame, /*[in]*/ int _nMaxWaitMs, /*[in]*/ const std::string &strHints) override
    {
        std::lock_guard<std::mutex> g(mx.mutex);

        auto z=mx.incoming_packets.find(strChannel);
        if(z!=mx.incoming_packets.end())
        {
            if(z->second.size())
            {
                auto m=*z->second.begin();
                z->second.pop_front();
                pBufferOrFrame=m;
                return  S_OK;
            }
        }
        return S_FALSE;
    }

    HRESULT PipePeek( /*[in]*/ const std::string &strChannel, /*[in]*/ int _nIndex, /*[out]*/ REF_getter<MF_BASE_TYPE>& pBufferOrFrame, /*[in]*/ int _nMaxWaitMs, /*[in]*/ const std::string &strHints) override
    {
        return E_NOTIMPL;
    }

    HRESULT PipeMessagePut(
        /*[in]*/ const std::string &strChannel,
        /*[in]*/ const std::string &strEventName,
        /*[in]*/ const std::string &strEventParam,
        /*[in]*/ int _nMaxWaitMs) override
    {
        return E_NOTIMPL;
    }

    HRESULT PipeMessageGet(
        /*[in]*/ const std::string &strChannel,
        /*[out]*/ std::string *pStrEventName,
        /*[out]*/ std::string *pStrEventParam,
        /*[in]*/ int _nMaxWaitMs) override
    {
        return E_NOTIMPL;
    }

    HRESULT PipeFlush( /*[in]*/ const std::string &strChannel, /*[in]*/ eMFFlashFlags _eFlashFlags) override
    {
        return E_NOTIMPL;
    }

    HRESULT PipeClose() override
    {
        return E_NOTIMPL;
    }


};

#endif
