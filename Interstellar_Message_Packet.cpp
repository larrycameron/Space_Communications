#include "Interstellar_Message_Packet.h"

#include <iostream>

bool Is_Valid_Node_ID(Network_Node_IDs node)
{
    switch (node)
    {
        case NODE_0:
        case NODE_1:
        case NODE_2:
        case NODE_EARTH:
        case NODE_MOON:
        case NODE_Spacecraft:
        case NODE_MARS:
            return true;

        default:
            return false;
    }
}

bool Interstellar_Message_Hub::Send_Message(int connection, const Message_Packet& packet)
{
    if (connection != 0)
    {
        std::cout << "Connection Failed" << std::endl;
        return false;
    }

    if (packet.ciphertext.empty())
    {
        std::cout << "Invalid encrypted packet." << std::endl;
        return false;
    }

    if (packet.iv.empty())
    {
        std::cout << "Invalid IV." << std::endl;
        return false;
    }

    if (packet.tag.empty())
    {
        std::cout << "Invalid authentication tag." << std::endl;
        return false;
    }

    if (!Is_Valid_Node_ID(packet.source))
    {
        std::cout << "Invalid source node." << std::endl;
        return false;
    }

    if (!Is_Valid_Node_ID(packet.destination))
    {
        std::cout << "Invalid destination node." << std::endl;
        return false;
    }


    std::cout << "Connection Established" << std::endl;

    std::cout << "Source Node: " << static_cast<int>(packet.source) << std::endl;

    std::cout << "Destination Node: " << static_cast<int>(packet.destination) << std::endl;

    std::cout << "Encrypted Bytes Sent: " << packet.ciphertext.size() << std::endl;

    return true;
}