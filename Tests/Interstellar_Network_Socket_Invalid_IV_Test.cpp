#include "Interstellar_Network_Socket.h"

#include <cstdint>
#include <vector>

int main()
{
    Secure_Message_Encryption encryption;

    std::vector<std::uint8_t> plaintext =
    {
        'T', 'e', 's', 't'
    };

    std::vector<std::uint8_t> key(32, 0x01);
    std::vector<std::uint8_t> invalid_iv(11, 0x02);

    std::vector<std::uint8_t> ciphertext;
    std::vector<std::uint8_t> tag;

    encryption.Encrypt(
        plaintext,
        key,
        invalid_iv,
        ciphertext,
        tag);

    return 0;
}