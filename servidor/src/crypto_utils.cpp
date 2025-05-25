#include "crypto_utils.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>
#include <cstring>
#include <algorithm>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <openssl/err.h>

void CryptoUtils::handleOpenSSLErrors() {
    ERR_print_errors_fp(stderr);
}

std::string CryptoUtils::generateDynamicToken(const std::string& secretKey, const std::string& transactionId) {
    long long timestamp = getUnixTimestamp();
    std::string tokenData = std::to_string(timestamp) + secretKey + transactionId;
    std::string token = sha256Hash(tokenData);
    
    std::cout << "[DEBUG] Generando token con:" << std::endl;
    std::cout << "[DEBUG]   Timestamp: " << timestamp << std::endl;
    std::cout << "[DEBUG]   Secret Key: " << secretKey.substr(0, 10) << "..." << std::endl;
    std::cout << "[DEBUG]   Transaction ID: " << transactionId << std::endl;
    std::cout << "[DEBUG]   Token data: " << tokenData.substr(0, 50) << "..." << std::endl;
    std::cout << "[DEBUG]   Generated token: " << token.substr(0, 16) << "..." << std::endl;
    
    return token;
}

bool CryptoUtils::validateDynamicToken(const std::string& token, const std::string& secretKey, 
                                     const std::string& transactionId, int maxAgeSeconds) {
    long long currentTime = getUnixTimestamp();
    
    std::cout << "[DEBUG] Validando token: " << token.substr(0, 16) << "..." << std::endl;
    std::cout << "[DEBUG] Transaction ID: " << transactionId << std::endl;
    std::cout << "[DEBUG] Secret Key: " << secretKey.substr(0, 10) << "..." << std::endl;
    std::cout << "[DEBUG] Current time: " << currentTime << std::endl;
    
    for (int i = 0; i <= maxAgeSeconds; i++) {
        long long testTime = currentTime - i;
        std::string tokenData = std::to_string(testTime) + secretKey + transactionId;
        std::string expectedToken = sha256Hash(tokenData);
        
        if (i < 3) { // Solo mostrar los primeros 3 intentos para no spam
            std::cout << "[DEBUG] Probando tiempo " << testTime << " -> " << expectedToken.substr(0, 16) << "..." << std::endl;
        }
        
        if (token == expectedToken) {
            std::cout << "[DEBUG] Token válido encontrado con tiempo: " << testTime << std::endl;
            return true;
        }
    }
    
    std::cout << "[DEBUG] Token no válido después de " << maxAgeSeconds << " intentos" << std::endl;
    return false;
}

std::string CryptoUtils::encryptAES256(const std::string& plaintext, const std::string& key, 
                                     const std::string& iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        handleOpenSSLErrors();
        return "";
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, 
                          reinterpret_cast<const unsigned char*>(key.c_str()),
                          reinterpret_cast<const unsigned char*>(iv.c_str())) != 1) {
        handleOpenSSLErrors();
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    std::vector<unsigned char> ciphertext(plaintext.length() + 16); // AES block size
    int len;
    int ciphertext_len;

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                         reinterpret_cast<const unsigned char*>(plaintext.c_str()),
                         plaintext.length()) != 1) {
        handleOpenSSLErrors();
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
        handleOpenSSLErrors();
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    
    return base64Encode(std::vector<unsigned char>(ciphertext.begin(), ciphertext.begin() + ciphertext_len));
}

std::string CryptoUtils::decryptAES256(const std::string& ciphertext, const std::string& key, 
                                     const std::string& iv) {
    std::cout << "[DEBUG] Descifrado AES - Clave: " << key.length() << " bytes, IV: " << iv.length() << " bytes" << std::endl;
    
    std::vector<unsigned char> encrypted = base64Decode(ciphertext);
    if (encrypted.empty()) {
        std::cout << "[ERROR] Error al decodificar Base64 del texto cifrado" << std::endl;
        return "";
    }
    
    std::cout << "[DEBUG] Datos cifrados decodificados: " << encrypted.size() << " bytes" << std::endl;
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        handleOpenSSLErrors();
        return "";
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
                          reinterpret_cast<const unsigned char*>(key.c_str()),
                          reinterpret_cast<const unsigned char*>(iv.c_str())) != 1) {
        std::cout << "[ERROR] Error en EVP_DecryptInit_ex" << std::endl;
        handleOpenSSLErrors();
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    std::vector<unsigned char> plaintext(encrypted.size() + 16);
    int len;
    int plaintext_len;

    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, encrypted.data(), encrypted.size()) != 1) {
        std::cout << "[ERROR] Error en EVP_DecryptUpdate" << std::endl;
        handleOpenSSLErrors();
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
        std::cout << "[ERROR] Error en EVP_DecryptFinal_ex" << std::endl;
        handleOpenSSLErrors();
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    
    std::string result(reinterpret_cast<char*>(plaintext.data()), plaintext_len);
    std::cout << "[DEBUG] Descifrado exitoso: " << result.length() << " bytes" << std::endl;
    
    return result;
}

std::string CryptoUtils::generateHMAC(const std::string& data, const std::string& key) {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;
    
    unsigned char* result = HMAC(EVP_sha256(),
                                key.c_str(), key.length(),
                                reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
                                digest, &digest_len);
    
    if (!result) {
        handleOpenSSLErrors();
        return "";
    }
    
    return bytesToHex(digest, digest_len);
}

