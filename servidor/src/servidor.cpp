#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <thread>
#include <vector>
#include <map>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>
#include "crypto_utils.h"

class TransactionServer {
private:
    int serverSocket;
    int port;
    std::string secretKey;
    std::string aesKey;
    std::map<std::string, double> accounts; // Simulación de cuentas
    std::vector<Transaction> transactionHistory;
    std::mutex accountsMutex;
    std::mutex historyMutex;
    bool running;

public:
    TransactionServer(int port = 8080) : port(port), running(false) {
        // Clave secreta compartida (obtener de variables de entorno si están disponibles)
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
        
        // Inicializar algunas cuentas de prueba
        accounts["1234567890123456"] = 5000.0;
        accounts["6543210987654321"] = 3000.0;
        accounts["1111222233334444"] = 1500.0;
        
        std::cout << "[INFO] Servidor inicializado en puerto " << port << std::endl;
        std::cout << "[DEBUG] Clave AES tiene " << aesKey.length() << " bytes" << std::endl;
        std::cout << "[DEBUG] Clave secreta tiene " << secretKey.length() << " bytes" << std::endl;
        std::cout << "[INFO] Cuentas de prueba disponibles:" << std::endl;
        for (const auto& account : accounts) {
            std::cout << "  - Cuenta: " << account.first << " Saldo: $" << account.second << std::endl;
        }
    }

    bool start() {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            std::cerr << "[ERROR] No se pudo crear el socket del servidor" << std::endl;
            return false;
        }

