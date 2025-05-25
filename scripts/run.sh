#!/bin/bash

# Script de ejecución automática para Sistema de Transacciones Seguras
# Autor: Equipo de Desarrollo
# Versión: 1.0

set -e

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'

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

log_step() {
    echo -e "${PURPLE}[STEP]${NC} $1"
}

# Variables globales
PROJECT_NAME="transacciones-seguras"
COMPOSE_FILE="docker-compose.yml"
MODE="interactive"

# Función para mostrar ayuda
show_help() {
    echo "=== SCRIPT DE EJECUCIÓN - SISTEMA DE TRANSACCIONES SEGURAS ==="
    echo ""
    echo "Uso: $0 [COMANDO] [OPCIONES]"
    echo ""
    echo "COMANDOS:"
    echo "  start                   Iniciar el sistema completo"
    echo "  stop                    Detener el sistema"
    echo "  restart                 Reiniciar el sistema"
    echo "  status                  Mostrar estado del sistema"
    echo "  logs                    Mostrar logs del sistema"
    echo "  shell                   Abrir shell en el cliente"
    echo "  test                    Ejecutar pruebas automáticas"
    echo "  clean                   Limpiar contenedores y volúmenes"
    echo "  demo                    Ejecutar demostración completa"
    echo ""
    echo "OPCIONES:"
    echo "  -d, --detach            Ejecutar en modo background"
    echo "  -f, --file FILE         Especificar archivo docker-compose"
    echo "  -v, --verbose           Mostrar salida detallada"
    echo "  -h, --help              Mostrar esta ayuda"
    echo ""
    echo "EJEMPLOS:"
    echo "  $0 start                Iniciar sistema en modo interactivo"
    echo "  $0 start -d             Iniciar sistema en background"
    echo "  $0 test                 Ejecutar suite de pruebas"
    echo "  $0 demo                 Demostración completa del sistema"
    echo ""
}

# Función para verificar dependencias
check_dependencies() {
    log_step "Verificando dependencias..."
    
    if ! command -v docker &> /dev/null; then
        log_error "Docker no está instalado"
        exit 1
    fi
    
    if ! command -v docker-compose &> /dev/null; then
        log_error "Docker Compose no está instalado"
        exit 1
    fi
    
    if [ ! -f "$COMPOSE_FILE" ]; then
        log_error "Archivo $COMPOSE_FILE no encontrado"
        exit 1
    fi
    
    log_success "Dependencias verificadas"
}

# Función para mostrar banner
show_banner() {
    echo -e "${CYAN}"
    echo "██████████████████████████████████████████████████████████████"
    echo "█                                                            █"
    echo "█    SISTEMA DE TRANSACCIONES SEGURAS                        █"
    echo "█    Implementación con Docker y Algoritmos Criptográficos   █"
    echo "█                                                            █"
    echo "██████████████████████████████████████████████████████████████"
    echo -e "${NC}"
    echo ""
}

# Función para iniciar el sistema
start_system() {
    local detach_flag=""
    
    if [ "$1" = "--detach" ] || [ "$1" = "-d" ]; then
        detach_flag="-d"
        MODE="background"
    fi
    
    log_step "Iniciando Sistema de Transacciones Seguras..."
    
    # Crear directorios necesarios
    mkdir -p logs
    
    # Iniciar servicios
    if [ -n "$detach_flag" ]; then
        docker-compose -f "$COMPOSE_FILE" up $detach_flag
        log_success "Sistema iniciado en modo background"
        
        # Esperar a que los servicios estén listos
        log_info "Esperando a que los servicios estén listos..."
        sleep 10
        
        # Verificar estado
        show_status
        
    else
        log_info "Iniciando en modo interactivo..."
        log_info "Presiona Ctrl+C para detener el sistema"
        echo ""
        
        # Trap para cleanup
        trap 'echo ""; log_info "Deteniendo sistema..."; docker-compose -f "$COMPOSE_FILE" down; exit 0' INT
        
        docker-compose -f "$COMPOSE_FILE" up
    fi
}

# Función para detener el sistema
stop_system() {
    log_step "Deteniendo Sistema de Transacciones Seguras..."
    
    docker-compose -f "$COMPOSE_FILE" down
    
    log_success "Sistema detenido"
}

# Función para reiniciar el sistema
restart_system() {
    log_step "Reiniciando Sistema de Transacciones Seguras..."
    
    docker-compose -f "$COMPOSE_FILE" restart
    
    log_success "Sistema reiniciado"
    
    # Mostrar estado después del reinicio
    sleep 5
    show_status
}

