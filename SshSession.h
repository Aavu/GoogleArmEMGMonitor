#ifndef SSHSESSION_H
#define SSHSESSION_H

#include <QObject>
#define SSH_NO_CPP_EXCEPTIONS
#include <libssh/libsshpp.hpp>
#include <iostream>
#include "ErrorDef.h"

class SshSession : public QObject {
    Q_OBJECT
public:
    SshSession() : m_pSession(new ssh::Session()) {}

    ~SshSession() {
        delete m_pSession;
    }

    Error_t connect(std::string host, std::string user = "pi", std::string password = "raspberry") {

        try {
                m_pSession->setOption(SSH_OPTIONS_HOST, host.c_str());
                m_pSession->setOption(SSH_OPTIONS_USER, user.c_str());
                m_pSession->connect();
                m_pSession->userauthPassword(password.c_str());
            } catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
                return kUnknownError;
            }
        return kNoError;
    }

    Error_t disconnect() {
        try {
            m_pSession->disconnect();
        } catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
            return kUnknownError;
        }
        return kNoError;
    }

    Error_t startServer() {
        ssh::Channel channel(*m_pSession);
        char buffer[256];
        int nbytes;
        channel.openSession();
        channel.requestExec("cd /home/pi/Documents/ClionProjects/EMGDataServer/cmake-build-debug-pi; ./EMGDataServer");
        nbytes = ssh_channel_read(channel.getCChannel(), buffer, sizeof(buffer), 0);
        while (nbytes > 0) {
//            std::cout << "SshMsg:\t" << std::string(buffer) << std::endl;
            if (std::string(buffer).find("listening") != std::string::npos) {
//                std::cout << "Ready!" << '\n';
                break;
            }
            nbytes = ssh_channel_read(channel.getCChannel(), buffer, sizeof(buffer), 0);
        }

        if (nbytes < 0) {
            channel.close();
            return kUnknownError;
        }

        channel.sendEof();
        channel.close();

        return kNoError;
    }

private:
    ssh::Session* m_pSession = nullptr;
};

#endif // SSHSESSION_H
