//
// Created by Raghavasimhan Sankaranarayanan on 7/29/20.
//

#include "KeyLogger.h"


KeyLogger::KeyLogger() :    m_bInitialized(false),
                            m_bRunning(false)
{
    init();
}

KeyLogger::~KeyLogger() {
    reset();
}

Error_t KeyLogger::init() {
    Util::listDirectory(m_inputDirectory, &m_filenames);
    LOG_TRACE("{} devices found in {}.", m_filenames.size(), m_inputDirectory);

    for (const auto& fName : m_filenames) {
        int fd = open(fName.c_str(), O_RDONLY);
        if (fd < 0) {
            LOG_ERROR("Failed to open {} for reading: {}", fName, strerror(errno));
            return kFileOpenError;
        }

        if(ioctl(fd, EVIOCGNAME(sizeof(m_deviceName) - 1), &m_deviceName) < 1) {
            m_deviceName[0] = '\0';
        }

        if (!hasKeyEvents(fd)) {
            LOG_TRACE("Discarding {} ({}): does not report any EV_KEY events.", fName, m_deviceName);
            continue;
        }

        if (!hasKey(fd, KEY_SPACE)) {
            LOG_TRACE("Discarding {} ({}): does not have a space key.", fName, m_deviceName);
            continue;
        }

        if (!hasKey(fd, KEY_ESC)) {
            LOG_TRACE("Discarding {} ({}): does not have a Esc key.", fName, m_deviceName);
            continue;
        }

        m_poll_fds.push_back(pollfd{fd, POLLIN, 0});
        LOG_TRACE("Adding device {}: {}", fName, m_deviceName);
    }

    if (m_poll_fds.empty()) {
        LOG_ERROR("No keyboard detected. Please connect a keyboard to the Arm and try again...");
        return kNotInitializedError;
    }

    m_events.reserve(m_poll_fds.size());
    m_bInitialized = true;
    return kNoError;
}

Error_t KeyLogger::reset() {
    m_bRunning = false;
    if (!m_bInitialized)
        return kNoError;

    if (m_pThread) {
        if (m_pThread->joinable())
            m_pThread->join();
    }
    delete m_pThread;
    m_pThread = nullptr;
    m_bInitialized = false;
    return kNoError;
}

bool KeyLogger::hasKeyEvents(int device_fd) {
    unsigned long evbit = 0;
    // Get the bit field of available event types.
    ioctl(device_fd, EVIOCGBIT(0, sizeof(evbit)), &evbit);
    return evbit & (1 << EV_KEY);
}

bool KeyLogger::hasKey(int device_fd, unsigned int key) {
    size_t nchar = KEY_MAX/8 + 1;
    unsigned char bits[nchar];
    // Get the bit fields of available keys.
    ioctl(device_fd, EVIOCGBIT(EV_KEY, sizeof(bits)), &bits);
    return bits[key/8] & (1 << (key % 8));
}

Error_t KeyLogger::track() {
    if (!m_bInitialized)
        return kNotInitializedError;

    m_bRunning = true;
    m_pThread = new std::thread(&KeyLogger::threadCallBack, this);

    return kNoError;
}

void KeyLogger::threadCallBack() {
//    static int count = 0; // For debugging
    while (m_bRunning) {
        // Wait for data to be available on one of the file descriptors without timeout (-1).
        auto ret = poll(m_poll_fds.data(), m_poll_fds.size(), 1000);
        if (ret < 0) {
            LOG_ERROR("Error while polling.");
            continue;
        } else if (ret == 0) {
            continue;
        }

        for (size_t i = 0; i < m_poll_fds.size(); i++) {
            if (m_poll_fds[i].revents & POLLIN) {
                input_event event;

                if (read(m_poll_fds[i].fd, &event, sizeof(event)) != sizeof(event)) {
                    LOG_ERROR("Failed to read an event");
                    break;
                }

                // When receiving a key event for space bar, add it to the list of events.
                // Don't process it yet as there might be other events in that report.
                if (event.type == EV_KEY) {
                    bool addToEvent = false;
                    for (const auto & [key, value] : keyStatesMap) {
                        if (event.code == value) {
                            addToEvent = true;
                            break;
                        }
                    }

                    if (event.code == KEY_ESC)
                        addToEvent = true;

                    if (addToEvent)
                        m_events[i].push_back(event);
                }

                // A SYN_REPORT event signals the end of a report, process all the
                // previously accumulated events.
                // At that point we only have events for B in the event vector.

                if (event.type == EV_SYN && event.code == SYN_REPORT) {
                    for (auto & it : m_events[i]) {
                        switch (it.value) {
                            case 0: {
                                // A value of 0 indicates a "key released" event.
                                if (it.code == KEY_ESC) {
                                    LOG_DEBUG("Recording stopped...");
                                    break;
                                }
                                m_keyState = {it.code, Up};
                                break;
                            }
                            case 1: // A value of 1 indicates a "key pressed" event.
                            case 2: {
                                if (it.code != KEY_ESC)
                                    m_keyState = {it.code, Down};
                                break;
                            }

                            default:
                                m_keyState = {0, Up};
                                break;
                        }
                    }
                    // Discard all processed events.
                    m_events[i].clear();
                }
            }
        }

    }
    LOG_TRACE("Terminating keyLogger thread...");
}

uint16_t KeyLogger::getKeyState() const {
    return m_keyState.key * m_keyState.position;
}

Error_t KeyLogger::stop() {
    return reset();
}

void KeyLogger::print(input_event* event) {
    std::cout   << (int)event->time.tv_sec   << "\t"
                << (int)event->time.tv_usec  << "\t"
                << (int)event->type          << "\t"
                << (int)event->code          << "\t"
                << (int)event->value         << std::endl;
}