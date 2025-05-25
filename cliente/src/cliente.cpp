#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include "crypto_utils.h"

class TransactionClient {
private:
    std::string serverHost;
    int serverPort;
    std::string secretKey;
    std::string aesKey;

public:
    TransactionClient(const std::string& host = "127.0.0.1", int port = 8080) 
        : serverHost(host), serverPort(port) {
        // Clave secreta compartida (debe ser la misma que el servidor)
        // Intentar obtener de variables de entorno primero
        const char* envSecretKey = std::getenv("SECRET_KEY");
        const char* envAesKey = std::getenv("AES_KEY");
        
        if (envSecretKey) {
            secretKey = std::string(envSecretKey);
        } else {
            secretKey = "mi_clave_secreta_muy_segura_2025";
        }
        
        if (envAesKey) {
            aesKey = std::string(envAesKey);
        } else {
            aesKey = "mi_clave_aes_256_bits_muy_segura"; // Exactamente 32 caracteres
        }
        
        std::cout << "[INFO] Cliente inicializado" << std::endl;
        std::cout << "[INFO] Servidor destino: " << serverHost << ":" << serverPort << std::endl;
    }

    bool sendTransaction(const Transaction& transaction) {
        std::cout << "\n=== ENVIANDO TRANSACCIÓN ===" << std::endl;
        std::cout << "[INFO] ID: " << transaction.id << std::endl;
        std::cout << "[INFO] Tipo: " << transaction.type << std::endl;
        std::cout << "[INFO] Monto: $" << transaction.amount << std::endl;

        // Crear socket
        int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket < 0) {
            std::cerr << "[ERROR] No se pudo crear el socket cliente" << std::endl;
            return false;
        }

        // Configurar dirección del servidor
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(serverPort);
        
        // Intentar primero con inet_pton, si falla usar gethostbyname
        if (inet_pton(AF_INET, serverHost.c_str(), &serverAddr.sin_addr) <= 0) {
            // Si no es una IP, resolver el hostname
            struct hostent* he = gethostbyname(serverHost.c_str());
            if (he == nullptr) {
                std::cerr << "[ERROR] No se pudo resolver el hostname: " << serverHost << std::endl;
                close(clientSocket);
                return false;
            }
            
            // Copiar la dirección IP del resultado
            memcpy(&serverAddr.sin_addr, he->h_addr_list[0], he->h_length);
        }

