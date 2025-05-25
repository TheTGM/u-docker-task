# Makefile para el Servidor de Transacciones
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
LDFLAGS = -lssl -lcrypto -pthread

# Directorios
SRCDIR = src

# Archivos fuente
CRYPTO_SRC = $(SRCDIR)/crypto_utils.cpp
SERVER_SRC = $(SRCDIR)/servidor.cpp
SOURCES = $(SERVER_SRC) $(CRYPTO_SRC)

# Ejecutable
TARGET = servidor

# Regla por defecto
all: $(TARGET)

# Compilar servidor
$(TARGET): $(SOURCES)
	@echo "Compilando servidor..."
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)
	@echo "Servidor compilado exitosamente"

# Limpiar archivos generados
clean:
	@echo "Limpiando archivos..."
	rm -f $(TARGET)

# Verificar dependencias
check:
	@echo "Verificando dependencias..."
	@which $(CXX) > /dev/null || (echo "ERROR: g++ no encontrado" && exit 1)
	@pkg-config --exists openssl || (echo "ERROR: OpenSSL no encontrado" && exit 1)
	@echo "Dependencias verificadas"

# Información del sistema
info:
	@echo "=== INFORMACIÓN DE COMPILACIÓN ==="
	@echo "Compilador: $(CXX)"
	@echo "Flags: $(CXXFLAGS)"
	@echo "Linker: $(LDFLAGS)"
	@echo "Archivos fuente: $(SOURCES)"
	@echo "Ejecutable: $(TARGET)"

# Declarar objetivos que no crean archivos
.PHONY: all clean check info