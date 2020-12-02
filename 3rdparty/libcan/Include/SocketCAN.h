/**
 * @file
 * This file declares an interface to SocketCAN,
 * to facilitates frame transmission and reception.
 */

#if (!defined(SOCKETCAN_H)) && (!defined(MINGW))
#define SOCKETCAN_H

#include <CANAdapter.h>
#include <CANFrame.h>
#include <CANFrameParser.h>

// IFNAMSIZ, ifreq
#include <net/if.h>
// Multi-threading
#include <pthread.h>
#include <iostream>
#include <cstdio>
// strncpy
#include <cstring>
// close
#include <unistd.h>
// socket
#include <sys/socket.h>
// SIOCGIFINDEX
#include <sys/ioctl.h>

#include "Logger.h"

/**
 * Interface request structure used for socket ioctl's
 */
typedef struct ifreq interface_request_t;

/**
 * Socket address type for CAN sockets
 */
typedef struct sockaddr_can can_socket_address_t;


/**
 * Facilitates frame transmission and reception via a CAN adapter
 */
class SocketCAN: public CANAdapter
{
  private:
    interface_request_t if_request;

    can_socket_address_t addr;

    pthread_t receiver_thread_id;

  public:
    /**
     * CAN socket file descriptor
     */
    int sockfd = -1;

    /**
     * Request for the child thread to terminate
     */
    bool terminate_receiver_thread = false;

    /** Constructor */
    SocketCAN();
    /** Destructor */
    ~SocketCAN() override;

    /**
     * Open and bind socket
     */
    void open(char*);

    /**
     * Close and unbind socket
     */
    void close();

    /**
     * Returns whether the socket is open or closed
     *
     * @retval true     Socket is open
     * @retval false    Socket is closed
     */
    bool is_open() const;

    /**
     * Sends the referenced frame to the bus
     */
    void transmit(can_frame_t*) override;

    /**
     * Starts a new thread, that will wait for socket events
     */
    void start_receiver_thread();
};

#endif
