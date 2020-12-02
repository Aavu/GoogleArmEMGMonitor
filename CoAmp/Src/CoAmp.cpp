//
// Created by Raghavasimhan Sankaranarayanan on 6/19/20.
//

#include "CoAmp.h"

CoAmp* CoAmp::pInstance = nullptr;

CoAmp::CoAmp() : m_pCANBus(nullptr),
                 m_piEMGSample(nullptr),
                 m_piTempEMGSample(nullptr),
                 m_pfEMGSample(nullptr),
                 m_updateIdx(0),
                 m_numSamples(0),
                 m_dataAvailable(false),
                 m_piBuffer(nullptr)
{
    LOG_TRACE("Created CoAmp instance");
}

CoAmp::~CoAmp() {
    reset();
    deAllocMemory();
    LOG_TRACE("Destroyed CoAmp instance");
}

CoAmp* CoAmp::getInstance() {
    if (pInstance == nullptr)
        pInstance = new CoAmp();

    return pInstance;
}

Error_t CoAmp::init(SensorParam_t& sensorParam, KeyLogger* pKeyLogger) {
    m_pKeyLogger = pKeyLogger;
    auto err = m_pKeyLogger->track();
    if (err != kNoError)
        return err;

    m_pCANBus = new SocketCAN();
    m_pCANBus->open((char*)"can0");

    m_sensorParam = sensorParam;
    allocMemory();

    m_pCANBus->reception_handler = &CoAmp::receptionHandler;
    sleep(1);

    if (!m_pCANBus->is_open()) {
        LOG_ERROR("Can bus open failed.");
        deAllocMemory();
        return kFileOpenError;
    }

    transmitGainCANMsg(m_sensorParam.iGain);
    transmitStartCANMsg();

    return kNoError;
}

void CoAmp::reset() {
    m_pCANBus->close();
    m_pKeyLogger->reset();
}

void CoAmp::allocMemory() {
    auto iWindowLength = m_sensorParam.iWindowLength;

    m_piEMGSample     = new uint16_t [TOTAL_CHANNELS];
    m_piTempEMGSample  = new uint16_t [iWindowLength * TOTAL_CHANNELS];

    m_pfEMGSample      = new float [TOTAL_CHANNELS];
    m_piBuffer         = new RingBuffer<uint16_t>(iWindowLength, TOTAL_CHANNELS, m_sensorParam.iHopLength, RingBuffer<uint16_t>::Replace);

    setZero();
}

void CoAmp::deAllocMemory() {
    delete m_pCANBus;
    delete [] m_piEMGSample;
    delete [] m_piTempEMGSample;

    delete [] m_pfEMGSample;
    delete m_piBuffer;
}

void CoAmp::transmitGainCANMsg(uint8_t gain) {
    LOG_INFO("Transmitting Sensor Gain...");
    for (int i=0; i<NUM_EMG_CHANNELS + 2; i++) {
        can_frame_t gainCANMsg;
        gainCANMsg.can_id = 0x150;
        gainCANMsg.can_dlc = 5;

        gainCANMsg.data[0] = 0x04;

        switch(i)
        {
            case 0:
                gainCANMsg.data[1] = 0x02;
                gainCANMsg.data[2] = 0xFE;
                gainCANMsg.data[3] = 0x03;
                gainCANMsg.data[4] = 0xE8;
                break;

            case 1:
                gainCANMsg.data[1] = 0x03;
                gainCANMsg.data[2] = 0xFE;
                gainCANMsg.data[3] = 0x00;
                gainCANMsg.data[4] = 0x10;
                break;

            default:
                gainCANMsg.data[1] = 0x01;
                gainCANMsg.data[2] = i - 1;     // Channel index
                gainCANMsg.data[3] = 0x00;
                gainCANMsg.data[4] = gain;      // Channel gain
                break;
        }

        m_pCANBus->transmit(&gainCANMsg);
    }
}

void CoAmp::transmitStartCANMsg() {
    can_frame_t startCANMsg;

    startCANMsg.can_id = 0x150;
    startCANMsg.can_dlc = 2;
    startCANMsg.data[0] = 0x50;
    startCANMsg.data[1] = 0x01;

    m_pCANBus->transmit(&startCANMsg);
}

