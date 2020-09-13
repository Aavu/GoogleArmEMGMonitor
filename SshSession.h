#ifndef SSHSESSION_H
#define SSHSESSION_H

#include <QObject>
#define SSH_NO_CPP_EXCEPTIONS
#include <libssh/libsshpp.hpp>
#include <iostream>
#include <QDebug>
#include "ErrorDef.h"

class SshSession : public QObject {
    Q_OBJECT
public:
    static inline QString SERVER_PATH = "/home/pi/EMGDataServer/";

    SshSession() : m_pSession(new ssh::Session()) {}

    ~SshSession() {
        delete m_pSession;
    }

    Error_t doConnect(const std::string& host = "googlearm.local", const std::string& user = "pi", const std::string& password = "raspberry") {
        try {
                m_pSession->setOption(SSH_OPTIONS_HOST, host.c_str());
                m_pSession->setOption(SSH_OPTIONS_USER, user.c_str());
                m_pSession->connect();
                m_pSession->userauthPassword(password.c_str());
            } catch (const std::exception& e) {
                qDebug() << "Connect failed. " << e.what();
                return kUnknownError;
            }
        return kNoError;
    }

    Error_t disconnect() {
        try {
            m_pSession->disconnect();
        } catch (const std::exception& e) {
            qDebug() << "Disconnect failed. " << e.what();
            return kUnknownError;
        }
        return kNoError;
    }

    Error_t startServer() {
        // Kill the app or the server if already running.
        ssh::Channel channel(*m_pSession);
        char buffer[512];
        int nbytes;
        channel.openSession();
        std::string cmd = "cd " + SERVER_PATH.toStdString() + ";" + "./checkPID.sh; sleep .25; ./EMGDataServer";
        channel.requestExec(cmd.c_str());

        nbytes = ssh_channel_read(channel.getCChannel(), buffer, sizeof (buffer), 0);
        while (nbytes > 0) {
            if (std::string(buffer).find("listening") != std::string::npos) {
                break;
            }
            nbytes = ssh_channel_read(channel.getCChannel(), buffer, sizeof (buffer), 0);
        }

        if (nbytes < 0) {
            channel.close();
            qDebug() << "Failed to start server.";
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