# Función para mostrar estado
show_status() {
    log_step "Estado del Sistema de Transacciones Seguras"
    echo ""
    
    # Estado de contenedores
    echo -e "${YELLOW}=== CONTENEDORES ===${NC}"
    docker-compose -f "$COMPOSE_FILE" ps
    echo ""
    
    # Estado de la red
    echo -e "${YELLOW}=== RED ===${NC}"
    docker network ls | grep transacciones || echo "Red no encontrada"
    echo ""
    
    # Uso de recursos
    echo -e "${YELLOW}=== USO DE RECURSOS ===${NC}"
    docker stats --no-stream --format "table {{.Container}}\t{{.CPUPerc}}\t{{.MemUsage}}" | grep -E "(servidor|cliente)" || echo "Contenedores no en ejecución"
    echo ""
    
    # Verificar conectividad
    if docker-compose -f "$COMPOSE_FILE" ps | grep -q "Up"; then
        log_info "Verificando conectividad..."
        
        # Test de conectividad servidor
        if docker exec servidor-transacciones nc -z localhost 8080 2>/dev/null; then
            log_success "Servidor respondiendo en puerto 8080"
        else
            log_warning "Servidor no responde en puerto 8080"
        fi
        
        # Test de conectividad cliente -> servidor
        if docker exec cliente-transacciones nc -z servidor 8080 2>/dev/null; then
            log_success "Cliente puede conectar al servidor"
        else
            log_warning "Cliente no puede conectar al servidor"
        fi
    fi
}

# Función para mostrar logs
show_logs() {
    log_step "Mostrando logs del sistema..."
    
    # Opción interactiva para seleccionar servicio
    echo "Seleccione el servicio para ver logs:"
    echo "1) Todos los servicios"
    echo "2) Servidor"
    echo "3) Cliente"
    echo "4) Monitor"
    echo -n "Opción [1-4]: "
    read -r option
    
    case $option in
        1)
            docker-compose -f "$COMPOSE_FILE" logs -f
            ;;
        2)
            docker-compose -f "$COMPOSE_FILE" logs -f servidor
            ;;
        3)
            docker-compose -f "$COMPOSE_FILE" logs -f cliente
            ;;
        4)
            docker-compose -f "$COMPOSE_FILE" logs -f log-monitor
            ;;
        *)
            log_warning "Opción inválida, mostrando todos los logs"
            docker-compose -f "$COMPOSE_FILE" logs -f
            ;;
    esac
}

# Función para abrir shell
open_shell() {
    log_step "Abriendo shell en el contenedor cliente..."
    
    # Verificar que el cliente esté ejecutándose
    if ! docker-compose -f "$COMPOSE_FILE" ps | grep cliente | grep -q Up; then
        log_warning "Cliente no está ejecutándose. Iniciando..."
        docker-compose -f "$COMPOSE_FILE" up -d cliente
        sleep 3
    fi
    
    log_info "Conectando al cliente... (escriba 'exit' para salir)"
    docker-compose -f "$COMPOSE_FILE" exec cliente /bin/sh
}

# Función para ejecutar pruebas
run_tests() {
    log_step "Ejecutando suite de pruebas automáticas..."
    
    # Verificar que el sistema esté ejecutándose
    if ! docker-compose -f "$COMPOSE_FILE" ps | grep servidor | grep -q Up; then
        log_info "Iniciando sistema para pruebas..."
        docker-compose -f "$COMPOSE_FILE" up -d
        sleep 10
    fi
    
    log_info "Ejecutando pruebas de transacciones..."
    
    # Suite de pruebas básicas
    echo ""
    echo -e "${CYAN}=== SUITE DE PRUEBAS ===${NC}"
    
    # Test 1: Consulta de saldo
    echo -e "${YELLOW}Test 1: Consulta de saldo${NC}"
    docker-compose -f "$COMPOSE_FILE" exec -T cliente ./cliente servidor 8080 balance 1234567890123456
    
    sleep 2
    
    # Test 2: Transferencia
    echo -e "${YELLOW}Test 2: Transferencia entre cuentas${NC}"
    docker-compose -f "$COMPOSE_FILE" exec -T cliente ./cliente servidor 8080 transfer 100.50 1234567890123456 6543210987654321
    
    sleep 2
    
    # Test 3: Pago de servicio
    echo -e "${YELLOW}Test 3: Pago de servicio${NC}"
    docker-compose -f "$COMPOSE_FILE" exec -T cliente ./cliente servidor 8080 payment 25.75 6543210987654321 EAAB001
    
    sleep 2
    
    # Test 4: Depósito
    echo -e "${YELLOW}Test 4: Depósito${NC}"
    docker-compose -f "$COMPOSE_FILE" exec -T cliente ./cliente servidor 8080 deposit 200.00 1111222233334444
    
    sleep 2
    
    # Test 5: Verificación de saldos finales
    echo -e "${YELLOW}Test 5: Verificación de saldos finales${NC}"
    docker-compose -f "$COMPOSE_FILE" exec -T cliente ./cliente servidor 8080 balance 1234567890123456
    docker-compose -f "$COMPOSE_FILE" exec -T cliente ./cliente servidor 8080 balance 6543210987654321
    docker-compose -f "$COMPOSE_FILE" exec -T cliente ./cliente servidor 8080 balance 1111222233334444
    
    echo ""
    log_success "Suite de pruebas completada"
}

