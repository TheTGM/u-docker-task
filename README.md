# 🔐 Sistema de Transacciones Seguras con Docker

## 📋 Descripción del Proyecto

Este proyecto implementa un sistema de transacciones seguras entre dos contenedores Docker, utilizando un mecanismo de autenticación por tokens dinámicos similar al sistema de clave dinámica de Bancolombia. El sistema emplea algoritmos criptográficos avanzados para garantizar la seguridad en las comunicaciones y transacciones.

### 🏗️ Arquitectura del Sistema

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

### 🔒 Características de Seguridad

- **🔑 Tokens Dinámicos**: SHA-256 con expiración de 30 segundos
- **🛡️ Cifrado AES-256-CBC**: Para proteger datos sensibles
- **✅ HMAC-SHA256**: Verificación de integridad de mensajes
- **🚫 Prevención de Replay**: Control de timestamps
- **🔐 Comunicación Segura**: Socket TCP cifrado end-to-end

## 🛠️ Requisitos Previos

### Software Necesario
- **Docker**: versión 20.10 o superior
- **Docker Compose**: versión 2.0 o superior
- **Git**: para clonar el repositorio

### Verificación de Requisitos
```bash
# Verificar instalaciones
docker --version
docker-compose --version
git --version
```

**Salida esperada:**
```
Docker version 24.0.0, build 1234567
Docker Compose version v2.17.0
git version 2.34.1
```

## 🚀 Instalación y Ejecución

### Paso 1: Clonar el Repositorio
```bash
git clone <URL-DEL-REPOSITORIO>
cd sistema-transacciones-seguras
```

### Paso 2: Construir las Imágenes Docker
```bash
# Construir ambos contenedores
docker-compose build

# O construir sin cache si hay problemas
docker-compose build --no-cache
```

### Paso 3: Ejecutar el Sistema
```bash
# Iniciar ambos contenedores
docker-compose up

# Para ejecutar en segundo plano
docker-compose up -d
```

### Paso 4: Verificar que el Sistema Esté Funcionando
```bash
# Verificar contenedores activos
docker-compose ps
```

**Salida esperada:**
```
NAME                       COMMAND                SERVICE     STATUS      PORTS
cliente-transacciones      "/bin/bash"            cliente     running     
servidor-transacciones     "./servidor 8080"     servidor    running     0.0.0.0:8080->8080/tcp
```

**⚠️ Importante**: Al iniciar el servidor **SI NO ESTA EN SEGUNDO PLANO DEBES ABRIR OTRA CONSOLA PARA PODER CONTINUAR CON LAS TRANSACCIONES**

### Conectar al Cliente
```bash
# Abrir shell en el contenedor cliente
docker exec -it cliente-transacciones /bin/sh
```

### Ejecutar Transacciones de Prueba

#### 1. Consulta de Saldo
```bash
./cliente servidor 8080 balance 1234567890123456
```

**Salida esperada:**
```
[INFO] Cliente inicializado
[INFO] Servidor destino: servidor:8080
[SUCCESS] Conexión establecida con el servidor
=== RESPUESTA DEL SERVIDOR ===
[SUCCESS] Transacción procesada exitosamente
[INFO] Resultado: BALANCE SUCCESS - Cuenta 1234567890123456: $5000
```

#### 2. Transferencia entre Cuentas
```bash
./cliente servidor 8080 transfer 100.50 1234567890123456 6543210987654321
```

#### 3. Pago de Servicio
```bash
./cliente servidor 8080 payment 25.75 6543210987654321 EAAB001
```

#### 4. Depósito
```bash
./cliente servidor 8080 deposit 200.00 1111222233334444
```

### Cuentas de Prueba Disponibles

| Número de Cuenta | Saldo Inicial | Descripción |
|------------------|---------------|-------------|
| `1234567890123456` | $5,000 | Cuenta principal |
| `6543210987654321` | $3,000 | Cuenta secundaria |
| `1111222233334444` | $1,500 | Cuenta de pruebas |

## 📊 Tipos de Transacciones Soportadas

### Sintaxis de Comandos

```bash
# Formato general
./cliente <servidor> <puerto> <comando> [argumentos...]

# Comandos disponibles:
./cliente servidor 8080 balance <numero_cuenta>
./cliente servidor 8080 transfer <monto> <cuenta_origen> <cuenta_destino>
./cliente servidor 8080 payment <monto> <cuenta_origen> <codigo_servicio>
./cliente servidor 8080 deposit <monto> <cuenta_destino>
```

### Ejemplos Completos

```bash
# 1. Verificar saldo inicial
./cliente servidor 8080 balance 1234567890123456

# 2. Realizar transferencia
./cliente servidor 8080 transfer 500.00 1234567890123456 6543210987654321

# 3. Verificar saldos después de transferencia
./cliente servidor 8080 balance 1234567890123456
./cliente servidor 8080 balance 6543210987654321

# 4. Pagar servicio público
./cliente servidor 8080 payment 89.50 6543210987654321 EAAB001

# 5. Realizar depósito
./cliente servidor 8080 deposit 300.00 1111222233334444
```

## 🔐 Detalles Técnicos de Seguridad

### Algoritmos Criptográficos Implementados

