#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QObject>
#include <QtNetwork>
#include "ErrorDef.h"

/**
 * @brief The TCPSocket class takes care of the TCP connection and receiving EMG data stream from the server
 */
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
    ~TCPSocket();

    /**
     * @brief Try Connecting to the server
     * @param host : hostname
     * @param port : port
     */
    void doConnect(const QString& host, uint16_t port);

    /**
     * @brief send a message to the server
     * @param pMsg : pointer to msg
     * @param size : size of msg
     * @return Error_t
     */
    Error_t send(const char* pMsg, qint64 size);

    /**
     * @brief send gain message to the server
     * @param iGain : gain
     * @return Error_t
     */
    Error_t send(uint16_t iGain);

    /**
     * @brief send Control message to the server
     * @param controlMsg : Control msg
     * @return Error_t
     */
    Error_t send(Message controlMsg);

    /**
     * @brief Disconnect from the server
     * @return Error_t
     */
    Error_t doDisconnect();

signals:

    /**
     * @brief signal when data is Available
     * @param data : The actual data as byte array
     */
    void dataAvailable(QByteArray data);

    /**
     * @brief signal to update UI
     * @param message : msg to update
     */
    void updateUI(QString message);

public slots:

    /**
     * @brief slot for connected
     */
    void connected();

    /**
     * @brief slot for disconnected
     */
    void disconnected();

    /**
     * @brief slot for num bytes Written
     * @param bytes : Number of bytes
     */
    void bytesWritten(qint64 bytes);

    /**
     * @brief slot for ready to Read
     */
    void readyRead();

private:
    QTcpSocket* m_pSocket;
    QByteArray m_buffer;
};

#endif // TCPSOCKET_H
