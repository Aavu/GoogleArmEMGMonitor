//
// Created by Raghavasimhan Sankaranarayanan on 7/29/20.
//

#ifndef EMGDATASERVER_KEYLOGGER_H
#define EMGDATASERVER_KEYLOGGER_H

#include "pch.h"
#include "Util.h"
#include "Logger.h"

// https://www.flipcode.com/archives/_kbhit_for_Linux.shtml
// https://faq.cprogramming.com/cgi-bin/smartfaq.cgi?answer=1042856625&id=1043284385

// https://android.googlesource.com/device/generic/brillo/+/d1917142dc905d808519023d80a664c066104600/examples/keyboard/keyboard_example.cpp

class KeyLogger {
public:
    enum keyPosition_t {
        Up = 0,
        Down = 1
    };

    struct keyState_t {
        uint16_t key = 0;
        keyPosition_t position = Up;
    };

    KeyLogger();
    ~KeyLogger();

    Error_t init();
    Error_t reset();

    Error_t track();
    Error_t stop();

    keyState_t getKeyState_t() { return m_keyState; }
    uint16_t getKeyState() const;

    void print(input_event* event);

private:
    static bool hasKeyEvents(int device_fd);
    static bool hasKey(int device_fd, unsigned int key);

    void threadCallBack();

    static const int kMaxDeviceName = 80;
    bool m_bInitialized = false;

    keyState_t m_keyState;

    const std::string m_inputDirectory = "/dev/input";
    std::vector<std::string> m_filenames;
    std::vector<pollfd> m_poll_fds;
    char m_deviceName[kMaxDeviceName];
    // Events accumulated for each device.
    std::vector<std::vector<input_event>> m_events;

    std::thread* m_pThread;
    bool m_bRunning = false;

    std::unordered_map<Util::states_t, int> keyStatesMap = {{Util::Pulse,           KEY_A},
                                                              {Util::Extension,     KEY_S},
                                                              {Util::Pronation,     KEY_D},
                                                              {Util::Supination,    KEY_F},
                                                              {Util::Fist,          KEY_G}};
};

#endif //EMGDATASERVER_KEYLOGGER_H
