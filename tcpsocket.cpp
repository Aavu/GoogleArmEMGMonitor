#include "tcpsocket.h"

TCPSocket::TCPSocket(QObject *parent) : QObject(parent)
{

}

TCPSocket::~TCPSocket() {
    doDisconnect();
    delete m_pSocket;
}

void TCPSocket::doConnect(const QString& host, uint16_t port) {
    m_pSocket = new QTcpSocket(this);

    connect(m_pSocket, SIGNAL(connected()),this, SLOT(connected()));
    connect(m_pSocket, SIGNAL(disconnected()),this, SLOT(disconnected()));
    connect(m_pSocket, SIGNAL(bytesWritten(qint64)),this, SLOT(bytesWritten(qint64)));
    connect(m_pSocket, SIGNAL(readyRead()),this, SLOT(readyRead()));

    qDebug() << "connecting...";

    // this is not blocking call
    m_pSocket->connectToHost(host, port); // 192.168.0.250   // 192.168.0.164

    // we need to wait...
    if(!m_pSocket->waitForConnected(5000))
    {
        qDebug() << "Error: " << m_pSocket->errorString();
    }
}

Error_t TCPSocket::doDisconnect() {
    if (m_pSocket->state() != QAbstractSocket::ConnectedState) {
        return kFileCloseError;
    }

    m_pSocket->disconnectFromHost();
    return kNoError;
}

void TCPSocket::connected()
{
    updateUI("connected...");
    qDebug() << "connected...";
}

void TCPSocket::disconnected()
{
    updateUI("disconnected...");
    qDebug() << "disconnected...";
}

void TCPSocket::bytesWritten(qint64 bytes)
{
    qDebug() << bytes << " bytes written...";
}

void TCPSocket::readyRead()
{
    dataAvailable(m_pSocket->readAll());
}

Error_t TCPSocket::send(const char* pMsg, qint64 size) {
    auto ret = m_pSocket->write(pMsg, size);
    if (ret != size)
        return kFileWriteError;

    return kNoError;
}

Error_t TCPSocket::send(uint16_t iGain) {
    const char msgType = 'g';
    auto ret = send(&msgType, 1);
    if (ret != kNoError)
        return ret;

    return send((const char*)&iGain, 2);
}

Error_t TCPSocket::send(Message controlMsg) {
    const char msgType = 'c';
    auto ret = send(&msgType, 1);
    if (ret != kNoError)
        return ret;

    return send((const char*)&controlMsg, 1);
}

