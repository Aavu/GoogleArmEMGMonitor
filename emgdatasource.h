#ifndef EMGDATASOURCE_H
#define EMGDATASOURCE_H

#include <QObject>
#include <QKeyEvent>
#include <iostream>
#include <QMutex>

#include "tcpsocket.h"
#include "RingBuffer.h"
#include "SshSession.h"
#include "MyTime.h"
#include "TempFile.h"

class EMGDataSource : public QObject
{
    Q_OBJECT
public:
    static const uint8_t timeStampSize          = 8;
    static const size_t ui16_size               = sizeof (uint16_t);
    static const uint8_t iBlockLength           = 128;
    static const uint8_t iNumEMGChannels        = 8;
    static const uint8_t iTotalChannels         = iNumEMGChannels + 2;
    static const size_t iTotalChannelSize       = iTotalChannels * ui16_size;
    static const int iSampleRate                = 1000;
    static inline qulonglong iNumSamplesToRecord= 60000;

    struct data_t {
        uint16_t timeStamp[timeStampSize];
        uint16_t buffer[iBlockLength*iTotalChannels];
    };

    explicit EMGDataSource(const std::string& host = "googlearm.local", size_t refresh_ms = 15, QObject *parent = nullptr);
    ~EMGDataSource();
    void init(TempFile* pTempFile);
    void parseDate();
    RingBuffer<uint16_t>* getDataBufferPtr();   // Not a good practice. Dont give acces to private member variables outside the class
    RingBuffer<uint64_t>* getTimeBufferPtr();
    Error_t sshStartServer();
    Error_t stop();

    void setRecording(bool bRec);
    Error_t setSensorGain(uint16_t iGain);
    uint16_t getSensorGain() { return m_iSensorGain; }

    void startRecording();
    void stopRecording();

signals:
    void getData(uint16_t* data);
    void sig_updateUI(QString message);
    void sig_nSamplesRecorded(qulonglong nSamplesRecorded);

private slots:
    void dataAvailable(QByteArray data);
    void slot_updateUI(QString message);
    void nSamplesRecorded(qulonglong nSamples);

private:
    SshSession* m_pSshSession = nullptr;
    TCPSocket* m_pSocket;
    const uint16_t* m_pTcpData;
    uint16_t* m_pResidualData;
    uint16_t* m_pSendData;
    uint64_t* m_pTimeStamp;
    std::string m_host;
    size_t m_iRefreshTime; // Since frequency is 1KHz, time (ms) is the same as num samples

    qulonglong m_iNumSamplesRecorded = 0;

    bool m_bRecording = false;

    TempFile* m_pTempFile = nullptr;
    QTextStream* m_pOutStream = nullptr;

    RingBuffer<uint16_t>* m_pBuffer     = nullptr;
    RingBuffer<uint64_t>* m_pTimeBuffer = nullptr;
    RingBuffer<uint16_t>* m_pFileBuffer = nullptr;

    // preferences
    uint16_t m_iSensorGain = 2;

    QMutex m_mtx;
};

#endif // EMGDATASOURCE_H
