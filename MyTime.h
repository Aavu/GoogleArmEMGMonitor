//#ifndef MYTIME_H
//#define MYTIME_H

//#include <chrono>
//#include <iostream>

//using namespace std::chrono;

//class Time {
//public:
//    union timeStamp_t {
//        uint64_t u64;
//        uint16_t u16[4];
//    };

//    static void getTimeStamp(timeStamp_t* pTimeStamp) {
//        pTimeStamp->u64 = getTimeStamp() - iProgramStartTime;
//    }

//    static uint64_t getTimeStamp() {
//        auto timeSinceEpoch = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
//        return timeSinceEpoch.count();
//    }

//    static void getTimeStamp(uint16_t* piTimeStamp) {
//        timeStamp_t ts;
//        getTimeStamp(&ts);
//        memcpy(piTimeStamp, &ts, sizeof(ts));
//        memset(&piTimeStamp[4], 0, sizeof(uint16_t)*4);
//    }

//    static uint64_t getTicks(const timeStamp_t* pTimeStamp) {
//        return pTimeStamp->u64;
//    }

//    static uint64_t getTicks(const uint16_t* piTimeStamp) {
//        timeStamp_t ts;
//        memcpy(&ts, piTimeStamp, sizeof (ts));
//        return getTicks(&ts);
//    }

//    static void convertToTimeStamp(timeStamp_t* pTimeStamp, const uint16_t* piTimeStamp) {
//        memcpy(pTimeStamp, piTimeStamp, sizeof (timeStamp_t));
//    }

//    static inline uint64_t iProgramStartTime = 0;

//};

//#endif // MYTIME_H
