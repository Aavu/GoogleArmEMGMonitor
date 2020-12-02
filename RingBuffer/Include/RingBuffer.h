//
// Created by Raghavasimhan Sankaranarayanan on 6/24/20.
//

#ifndef GOOGLEDRUMMINGARM_RINGBUFFER_H
#define GOOGLEDRUMMINGARM_RINGBUFFER_H

#include "pch.h"

#include "Data.h"
#include "Logger.h"
#include "ErrorDef.h"

template<class T>
class RingBuffer {
public:
    enum OverflowMode_t {
        Ignore = 0,
        Replace,
        RaiseError
    };

    enum State_t {
        Empty = 0,
        Full,
        Active
    };

    enum Index_t {
        Read = -1,
        Write = 1
    };

    explicit RingBuffer(size_t iMaxSize = 1024, size_t iNumChannels = 8, size_t iHopLength = 1, OverflowMode_t mode = Replace);
    ~RingBuffer();

    void setOverflowMode(OverflowMode_t mode);

    Error_t push(const T* pVal, size_t iNumSamples = 1);
    Error_t pop(T* pVal, size_t iNumSamples = 1, bool bUpdateIdx = true);
    void reset(bool setZero = true);
    bool isAvailable(size_t iNumSamples = 1);
    size_t getWriteIdx();
//    void setWriteIdx(int iIdx);
    size_t getReadIdx();
//    void setReadIdx(int iIdx);
    size_t getLength();
    size_t getCapacity();
    size_t getNumSamplesInBuffer();
    State_t getCurrentState();
    void print();

private:
    int m_iCapacity;
    int m_iLength;
    int m_iChannels;
    int m_iHopLength;
    T* m_ptBuffer;
//    Data_t<float>* m_pfDataBuffer;

    int m_iWriteIdx             = 0;
    int m_iReadIdx              = 0;

    std::mutex m_mtx;

    State_t m_bufferState;
    OverflowMode_t m_mode;

    void copy(T* pDst, const T* pSrc, size_t iNumSamples = 1);
    void incIdx(int& riIdx, Index_t type = Read, int iOffset = 1);
    void incLength(int iBy = 1);
};

#endif //GOOGLEDRUMMINGARM_RINGBUFFER_H
