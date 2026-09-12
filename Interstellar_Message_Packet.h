#ifndef INTERSTELLAR_MESSAGE_PACKET_H
#define INTERSTELLAR_MESSAGE_PACKET_H


#include "Interstellar_Communications_Network.h"

#include <cstdint>
#include <vector>

struct Message_Packet
{
    Network_Node_IDs source;
    Network_Node_IDs destination;

    std::vector<std::uint8_t> ciphertext;
    std::vector<std::uint8_t> iv;
    std::vector<std::uint8_t> tag;
};

class Interstellar_Message_Hub
{
public:
   bool Send_Message(int connection, const Message_Packet& packet);



};
#endif