        // Permitir reutilizar la dirección
        int opt = 1;
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "[WARNING] No se pudo configurar SO_REUSEADDR" << std::endl;
        }

        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "[ERROR] No se pudo hacer bind en el puerto " << port << std::endl;
            close(serverSocket);
            return false;
        }

        if (listen(serverSocket, 5) < 0) {
            std::cerr << "[ERROR] Error al poner el socket en modo listen" << std::endl;
            close(serverSocket);
            return false;
        }

        running = true;
        std::cout << "[SUCCESS] Servidor escuchando en puerto " << port << std::endl;
        std::cout << "[INFO] Esperando conexiones de clientes..." << std::endl;

        return true;
    }

    void run() {
        while (running) {
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            if (clientSocket < 0) {
                if (running) {
                    std::cerr << "[ERROR] Error al aceptar conexión del cliente" << std::endl;
                }
                continue;
            }

            std::cout << "[INFO] Nueva conexión de cliente aceptada" << std::endl;
            
            // Manejar cliente en un hilo separado
            std::thread clientThread(&TransactionServer::handleClient, this, clientSocket);
            clientThread.detach();
        }
    }

    void handleClient(int clientSocket) {
        char buffer[4096];
        std::string receivedData;

        while (true) {
            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            
            if (bytesReceived <= 0) {
                std::cout << "[INFO] Cliente desconectado" << std::endl;
                break;
            }

            receivedData += std::string(buffer, bytesReceived);
            
            // Verificar si recibimos un mensaje completo (terminado en \n)
            size_t pos = receivedData.find('\n');
            if (pos != std::string::npos) {
                std::string message = receivedData.substr(0, pos);
                receivedData = receivedData.substr(pos + 1);
                
                std::string response = processTransaction(message);
                send(clientSocket, response.c_str(), response.length(), 0);
            }
        }

        close(clientSocket);
    }

    std::string processTransaction(const std::string& encryptedMessage) {
        std::cout << "[INFO] Procesando transacción recibida..." << std::endl;

        try {
            // Parsear el mensaje: formato "IV:ENCRYPTED_DATA:HMAC"
            size_t firstColon = encryptedMessage.find(':');
            size_t secondColon = encryptedMessage.find(':', firstColon + 1);
            
            if (firstColon == std::string::npos || secondColon == std::string::npos) {
                return createErrorResponse("Formato de mensaje inválido");
            }

            std::string iv = encryptedMessage.substr(0, firstColon);
            std::string encryptedData = encryptedMessage.substr(firstColon + 1, secondColon - firstColon - 1);
            std::string receivedHMAC = encryptedMessage.substr(secondColon + 1);

            std::cout << "[DEBUG] IV Base64 recibido: " << iv.substr(0, 20) << "..." << std::endl;
            std::cout << "[DEBUG] Datos cifrados recibidos: " << encryptedData.length() << " caracteres" << std::endl;
            std::cout << "[DEBUG] HMAC recibido: " << receivedHMAC.substr(0, 16) << "..." << std::endl;

            // Decodificar IV de Base64
            std::vector<unsigned char> ivBytes = CryptoUtils::base64Decode(iv);
            if (ivBytes.size() != 16) {
                std::cout << "[ERROR] IV debe ser de 16 bytes, recibido: " << ivBytes.size() << " bytes" << std::endl;
                return createErrorResponse("IV inválido");
            }
            std::string ivDecoded(ivBytes.begin(), ivBytes.end());

            std::cout << "[DEBUG] IV decodificado: " << ivBytes.size() << " bytes" << std::endl;

            // Verificar HMAC
            std::string dataToVerify = iv + ":" + encryptedData;
            if (!CryptoUtils::verifyHMAC(dataToVerify, receivedHMAC, secretKey)) {
                std::cout << "[ERROR] HMAC inválido - posible manipulación de datos" << std::endl;
                return createErrorResponse("Verificación de integridad fallida");
            }

            // Descifrar datos
            std::string decryptedData = CryptoUtils::decryptAES256(encryptedData, aesKey, ivDecoded);
            if (decryptedData.empty()) {
                std::cout << "[ERROR] Error al descifrar los datos" << std::endl;
                return createErrorResponse("Error de descifrado");
            }

            std::cout << "[SUCCESS] Datos descifrados correctamente" << std::endl;
            std::cout << "[DEBUG] Datos descifrados: " << decryptedData.substr(0, 100) << "..." << std::endl;
            std::cout << "[DEBUG] Longitud de datos descifrados: " << decryptedData.length() << " bytes" << std::endl;

            // Parsear la transacción
            Transaction transaction = parseTransaction(decryptedData);
            
            // Validar token dinámico
            if (!CryptoUtils::validateDynamicToken(transaction.dynamicToken, secretKey, transaction.id)) {
                std::cout << "[ERROR] Token dinámico inválido o expirado" << std::endl;
                return createErrorResponse("Token dinámico inválido");
            }

            std::cout << "[SUCCESS] Token dinámico válido" << std::endl;

            // Procesar la transacción según su tipo
            std::string result = executeTransaction(transaction);
            
            // Registrar en historial
            {
                std::lock_guard<std::mutex> lock(historyMutex);
                transactionHistory.push_back(transaction);
            }

            return createSuccessResponse(result, transaction.id);

        } catch (const std::exception& e) {
            std::cout << "[ERROR] Excepción al procesar transacción: " << e.what() << std::endl;
            return createErrorResponse("Error interno del servidor");
        }
    }

    Transaction parseTransaction(const std::string& data) {
        Transaction t;
        std::vector<std::string> parts;
        std::stringstream ss(data);
        std::string item;

        std::cout << "[DEBUG] Datos recibidos para parsear: " << data.substr(0, 100) << "..." << std::endl;

        // Split por '|'
        while (std::getline(ss, item, '|')) {
            parts.push_back(item);
        }

        std::cout << "[DEBUG] Número de partes encontradas: " << parts.size() << std::endl;
        
        if (parts.size() >= 8) {
            t.id = parts[0];
            t.timestamp = parts[1];
            t.type = parts[2];
            t.amount = std::stod(parts[3]);
            t.accountFrom = parts[4];
            t.accountTo = parts[5];
            t.serviceCode = parts[6];
            t.dynamicToken = parts[7];
            
            std::cout << "[DEBUG] Transaction ID parseado: '" << t.id << "'" << std::endl;
            std::cout << "[DEBUG] Tipo: '" << t.type << "'" << std::endl;
            std::cout << "[DEBUG] Token: '" << t.dynamicToken.substr(0, 16) << "...'" << std::endl;
        } else {
            std::cout << "[ERROR] Datos de transacción incompletos. Partes: " << parts.size() << std::endl;
            for (size_t i = 0; i < parts.size(); i++) {
                std::cout << "[DEBUG] Parte " << i << ": '" << parts[i] << "'" << std::endl;
            }
        }

        return t;
    }

    std::string executeTransaction(const Transaction& t) {
        std::lock_guard<std::mutex> lock(accountsMutex);

        std::cout << "[INFO] Ejecutando transacción tipo: " << t.type << std::endl;
        std::cout << "[INFO] ID Transacción: " << t.id << std::endl;
        std::cout << "[INFO] Monto: $" << t.amount << std::endl;

        if (t.type == "TRANSFER") {
            return processTransfer(t);
        } else if (t.type == "BALANCE") {
            return processBalance(t);
        } else if (t.type == "PAYMENT") {
            return processPayment(t);
        } else if (t.type == "DEPOSIT") {
            return processDeposit(t);
        } else {
            return "ERROR: Tipo de transacción no soportado";
        }
    }

    std::string processTransfer(const Transaction& t) {
        if (accounts.find(t.accountFrom) == accounts.end()) {
            return "ERROR: Cuenta origen no existe";
        }
        if (accounts.find(t.accountTo) == accounts.end()) {
            return "ERROR: Cuenta destino no existe";
        }
        if (accounts[t.accountFrom] < t.amount) {
            return "ERROR: Saldo insuficiente";
        }

        accounts[t.accountFrom] -= t.amount;
        accounts[t.accountTo] += t.amount;

        std::stringstream ss;
        ss << "TRANSFER SUCCESS - $" << t.amount << " transferidos de " 
           << t.accountFrom << " a " << t.accountTo;
        ss << " | Saldo origen: $" << accounts[t.accountFrom];
        ss << " | Saldo destino: $" << accounts[t.accountTo];

        std::cout << "[SUCCESS] " << ss.str() << std::endl;
        return ss.str();
    }

    std::string processBalance(const Transaction& t) {
        if (accounts.find(t.accountFrom) == accounts.end()) {
            return "ERROR: Cuenta no existe";
        }

        std::stringstream ss;
        ss << "BALANCE SUCCESS - Cuenta " << t.accountFrom << ": $" << accounts[t.accountFrom];
        
        std::cout << "[SUCCESS] " << ss.str() << std::endl;
        return ss.str();
    }

    std::string processPayment(const Transaction& t) {
        if (accounts.find(t.accountFrom) == accounts.end()) {
            return "ERROR: Cuenta no existe";
        }
        if (accounts[t.accountFrom] < t.amount) {
            return "ERROR: Saldo insuficiente";
        }

        accounts[t.accountFrom] -= t.amount;

        std::stringstream ss;
        ss << "PAYMENT SUCCESS - $" << t.amount << " pagados a servicio " << t.serviceCode;
        ss << " desde cuenta " << t.accountFrom;
        ss << " | Saldo restante: $" << accounts[t.accountFrom];

        std::cout << "[SUCCESS] " << ss.str() << std::endl;
        return ss.str();
    }

    std::string processDeposit(const Transaction& t) {
        if (accounts.find(t.accountTo) == accounts.end()) {
            return "ERROR: Cuenta destino no existe";
        }

        accounts[t.accountTo] += t.amount;

        std::stringstream ss;
        ss << "DEPOSIT SUCCESS - $" << t.amount << " depositados en cuenta " << t.accountTo;
        ss << " | Saldo actual: $" << accounts[t.accountTo];

        std::cout << "[SUCCESS] " << ss.str() << std::endl;
        return ss.str();
    }

    std::string createSuccessResponse(const std::string& result, const std::string& transactionId) {
        std::stringstream ss;
        ss << "SUCCESS|" << CryptoUtils::getCurrentTimestamp() << "|" << transactionId << "|" << result;
        return ss.str();
    }

    std::string createErrorResponse(const std::string& error) {
        std::stringstream ss;
        ss << "ERROR|" << CryptoUtils::getCurrentTimestamp() << "|" << error;
        return ss.str();
    }

    void stop() {
        running = false;
        if (serverSocket >= 0) {
            close(serverSocket);
        }
        std::cout << "[INFO] Servidor detenido" << std::endl;
    }

    void printStatus() {
        std::cout << "\n=== ESTADO DEL SERVIDOR ===" << std::endl;
        std::cout << "Puerto: " << port << std::endl;
        std::cout << "Estado: " << (running ? "EJECUTÁNDOSE" : "DETENIDO") << std::endl;
        
        {
            std::lock_guard<std::mutex> lock(accountsMutex);
            std::cout << "\n--- CUENTAS ---" << std::endl;
            for (const auto& account : accounts) {
                std::cout << "Cuenta: " << account.first << " - Saldo: $" << account.second << std::endl;
            }
        }

        {
            std::lock_guard<std::mutex> lock(historyMutex);
            std::cout << "\n--- HISTORIAL DE TRANSACCIONES ---" << std::endl;
            std::cout << "Total de transacciones: " << transactionHistory.size() << std::endl;
            
            // Mostrar las últimas 5 transacciones
            int count = 0;
            for (auto it = transactionHistory.rbegin(); it != transactionHistory.rend() && count < 5; ++it, ++count) {
                std::cout << "  " << it->timestamp << " - " << it->type << " - $" << it->amount << std::endl;
            }
        }
        std::cout << "==========================\n" << std::endl;
    }
};

