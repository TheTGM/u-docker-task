# Makefile para el Cliente de Transacciones
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
LDFLAGS = -lssl -lcrypto -pthread

# Directorios
SRCDIR = src

# Archivos fuente
CRYPTO_SRC = $(SRCDIR)/crypto_utils.cpp
CLIENT_SRC = $(SRCDIR)/cliente.cpp
SOURCES = $(CLIENT_SRC) $(CRYPTO_SRC)

# Ejecutable
TARGET = cliente

# Regla por defecto
all: $(TARGET)

# Compilar cliente
$(TARGET): $(SOURCES)
	@echo "Compilando cliente..."
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)
	@echo "Cliente compilado exitosamente"

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