# Función para limpieza
clean_system() {
    log_step "Limpiando sistema..."
    
    echo "¿Está seguro de que desea limpiar todos los contenedores y volúmenes? [y/N]"
    read -r confirm
    
    if [[ $confirm =~ ^[Yy]$ ]]; then
        # Detener servicios
        docker-compose -f "$COMPOSE_FILE" down -v --remove-orphans
        
        # Eliminar imágenes del proyecto
        docker images | grep "$PROJECT_NAME" | awk '{print $3}' | xargs -r docker rmi -f 2>/dev/null || true
        
        # Limpiar volúmenes huérfanos
        docker volume prune -f
        
        # Limpiar redes huérfanas
        docker network prune -f
        
        log_success "Sistema limpiado"
    else
        log_info "Limpieza cancelada"
    fi
}

# Función para demostración completa
run_demo() {
    log_step "Iniciando demostración completa del sistema..."
    echo ""
    
    # Banner de demo
    echo -e "${PURPLE}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║                    DEMOSTRACIÓN INTERACTIVA                  ║"
    echo "║             Sistema de Transacciones Seguras                ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    echo ""
    
    # Iniciar sistema si no está ejecutándose
    if ! docker-compose -f "$COMPOSE_FILE" ps | grep servidor | grep -q Up; then
        log_info "Iniciando sistema para demostración..."
        docker-compose -f "$COMPOSE_FILE" up -d
        
        log_info "Esperando a que los servicios estén listos..."
        sleep 15
    fi
    
    # Mostrar estado inicial
    echo -e "${CYAN}=== ESTADO INICIAL DEL SISTEMA ===${NC}"
    show_status
    
    echo ""
    log_info "Presione Enter para continuar con la demostración..."
    read -r
    
    # Demostración paso a paso
    echo -e "${CYAN}=== DEMOSTRACIÓN DE FUNCIONALIDADES ===${NC}"
    echo ""
    
    # Paso 1: Consultas de saldo inicial
    echo -e "${YELLOW}📊 PASO 1: Consultar saldos iniciales${NC}"
    echo "Verificando saldos de las cuentas de prueba..."
    sleep 2
    
    docker-compose -f "$COMPOSE_FILE" exec -T cliente ./cliente servidor 8080 balance 1234567890123456
    docker-compose -f "$COMPOSE_FILE" exec -T cliente ./cliente servidor 8080 balance 6543210987654321
    docker-compose -f "$COMPOSE_FILE" exec -T cliente ./cliente servidor 8080 balance 1111222233334444
    
    echo ""
    log_info "Presione Enter para continuar..."
    read -r
    
    # Paso 2: Transferencia entre cuentas
    echo -e "${YELLOW}💸 PASO 2: Transferencia segura entre cuentas${NC}"
    echo "Transfiriendo $250.75 de cuenta 1234... a cuenta 6543..."
    echo "Se utilizará:"
    echo "  • Token dinámico para autenticación"
    echo "  • Cifrado AES-256 para los datos"
    echo "  • HMAC-SHA256 para verificar integridad"
    sleep 3
    
    docker-compose -f "$COMPOSE_FILE" exec -T cliente ./cliente servidor 8080 transfer 250.75 1234567890123456 6543210987654321
    
    echo ""
    log_info "Presione Enter para continuar..."
    read -r
    
    # Paso 3: Pago de servicio
    echo -e "${YELLOW}💡 PASO 3: Pago de servicio público${NC}"
    echo "Pagando factura de energía por $89.50..."
    sleep 2
    
    docker-compose -f "$COMPOSE_FILE" exec -T cliente ./cliente servidor 8080 payment 89.50 6543210987654321 EAAB001
    
    echo ""
    log_info "Presione Enter para continuar..."
    read -r
    
    # Paso 4: Depósito
    echo -e "${YELLOW}💰 PASO 4: Depósito a cuenta${NC}"
    echo "Depositando $500.00 en cuenta 1111..."
    sleep 2
    
    docker-compose -f "$COMPOSE_FILE" exec -T cliente ./cliente servidor 8080 deposit 500.00 1111222233334444
    
    echo ""
    log_info "Presione Enter para ver los resultados finales..."
    read -r
    
    # Paso 5: Verificación final
    echo -e "${YELLOW}📋 PASO 5: Verificación de saldos finales${NC}"
    echo "Consultando saldos después de las transacciones..."
    sleep 2
    
    docker-compose -f "$COMPOSE_FILE" exec -T cliente ./cliente servidor 8080 balance 1234567890123456
    docker-compose -f "$COMPOSE_FILE" exec -T cliente ./cliente servidor 8080 balance 6543210987654321
    docker-compose -f "$COMPOSE_FILE" exec -T cliente ./cliente servidor 8080 balance 1111222233334444
    
    echo ""
    echo -e "${GREEN}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║                  DEMOSTRACIÓN COMPLETADA                     ║"
    echo "║                                                              ║"
    echo "║  ✓ Autenticación con tokens dinámicos                       ║"
    echo "║  ✓ Cifrado end-to-end con AES-256                          ║"
    echo "║  ✓ Verificación de integridad con HMAC                     ║"
    echo "║  ✓ Comunicación segura entre contenedores                  ║"
    echo "║  ✓ Procesamiento de múltiples tipos de transacciones       ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    
    echo ""
    log_info "¿Desea mantener el sistema ejecutándose? [Y/n]"
    read -r keep_running
    
    if [[ $keep_running =~ ^[Nn]$ ]]; then
        log_info "Deteniendo sistema..."
        docker-compose -f "$COMPOSE_FILE" down
        log_success "Sistema detenido"
    else
        log_info "Sistema permanece ejecutándose"
        log_info "Use '$0 shell' para interactuar con el cliente"
        log_info "Use '$0 stop' para detener el sistema"
    fi
}

