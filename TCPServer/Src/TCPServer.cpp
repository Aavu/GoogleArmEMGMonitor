//
// Created by Raghavasimhan Sankaranarayanan on 7/15/20.
//

#include "TCPServer.h"

Error_t TCPServer::init(int port) {
    m_iPort = port;
    bzero(&m_serverAddress, sizeof(m_serverAddress));
    m_serverAddress.sin_family = AF_INET;
    m_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    m_serverAddress.sin_port = htons(m_iPort);

    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockfd == -1) {
        LOG_ERROR("socket creation failed...");
        return kUnknownError;
    }
    LOG_DEBUG("Socket successfully created.");

    auto err = setSocketOptions();
    if (err != kNoError) {
        return err;
    }

    auto iErr = bind(m_sockfd, (sockaddr*)&m_serverAddress, sizeof(m_serverAddress));
    if (iErr != 0) {
        LOG_ERROR("socket bind failed... Err: {}", err);
        return kUnknownError;
    }
    LOG_DEBUG("Socket successfully binded.");

    return kNoError;
}

Error_t TCPServer::setSocketOptions() const {
    int opt = 1;
    if (setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        LOG_ERROR("set socket option (address) failed");
        return kUnknownError;
    }

    if(setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEPORT, (char *)&opt, sizeof(opt)) < 0) {
        LOG_ERROR("set socket option (port) failed");
        return kUnknownError;
    }

    /* This affects the accept function too. So not using it. */

//    struct timeval tv{1, 0};
//
//    if(setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) < 0) {
//        LOG_ERROR("set socket option (Receive timeout) failed");
//        return kUnknownError;
//    }

    return kNoError;
}

Error_t TCPServer::start() {
    if ((listen(m_sockfd, 5)) != 0) {
        LOG_ERROR("Listen failed...");
        return kUnknownError;
    }

    LOG_INFO("TCP Server listening.");
    return acceptClient();
}

//Error_t TCPServer::send(uint16_t *pEMGSamples, int iNumValues) const {
//    int nBytes = int(sizeof(uint16_t)) * iNumValues;
//    int ret = write(m_connfd, pEMGSamples, nBytes);
////    std::cout << "ret: " << ret << "\tsize: " << nBytes << std::endl;
//    if (ret != nBytes) {
//        LOG_ERROR("TCP connFD write error. {}", errno);
//        return kFileWriteError;
//    }
//
//    return kNoError;
//}

Error_t TCPServer::send(uint16_t *pData_t, int iNumValues) const {
    int nBytes = (int(sizeof(uint16_t)) * iNumValues); // + (int(sizeof(char)) * Time::timeStampSize);

//    auto ch = 10;
//    auto chunk = iNumValues / ch;
//
//    for (int i=0; i<chunk; i++) {
//        for(int j=0; j<ch; j++) {
//            std::cout << pData_t[(i*ch) + j] << "\t";
//        }
//        std::cout << std::endl;
//    }
//    std::cout << std::endl;

    int ret = write(m_connfd, pData_t, nBytes);
//    std::cout << "ret: " << ret << "\tsize: " << nBytes << std::endl << std::endl;
    if (ret != nBytes) {
        LOG_ERROR("TCP connFD write error. {}", errno);
        return kFileWriteError;
    }

    return kNoError;
}

TCPServer::~TCPServer() {
    stop();
}

Error_t TCPServer::acceptClient() {
    socklen_t len = sizeof(m_cli);
    m_connfd = accept(m_sockfd, (sockaddr*)&m_cli, &len); // blocks until there is a connection
    if (m_connfd < 0) {
        LOG_ERROR("Server accept failed.");
        return kUnknownError;
    }
    LOG_INFO("Client successfully connected to the server!");
    return kNoError;
}

Error_t TCPServer::receive(char* pMsg, int &size) const {
    size = receiveMsgType();
    if (size == None)
        return kFileEOFError;
    int ret = recv(m_connfd, pMsg, size, MSG_WAITALL);
    if (ret == -1)
    {
        LOG_ERROR("Server receive error.");
        return kFileReadError;
    }
    else if (ret == 0)
    {
        LOG_INFO("Peer closed connection.");
        return kFileEOFError;
    }
    else if (ret != (int)sizeof(char)*size)
    {
        LOG_WARN("Server received less bytes than requested");
    }
    return kNoError;
}

Error_t TCPServer::stop() const {
    auto ret = close(m_sockfd);
    if (ret != 0)
        return kFileCloseError;
    return kNoError;
}

TCPServer::message_t TCPServer::receiveMsgType() const {
    char msg;
    auto ret = recv(m_connfd, &msg, sizeof(char), MSG_WAITFORONE);
    if (ret == -1) {
        LOG_ERROR("Server receive error.");
        return None;
    } else if (ret == 0) {
        LOG_INFO("Peer closed connection.");
        return None;
    }

    switch (msg) {
        case 'c':
            return Control;
        case 'g':
            return Gain;
        default:
            return None;
    }
}