        // Conectar al servidor
        std::cout << "[INFO] Conectando al servidor..." << std::endl;
        if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "[ERROR] No se pudo conectar al servidor" << std::endl;
            close(clientSocket);
            return false;
        }

        std::cout << "[SUCCESS] Conexión establecida con el servidor" << std::endl;

        // Preparar y enviar transacción
        std::string encryptedMessage = prepareSecureMessage(transaction);
        if (encryptedMessage.empty()) {
            std::cerr << "[ERROR] Error al preparar mensaje seguro" << std::endl;
            close(clientSocket);
            return false;
        }

        encryptedMessage += "\n"; // Terminador de mensaje
        
        std::cout << "[INFO] Enviando transacción cifrada..." << std::endl;
        if (send(clientSocket, encryptedMessage.c_str(), encryptedMessage.length(), 0) < 0) {
            std::cerr << "[ERROR] Error al enviar datos al servidor" << std::endl;
            close(clientSocket);
            return false;
        }

        // Recibir respuesta
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesReceived <= 0) {
            std::cerr << "[ERROR] No se recibió respuesta del servidor" << std::endl;
            close(clientSocket);
            return false;
        }

        std::string response(buffer, bytesReceived);
        processServerResponse(response);

        close(clientSocket);
        return true;
    }

    std::string prepareSecureMessage(const Transaction& transaction) {
        std::cout << "[INFO] Preparando mensaje seguro..." << std::endl;

        // Serializar transacción
        std::string transactionData = transaction.serialize();
        std::cout << "[DEBUG] Datos de transacción serializados: " << transactionData.length() << " bytes" << std::endl;

        // Generar IV aleatorio para AES (exactamente 16 bytes)
        std::string iv = CryptoUtils::generateRandomBytes(16);
        if (iv.length() != 16) {
            std::cerr << "[ERROR] Error al generar IV (tamaño: " << iv.length() << ")" << std::endl;
            return "";
        }

        // Verificar que la clave AES sea de 32 bytes
        if (aesKey.length() != 32) {
            std::cerr << "[ERROR] Clave AES debe ser de 32 bytes, actual: " << aesKey.length() << std::endl;
            return "";
        }

        // Cifrar datos con AES-256
        std::cout << "[INFO] Cifrando datos con AES-256..." << std::endl;
        std::string encryptedData = CryptoUtils::encryptAES256(transactionData, aesKey, iv);
        if (encryptedData.empty()) {
            std::cerr << "[ERROR] Error al cifrar datos" << std::endl;
            return "";
        }

        // Convertir IV a base64 para transmisión
        std::vector<unsigned char> ivBytes(iv.begin(), iv.end());
        std::string ivBase64 = CryptoUtils::base64Encode(ivBytes);

        // Crear HMAC para verificación de integridad
        std::string dataToSign = ivBase64 + ":" + encryptedData;
        std::string hmac = CryptoUtils::generateHMAC(dataToSign, secretKey);
        if (hmac.empty()) {
            std::cerr << "[ERROR] Error al generar HMAC" << std::endl;
            return "";
        }

        std::cout << "[SUCCESS] Mensaje seguro preparado" << std::endl;
        std::cout << "[DEBUG] IV Base64 length: " << ivBase64.length() << std::endl;
        std::cout << "[DEBUG] Encrypted data length: " << encryptedData.length() << std::endl;
        std::cout << "[DEBUG] HMAC length: " << hmac.length() << std::endl;

        // Formato final: IV:ENCRYPTED_DATA:HMAC
        return ivBase64 + ":" + encryptedData + ":" + hmac;
    }

    void processServerResponse(const std::string& response) {
        std::cout << "\n=== RESPUESTA DEL SERVIDOR ===" << std::endl;

        std::vector<std::string> parts;
        std::stringstream ss(response);
        std::string item;

        // Split por '|'
        while (std::getline(ss, item, '|')) {
            parts.push_back(item);
        }

        if (parts.size() < 2) {
            std::cout << "[ERROR] Respuesta del servidor con formato inválido" << std::endl;
            return;
        }

        std::string status = parts[0];
        std::string timestamp = parts[1];

        if (status == "SUCCESS") {
            std::cout << "[SUCCESS] Transacción procesada exitosamente" << std::endl;
            if (parts.size() >= 4) {
                std::string transactionId = parts[2];
                std::string result = parts[3];
                std::cout << "[INFO] ID Transacción: " << transactionId << std::endl;
                std::cout << "[INFO] Resultado: " << result << std::endl;
            }
        } else if (status == "ERROR") {
            std::cout << "[ERROR] Error en el servidor" << std::endl;
            if (parts.size() >= 3) {
                std::string errorMsg = parts[2];
                std::cout << "[ERROR] Detalle: " << errorMsg << std::endl;
            }
        }

        std::cout << "[INFO] Timestamp: " << timestamp << std::endl;
        std::cout << "==============================\n" << std::endl;
    }

    Transaction createTransferTransaction(double amount, const std::string& fromAccount, 
                                        const std::string& toAccount) {
        Transaction t;
        t.id = CryptoUtils::generateUUID();
        t.timestamp = CryptoUtils::getCurrentTimestamp();
        t.type = "TRANSFER";
        t.amount = amount;
        t.accountFrom = fromAccount;
        t.accountTo = toAccount;
        t.dynamicToken = CryptoUtils::generateDynamicToken(secretKey, t.id);

        std::cout << "[INFO] Token dinámico generado: " << t.dynamicToken.substr(0, 16) << "..." << std::endl;
        return t;
    }

    Transaction createBalanceTransaction(const std::string& account) {
        Transaction t;
        t.id = CryptoUtils::generateUUID();
        t.timestamp = CryptoUtils::getCurrentTimestamp();
        t.type = "BALANCE";
        t.amount = 0.0;
        t.accountFrom = account;
        t.dynamicToken = CryptoUtils::generateDynamicToken(secretKey, t.id);

        std::cout << "[INFO] Token dinámico generado: " << t.dynamicToken.substr(0, 16) << "..." << std::endl;
        return t;
    }

    Transaction createPaymentTransaction(double amount, const std::string& fromAccount, 
                                       const std::string& serviceCode) {
        Transaction t;
        t.id = CryptoUtils::generateUUID();
        t.timestamp = CryptoUtils::getCurrentTimestamp();
        t.type = "PAYMENT";
        t.amount = amount;
        t.accountFrom = fromAccount;
        t.serviceCode = serviceCode;
        t.dynamicToken = CryptoUtils::generateDynamicToken(secretKey, t.id);

        std::cout << "[INFO] Token dinámico generado: " << t.dynamicToken.substr(0, 16) << "..." << std::endl;
        return t;
    }

    Transaction createDepositTransaction(double amount, const std::string& toAccount) {
        Transaction t;
        t.id = CryptoUtils::generateUUID();
        t.timestamp = CryptoUtils::getCurrentTimestamp();
        t.type = "DEPOSIT";
        t.amount = amount;
        t.accountTo = toAccount;
        t.dynamicToken = CryptoUtils::generateDynamicToken(secretKey, t.id);

        std::cout << "[INFO] Token dinámico generado: " << t.dynamicToken.substr(0, 16) << "..." << std::endl;
        return t;
    }
};

