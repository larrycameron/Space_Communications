#ifndef INTERSTELLAR_NETWORK_SOCKET_H
#define INTERSTELLAR_NETWORK_SOCKET_H

#include <cstdint>
#include <vector>

class Secure_Message_Encryption
{
private:
    void Handle_Errors();

public:
    void Encrypt(
        const std::vector<std::uint8_t>& plaintext,
        const std::vector<std::uint8_t>& key,
        const std::vector<std::uint8_t>& iv,
        std::vector<std::uint8_t>& ciphertext,
        std::vector<std::uint8_t>& tag);

    bool Decrypt(
        const std::vector<std::uint8_t>& ciphertext,
        const std::vector<std::uint8_t>& key,
        const std::vector<std::uint8_t>& iv,
        const std::vector<std::uint8_t>& tag,
        std::vector<std::uint8_t>& plaintext);
};

#endif