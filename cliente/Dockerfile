# Dockerfile para el Cliente de Transacciones
# Utiliza construcción multi-stage para optimizar el tamaño final

# Stage 1: Builder
FROM alpine:3.18 AS builder

# Instalar dependencias de compilación
RUN apk add --no-cache \
    g++ \
    make \
    openssl-dev \
    linux-headers \
    musl-dev

# Crear directorio de trabajo
WORKDIR /build

# Copiar archivos fuente
COPY src/ ./src/
COPY Makefile ./

# Compilar el proyecto
RUN make clean && make all

# Verificar que el binario se compiló correctamente
RUN ls -la cliente && ldd cliente

# Stage 2: Runtime
FROM alpine:3.18

# Instalar solo las librerías necesarias en runtime
RUN apk add --no-cache \
    libstdc++ \
    openssl \
    ca-certificates \
    netcat-openbsd

# Crear usuario no-root para seguridad
RUN addgroup -g 1000 appgroup && \
    adduser -D -s /bin/sh -u 1000 -G appgroup appuser

# Crear directorio de aplicación
WORKDIR /app

# Copiar binario compilado desde el stage builder
COPY --from=builder /build/cliente ./cliente

# Crear script de pruebas
RUN echo '#!/bin/sh' > test_transactions.sh && \
    echo 'echo "=== PRUEBAS AUTOMÁTICAS DE TRANSACCIONES ==="' >> test_transactions.sh && \
    echo 'echo "Esperando que el servidor esté listo..."' >> test_transactions.sh && \
    echo 'sleep 5' >> test_transactions.sh && \
    echo 'echo "1. Consultando saldo inicial..."' >> test_transactions.sh && \
    echo './cliente servidor 8080 balance 1234567890123456' >> test_transactions.sh && \
    echo 'echo "2. Realizando transferencia..."' >> test_transactions.sh && \
    echo './cliente servidor 8080 transfer 100.50 1234567890123456 6543210987654321' >> test_transactions.sh && \
    echo 'echo "3. Consultando saldo después de transferencia..."' >> test_transactions.sh && \
    echo './cliente servidor 8080 balance 1234567890123456' >> test_transactions.sh && \
    echo 'echo "4. Realizando pago de servicio..."' >> test_transactions.sh && \
    echo './cliente servidor 8080 payment 25.75 6543210987654321 EAAB001' >> test_transactions.sh && \
    echo 'echo "5. Realizando depósito..."' >> test_transactions.sh && \
    echo './cliente servidor 8080 deposit 200.00 1111222233334444' >> test_transactions.sh && \
    echo 'echo "=== PRUEBAS COMPLETADAS ==="' >> test_transactions.sh && \
    chmod +x test_transactions.sh

# Cambiar permisos
RUN chmod +x cliente && \
    chown -R appuser:appgroup /app

# Cambiar a usuario no-root
USER appuser

# Comando por defecto - modo interactivo
CMD ["/bin/sh"]

# Metadata
LABEL maintainer="Equipo de Desarrollo"
LABEL description="Cliente de Transacciones Seguras con cifrado end-to-end"
LABEL version="1.0"