void CoAmp::receptionHandler(can_frame_t *emgCANMsg) {
//    pInstance->evalTime(emgCANMsg);
    auto* pSample = pInstance->m_piEMGSample;
    switch (emgCANMsg->can_id) {
        case 0x451:
            pSample[1] = ((uint16_t)emgCANMsg->data[1] << 8) | emgCANMsg->data[2];
            pSample[2] = ((uint16_t)emgCANMsg->data[3] << 8) | emgCANMsg->data[4];
            pSample[3] = ((uint16_t)emgCANMsg->data[5] << 8) | emgCANMsg->data[6];
            break;
        case 0x452:
            pSample[4] = ((uint16_t)emgCANMsg->data[1] << 8) | emgCANMsg->data[2];
            pSample[5] = ((uint16_t)emgCANMsg->data[3] << 8) | emgCANMsg->data[4];
            pSample[6] = ((uint16_t)emgCANMsg->data[5] << 8) | emgCANMsg->data[6];
            break;
        case 0x453: {
            pSample[7] = ((uint16_t) emgCANMsg->data[1] << 8) | emgCANMsg->data[2];
            pSample[8] = ((uint16_t) emgCANMsg->data[3] << 8) | emgCANMsg->data[4];

            if (pInstance->m_pKeyLogger)
                pSample[0] = pInstance->m_pKeyLogger->getKeyState();

            pSample[9] = ++pInstance->m_numSamples;
            pInstance->m_piBuffer->push(pSample);

//            for (uint8_t i=0; i<TOTAL_CHANNELS; i++) {
//                std::cout << pSample[i] << "\t";
//            }
//            std::cout << std::endl;
            break;
        }
        default:
            LOG_DEBUG("Unknown can ID: {}\tLength: {}", emgCANMsg->can_id, emgCANMsg->can_dlc);
    }
}

Error_t CoAmp::getSamples(uint16_t *piSample, size_t iNumSamples) {
    return m_piBuffer->pop(piSample, iNumSamples);
}

Error_t CoAmp::getSamples(float *pfSample, size_t iNumSamples) {
    auto err = getSamples(m_piTempEMGSample, iNumSamples);
    if (err != kNoError)
        return err;

    // TODO: handle index and key log separately
    for (size_t i=0; i<iNumSamples * TOTAL_CHANNELS; i++) {
        pfSample[i] = float(m_piTempEMGSample[i] - MAX_15) / MAX_15;
    }

    return kNoError;
}

Error_t CoAmp::getData(Data_t<uint16_t> *piData) {
    if (!piData->isInitialized())
        return kNotInitializedError;
    auto* piSamples = piData->getSamplesPointer();
    auto n = piData->getNumSamples();
//    piData->setTimeStamp(Time::getTimeStamp()); // Not the best place to put. But a good approximation
    return getSamples(piSamples, n);
}

Error_t CoAmp::getData(Data_t<float> *pfData) {
    if (!pfData->isInitialized())
        return kNotInitializedError;
    auto* pfSamples = pfData->getSamplesPointer();
    auto n = pfData->getNumSamples();
//    pfData->setTimeStamp(Time::getTimeStamp()); // Not the best place to put. But a good approximation
    return getSamples(pfSamples, n);
}

void CoAmp::setZero() {
    std::fill(m_piEMGSample, &m_piEMGSample[TOTAL_CHANNELS], MAX_15);
    std::fill(m_piTempEMGSample, &m_piTempEMGSample[m_sensorParam.iWindowLength * TOTAL_CHANNELS], MAX_15);
}

void CoAmp::evalTime(can_frame_t* emgCANMsg) {
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_timeNow);
    m_timeNow = std::chrono::high_resolution_clock::now();

    std::cout << "Time diff: " << diff.count() << "\t Msg ID: " << emgCANMsg->can_id << std::endl;
}

bool CoAmp::isAvailable(size_t iNumSamples) const {
    return m_piBuffer->isAvailable(iNumSamples);
}

CoAmp::SensorParam_t CoAmp::getSensorParam() {
    return m_sensorParam;
}

int CoAmp::getNumAvailableSamples() const {
    return m_piBuffer->getLength();
}
