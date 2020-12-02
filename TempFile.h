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

/**
 * @brief The TempFile class is rresponsible for handling the temp files that are used for recording EMG data
 */
class TempFile: public QObject
{
    Q_OBJECT
public:
    /**
     * @brief TempFile Constructor
     */
    TempFile() {}

    /**
     * @brief TempFile Destructor
     */
    ~TempFile() {
        destroy();
    }

    /**
     * @brief create a tempfile
     * @param iBlockLength : The block size
     * @param iTotalChannels : Number of channels
     * @return Error_t
     */
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

    /**
     * @brief destroy the tempfile if any
     */
    void destroy() {
        delete m_pFile;
        m_pFile = nullptr;
        m_bNewDataInTemp = false;
        m_bInitialized = false;
    }

    /**
     * @brief copy the contents of the temp file to an actual File
     * @param filename : Path of the actual file
     * @param shouldDeleteTemp : Whether to delete temp or not
     * @return Error_t
     */
    Error_t copyToFile(const QString& filename, bool shouldDeleteTemp=true) {
        if (!QFile::copy(m_pFile->fileName(), filename))
            return kFileWriteError;

        m_bNewDataInTemp = false;
        if (shouldDeleteTemp)
            destroy();

        return kNoError;
    }

    /**
     * @brief write data to temp file
     * @param piData : pointer to the data buffer
     * @param iBlockLength : the blocksize
     * @param iChannels : number of channels
     * @return Error_t
     */
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

    /**
     * @brief start Writing to file in a seperate thread
     * @return Error_t
     */
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

    /**
     * @brief stop Writing file
     * @return Error_t
     */
    Error_t stopWriting() {
        m_bRunning = false;
        while(m_pWriteThread->isRunning());
        m_ulNSamplesRecorded = 0;
        return kNoError;
    }


    //Getters and setters
    bool isInitialized() { return m_bInitialized; }
    QString getFilename() { return m_filename; }
    bool hasUnSavedData() { return m_bNewDataInTemp; }

    void setFileBuffer(RingBuffer<uint16_t>* pBuf) { m_pFileBuf = pBuf;}

    void setCurrentSessionGesture(int iGesture) {
        if (iGesture > 0)
            m_iCurrentSessionGesture = iGesture;
    }

signals:
    /**
     * @brief signal for number of Samples Recorded
     * @param nSamples
     */
    void sig_nSamplesRecorded(qulonglong nSamples);

private:
    /**
     * @brief The callback for writing thread. It is called when startWriting is called and stopped when stopWriting is called
     */
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
    std::atomic<bool> m_bRunning = false;

    int m_iCurrentSessionGesture = -1;

    static inline qulonglong m_ulNSamplesRecorded = 0;
};

#endif // FILESTREAMHANDLER_H
