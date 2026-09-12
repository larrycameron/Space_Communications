#include "Interstellar_Network_Socket.h"

#include <cstdlib>
#include <iostream>

#include <openssl/evp.h>

void Secure_Message_Encryption::Handle_Errors()
{
    std::cerr << "A cryptographic error occurred." << std::endl;
    std::exit(1);
}

void Secure_Message_Encryption::Encrypt(const std::vector<std::uint8_t>& plaintext,const std::vector<std::uint8_t>& key,const std::vector<std::uint8_t>& iv,std::vector<std::uint8_t>& ciphertext, std::vector<std::uint8_t>& tag)
{
        // AES-256 key must contain 32 bytes.
    // The GCM IV must contain 12 bytes.
    if (key.size() != 32 || iv.size() != 12)
    {
        Handle_Errors();
    }

    // Create the OpenSSL encryption context.
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (!ctx)
    {
        Handle_Errors();
    }

    // Select AES-256-GCM.
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr))
    {
        EVP_CIPHER_CTX_free(ctx);
        Handle_Errors();
    }

    // Set the IV length.
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN,  static_cast<int>(iv.size()), nullptr))
    {
        EVP_CIPHER_CTX_free(ctx);
        Handle_Errors();
    }

    // Give OpenSSL the key and IV.
    if (1 != EVP_EncryptInit_ex(ctx, nullptr,  nullptr, key.data(),  iv.data()))
    {
        EVP_CIPHER_CTX_free(ctx);
        Handle_Errors();
    }

    // Reserve space for the ciphertext.
    ciphertext.resize(plaintext.size() + EVP_MAX_BLOCK_LENGTH);

    int len = 0;

    // Encrypt the plaintext.
    if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), static_cast<int>(plaintext.size())))
    {
        EVP_CIPHER_CTX_free(ctx);
        Handle_Errors();
    }

    int ciphertext_length = len;
    int finalize_length = 0;

    // Finalize the encryption.
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + ciphertext_length,  &finalize_length))
    {
        EVP_CIPHER_CTX_free(ctx);
        Handle_Errors();
    }

    ciphertext_length += finalize_length;
    ciphertext.resize(ciphertext_length);

    // Retrieve the 16-byte authentication tag.
    tag.resize(16);

    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG,  static_cast<int>(tag.size()), tag.data()))
    {
        EVP_CIPHER_CTX_free(ctx);
        Handle_Errors();
    }

    // Release the temporary OpenSSL context.
    EVP_CIPHER_CTX_free(ctx);
}    

bool Secure_Message_Encryption::Decrypt(const std::vector<std::uint8_t>& ciphertext, const std::vector<std::uint8_t>& key,const std::vector<std::uint8_t>& iv,const std::vector<std::uint8_t>& tag, std::vector<std::uint8_t>& plaintext)
{
    
        if (key.size() != 32 || iv.size() != 12 || tag.size() != 16)
        {
            return false;
        }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (!ctx)
    {
        return false;
    }

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr))
    {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(iv.size()), nullptr))
    {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    if (1 != EVP_DecryptInit_ex(ctx,nullptr,nullptr,key.data(),iv.data()))
    {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    plaintext.resize(ciphertext.size());

    int len = 0;

    if (1 != EVP_DecryptUpdate(ctx,plaintext.data(),&len,ciphertext.data(),static_cast<int>(ciphertext.size())))
    {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    int plaintext_length = len;

    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, static_cast<int>(tag.size()), const_cast<std::uint8_t*>(tag.data())))
    {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    int finalize_length = 0;

    int result = EVP_DecryptFinal_ex(ctx, plaintext.data() + plaintext_length,  &finalize_length);

    EVP_CIPHER_CTX_free(ctx);

    if (result > 0)
    {
        plaintext_length += finalize_length;
        plaintext.resize(plaintext_length);
        return true;
    }

    plaintext.clear();
    return false;
}
    

