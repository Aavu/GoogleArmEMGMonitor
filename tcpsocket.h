#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QObject>
#include <QtNetwork>
#include "ErrorDef.h"

class TCPSocket : public QObject
{
    Q_OBJECT
public:    
    enum Message {
        Stop = '0',
        Exit = '1',
        Start = '2',
    };

    explicit TCPSocket(QObject *parent = nullptr);

    void doConnect();

    Error_t send(const char* pMsg, qint64 size);
    Error_t send(uint16_t iGain);
    Error_t send(Message controlMsg);

signals:
    void dataAvailable(QByteArray data);
    void updateUI(QString message);

public slots:
    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();

private:
    QTcpSocket* m_pSocket;
    QByteArray m_buffer;
};

#endif // TCPSOCKET_H