# Función para mostrar información del proyecto
show_project_info() {
    echo -e "${CYAN}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║                    INFORMACIÓN DEL PROYECTO                  ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    echo ""
    echo "📁 Proyecto: Sistema de Transacciones Seguras"
    echo "🎓 Curso: Análisis de Algoritmos"
    echo "🔐 Tecnologías:"
    echo "   • Docker & Docker Compose"
    echo "   • C++ con OpenSSL"
    echo "   • Algoritmos criptográficos: AES-256, SHA-256, HMAC"
    echo "   • Comunicación por sockets TCP"
    echo ""
    echo "🏗️  Arquitectura:"
    echo "   • 2 contenedores Docker independientes"
    echo "   • Servidor: Procesa transacciones seguras"
    echo "   • Cliente: Envía transacciones cifradas"
    echo "   • Red privada para comunicación"
    echo ""
    echo "🔒 Características de Seguridad:"
    echo "   • Tokens dinámicos con expiración (30 segundos)"
    echo "   • Cifrado AES-256-CBC para datos sensibles"
    echo "   • HMAC-SHA256 para verificación de integridad"
    echo "   • Prevención de ataques de replay"
    echo ""
    echo "📊 Métricas objetivo:"
    echo "   • Tamaño de contenedores: < 50MB cada uno"
    echo "   • Tiempo de respuesta: < 100ms"
    echo "   • Throughput: > 1000 transacciones/min"
    echo ""
}

# Procesar argumentos de línea de comandos
DETACH=false
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        start)
            COMMAND="start"
            shift
            ;;
        stop)
            COMMAND="stop"
            shift
            ;;
        restart)
            COMMAND="restart"
            shift
            ;;
        status)
            COMMAND="status"
            shift
            ;;
        logs)
            COMMAND="logs"
            shift
            ;;
        shell)
            COMMAND="shell"
            shift
            ;;
        test)
            COMMAND="test"
            shift
            ;;
        clean)
            COMMAND="clean"
            shift
            ;;
        demo)
            COMMAND="demo"
            shift
            ;;
        info)
            COMMAND="info"
            shift
            ;;
        -d|--detach)
            DETACH=true
            shift
            ;;
        -f|--file)
            COMPOSE_FILE="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            log_error "Comando desconocido: $1"
            show_help
            exit 1
            ;;
    esac
done

# Mostrar banner
show_banner

# Verificar dependencias
check_dependencies

# Ejecutar comando
case ${COMMAND:-""} in
    start)
        if [ "$DETACH" = true ]; then
            start_system --detach
        else
            start_system
        fi
        ;;
    stop)
        stop_system
        ;;
    restart)
        restart_system
        ;;
    status)
        show_status
        ;;
    logs)
        show_logs
        ;;
    shell)
        open_shell
        ;;
    test)
        run_tests
        ;;
    clean)
        clean_system
        ;;
    demo)
        run_demo
        ;;
    info)
        show_project_info
        ;;
    "")
        log_info "No se especificó comando. Mostrando ayuda..."
        echo ""
        show_help
        ;;
    *)
        log_error "Comando no reconocido: $COMMAND"
        exit 1
        ;;
esac