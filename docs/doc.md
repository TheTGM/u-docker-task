# 📁 Guía Completa de Archivos - Sistema de Transacciones Seguras

## 🏗️ Estructura Completa del Proyecto

```
sistema-transacciones-seguras/
├── README.md                     # Documentación principal
├── docker-compose.yml            # Orquestación de contenedores
├── .gitignore                    # Archivos a ignorar en Git
├── LICENSE                       # Licencia del proyecto
│
├── servidor/                     # 📁 CONTENEDOR DEL SERVIDOR
│   ├── Dockerfile               # Docker config del servidor
│   ├── Makefile                 # Compilación del servidor
│   └── src/
│       ├── servidor.cpp         # Código principal del servidor
│       ├── crypto_utils.cpp     # Funciones criptográficas
│       └── crypto_utils.h       # Headers criptográficos
│
├── cliente/                      # 📁 CONTENEDOR DEL CLIENTE
│   ├── Dockerfile               # Docker config del cliente
│   ├── Makefile                 # Compilación del cliente
│   └── src/
│       ├── cliente.cpp          # Código principal del cliente
│       ├── crypto_utils.cpp     # Funciones criptográficas (copia)
│       └── crypto_utils.h       # Headers criptográficos (copia)
│
├── scripts/                      # 📁 SCRIPTS DE AUTOMATIZACIÓN
│   ├── build.sh                 # Script de construcción
│   ├── run.sh                   # Script de ejecución
│   └── test_transaction.sh      # Script de pruebas
│
├── docs/                         # 📁 DOCUMENTACIÓN ADICIONAL
│   ├── arquitectura.md          # Documentación de arquitectura
│   ├── algoritmos.md            # Explicación de algoritmos
│   └── ejemplos.md              # Ejemplos de uso
│
└── logs/                         # 📁 LOGS (se crea automáticamente)
```

## 🛠️ Creación Paso a Paso

### **PASO 1: Crear la estructura de directorios**

```bash
# Crear directorio principal del proyecto
mkdir sistema-transacciones-seguras
cd sistema-transacciones-seguras

# Crear subdirectorios
mkdir -p servidor/src
mkdir -p cliente/src
mkdir -p scripts
mkdir -p docs
mkdir -p logs
```

### **PASO 2: Crear archivos en la raíz del proyecto**

#### **README.md**
```bash
# Copiar el contenido del artifact "Sistema de Transacciones Seguras con Docker - Documentación Completa"
touch README.md
# Pegar aquí todo el contenido del README que te creé
```

#### **docker-compose.yml**
```bash
# Copiar el contenido del artifact "docker-compose.yml - Orquestación de Contenedores"
touch docker-compose.yml
# Pegar aquí el contenido del docker-compose
```

#### **.gitignore**
```bash
cat > .gitignore << 'EOF'
# Archivos compilados
*.o
*.so
*.a
servidor/servidor
cliente/cliente

# Logs
logs/*.log
*.log

# Docker
.env

# IDE
.vscode/
.idea/
*.swp
*.swo

# Sistema
.DS_Store
Thumbs.db

# Temporales
*.tmp
test_results.log
server_logs.tmp
EOF
```

#### **LICENSE**
```bash
cat > LICENSE << 'EOF'
MIT License

Copyright (c) 2025 Equipo Sistema Transacciones Seguras

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
EOF
```

### **PASO 3: Crear archivos del servidor**

#### **servidor/Dockerfile**
```bash
# Copiar el contenido del artifact "Dockerfile - Servidor"
touch servidor/Dockerfile
# Pegar aquí el contenido del Dockerfile del servidor
```

#### **servidor/Makefile**
```bash
cat > servidor/Makefile << 'EOF'
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
LDFLAGS = -lssl -lcrypto -pthread
SRCDIR = src
SOURCES = $(SRCDIR)/servidor.cpp $(SRCDIR)/crypto_utils.cpp
TARGET = servidor

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
EOF
```

#### **servidor/src/crypto_utils.h**
```bash
# Copiar el contenido del artifact "crypto_utils.h - Cabecera de Utilidades Criptográficas"
touch servidor/src/crypto_utils.h
# Pegar aquí el contenido del header
```

#### **servidor/src/crypto_utils.cpp**
```bash
# Copiar el contenido del artifact "crypto_utils.cpp - Implementación de Utilidades Criptográficas"
touch servidor/src/crypto_utils.cpp
# Pegar aquí el contenido de la implementación
```

#### **servidor/src/servidor.cpp**
```bash
# Copiar el contenido del artifact "servidor.cpp - Servidor de Transacciones"
touch servidor/src/servidor.cpp
# Pegar aquí el contenido del servidor
```

### **PASO 4: Crear archivos del cliente**

#### **cliente/Dockerfile**
```bash
# Copiar el contenido del artifact "Dockerfile - Cliente"
touch cliente/Dockerfile
# Pegar aquí el contenido del Dockerfile del cliente
```

#### **cliente/Makefile**
```bash
cat > cliente/Makefile << 'EOF'
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
LDFLAGS = -lssl -lcrypto -pthread
SRCDIR = src
SOURCES = $(SRCDIR)/cliente.cpp $(SRCDIR)/crypto_utils.cpp
TARGET = cliente

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
EOF
```