#### 1. Generación de Token Dinámico
```cpp
// Pseudocódigo
timestamp = getCurrentUnixTimestamp()
secret_key = "mi_clave_secreta_muy_segura_2025"
token_data = timestamp + secret_key + transaction_id
dynamic_token = SHA256(token_data)
```

- **Algoritmo**: SHA-256
- **Vida útil**: 30 segundos
- **Prevención de replay**: Validación de timestamp

#### 2. Cifrado de Datos
```cpp
// Configuración AES
Algorithm: AES-256-CBC
Key Size: 256 bits (32 bytes)
IV: Random 16 bytes per transaction
Mode: CBC (Cipher Block Chaining)
```

#### 3. Verificación de Integridad
```cpp
// HMAC para verificar integridad
hmac = HMAC-SHA256(iv_base64 + ":" + encrypted_data, secret_key)
```

### Flujo de Comunicación Segura

1. **Cliente genera transacción** con ID único y timestamp
2. **Cliente genera token dinámico** usando SHA-256
3. **Cliente cifra datos** con AES-256-CBC
4. **Cliente genera HMAC** para verificar integridad
5. **Cliente envía**: `IV_base64:encrypted_data:hmac`
6. **Servidor verifica HMAC** de integridad
7. **Servidor descifra datos** con AES-256
8. **Servidor valida token dinámico** (ventana de 30 segundos)
9. **Servidor procesa transacción** y responde

## 🛠️ Gestión del Sistema

### Comandos Útiles

```bash
# Ver logs del servidor
docker logs servidor-transacciones -f

# Ver logs del cliente
docker logs cliente-transacciones -f

# Reiniciar el sistema
docker-compose restart

# Detener el sistema
docker-compose down

# Ver estado de contenedores
docker-compose ps

# Ver uso de recursos
docker stats

# Limpiar sistema (si hay problemas)
docker-compose down
docker system prune -f
docker-compose build --no-cache
```

### Monitoreo de Transacciones

El servidor muestra información detallada en tiempo real:

```
[INFO] Nueva conexión de cliente aceptada
[INFO] Procesando transacción recibida...
[SUCCESS] Datos descifrados correctamente
[SUCCESS] Token dinámico válido
[INFO] Ejecutando transacción tipo: TRANSFER
[SUCCESS] TRANSFER SUCCESS - $100.50 transferidos de 1234567890123456 a 6543210987654321
```

## 🚨 Solución de Problemas

### Problemas Comunes y Soluciones

#### 1. Error: "No se pudo conectar al servidor"
```bash
# Verificar que el servidor esté ejecutándose
docker-compose ps

# Verificar logs del servidor
docker logs servidor-transacciones

# Si es necesario, reiniciar
docker-compose restart servidor
```

#### 2. Error: "Token dinámico inválido"
```bash
# Verificar sincronización de tiempo entre contenedores
docker exec servidor-transacciones date
docker exec cliente-transacciones date

# El token expira en 30 segundos, ejecutar transacciones sin demora
```

#### 3. Error: "Puerto 8080 en uso"
```bash
# Verificar puertos ocupados
netstat -tlnp | grep 8080

# Cambiar puerto en docker-compose.yml si es necesario
ports:
  - "8081:8080"  # Puerto externo diferente
```

#### 4. Error de compilación Docker
```bash
# Limpiar sistema Docker
docker system prune -a -f

# Reconstruir sin cache
docker-compose build --no-cache
```

#### 5. Problemas de conectividad entre contenedores
```bash
# Verificar red Docker
docker network ls
docker network inspect transacciones-network

# Probar conectividad
docker exec cliente-transacciones ping servidor
```

### Limpieza Completa del Sistema

Si experimentas problemas persistentes:

```bash
# 1. Detener todo
docker-compose down

# 2. Limpiar completamente
docker system prune -a -f
docker volume prune -f
docker network prune -f

# 3. Reconstruir desde cero
docker-compose build --no-cache
docker-compose up
```

## 🔧 Configuración Avanzada

### Variables de Entorno

El sistema utiliza las siguientes variables de entorno configuradas en `docker-compose.yml`:

```yaml
environment:
  - SECRET_KEY=mi_clave_secreta_muy_segura_2025
  - AES_KEY=mi_clave_aes_256_bits_muy_segura
  - SERVER_PORT=8080
  - LOG_LEVEL=INFO
```

### Personalización de Claves

Para usar claves personalizadas, modifica el archivo `docker-compose.yml`:

```yaml
environment:
  - SECRET_KEY=tu_clave_secreta_personalizada_aqui
  - AES_KEY=tu_clave_aes_exactamente_32_bytes_!
```

**⚠️ Importante**: La clave AES debe tener exactamente 32 caracteres (256 bits).

## 🎉 ¡Sistema Listo para Usar!

### Inicio Rápido
```bash
# 1. Clonar repositorio
git clone https://github.com/TheTGM/u-docker-task
cd sistema-transacciones-seguras

# 2. Construir y ejecutar
docker-compose up

# 3. En otra terminal, conectar al cliente
docker exec -it cliente-transacciones /bin/sh

# 4. Ejecutar primera transacción
./cliente servidor 8080 balance 1234567890123456
```

---


## 👥 Información del Equipo
- Mateo Bolivar Arroyave
- Juan Esteban Garcia Ocampo


*Última actualización: Mayo 25, 2025*
