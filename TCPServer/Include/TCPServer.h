//
// Created by Raghavasimhan Sankaranarayanan on 7/15/20.
//

#ifndef EMGDATASERVER_TCPSERVER_H
#define EMGDATASERVER_TCPSERVER_H

#include "pch.h"
#include "MyTime.h"
#include "Logger.h"
#include "ErrorDef.h"

class TCPServer {
public:
    enum message_t {
        None = 0,
        Control = 1,
        Gain = 2
    };

    ~TCPServer();
    Error_t init(int port);
    Error_t start();
    [[nodiscard]] Error_t stop() const;

//    Error_t send(uint16_t* pEMGSamples, int iNumValues) const;
    Error_t send(uint16_t *pData_t, int iNumValues) const;

    Error_t receive(char* pMsg, int& size) const;

private:
    Error_t acceptClient();
    [[nodiscard]] Error_t setSocketOptions() const;
    [[nodiscard]] message_t receiveMsgType() const;

    int m_sockfd    = -1;
    int m_connfd    = -1;
    int m_iPort     = 8080;
    sockaddr_in m_serverAddress{}, m_cli{};
};


#endif //EMGDATASERVER_TCPSERVER_H
