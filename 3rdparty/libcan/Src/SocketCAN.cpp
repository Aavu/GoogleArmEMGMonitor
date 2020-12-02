/**
 * @file
 * This file implements functions to receive
 * and transmit CAN frames via SocketCAN.
 */

#include <SocketCAN.h>

SocketCAN::SocketCAN()
   :CANAdapter(),
    receiver_thread_id(0),
    sockfd(-1)
{
    adapter_type = ADAPTER_SOCKETCAN;
    LOG_TRACE("SocketCAN adapter created.");
}


SocketCAN::~SocketCAN()
{
    if (this->is_open())
    {
        this->close();
    }
    LOG_TRACE("SocketCAN adapter Destroyed.");
}


void SocketCAN::open(char* interface)
{
    // Request a socket
    sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sockfd == -1)
    {
        LOG_ERROR("Unable to create a CAN socket.");
        return;
    }
    LOG_DEBUG("Created CAN socket with descriptor {0}.", sockfd);

    // Get the index of the network interface
    strncpy(if_request.ifr_name, interface, IFNAMSIZ);
    if (ioctl(sockfd, SIOCGIFINDEX, &if_request) == -1)
    {
        LOG_ERROR("Unable to select CAN interface {0}: I/O control error.", interface);

        // Invalidate unusable socket
        close();
        return;
    }
    LOG_DEBUG("Found: {0} has interface index {1}.", interface, if_request.ifr_ifindex);

    // Bind the socket to the network interface
    addr.can_family = AF_CAN;
    addr.can_ifindex = if_request.ifr_ifindex;
    int rc = bind(
        sockfd,
        reinterpret_cast<struct sockaddr*>(&addr),
        sizeof(addr)
    );
    if (rc == -1)
    {
        LOG_ERROR("Failed to bind socket to network interface");

        // Invalidate unusable socket
        close();
        return;
    }
    LOG_DEBUG("Successfully bound socket to interface {0}.", if_request.ifr_ifindex);

    // Start a separate, event-driven thread for frame reception
    start_receiver_thread();
}


void SocketCAN::close()
{
    terminate_receiver_thread = true;

    if (!is_open())
        return;

    // Close the file descriptor for our socket
    ::close(sockfd);
    sockfd = -1;

    LOG_TRACE("CAN socket destroyed");
}


bool SocketCAN::is_open() const
{
    return (sockfd != -1);
}


void SocketCAN::transmit(can_frame_t* frame)
{
    CANAdapter::transmit(frame);
    if (!is_open())
    {
        LOG_ERROR("Unable to transmit: Socket not open");
        return;
    }

    if (write(sockfd, frame, sizeof(*frame)) != sizeof(*frame)) {
        LOG_ERROR("CAN transmission error.");
    }
}


static void* socketcan_receiver_thread(void* argv)
{
    /*
     * The first and only argument to this function
     * is the pointer to the object, which started the thread.
     */
    auto* sock = (SocketCAN*) argv;

    // Holds the set of descriptors, that 'select' shall monitor
    fd_set descriptors;

    // Highest file descriptor in set
    int maxfd = sock->sockfd;

    // How long 'select' shall wait before returning with timeout
    struct timeval timeout;

    // Buffer to store incoming frame
    can_frame_t rx_frame;

    // Run until termination signal received
    while (!sock->terminate_receiver_thread)
    {
        // Clear descriptor set
        FD_ZERO(&descriptors);
        // Add socket descriptor
        FD_SET(sock->sockfd, &descriptors);
        LOG_TRACE("Added {0} to monitored descriptors.", sock->sockfd);

        // Set timeout
        timeout.tv_sec  = 1;
        timeout.tv_usec = 0;

        // Wait until timeout or activity on any descriptor
        if (select(maxfd+1, &descriptors, nullptr, nullptr, &timeout) == 1)
        {
            int len = read(sock->sockfd, &rx_frame, CAN_MTU);
            LOG_TRACE("Received {0} bytes: Frame from 0x{1:x}, DLC={2}", len, rx_frame.can_id, rx_frame.can_dlc);

            if (len < 0)
                continue;

            if (sock->reception_handler != nullptr)
            {
                LOG_TRACE("Invoking Callback...");
                sock->reception_handler(&rx_frame);
            }

            if (sock->parser != nullptr)
            {
                LOG_TRACE("Invoking parser...");
                sock->parser->parse_frame(&rx_frame);
            }
            else
            {
                LOG_TRACE("sock->parser is NULL");
            }
        }
        else
        {
            LOG_TRACE("Received nothing");
        }
    }

    LOG_DEBUG("Receiver thread terminated");

    // Thread terminates
    return nullptr;
}


void SocketCAN::start_receiver_thread()
{
    /*
     * Frame reception is accomplished in a separate, event-driven thread.
     *
     * See also: https://www.thegeekstuff.com/2012/04/create-threads-in-linux/
     */
    terminate_receiver_thread = false;
    int rc = pthread_create(&receiver_thread_id, nullptr, &socketcan_receiver_thread, this);
    if (rc != 0)
    {
        LOG_ERROR("Unable to run receiver thread");
        return;
    }
    LOG_DEBUG("Successfully started receiver thread with ID {0}.\n", (int) receiver_thread_id);
}