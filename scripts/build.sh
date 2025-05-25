#!/bin/bash

# Script de construcción automática para Sistema de Transacciones Seguras
# Autor: Equipo de Desarrollo
# Versión: 1.0

set -e  # Salir si cualquier comando falla

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Función para logging
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Función para mostrar ayuda
show_help() {
    echo "=== SCRIPT DE CONSTRUCCIÓN - SISTEMA DE TRANSACCIONES SEGURAS ==="
    echo ""
    echo "Uso: $0 [OPCIONES]"
    echo ""
    echo "OPCIONES:"
    echo "  -h, --help              Mostrar esta ayuda"
    echo "  -c, --clean             Limpiar imágenes existentes antes de construir"
    echo "  -n, --no-cache          Construir sin usar cache de Docker"
    echo "  -p, --push              Subir imágenes a DockerHub después de construir"
    echo "  -t, --test              Ejecutar pruebas después de la construcción"
    echo "  -v, --verbose           Mostrar salida detallada"
    echo "  --production            Construir para producción (optimizado)"
    echo ""
    echo "EJEMPLOS:"
    echo "  $0                      Construcción estándar"
    echo "  $0 -c -n               Construcción limpia sin cache"
    echo "  $0 --production -p      Construcción para producción y push"
    echo ""
}

# Variables por defecto
CLEAN=false
NO_CACHE=false
PUSH=false
TEST=false
VERBOSE=false
PRODUCTION=false
DOCKER_REGISTRY=""
PROJECT_NAME="transacciones-seguras"

# Procesar argumentos
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -n|--no-cache)
            NO_CACHE=true
            shift
            ;;
        -p|--push)
            PUSH=true
            shift
            ;;
        -t|--test)
            TEST=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --production)
            PRODUCTION=true
            shift
            ;;
        --registry=*)
            DOCKER_REGISTRY="${1#*=}"
            shift
            ;;
        *)
            log_error "Opción desconocida: $1"
            show_help
            exit 1
            ;;
    esac
done

# Banner inicial
echo "=================================================================="
echo "   SISTEMA DE TRANSACCIONES SEGURAS - SCRIPT DE CONSTRUCCIÓN"
echo "=================================================================="
echo ""

# Verificar dependencias
log_info "Verificando dependencias..."

if ! command -v docker &> /dev/null; then
    log_error "Docker no está instalado o no está en el PATH"
    exit 1
fi

if ! command -v docker-compose &> /dev/null; then
    log_error "Docker Compose no está instalado o no está en el PATH"
    exit 1
fi

log_success "Dependencias verificadas"

# Mostrar información del sistema
log_info "Información del sistema:"
echo "  - Docker: $(docker --version)"
echo "  - Docker Compose: $(docker-compose --version)"
echo "  - Fecha: $(date)"
echo "  - Usuario: $(whoami)"
echo ""

# Configurar opciones de construcción
BUILD_ARGS=""
if [ "$NO_CACHE" = true ]; then
    BUILD_ARGS="$BUILD_ARGS --no-cache"
    log_info "Construcción sin cache habilitada"
fi

if [ "$VERBOSE" = true ]; then
    BUILD_ARGS="$BUILD_ARGS --progress=plain"
    log_info "Modo verbose habilitado"
fi

if [ "$PRODUCTION" = true ]; then
    BUILD_ARGS="$BUILD_ARGS --target production"
    log_info "Modo producción habilitado"
fi

# Limpiar imágenes existentes si se solicita
if [ "$CLEAN" = true ]; then
    log_info "Limpiando imágenes existentes..."
    
    # Detener contenedores relacionados
    docker-compose down 2>/dev/null || true
    
    # Eliminar imágenes del proyecto
    docker images | grep "$PROJECT_NAME" | awk '{print $3}' | xargs -r docker rmi -f 2>/dev/null || true
    
    # Limpiar imágenes huérfanas
    docker image prune -f
    
    log_success "Limpieza completada"
fi

# Crear directorios necesarios
log_info "Creando estructura de directorios..."
mkdir -p logs
mkdir -p scripts
mkdir -p docs
log_success "Directorios creados"

# Construcción principal
log_info "Iniciando construcción de imágenes Docker..."

# Construir servidor
log_info "Construyendo imagen del servidor..."
if [ "$VERBOSE" = true ]; then
    docker build $BUILD_ARGS -t "${PROJECT_NAME}-servidor:latest" ./servidor
else
    docker build $BUILD_ARGS -t "${PROJECT_NAME}-servidor:latest" ./servidor > /dev/null
fi
log_success "Imagen del servidor construida: ${PROJECT_NAME}-servidor:latest"

# Construir cliente
log_info "Construyendo imagen del cliente..."
if [ "$VERBOSE" = true ]; then
    docker build $BUILD_ARGS -t "${PROJECT_NAME}-cliente:latest" ./cliente
else
    docker build $BUILD_ARGS -t "${PROJECT_NAME}-cliente:latest" ./cliente > /dev/null
fi
log_success "Imagen del cliente construida: ${PROJECT_NAME}-cliente:latest"

# Mostrar información de las imágenes
log_info "Información de las imágenes construidas:"
echo ""
docker images | grep "$PROJECT_NAME" | while read line; do
    echo "  $line"
done
echo ""

# Calcular tamaños totales
SERVIDOR_SIZE=$(docker images "${PROJECT_NAME}-servidor:latest" --format "table {{.Size}}" | tail -n 1)
CLIENTE_SIZE=$(docker images "${PROJECT_NAME}-cliente:latest" --format "table {{.Size}}" | tail -n 1)

log_info "Tamaños de imagen:"
echo "  - Servidor: $SERVIDOR_SIZE"
echo "  - Cliente: $CLIENTE_SIZE"