// Variable global para manejo de señales
TransactionServer* globalServer = nullptr;

void signalHandler(int signal) {
    std::cout << "\n[INFO] Señal recibida (" << signal << "). Cerrando servidor..." << std::endl;
    if (globalServer) {
        globalServer->stop();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    std::cout << "=== SERVIDOR DE TRANSACCIONES SEGURAS ===" << std::endl;
    std::cout << "Implementado con algoritmos criptográficos avanzados" << std::endl;
    std::cout << "- AES-256-CBC para cifrado" << std::endl;
    std::cout << "- HMAC-SHA256 para integridad" << std::endl;
    std::cout << "- SHA-256 para tokens dinámicos" << std::endl;
    std::cout << "==========================================\n" << std::endl;

    int port = 8080;
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }

    TransactionServer server(port);
    globalServer = &server;

    // Configurar manejo de señales
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    if (!server.start()) {
        std::cerr << "[ERROR] No se pudo iniciar el servidor" << std::endl;
        return 1;
    }

    std::cout << "[INFO] Servidor ejecutándose. Presiona Ctrl+C para detener." << std::endl;
    std::cout << "[INFO] Para interactuar con el servidor, use el cliente desde otro contenedor." << std::endl;

    // Ejecutar servidor sin hilo de consola para Docker
    server.run();

    return 0;
}