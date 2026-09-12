#include "Interstellar_Network_Socket.h"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

bool Vectors_Equal(
    const std::vector<std::uint8_t>& a,
    const std::vector<std::uint8_t>& b)
{
    return a == b;
}

void Print_Test_Result(
    const std::string& test_name,
    bool passed)
{
    std::cout << test_name
              << ": "
              << (passed ? "PASS" : "FAIL")
              << '\n';
}

int main()
{
    Secure_Message_Encryption encryption;

    // =========================================================
    // ENC-021 — AES-256-GCM Known-Answer Test
    // =========================================================

    std::vector<std::uint8_t> kat_key(32, 0x00);
    std::vector<std::uint8_t> kat_iv(12, 0x00);
    std::vector<std::uint8_t> kat_plaintext(16, 0x00);

    std::vector<std::uint8_t> expected_ciphertext =
    {
        0xce, 0xa7, 0x40, 0x3d,
        0x4d, 0x60, 0x6b, 0x6e,
        0x07, 0x4e, 0xc5, 0xd3,
        0xba, 0xf3, 0x9d, 0x18
    };

    std::vector<std::uint8_t> expected_tag =
    {
        0xd0, 0xd1, 0xc8, 0xa7,
        0x99, 0x99, 0x6b, 0xf0,
        0x26, 0x5b, 0x98, 0xb5,
        0xd4, 0x8a, 0xb9, 0x19
    };

    std::vector<std::uint8_t> kat_ciphertext;
    std::vector<std::uint8_t> kat_tag;

    encryption.Encrypt(
        kat_plaintext,
        kat_key,
        kat_iv,
        kat_ciphertext,
        kat_tag);

    bool test21 =
        Vectors_Equal(kat_ciphertext, expected_ciphertext) &&
        Vectors_Equal(kat_tag, expected_tag);

    Print_Test_Result(
        "ENC-021 AES-256-GCM Known-Answer Test",
        test21);

    return test21 ? 0 : 1;
}