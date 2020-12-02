//
// Created by Raghavasimhan Sankaranarayanan on 7/16/20.
//

#ifndef EMGDATASERVER_MYTIME_H
#define EMGDATASERVER_MYTIME_H

#include <chrono>
#include <iostream>
#include <bitset>

using namespace std::chrono;
using namespace std::chrono_literals;


class Time {
public:
    union timeStamp_t {
        uint64_t u64;
        uint16_t u16[4];
    };

    [[maybe_unused]] static void sleep(int seconds) {
        std::this_thread::sleep_for(::seconds(seconds));
    }

    [[maybe_unused]] static void msleep(int milli) {
        std::this_thread::sleep_for(::milliseconds(milli));
    }

    [[maybe_unused]] static void usleep(int micro) {
        std::this_thread::sleep_for(::microseconds(micro));
    }

    static void getTimeStamp(uint16_t* piTimeStamp) {
        auto timeSinceEpoch = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
        timeStamp_t ts{};
        ts.u64 = timeSinceEpoch.count();
//        std::cout << std::bitset<64>(ts.u64) << std::endl;
        memcpy(piTimeStamp, &ts, sizeof(ts));
        memset(&piTimeStamp[4], 0, sizeof(uint16_t)*4);
    }

};
#endif //EMGDATASERVER_MYTIME_H
