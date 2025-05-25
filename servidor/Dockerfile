# Dockerfile simplificado para el Servidor de Transacciones
FROM alpine:3.18 AS builder

# Instalar dependencias mínimas
RUN apk add --no-cache g++ openssl-dev linux-headers musl-dev

# Directorio de trabajo
WORKDIR /app

# Copiar archivos fuente
COPY src/ ./

# Compilar con flags básicos (sin warnings estrictos)
RUN g++ -std=c++17 -O2 servidor.cpp crypto_utils.cpp -o servidor -lssl -lcrypto -pthread

# Imagen final
FROM alpine:3.18
RUN apk add --no-cache libstdc++ openssl
WORKDIR /app
COPY --from=builder /app/servidor ./servidor
RUN chmod +x servidor
EXPOSE 8080
CMD ["./servidor", "8080"]