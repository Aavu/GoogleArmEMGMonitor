#ifndef FILESTREAMHANDLER_H
#define FILESTREAMHANDLER_H

#include <fstream>
#include <iostream>
#include <QObject>
#include <QTemporaryFile>
#include <QFile>
#include <QtDebug>
#include <QThread>

#include "RingBuffer.h"
#include "ErrorDef.h"

class TempFile: public QObject
{
    Q_OBJECT
public:
    TempFile() {
    }

    ~TempFile() {
        destroy();
    }

    Error_t create(size_t iBlockLength = 128, size_t iTotalChannels = 10) {
        m_iBlockLength = iBlockLength;
        m_iTotalChannels = iTotalChannels;

        destroy();
        m_pFile = new QTemporaryFile();
        if (!m_pFile->open())
            return kFileOpenError;
        m_filename = m_pFile->fileName();
        qDebug() << m_filename;

        m_piData = new uint16_t[m_iBlockLength * m_iTotalChannels];
        m_pWriteThread = QThread::create(&TempFile::writeThreadHandler, this);
        m_bNewDataInTemp = false;
        m_bInitialized = true;
        return kNoError;
    }

    void destroy() {
        delete m_pFile;
        m_pFile = nullptr;
        m_bNewDataInTemp = false;
        m_bInitialized = false;
    }

    Error_t copyToFile(const QString& filename, bool shouldDeleteTemp=true) {
        if (!QFile::copy(m_pFile->fileName(), filename))
            return kFileWriteError;

        m_bNewDataInTemp = false;
        if (shouldDeleteTemp)
            destroy();

        return kNoError;
    }

    Error_t write(uint16_t* piData, size_t iBlockLength, size_t iChannels) {
        if (!m_bInitialized)
            return kNotInitializedError;

        if (!m_pFile->isOpen())
            return kFileOpenError;

        char data[100];
        std::string tmp = "";
        for (size_t i=0; i < iBlockLength; i++) {
            tmp = "";
            for (size_t j=0; j < iChannels; j++) {
                sprintf(data, "%i\t", piData[(i*iChannels) + j]);
                tmp += data;
            }
            tmp += "\n";
            auto size = tmp.length()*sizeof(char);
            auto ret = m_pFile->write(tmp.c_str(), size);
            if (ret != (long)size)
                return kFileWriteError;
        }

        m_bNewDataInTemp = true;
        return kNoError;
    }

    Error_t startWriting() {
        if (!m_bInitialized || !m_pFileBuf)
            return kNotInitializedError;

        if (!m_pFile->isOpen())
            return kFileOpenError;

        m_ulNSamplesRecorded = 0;
        m_bRunning = true;
        if (!m_pWriteThread->isRunning())
            m_pWriteThread->start();

        return kNoError;
    }

    Error_t stopWriting() {
        m_bRunning = false;
        while(!m_pWriteThread->isFinished());
        m_ulNSamplesRecorded = 0;
        return kNoError;
    }


    bool isInitialized() { return m_bInitialized; }
    QString getFilename() { return m_filename; }
    bool hasUnSavedData() { return m_bNewDataInTemp; }

    void setFileBuffer(RingBuffer<uint16_t>* pBuf) {
        m_pFileBuf = pBuf;
    }

    void setCurrentSessionGesture(int iGesture) {
        if (iGesture > 0)
            m_iCurrentSessionGesture = iGesture;
    }

signals:
    void sig_nSamplesRecorded(qulonglong nSamples);

private:
    void writeThreadHandler() {
        while(m_bRunning) {
            if (!m_pFileBuf->isAvailable(m_iBlockLength))
                continue;

            char data[100];

            auto err = m_pFileBuf->pop(m_piData, m_iBlockLength);
            if (err != kNoError)
                return;

            for (size_t i=0; i < m_iBlockLength; i++) {
                // Replace any non zero value with the current gesture
                std::string tmp = (m_piData[(i*m_iTotalChannels)] != 0) ? std::to_string(m_iCurrentSessionGesture) + "\t" : "0\t";

                for (size_t j=1; j < m_iTotalChannels; j++) {
                    sprintf(data, "%i\t", m_piData[(i*m_iTotalChannels) + j]);
                    tmp += data;
                }
                tmp += "\n";
                auto size = tmp.length()*sizeof(char);
                auto ret = m_pFile->write(tmp.c_str(), size);
                if (ret != (long)size)
                    return;
            }

            m_ulNSamplesRecorded += m_iBlockLength;
            sig_nSamplesRecorded(m_ulNSamplesRecorded);
//            qDebug() << m_ulNSamplesRecorded;

//            if (++m_ulNSamplesRecorded % m_iBlockLength == 0) {
//                sig_nSamplesRecorded(m_ulNSamplesRecorded);
//                qDebug() << m_ulNSamplesRecorded;
//            }

            m_bNewDataInTemp = true;
        }
    }

    QTemporaryFile* m_pFile = nullptr;
    QString m_filename = "";

    size_t m_iBlockLength;
    size_t m_iTotalChannels;
    bool m_bInitialized = false;

    RingBuffer<uint16_t>* m_pFileBuf = nullptr;
    uint16_t* m_piData = nullptr;

    bool m_bNewDataInTemp = false;

    QThread* m_pWriteThread = nullptr;
    bool m_bRunning = false;

    int m_iCurrentSessionGesture = -1;

    static inline qulonglong m_ulNSamplesRecorded = 0;
};

#endif // FILESTREAMHANDLER_H