# Verificar que las imágenes cumplen con los requisitos de tamaño
if [[ "$SERVIDOR_SIZE" =~ "MB" ]]; then
    SIZE_NUM=$(echo "$SERVIDOR_SIZE" | grep -o '^[0-9.]*')
    if (( $(echo "$SIZE_NUM > 50" | bc -l) )); then
        log_warning "Imagen del servidor (${SIZE_NUM}MB) excede el límite recomendado de 50MB"
    fi
fi

# Etiquetar imágenes con registry si se especificó
if [ -n "$DOCKER_REGISTRY" ]; then
    log_info "Etiquetando imágenes para registry: $DOCKER_REGISTRY"
    docker tag "${PROJECT_NAME}-servidor:latest" "$DOCKER_REGISTRY/${PROJECT_NAME}-servidor:latest"
    docker tag "${PROJECT_NAME}-cliente:latest" "$DOCKER_REGISTRY/${PROJECT_NAME}-cliente:latest"
    log_success "Imágenes etiquetadas para registry"
fi

# Ejecutar pruebas si se solicita
if [ "$TEST" = true ]; then
    log_info "Ejecutando pruebas de construcción..."
    
    # Verificar que los contenedores pueden iniciarse
    log_info "Probando inicio de contenedores..."
    
    # Crear red de prueba
    docker network create test-net 2>/dev/null || true
    
    # Probar servidor
    SERVIDOR_ID=$(docker run -d --name test-servidor --network test-net -p 18080:8080 "${PROJECT_NAME}-servidor:latest")
    sleep 5
    
    if docker ps | grep -q test-servidor; then
        log_success "Servidor inicia correctamente"
    else
        log_error "Error al iniciar servidor"
        docker logs test-servidor
        exit 1
    fi
    
    # Probar cliente
    CLIENTE_ID=$(docker run -d --name test-cliente --network test-net "${PROJECT_NAME}-cliente:latest" sleep 10)
    sleep 2
    
    if docker ps | grep -q test-cliente; then
        log_success "Cliente inicia correctamente"
    else
        log_error "Error al iniciar cliente"
        docker logs test-cliente
    fi
    
    # Limpiar contenedores de prueba
    docker stop test-servidor test-cliente 2>/dev/null || true
    docker rm test-servidor test-cliente 2>/dev/null || true
    docker network rm test-net 2>/dev/null || true
    
    log_success "Pruebas de construcción completadas"
fi

# Subir a DockerHub si se solicita
if [ "$PUSH" = true ]; then
    if [ -z "$DOCKER_REGISTRY" ]; then
        log_error "Registry no especificado. Use --registry=usuario/repositorio"
        exit 1
    fi
    
    log_info "Subiendo imágenes a DockerHub..."
    
    # Verificar login
    if ! docker info | grep -q "Username:"; then
        log_warning "No hay sesión activa en DockerHub. Ejecutando docker login..."
        docker login
    fi
    
    # Push servidor
    log_info "Subiendo imagen del servidor..."
    docker push "$DOCKER_REGISTRY/${PROJECT_NAME}-servidor:latest"
    
    # Push cliente
    log_info "Subiendo imagen del cliente..."
    docker push "$DOCKER_REGISTRY/${PROJECT_NAME}-cliente:latest"
    
    log_success "Imágenes subidas exitosamente a DockerHub"
    
    # Mostrar comandos para usar las imágenes
    echo ""
    log_info "Para usar las imágenes desde DockerHub:"
    echo "  docker pull $DOCKER_REGISTRY/${PROJECT_NAME}-servidor:latest"
    echo "  docker pull $DOCKER_REGISTRY/${PROJECT_NAME}-cliente:latest"
fi

# Generar docker-compose con imágenes del registry si aplica
if [ -n "$DOCKER_REGISTRY" ]; then
    log_info "Generando docker-compose.prod.yml con imágenes del registry..."
    
    cat > docker-compose.prod.yml << EOF
version: '3.8'

services:
  servidor:
    image: $DOCKER_REGISTRY/${PROJECT_NAME}-servidor:latest
    container_name: servidor-transacciones-prod
    ports:
      - "8080:8080"
    networks:
      - transacciones-net
    restart: unless-stopped

  cliente:
    image: $DOCKER_REGISTRY/${PROJECT_NAME}-cliente:latest
    container_name: cliente-transacciones-prod
    networks:
      - transacciones-net
    depends_on:
      - servidor
    stdin_open: true
    tty: true

networks:
  transacciones-net:
    driver: bridge
EOF
    
    log_success "docker-compose.prod.yml generado"
fi

# Resumen final
echo ""
echo "=================================================================="
log_success "CONSTRUCCIÓN COMPLETADA EXITOSAMENTE"
echo "=================================================================="
echo ""
log_info "Resumen:"
echo "  ✓ Imagen servidor: ${PROJECT_NAME}-servidor:latest ($SERVIDOR_SIZE)"
echo "  ✓ Imagen cliente: ${PROJECT_NAME}-cliente:latest ($CLIENTE_SIZE)"

if [ "$PUSH" = true ]; then
    echo "  ✓ Imágenes subidas a: $DOCKER_REGISTRY"
fi

if [ "$TEST" = true ]; then
    echo "  ✓ Pruebas ejecutadas exitosamente"
fi

echo ""
log_info "Próximos pasos:"
echo "  1. Ejecutar: docker-compose up"
echo "  2. En otra terminal: docker exec -it cliente-transacciones /bin/sh"
echo "  3. Ejecutar transacciones de prueba"
echo ""
echo "¡Sistema listo para usar! 🎉"