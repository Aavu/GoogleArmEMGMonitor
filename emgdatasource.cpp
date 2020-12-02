#include "emgdatasource.h"

EMGDataSource::EMGDataSource(size_t refresh_ms,
                             QObject *parent) :
                            QObject(parent),
                            m_pSshSession(nullptr),
                            m_pSocket(nullptr),
                            m_iRefreshTime(refresh_ms)
{
}

EMGDataSource::~EMGDataSource() {
    delete m_pSshSession;
    delete m_pBuffer;
    delete m_pFileBuffer;
    delete[] m_pSendData;
    delete[] m_pResidualData;
    delete m_pSocket;
}

void EMGDataSource::init(TempFile* pTempFile) {
    m_pTempFile = pTempFile;
    connect(m_pTempFile, SIGNAL(sig_nSamplesRecorded(qulonglong)), this, SLOT(nSamplesRecorded(qulonglong)));

    m_pBuffer       = new RingBuffer<uint16_t>(10*iBlockLength, iTotalChannels, m_iRefreshTime);
    m_pFileBuffer   = new RingBuffer<uint16_t>(10*iBlockLength, iTotalChannels, iBlockLength);
    m_pSendData     = new uint16_t[m_iRefreshTime*iTotalChannels]; // Since frequency is 1KHz, time (ms) is the same as num samples
    m_pResidualData = new uint16_t[iTotalChannels];
    m_pTempFile->setFileBuffer(m_pFileBuffer);
}

Error_t EMGDataSource::initSocket(const QString& host, uint16_t port) {
    m_host = host;
    m_iPort = port;

    if (m_pSocket) {
        m_pSocket->doDisconnect();
        delete m_pSocket;
        m_pSocket = nullptr;
    }

    m_pSocket = new TCPSocket();

    connect(m_pSocket, SIGNAL(updateUI(QString)), this, SLOT(slot_updateUI(QString)));
    connect(m_pSocket, SIGNAL(dataAvailable(QByteArray)), this, SLOT(dataAvailable(QByteArray)));
    m_pSocket->doConnect(m_host, m_iPort);

    return kNoError;
}

void EMGDataSource::dataAvailable(QByteArray data) {
    QMutexLocker locker(&m_mtx);
    static size_t r_headSize    = 0; static size_t r_headIdx    = 0;
    static size_t r_tailSize    = 0; static size_t r_tailIdx    = 0;
    static size_t tcpDataSize   = 0; static size_t tcpDataLength = 0;
    static size_t writeSize     = 0;

    auto err = kNoError;

    // get tail residuals from prev data chunk
    memcpy(m_pResidualData, &m_pTcpData[tcpDataLength - r_tailIdx], r_tailSize);

    // get current headsize from prev tail size
    r_headSize = (iTotalChannelSize - r_tailSize) % iTotalChannelSize;
    r_headIdx = r_headSize / ui16_size;

    // Init current data chunk info
    m_pTcpData = (const uint16_t*) data.constData();
    tcpDataSize = data.size();
    tcpDataLength = tcpDataSize / ui16_size;

    // get the head residual from current chunk
    memcpy(&m_pResidualData[r_tailIdx], m_pTcpData, r_headSize);

    r_tailSize = (tcpDataSize - r_headSize) % iTotalChannelSize;
    r_tailIdx = r_tailSize / ui16_size;
    writeSize = tcpDataSize - r_headSize - r_tailSize;

    // Push any residual data to buffers
    if (r_headSize) {
        err = m_pBuffer->push(m_pResidualData);
        if (err != kNoError) {
            qDebug() << "graph Buf residual write Error: " << err;
            return;
        }

        if (m_bRecording) {
            err = m_pFileBuffer->push(m_pResidualData);
            if (err != kNoError) {
                qDebug() << "graph Buf residual write Error: " << err;
                return;
            }
        }
    }

    // Push current data chunk to buffers
    err = m_pBuffer->push(&m_pTcpData[r_headIdx], writeSize/iTotalChannelSize);
    if (err != kNoError) {
        qDebug() << "graph Buf write Error: " << err;
        return;
    }

    if (m_bRecording) {
        err = m_pFileBuffer->push(&m_pTcpData[r_headIdx], writeSize/iTotalChannelSize);
        if (err != kNoError) {
            qDebug() << "graph Buf write Error: " << err;
            return;
        }
    }
}


void EMGDataSource::startRecording() {
    m_pTempFile->startWriting();
}

void EMGDataSource::stopRecording() {
    m_pTempFile->stopWriting();
}

void EMGDataSource::nSamplesRecorded(qulonglong nSamples) {
//    qDebug() << nSamples;
    sig_nSamplesRecorded(nSamples);
}

RingBuffer<uint16_t>* EMGDataSource::getDataBufferPtr() {
    return m_pBuffer;
}

Error_t EMGDataSource::sshStartServer(const std::string& host, uint16_t port) {
    auto err = kNoError;

    m_pSshSession = new SshSession();

    err = m_pSshSession->doConnect(host);
    if (err != kNoError) {
        delete m_pSshSession;
        m_pSshSession = nullptr;
        return err;
    }

    err = m_pSshSession->startServer();
    if (err != kNoError) {
        m_pSshSession->disconnect();
        delete m_pSshSession;
        m_pSshSession = nullptr;
        return err;
    }

    m_pSshSession->disconnect();
    delete m_pSshSession;
    m_pSshSession = nullptr;

    return initSocket(QString(host.c_str()), port);
}

void EMGDataSource::slot_updateUI(QString message) {
    sig_updateUI(message);
}

Error_t EMGDataSource::stop() {
    return m_pSocket->send(TCPSocket::Exit);
}

void EMGDataSource::setRecording(bool bRec) {
    if (m_bRecording != bRec) {
        m_bRecording = bRec;

        if (m_bRecording) {
            startRecording();
        } else {
            stopRecording();
        }
    }

}

Error_t EMGDataSource::setSensorGain(uint16_t iGain) {
    m_iSensorGain = iGain;
    return m_pSocket->send(iGain);
}

void EMGDataSource::setCurrentSessionGesture(int iGesture) {
    if (m_pTempFile)
        m_pTempFile->setCurrentSessionGesture(iGesture);
}