#### **cliente/src/** (Copiar los mismos archivos del servidor)
```bash
# Copiar los archivos criptográficos del servidor al cliente
cp servidor/src/crypto_utils.h cliente/src/
cp servidor/src/crypto_utils.cpp cliente/src/

# Crear el archivo del cliente
# Copiar el contenido del artifact "cliente.cpp - Cliente de Transacciones"
touch cliente/src/cliente.cpp
# Pegar aquí el contenido del cliente
```

### **PASO 5: Crear scripts de automatización**

#### **scripts/build.sh**
```bash
# Copiar el contenido del artifact "build.sh - Script de Construcción Automática"
touch scripts/build.sh
chmod +x scripts/build.sh
# Pegar aquí el contenido del script de build
```

#### **scripts/run.sh**
```bash
# Copiar el contenido del artifact "run.sh - Script de Ejecución Automática"
touch scripts/run.sh
chmod +x scripts/run.sh
# Pegar aquí el contenido del script de run
```

#### **scripts/test_transaction.sh**
```bash
# Copiar el contenido del artifact "test_transaction.sh - Script de Pruebas Automatizadas"
touch scripts/test_transaction.sh
chmod +x scripts/test_transaction.sh
# Pegar aquí el contenido del script de pruebas
```

### **PASO 6: Crear documentación adicional**

#### **docs/arquitectura.md**
```bash
cat > docs/arquitectura.md << 'EOF'
# Arquitectura del Sistema

## Diagrama de Componentes

```
┌─────────────────┐                    ┌─────────────────┐
│   CLIENTE       │                    │   SERVIDOR      │
│   (Container 1) │◄──────────────────►│   (Container 2) │
│                 │   Transacciones    │                 │
│ • Genera Token  │   Seguras con      │ • Valida Token  │
│ • Envía Trans.  │   Token Dinámico   │ • Procesa Trans │
│ • AES Encrypt   │                    │ • AES Decrypt   │
└─────────────────┘                    └─────────────────┘
```

## Flujo de Comunicación

1. Cliente genera token dinámico basado en timestamp + SHA-256
2. Cliente cifra datos con AES-256-CBC
3. Cliente genera HMAC-SHA256 para integridad
4. Cliente envía: IV:ENCRYPTED_DATA:HMAC
5. Servidor verifica HMAC
6. Servidor descifra datos
7. Servidor valida token dinámico
8. Servidor procesa transacción
9. Servidor responde con resultado cifrado
EOF
```

#### **docs/algoritmos.md**
```bash
cat > docs/algoritmos.md << 'EOF'
# Algoritmos Criptográficos Implementados

## 1. Generación de Token Dinámico
- **Algoritmo**: SHA-256
- **Entrada**: timestamp + clave_secreta + transaction_id
- **Salida**: Hash hexadecimal de 64 caracteres
- **Vida útil**: 30 segundos

## 2. Cifrado de Datos
- **Algoritmo**: AES-256-CBC
- **Tamaño de clave**: 256 bits
- **Vector de inicialización**: 16 bytes aleatorios
- **Padding**: PKCS#7

## 3. Verificación de Integridad
- **Algoritmo**: HMAC-SHA256
- **Clave**: Clave secreta compartida
- **Datos**: IV + datos cifrados

## 4. Generación de UUIDs
- **Formato**: UUID versión 4
- **Entropía**: Generador criptográficamente seguro
EOF
```

#### **docs/ejemplos.md**
```bash
cat > docs/ejemplos.md << 'EOF'
# Ejemplos de Uso

## Comandos Básicos

### Consultar Saldo
```bash
./cliente servidor 8080 balance 1234567890123456
```

### Transferir Dinero
```bash
./cliente servidor 8080 transfer 100.50 1234567890123456 6543210987654321
```

### Pagar Servicio
```bash
./cliente servidor 8080 payment 75.25 1234567890123456 EAAB001
```

### Depositar Dinero
```bash
./cliente servidor 8080 deposit 200.00 1234567890123456
```

## Cuentas de Prueba
- 1234567890123456 (Saldo inicial: $5000)
- 6543210987654321 (Saldo inicial: $3000)
- 1111222233334444 (Saldo inicial: $1500)
EOF
```

## 🚀 Comandos de Verificación

Una vez creada toda la estructura, puedes verificar que esté correcta:

```bash
# Verificar estructura de directorios
find sistema-transacciones-seguras -type d

# Verificar archivos de código fuente
find sistema-transacciones-seguras -name "*.cpp" -o -name "*.h"

# Verificar scripts
ls -la sistema-transacciones-seguras/scripts/

# Verificar configuración Docker
ls -la sistema-transacciones-seguras/*/Dockerfile
ls -la sistema-transacciones-seguras/docker-compose.yml
```

## ✅ Lista de Verificación Final

- [ ] **Directorio principal**: `sistema-transacciones-seguras/`
- [ ] **Archivos raíz**: README.md, docker-compose.yml, .gitignore, LICENSE
- [ ] **Servidor**: Dockerfile, Makefile, 3 archivos .cpp/.h
- [ ] **Cliente**: Dockerfile, Makefile, 3 archivos .cpp/.h  
- [ ] **Scripts**: 3 archivos .sh con permisos de ejecución
- [ ] **Docs**: 3 archivos .md de documentación
- [ ] **Logs**: Directorio vacío (se llenará automáticamente)

¡Con esta estructura tendrás todo listo para construir y ejecutar el sistema! 🎉