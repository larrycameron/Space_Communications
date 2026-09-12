#include <iostream>
#include "Interstellar_Message_Packet.h"

int main()
{
    Interstellar_Message_Hub hub;

    Message_Packet packet;

    packet.source = NODE_EARTH;
    packet.destination = NODE_MARS;

    packet.ciphertext = {10, 20, 30, 40};
    packet.iv = {1, 2, 3, 4};
    packet.tag = {5, 6, 7, 8};

    bool result = hub.Send_Message(0, packet);

    std::cout << "Send Result: "
              << result
              << std::endl;

    return 0;
}