void printUsage(const char* programName) {
    std::cout << "\n=== CLIENTE DE TRANSACCIONES SEGURAS ===" << std::endl;
    std::cout << "Uso: " << programName << " <host> <puerto> <comando> [argumentos...]" << std::endl;
    std::cout << "\nComandos disponibles:" << std::endl;
    std::cout << "  transfer <monto> <cuenta_origen> <cuenta_destino>" << std::endl;
    std::cout << "    Ejemplo: " << programName << " 127.0.0.1 8080 transfer 100.50 1234567890123456 6543210987654321" << std::endl;
    std::cout << "  balance <cuenta>" << std::endl;
    std::cout << "    Ejemplo: " << programName << " 127.0.0.1 8080 balance 1234567890123456" << std::endl;
    std::cout << "  payment <monto> <cuenta_origen> <codigo_servicio>" << std::endl;
    std::cout << "    Ejemplo: " << programName << " 127.0.0.1 8080 payment 75.25 1234567890123456 EAAB001" << std::endl;
    std::cout << "  deposit <monto> <cuenta_destino>" << std::endl;
    std::cout << "    Ejemplo: " << programName << " 127.0.0.1 8080 deposit 200.00 1234567890123456" << std::endl;
    std::cout << "\nCuentas de prueba disponibles:" << std::endl;
    std::cout << "  - 1234567890123456 (Saldo inicial: $5000)" << std::endl;
    std::cout << "  - 6543210987654321 (Saldo inicial: $3000)" << std::endl;
    std::cout << "  - 1111222233334444 (Saldo inicial: $1500)" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printUsage(argv[0]);
        return 1;
    }

    std::string host = argv[1];
    int port = std::atoi(argv[2]);
    std::string command = argv[3];

    TransactionClient client(host, port);

    if (command == "transfer") {
        if (argc != 7) {
            std::cerr << "[ERROR] Comando transfer requiere: <monto> <cuenta_origen> <cuenta_destino>" << std::endl;
            return 1;
        }
        
        double amount = std::stod(argv[4]);
        std::string fromAccount = argv[5];
        std::string toAccount = argv[6];
        
        Transaction t = client.createTransferTransaction(amount, fromAccount, toAccount);
        client.sendTransaction(t);
        
    } else if (command == "balance") {
        if (argc != 5) {
            std::cerr << "[ERROR] Comando balance requiere: <cuenta>" << std::endl;
            return 1;
        }
        
        std::string account = argv[4];
        Transaction t = client.createBalanceTransaction(account);
        client.sendTransaction(t);
        
    } else if (command == "payment") {
        if (argc != 7) {
            std::cerr << "[ERROR] Comando payment requiere: <monto> <cuenta_origen> <codigo_servicio>" << std::endl;
            return 1;
        }
        
        double amount = std::stod(argv[4]);
        std::string fromAccount = argv[5];
        std::string serviceCode = argv[6];
        
        Transaction t = client.createPaymentTransaction(amount, fromAccount, serviceCode);
        client.sendTransaction(t);
        
    } else if (command == "deposit") {
        if (argc != 6) {
            std::cerr << "[ERROR] Comando deposit requiere: <monto> <cuenta_destino>" << std::endl;
            return 1;
        }
        
        double amount = std::stod(argv[4]);
        std::string toAccount = argv[5];
        
        Transaction t = client.createDepositTransaction(amount, toAccount);
        client.sendTransaction(t);
        
    } else {
        std::cerr << "[ERROR] Comando no reconocido: " << command << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}