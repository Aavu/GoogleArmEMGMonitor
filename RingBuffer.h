#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <mutex>

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

    explicit RingBuffer(size_t iMaxSize = 1024,
                        size_t iNumChannels = 8,
                        size_t iHopLength = 1,
                        OverflowMode_t mode = Replace) :
                        m_iCapacity(int(iMaxSize * iNumChannels)),
                        m_iChannels(iNumChannels),
                        m_iHopLength(iHopLength),
                        m_mode(mode)
    {
        m_ptBuffer = new T [m_iCapacity];
        reset();
    }

    ~RingBuffer() {
        delete [] m_ptBuffer;
    }

    void setOverflowMode(OverflowMode_t mode) { m_mode = mode; }

    Error_t push(const T* pVal, size_t iNumSamples = 1) {
        std::lock_guard<std::mutex> lock(m_mtx);

        bool replace = false;
        if (m_iLength + ((int)iNumSamples * m_iChannels) > m_iCapacity) {
            switch (m_mode) {
                case Ignore:
                    return kBufferFullError;
                case Replace:
                    replace = true;
                    break;
                case RaiseError:
                    return kBufferFullError;
            }
        }

        int iNumValues2End = std::min((int)iNumSamples, (m_iCapacity - m_iWriteIdx)/m_iChannels);

        copy(&m_ptBuffer[m_iWriteIdx], pVal, iNumValues2End);

        if (iNumValues2End < (int) iNumSamples) {
            copy(m_ptBuffer, &pVal[iNumValues2End * m_iChannels], iNumSamples - iNumValues2End);
        }

        incIdx(m_iWriteIdx, Write, iNumSamples);

        // check if the buffer would be full and update readidx accordingly
        if (replace)
            m_iReadIdx = m_iWriteIdx;

        return kNoError;
    }

    Error_t pop(T* pVal, size_t iNumSamples = 1, bool bUpdateIdx = true) {
        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_bufferState == Empty) {
            return kBufferEmptyError;
        }

        if (!isAvailable(iNumSamples)) {
            return kNotEnoughSamplesError;
        }

        int iNumSamples2End = std::min((int)iNumSamples, (m_iCapacity - m_iReadIdx)/m_iChannels);
        copy(pVal, &m_ptBuffer[m_iReadIdx], iNumSamples2End);

        if (iNumSamples2End < (int)iNumSamples)
            copy(&pVal[iNumSamples2End], m_ptBuffer, iNumSamples - iNumSamples2End);

        if (bUpdateIdx)
            incIdx(m_iReadIdx, Read, m_iHopLength);

    //    print();
        return kNoError;
    }

    void reset(bool setZero = true) {
        if (setZero)
            memset(m_ptBuffer, 0, sizeof(T) * m_iCapacity);

        m_iReadIdx  = 0;
        m_iWriteIdx = 0;
    }

    bool isAvailable(size_t iNumSamples = 1) {
        if (m_bufferState == Empty)
            return false;
        return ((int)(iNumSamples*m_iChannels)) <= m_iLength;
    }

    size_t getWriteIdx() { return m_iWriteIdx; }
    size_t getReadIdx() { return m_iReadIdx; }
    size_t getLength() { return m_iLength; }
    size_t getCapacity() { return m_iCapacity; }
    size_t getNumSamplesInBuffer() { return m_iLength; }
    State_t getCurrentState() { return m_bufferState; }
    void print() {}

private:
    int m_iCapacity     = 0;
    int m_iLength       = 0;
    int m_iChannels     = 0;
    int m_iHopLength    = 0;
    T* m_ptBuffer       = nullptr;

    int m_iWriteIdx     = 0;
    int m_iReadIdx      = 0;

    std::mutex m_mtx;

    State_t m_bufferState = Empty;
    OverflowMode_t m_mode = Replace;

    void copy(T* pDst, const T* pSrc, size_t iNumSamples = 1) {
        memcpy(pDst, pSrc, sizeof(T) * iNumSamples * m_iChannels);
    }

    void incIdx(int& riIdx, Index_t type = Read, int iOffset = 1) {
        iOffset *= m_iChannels;
        while ((riIdx + iOffset) < 0) {
            iOffset += m_iCapacity;
        }
        riIdx = (riIdx + iOffset) % m_iCapacity;

        incLength(type*iOffset);
    }

    void incLength(int iBy = 1) {
        m_iLength = std::min(std::max(0, (m_iLength + iBy)), m_iCapacity);
        if (m_iLength == 0) {
            reset(false);
            m_bufferState = Empty;
        } else if (m_iLength == m_iCapacity) {
            m_bufferState = Full;
        } else {
            m_bufferState = Active;
        }
    }
};

#endif // RINGBUFFER_H
