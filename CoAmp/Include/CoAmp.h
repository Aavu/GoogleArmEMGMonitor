//
// Created by Raghavasimhan Sankaranarayanan on 6/19/20.
//

#ifndef GOOGLEDRUMMINGARM_COAMP_H
#define GOOGLEDRUMMINGARM_COAMP_H

#include "pch.h"
#include "KeyLogger.h"
#include "MyTime.h"
#include "SocketCAN.h"
#include "Data.h"
#include "RingBuffer.h"
#include "Logger.h"
#include "ErrorDef.h"
#include "Util.h"


// Singleton class
class CoAmp {
public:
    struct SensorParam_t {
        size_t iGain            = DEFAULT_GAIN;
        size_t iWindowLength    = 128;
        size_t iHopLength       = 1;
    };

    static const uint8_t NUM_EMG_CHANNELS   = 8;
    static const uint8_t TOTAL_CHANNELS     = 8 + 1 + 1; // +2 for Index, key log
    static const uint8_t DEFAULT_GAIN       = 4;
    const uint16_t MAX_15                   = 1 << 15;  // Cannot use static variable with std::fill

    // Deleting Copy constructor
    CoAmp (const CoAmp&) = delete;
    CoAmp& operator= (const CoAmp&) = delete;

    ~CoAmp();
    static CoAmp* getInstance();
    Error_t init(SensorParam_t& sensorParam, KeyLogger* pKeyLogger);
    void reset();
    void allocMemory();
    void deAllocMemory();

    Error_t getData(Data_t<uint16_t>* piData);
    Error_t getData(Data_t<float>* pfData);

    Error_t getSamples(uint16_t* piSample, size_t iNumSamples = 1);
    Error_t getSamples(float* pfSample, size_t iNumSamples = 1);

    SensorParam_t getSensorParam();
    [[nodiscard]] bool isAvailable(size_t iNumSamples = 1) const;
    [[nodiscard]] int getNumAvailableSamples() const;

    KeyLogger* getKeyLoggerInstance() { return m_pKeyLogger; }
private:
    CoAmp();

    void transmitGainCANMsg(uint8_t gain);
    void transmitStartCANMsg();

    static void receptionHandler(can_frame_t* emgCANMsg);

    // helpers
    void setZero();
    void evalTime(can_frame_t* emgCANMsg);

    // variables
    static CoAmp* pInstance;
    SocketCAN* m_pCANBus;
    uint16_t* m_piEMGSample;
    uint16_t* m_piTempEMGSample;
    float* m_pfEMGSample;
    uint8_t m_updateIdx;
    uint16_t m_numSamples;
    bool m_dataAvailable;

    RingBuffer<uint16_t>* m_piBuffer;
    SensorParam_t m_sensorParam;
    KeyLogger* m_pKeyLogger;

    std::mutex m_mtx;
    std::condition_variable m_cv;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_timeNow = std::chrono::high_resolution_clock::now();
};

#endif //GOOGLEDRUMMINGARM_COAMP_H