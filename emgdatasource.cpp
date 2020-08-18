#include "emgdatasource.h"

EMGDataSource::EMGDataSource(const std::string& host,
                             size_t refresh_ms,
                             QObject *parent) :
                            QObject(parent),
                            m_host(host),
                            m_iRefreshTime(refresh_ms)
{

}

EMGDataSource::~EMGDataSource() {
    delete m_pSshSession;
    delete m_pSocket;
    delete m_pBuffer;
    delete m_pFileBuffer;
    delete[] m_pSendData;
    delete[] m_pResidualData;
}

void EMGDataSource::init(TempFile* pTempFile) {
    m_pTempFile = pTempFile;
    connect(m_pTempFile, SIGNAL(sig_nSamplesRecorded(qulonglong)), this, SLOT(nSamplesRecorded(qulonglong)));

    m_pOutStream = new QTextStream((QIODevice*)m_pTempFile);
    //    m_pSshSession = new SshSession();
    //    sshStartServer();
    m_pSocket = new TCPSocket();
    m_pBuffer       = new RingBuffer<uint16_t>(10*iBlockLength, iTotalChannels, m_iRefreshTime);
    m_pFileBuffer   = new RingBuffer<uint16_t>(10*iBlockLength, iTotalChannels, iBlockLength);
    m_pSendData     = new uint16_t[m_iRefreshTime*iTotalChannels]; // Since frequency is 1KHz, time (ms) is the same as num samples
    m_pResidualData = new uint16_t[iTotalChannels];
    m_pTempFile->setFileBuffer(m_pFileBuffer);

    connect(m_pSocket, SIGNAL(updateUI(QString)), this, SLOT(slot_updateUI(QString)));
    connect(m_pSocket, SIGNAL(dataAvailable(QByteArray)), this, SLOT(dataAvailable(QByteArray)));
    m_pSocket->doConnect();
}

//void EMGDataSource::dataAvailable(QByteArray data) {
//    static auto r_headSize = 0;
//    static auto r_tailSize = 0;
//    static auto oneMore = 0;

//    m_pTcpData = (uint16_t*)(data.data());
//    auto err = kNoError;

//    // Get the head residuals first
////    memcpy(m_pResidualData + r_tailSize, m_pTcpData, r_headSize);
//    for (size_t i=0; i<r_headSize/ui16_size; i++) {
//        (m_pResidualData + r_tailSize)[i] = m_pTcpData[i];
//    }

//    // Print residual
//    if (m_bRecording) {
//        if (r_headSize || r_tailSize) {
//            std::cout << "\n1) residual head: " << r_headSize << "\t" << "2) residual tail: " << r_tailSize << "\n";
//            for (size_t i=0; i<iTotalChannels; i++) {
//                 std::cout << m_pResidualData[i] << "\t";
//            }
//            std::cout << std::endl;
//        }
//    }

//    // TODO : Copy the residual data to buffer
//    if (r_headSize + r_tailSize == iTotalChannelSize) {
//        err = m_pBuffer->push(m_pResidualData);
//        if (err != kNoError) {
//            qDebug() << "graph Buf Error: " << err;
//            return;
//        }

//        if (m_bRecording) {
//            err = m_pFileBuffer->push(m_pResidualData);
//            if (err != kNoError) {
//                qDebug() << "File buf Error: " << err;
//                return;
//            }
//        }

//        if (m_bRecording) {
//            std::cout << "residual total:\t";
//            for (size_t i=0; i<iTotalChannels; i++) {
//                 std::cout << m_pResidualData[i] << "\t";
//            }
//            std::cout << std::endl;
//        }
//    }

//    r_tailSize = (data.size() - r_headSize) % iTotalChannelSize;
//    auto writeSize = data.size() - r_headSize - r_tailSize;
//    auto writeLength = writeSize / iTotalChannelSize;

//    // Copy new data to graph buffer
//    err = m_pBuffer->push(m_pTcpData + r_headSize, writeLength);
//    if (err != kNoError) {
//        qDebug() << "graph Buf Error: " << err;
//        return;
//    }

//    // Copy new data to file buffer
//    if (m_bRecording) {
//        err = m_pFileBuffer->push(m_pTcpData + r_headSize, writeLength);
//        if (err != kNoError) {
//            qDebug() << "File buf Error: " << err;
//            return;
//        }
//    }

//    auto index = *(m_pTcpData + r_headSize);

