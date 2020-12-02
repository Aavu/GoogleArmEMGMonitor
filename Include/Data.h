//
// Created by Raghavasimhan Sankaranarayanan on 6/27/20.
//

#ifndef EMGDATASERVER_DATA_H
#define EMGDATASERVER_DATA_H

#include <utility>

#include "pch.h"

template <class T>
class Data_t {
public:
    Data_t(size_t iNumSamples, size_t iNumEMGChannels) :    m_iNumSamples(iNumSamples),
                                                            m_iNumEMGChannels(iNumEMGChannels),
                                                            m_iLength(iNumSamples * iNumEMGChannels)
    {
        m_ptEMGSamples = new T [m_iLength];
        m_bInitialized = true;
    }

    ~Data_t() {
        delete [] m_ptEMGSamples;
        m_bInitialized = false;
    }

    [[nodiscard]] bool isInitialized() const {
        return m_bInitialized;
    }

    T* getSamplesPointer() const {
        return m_ptEMGSamples;
    }

    [[nodiscard]] size_t getNumSamples() const {
        return m_iNumSamples;
    }

    [[nodiscard]] size_t getNumChannels() const {
        return m_iNumEMGChannels;
    }

    void setRMS(float fRMS) {
        m_fRMS = fRMS;
    }

    [[nodiscard]] float getRMS() const {
        return m_fRMS;
    }

    void setPrediction(int iPrediction) {
        m_iPrediction = iPrediction;
    }

    [[nodiscard]] int getPrediction() const {
        return m_iPrediction;
    }

    [[nodiscard]] size_t getLength() const {
        return m_iLength;
    }

    void setTimeStamp(std::string timeStamp) {
        m_sTimeStamp = std::move(timeStamp);
    }

    std::string getTimeStamp() {
        return m_sTimeStamp;
    }

private:
    bool m_bInitialized         = false;
    size_t m_iNumSamples        = 0;
    size_t m_iNumEMGChannels    = 0;
    T* m_ptEMGSamples           = nullptr;
    float m_fRMS                = 0;
    int m_iPrediction           = -1;
    size_t m_iLength            = 0;
    std::string m_sTimeStamp    = "";
};

#endif //EMGDATASERVER_DATA_H
