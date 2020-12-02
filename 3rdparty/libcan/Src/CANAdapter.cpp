
#include <CANAdapter.h>


CANAdapter::CANAdapter()
    :adapter_type(ADAPTER_NONE),
     reception_handler(nullptr),
     parser(nullptr)
{
    LOG_DEBUG("CAN adapter created.");
}


CANAdapter::~CANAdapter()
{
    LOG_DEBUG("Destroying CAN adapter...");
}


void CANAdapter::transmit(can_frame_t* frame)
{
    if (adapter_type == ADAPTER_NONE)
    {
        LOG_ERROR("Unable to send: Unspecified CAN adapter");
        return;
    }
    LOG_DEBUG("Transmitting frame with id : 0x{0:x}", frame->can_id);
}