bool CryptoUtils::verifyHMAC(const std::string& data, const std::string& hmac, const std::string& key) {
    std::string computedHMAC = generateHMAC(data, key);
    return computedHMAC == hmac;
}

std::string CryptoUtils::generateRandomBytes(int length) {
    std::vector<unsigned char> buffer(length);
    if (RAND_bytes(buffer.data(), length) != 1) {
        handleOpenSSLErrors();
        return "";
    }
    return std::string(reinterpret_cast<char*>(buffer.data()), length);
}

std::string CryptoUtils::generateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    int i;
    ss << std::hex;
    for (i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4";
    for (i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen);
    for (i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (i = 0; i < 12; i++) {
        ss << dis(gen);
    }
    return ss.str();
}

std::string CryptoUtils::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
    return ss.str();
}

long long CryptoUtils::getUnixTimestamp() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string CryptoUtils::bytesToHex(const unsigned char* bytes, int length) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < length; i++) {
        ss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return ss.str();
}

std::vector<unsigned char> CryptoUtils::hexToBytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        unsigned char byte = static_cast<unsigned char>(
            std::strtol(hex.substr(i, 2).c_str(), nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

std::string CryptoUtils::sha256Hash(const std::string& input) {
    unsigned char hash[32]; // SHA256 produce 32 bytes
    
    // Usar EVP API moderno en lugar de SHA256_* deprecado
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        handleOpenSSLErrors();
        return "";
    }
    
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(mdctx);
        handleOpenSSLErrors();
        return "";
    }
    
    if (EVP_DigestUpdate(mdctx, input.c_str(), input.length()) != 1) {
        EVP_MD_CTX_free(mdctx);
        handleOpenSSLErrors();
        return "";
    }
    
    unsigned int md_len;
    if (EVP_DigestFinal_ex(mdctx, hash, &md_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        handleOpenSSLErrors();
        return "";
    }
    
    EVP_MD_CTX_free(mdctx);
    
    return bytesToHex(hash, md_len);
}

bool CryptoUtils::isTimestampValid(long long timestamp, int maxAgeSeconds) {
    long long currentTime = getUnixTimestamp();
    return (currentTime - timestamp) <= maxAgeSeconds;
}

// Base64 encoding/decoding simplificado
std::string CryptoUtils::base64Encode(const std::vector<unsigned char>& data) {
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for (size_t idx = 0; idx < data.size(); idx++) {
        char_array_3[i++] = data[idx];
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';
    }

    return ret;
}

std::vector<unsigned char> CryptoUtils::base64Decode(const std::string& encoded_string) {
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<unsigned char> ret;

    while (in_len-- && ( encoded_string[in] != '=') && 
           (isalnum(encoded_string[in]) || (encoded_string[in] == '+') || (encoded_string[in] == '/'))) {
        char_array_4[i++] = encoded_string[in]; in++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i) {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j <4; j++)
            char_array_4[j] = chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
    }

    return ret;
}

// Implementación de Transaction
std::string Transaction::toJson() const {
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"id\": \"" << id << "\",\n";
    ss << "  \"timestamp\": \"" << timestamp << "\",\n";
    ss << "  \"type\": \"" << type << "\",\n";
    ss << "  \"amount\": " << amount << ",\n";
    ss << "  \"account_from\": \"" << accountFrom << "\",\n";
    ss << "  \"account_to\": \"" << accountTo << "\",\n";
    ss << "  \"service_code\": \"" << serviceCode << "\",\n";
    ss << "  \"dynamic_token\": \"" << dynamicToken << "\",\n";
    ss << "  \"hmac\": \"" << hmac << "\"\n";
    ss << "}";
    return ss.str();
}

std::string Transaction::serialize() const {
    std::string result = id + "|" + timestamp + "|" + type + "|" + std::to_string(amount) + "|" + 
                        accountFrom + "|" + accountTo + "|" + serviceCode + "|" + dynamicToken + "|" + hmac;
    
    std::cout << "[DEBUG] Serializando transacción:" << std::endl;
    std::cout << "[DEBUG]   ID: '" << id << "'" << std::endl;
    std::cout << "[DEBUG]   Timestamp: '" << timestamp << "'" << std::endl;
    std::cout << "[DEBUG]   Type: '" << type << "'" << std::endl;
    std::cout << "[DEBUG]   Amount: " << amount << std::endl;
    std::cout << "[DEBUG]   Token: '" << dynamicToken.substr(0, 16) << "...'" << std::endl;
    std::cout << "[DEBUG] Resultado serializado: " << result.substr(0, 100) << "..." << std::endl;
    
    return result;
}

Transaction Transaction::fromJson(const std::string& json) {
    Transaction t;
    // Implementación básica de parsing JSON
    // Por simplicidad, aquí está una implementación básica
    
    if (json.find("\"id\"") != std::string::npos) {
        size_t start = json.find("\"id\": \"") + 7;
        size_t end = json.find("\"", start);
        if (start < json.length() && end != std::string::npos) {
            t.id = json.substr(start, end - start);
        }
    }
    
    return t;
}