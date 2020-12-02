//
// Created by Raghavasimhan Sankaranarayanan on 6/24/20.
//

#include "RingBuffer.h"

template<class T>
RingBuffer<T>::RingBuffer(  size_t iMaxSize,
                            size_t iNumChannels,
                            size_t iHopLength,
                            OverflowMode_t mode) :
                            m_iCapacity(int(iMaxSize * iNumChannels)),
                            m_iLength(0),
                            m_iChannels(iNumChannels),
                            m_iHopLength(iHopLength),
//                            m_pfDataBuffer(nullptr),
                            m_bufferState(Empty),
                            m_mode(mode)
{
    m_ptBuffer = new T [m_iCapacity];
    reset();
}

template<typename T>
RingBuffer<T>::~RingBuffer() {
    delete [] m_ptBuffer;
}

template<class T>
void RingBuffer<T>::setOverflowMode(RingBuffer::OverflowMode_t mode) {
    m_mode = mode;
}

template<class T>
bool RingBuffer<T>::isAvailable(size_t iNumSamples) {
    if (m_bufferState == Empty)
        return false;
//    std::cout << ((int)iNumSamples) << "\t" << m_iLength << std::endl;
    return ((int)(iNumSamples*m_iChannels)) <= m_iLength;
}

// length = 5; rid=0; wid=0;
// new val = 9;
/*  1    2   3   4   5  */
/*  0    1   2   3   4   */
// incIdx : rid=1

template<class T>
Error_t RingBuffer<T>::push(const T *pVal, size_t iNumSamples) {
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

template<class T>
Error_t RingBuffer<T>::pop(T *pVal, size_t iNumSamples, bool bUpdateIdx) {
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

template<class T>
void RingBuffer<T>::reset(bool setZero) {
    if (setZero)
        memset(m_ptBuffer, 0, sizeof(T) * m_iCapacity);

    m_iReadIdx  = 0;
    m_iWriteIdx = 0;
}

template<class T>
size_t RingBuffer<T>::getWriteIdx() {
    return m_iWriteIdx;
}

//template<class T>
//void RingBuffer<T>::setWriteIdx(int iIdx) {
//    incIdx(m_iWriteIdx, (iIdx - m_iWriteIdx), Write);
//}

template<class T>
size_t RingBuffer<T>::getReadIdx() {
    return m_iReadIdx;
}

//template<class T>
//void RingBuffer<T>::setReadIdx(int iIdx) {
//    incIdx(m_iReadIdx, (iIdx - m_iReadIdx), Read);
//}

template<class T>
size_t RingBuffer<T>::getLength() {
    return m_iLength;
}

template<class T>
size_t RingBuffer<T>::getCapacity() {
    return m_iCapacity;
}

template<class T>
size_t RingBuffer<T>::getNumSamplesInBuffer() {
    return m_iLength;
}

template<class T>
void RingBuffer<T>::print() {
//    std::cout << "\nsize: " << m_iCapacity << std::endl;
//    std::cout << "channels: " << m_iChannels << std::endl;
//    for (int i=0; i < m_iCapacity; i++) {
//        for (int j=0; j<m_iChannels; j++) {
//            std::cout << m_pptBuffer[i][j] << "\t";
//        }
//        std::cout << std::endl;
//    }
//    std::cout << "Current state: ";
//    if (m_bufferState == Full) {
//        std::cout << "Full";
//    } else if (m_bufferState == Empty) {
//        std::cout << "Empty";
//    } else {
//        std::cout << "Active";
//    }
//    std::cout << std::endl;
}

template<class T>
void RingBuffer<T>::incLength(int iBy) {
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

template<class T>
void RingBuffer<T>::copy(T *pDst, const T *pSrc, size_t iNumSamples) {
    memcpy(pDst, pSrc, sizeof(T) * iNumSamples * m_iChannels);
}

template<class T>
void RingBuffer<T>::incIdx(int &riIdx, Index_t type, int iOffset) {
//    std::cout << m_iCapacity << std::endl;
    iOffset *= m_iChannels;
    while ((riIdx + iOffset) < 0) {
        iOffset += m_iCapacity;
    }
    riIdx = (riIdx + iOffset) % m_iCapacity;

    incLength(type*iOffset);
}

template<class T>
typename RingBuffer<T>::State_t RingBuffer<T>::getCurrentState() {
    return m_bufferState;
}
