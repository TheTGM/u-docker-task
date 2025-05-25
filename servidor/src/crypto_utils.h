#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <string>
#include <vector>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>

class CryptoUtils {
public:
    // Generación de tokens dinámicos
    static std::string generateDynamicToken(const std::string& secretKey, const std::string& transactionId);
    
    // Validación de tokens
    static bool validateDynamicToken(const std::string& token, const std::string& secretKey, 
                                   const std::string& transactionId, int maxAgeSeconds = 30);
    
    // Cifrado AES-256-CBC
    static std::string encryptAES256(const std::string& plaintext, const std::string& key, 
                                   const std::string& iv);
    static std::string decryptAES256(const std::string& ciphertext, const std::string& key, 
                                   const std::string& iv);
    
    // Generación de HMAC-SHA256
    static std::string generateHMAC(const std::string& data, const std::string& key);
    static bool verifyHMAC(const std::string& data, const std::string& hmac, const std::string& key);
    
    // Utilidades generales
    static std::string generateRandomBytes(int length);
    static std::string generateUUID();
    static std::string getCurrentTimestamp();
    static long long getUnixTimestamp();
    static std::string bytesToHex(const unsigned char* bytes, int length);
    static std::vector<unsigned char> hexToBytes(const std::string& hex);
    static std::string sha256Hash(const std::string& input);
    
    // Validación de tiempo
    static bool isTimestampValid(long long timestamp, int maxAgeSeconds = 30);
    
    // Funciones de codificación Base64 (públicas)
    static std::string base64Encode(const std::vector<unsigned char>& data);
    static std::vector<unsigned char> base64Decode(const std::string& encoded);

private:
    static void handleOpenSSLErrors();
};

// Estructura para las transacciones
struct Transaction {
    std::string id;
    std::string timestamp;
    std::string type;
    double amount;
    std::string accountFrom;
    std::string accountTo;
    std::string serviceCode;
    std::string dynamicToken;
    std::string hmac;
    
    std::string toJson() const;
    static Transaction fromJson(const std::string& json);
    std::string serialize() const;
};

#endif // CRYPTO_UTILS_H