//    r_headSize = (iTotalChannelSize - r_tailSize) % iTotalChannelSize;

//    // Get the tail residuals
////    memcpy(m_pResidualData, m_pTcpData + (data.size() - r_tailSize), r_tailSize);
//    for (size_t i=0; i<r_tailSize/ui16_size; i++) {
//        m_pResidualData[i] = (m_pTcpData + (data.size() - r_tailSize))[i];
//    }

//    // Print residual
//    if (m_bRecording) {
//        if (r_headSize || r_tailSize) {
//            std::cout << "\n2) residual head: " << r_headSize << "\t" << "2) residual tail: " << r_tailSize << "\n";
//            for (size_t i=0; i<iTotalChannels; i++) {
//                 std::cout << m_pResidualData[i] << "\t";
//            }
//            std::cout << std::endl;
//        }
//    }

//    if (m_bRecording) {
//        std::cout << "Index: " << index << "\tResidual head size: " << r_headSize << "\tResidual tail size: " << r_tailSize << "\tData size: " << data.size() << "\twrite size: " << writeSize << std::endl;
//        if (r_tailSize > 0 || oneMore == 1) {
//            for (size_t i=0; i < data.size() / ui16_size; i++) {
//                if (i % 10 == 0)
//                    std::cout << std::endl;
//                std::cout << m_pTcpData[i] << "\t";
//            }
//            std::cout << "\n" << std::endl;
//            oneMore++;
//        }
//    }
//}

void EMGDataSource::dataAvailable(QByteArray data) {
    m_mtx.lock();
    static size_t r_headSize    = 0; static size_t r_headIdx    = 0;
    static size_t r_tailSize    = 0; static size_t r_tailIdx    = 0;
    static size_t tcpDataSize   = 0; static size_t tcpDataLength = 0;
    static size_t writeSize     = 0;
//    static auto oneMore = 0;

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

//    auto index = m_pTcpData[r_headIdx];

    // Push any residual data to buffers
    if (r_headSize) {
        err = m_pBuffer->push(m_pResidualData);
        if (err != kNoError) {
            qDebug() << "graph Buf residual write Error: " << err;
            goto error;
        }

        if (m_bRecording) {
            err = m_pFileBuffer->push(m_pResidualData);
            if (err != kNoError) {
                qDebug() << "graph Buf residual write Error: " << err;
                goto error;
            }
        }
    }

//    // Print residual
//    if (m_bRecording) {
//        if (r_headSize || r_tailSize) {
//            std::cout << "\nresidual head: " << r_headSize << "\t" << "residual tail: " << r_tailSize << "\n";
//            for (size_t i=0; i<iTotalChannels; i++) {
//                 std::cout << m_pResidualData[i] << "\t";
//            }
//            std::cout << std::endl;
//        }
//    }

    // Push current data chunk to buffers
    err = m_pBuffer->push(&m_pTcpData[r_headIdx], writeSize/iTotalChannelSize);
    if (err != kNoError) {
        qDebug() << "graph Buf write Error: " << err;
        goto error;
    }

    if (m_bRecording) {
        err = m_pFileBuffer->push(&m_pTcpData[r_headIdx], writeSize/iTotalChannelSize);
        if (err != kNoError) {
            qDebug() << "graph Buf write Error: " << err;
            goto error;
        }
    }

//    if (m_bRecording) {
//        std::cout << "Index: " << index << "\tResidual head size: " << r_headSize << "\tResidual tail size: " << r_tailSize << "\tData size: " << data.size() << "\twrite size: " << writeSize << std::endl;
//        if (r_tailSize > 0 || (oneMore > 0 && oneMore < 3)) {
//            for (size_t i=0; i < data.size() / ui16_size; i++) {
//                if (i % 10 == 0)
//                    std::cout << std::endl;
//                std::cout << m_pTcpData[i] << "\t";
//            }
//            std::cout << "\n" << std::endl;
//            oneMore++;
//        }
//    }


error:
    m_mtx.unlock();
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

RingBuffer<uint64_t>* EMGDataSource::getTimeBufferPtr() {
    return m_pTimeBuffer;
}

Error_t EMGDataSource::sshStartServer() {
    if (!m_pSshSession)
        return kNotInitializedError;

    m_pSshSession->connect(m_host);
    // TODO: Error handling
    m_pSshSession->startServer();
    m_pSshSession->disconnect();
    return kNoError;
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
