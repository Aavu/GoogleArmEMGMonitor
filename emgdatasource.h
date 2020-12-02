#ifndef EMGDATASOURCE_H
#define EMGDATASOURCE_H

#include <QObject>
#include <QKeyEvent>
#include <iostream>
#include <QMutex>
#include <QMutexLocker>

#include "tcpsocket.h"
#include "RingBuffer.h"
#include "SshSession.h"
#include "MyTime.h"
#include "TempFile.h"

/**
 * @brief The EMGDataSource acts as the source of EMG data. This class also handles recording data to file with ground truth
 */
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

    /**
     * @brief EMGDataSource Constructor
     * @param refresh_ms : The refresh time in millis
     * @param parent : A pointer to the parent QObject
     */
    explicit EMGDataSource(size_t refresh_ms = 15, QObject *parent = nullptr);

    /**
     * @brief EMGDataSource Destructor
     */
    ~EMGDataSource();

    /**
     * @brief initialize the object
     * @param pTempFile : A pointer to the temp file to save the data
     */
    void init(TempFile* pTempFile);

    /**
     * @brief Get the poiinter of the current data buffer
     * @return Pointer to the data buffer
     */
    RingBuffer<uint16_t>* getDataBufferPtr();

    /**
     * @brief Start SSH Server
     * @param host : The hostname
     * @param port : The port
     * @return Error_t
     */
    Error_t sshStartServer(const std::string& host, uint16_t port);

    /**
     * @brief stop the data stream
     * @return Error_t
     */
    Error_t stop();

    /**
     * @brief set Recording On or  Off
     * @param bRec : True if On and false if Off
     */
    void setRecording(bool bRec);

    /**
     * @brief set the Sensor Gain
     * @param iGain : Gain value
     * @return Error_t
     */
    Error_t setSensorGain(uint16_t iGain);

    /**
     * @brief get current Sensor Gain
     * @return Gain value
     */
    uint16_t getSensorGain() { return m_iSensorGain; }

    /**
     * @brief start Recording
     */
    void startRecording();

    /**
     * @brief stop Recording
     */
    void stopRecording();

    /**
     * @brief set the Current Session Gesture
     * @param iGesture : The gesture id
     */
    void setCurrentSessionGesture(int iGesture);

signals:

    /**
     * @brief get Data
     * @param data : Pointer to data
     */
    void getData(uint16_t* data);

    /**
     * @brief signal to update UI
     * @param message : Msg string to update
     */
    void sig_updateUI(QString message);

    /**
     * @brief signal number of Samples Recorded
     * @param nSamplesRecorded : How many
     */
    void sig_nSamplesRecorded(qulonglong nSamplesRecorded);

private slots:

    /**
     * @brief data Available
     * @param data : The actual data
     */
    void dataAvailable(QByteArray data);

    /**
     * @brief slot for updating UI
     * @param message : the message string
     */
    void slot_updateUI(QString message);

    /**
     * @brief slot for number of Samples Recorded
     * @param nSamples : How many
     */
    void nSamplesRecorded(qulonglong nSamples);

private:
    /**
     * @brief Initialize the Socket
     * @param host : Hostname
     * @param port : Port
     * @return Error_t
     */
    Error_t initSocket(const QString& host, uint16_t port);

    SshSession* m_pSshSession = nullptr;
    TCPSocket* m_pSocket = nullptr;
    const uint16_t* m_pTcpData;
    uint16_t* m_pResidualData;
    uint16_t* m_pSendData;
    uint64_t* m_pTimeStamp;
    QString m_host;
    uint16_t m_iPort;
    size_t m_iRefreshTime; // Since frequency is 1KHz, time (ms) is the same as num samples

    qulonglong m_iNumSamplesRecorded = 0;

    bool m_bRecording = false;

    TempFile* m_pTempFile = nullptr;

    RingBuffer<uint16_t>* m_pBuffer     = nullptr;
    RingBuffer<uint16_t>* m_pFileBuffer = nullptr;

    // preferences
    uint16_t m_iSensorGain = 8;

    // Threads
    QMutex m_mtx;
};

#endif // EMGDATASOURCE_H
