#!/bin/bash

# Script de pruebas automatizadas para Sistema de Transacciones Seguras
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

# Variables globales
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
TEST_LOG="test_results.log"
START_TIME=$(date +%s)

# Función para logging
log_test() {
    echo -e "${BLUE}[TEST]${NC} $1" | tee -a "$TEST_LOG"
}

log_pass() {
    echo -e "${GREEN}[PASS]${NC} $1" | tee -a "$TEST_LOG"
    ((PASSED_TESTS++))
}

log_fail() {
    echo -e "${RED}[FAIL]${NC} $1" | tee -a "$TEST_LOG"
    ((FAILED_TESTS++))
}

log_info() {
    echo -e "${CYAN}[INFO]${NC} $1" | tee -a "$TEST_LOG"
}

log_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1" | tee -a "$TEST_LOG"
}

# Función para mostrar banner de pruebas
show_test_banner() {
    echo -e "${PURPLE}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║              SUITE DE PRUEBAS AUTOMATIZADAS                 ║"
    echo "║           Sistema de Transacciones Seguras                  ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    echo ""
}

# Función para ejecutar un comando de cliente y verificar resultado
execute_transaction() {
    local description="$1"
    local command="$2"
    local expected_pattern="$3"
    local should_succeed="$4"
    
    ((TOTAL_TESTS++))
    log_test "$description"
    
    # Ejecutar comando y capturar salida
    local output
    local exit_code
    
    if output=$(timeout 30 docker-compose exec -T cliente $command 2>&1); then
        exit_code=0
    else
        exit_code=$?
    fi
    
    # Verificar resultado
    if [ "$should_succeed" = "true" ]; then
        if [ $exit_code -eq 0 ] && echo "$output" | grep -q "$expected_pattern"; then
            log_pass "$description - Éxito"
            echo "   Output: $(echo "$output" | grep "$expected_pattern" | head -1)"
        else
            log_fail "$description - Falló"
            echo "   Expected: $expected_pattern"
            echo "   Got: $output"
            echo "   Exit code: $exit_code"
        fi
    else
        if [ $exit_code -ne 0 ] || echo "$output" | grep -q "ERROR"; then
            log_pass "$description - Falló como se esperaba"
        else
            log_fail "$description - Debería haber fallado"
            echo "   Output: $output"
        fi
    fi
    
    echo "" | tee -a "$TEST_LOG"
    sleep 1
}

# Función para verificar conectividad básica
test_connectivity() {
    log_info "=== PRUEBAS DE CONECTIVIDAD ==="
    
    # Test 1: Verificar que el servidor responde
    ((TOTAL_TESTS++))
    log_test "Verificar conectividad del servidor"
    
    if docker exec servidor-transacciones nc -z localhost 8080 2>/dev/null; then
        log_pass "Servidor responde en puerto 8080"
    else
        log_fail "Servidor no responde en puerto 8080"
    fi
    
    # Test 2: Verificar conectividad cliente -> servidor
    ((TOTAL_TESTS++))
    log_test "Verificar conectividad cliente -> servidor"
    
    if docker exec cliente-transacciones nc -z servidor 8080 2>/dev/null; then
        log_pass "Cliente puede conectar al servidor"
    else
        log_fail "Cliente no puede conectar al servidor"
    fi
    
    echo ""
}

# Función para pruebas de transacciones válidas
test_valid_transactions() {
    log_info "=== PRUEBAS DE TRANSACCIONES VÁLIDAS ==="
    
    # Test 1: Consulta de saldo - Cuenta existente
    execute_transaction \
        "Consulta de saldo - Cuenta válida" \
        "./cliente servidor 8080 balance 1234567890123456" \
        "BALANCE SUCCESS" \
        "true"
    
    # Test 2: Transferencia entre cuentas válidas
    execute_transaction \
        "Transferencia entre cuentas válidas" \
        "./cliente servidor 8080 transfer 50.00 1234567890123456 6543210987654321" \
        "TRANSFER SUCCESS" \
        "true"
    
    # Test 3: Pago de servicio
    execute_transaction \
        "Pago de servicio público" \
        "./cliente servidor 8080 payment 25.50 6543210987654321 EAAB001" \
        "PAYMENT SUCCESS" \
        "true"
    
    # Test 4: Depósito a cuenta válida
    execute_transaction \
        "Depósito a cuenta válida" \
        "./cliente servidor 8080 deposit 100.00 1111222233334444" \
        "DEPOSIT SUCCESS" \
        "true"
    
    # Test 5: Múltiples transacciones consecutivas
    execute_transaction \
        "Primera transferencia consecutiva" \
        "./cliente servidor 8080 transfer 10.00 1234567890123456 6543210987654321" \
        "TRANSFER SUCCESS" \
        "true"
    
    execute_transaction \
        "Segunda transferencia consecutiva" \
        "./cliente servidor 8080 transfer 15.00 6543210987654321 1111222233334444" \
        "TRANSFER SUCCESS" \
        "true"
}

# Función para pruebas de transacciones inválidas
test_invalid_transactions() {
    log_info "=== PRUEBAS DE TRANSACCIONES INVÁLIDAS ==="
    
    # Test 1: Consulta de saldo - Cuenta inexistente
    execute_transaction \
        "Consulta de saldo - Cuenta inexistente" \
        "./cliente servidor 8080 balance 9999999999999999" \
        "ERROR.*no existe" \
        "false"
    
    # Test 2: Transferencia con saldo insuficiente
    execute_transaction \
        "Transferencia con saldo insuficiente" \
        "./cliente servidor 8080 transfer 99999.00 1234567890123456 6543210987654321" \
        "ERROR.*insuficiente" \
        "false"
    
    # Test 3: Transferencia a cuenta inexistente
    execute_transaction \
        "Transferencia a cuenta inexistente" \
        "./cliente servidor 8080 transfer 10.00 1234567890123456 9999999999999999" \
        "ERROR.*no existe" \
        "false"
    
    # Test 4: Transferencia desde cuenta inexistente
    execute_transaction \
        "Transferencia desde cuenta inexistente" \
        "./cliente servidor 8080 transfer 10.00 9999999999999999 6543210987654321" \
        "ERROR.*no existe" \
        "false"
}

# Función para pruebas de seguridad
test_security_features() {
    log_info "=== PRUEBAS DE CARACTERÍSTICAS DE SEGURIDAD ==="
    
    # Test 1: Verificar que los tokens son diferentes para cada transacción
    log_test "Verificar unicidad de tokens dinámicos"
    
    local output1 output2
    output1=$(docker-compose exec -T cliente ./cliente servidor 8080 balance 1234567890123456 2>&1 | grep "Token dinámico generado" || echo "")
    sleep 2
    output2=$(docker-compose exec -T cliente ./cliente servidor 8080 balance 1234567890123456 2>&1 | grep "Token dinámico generado" || echo "")
    
    if [ -n "$output1" ] && [ -n "$output2" ] && [ "$output1" != "$output2" ]; then
        log_pass "Tokens dinámicos son únicos para cada transacción"
    else
        log_fail "Tokens dinámicos no son únicos"
    fi
    ((TOTAL_TESTS++))
    
    # Test 2: Verificar cifrado de datos (no debe haber datos en texto plano en logs)
    log_test "Verificar cifrado de datos sensibles"
    
    # Capturar logs del servidor durante una transacción
    docker-compose logs servidor > server_logs.tmp 2>&1 &
    LOG_PID=$!
    
    # Ejecutar transacción
    docker-compose exec -T cliente ./cliente servidor 8080 transfer 1.00 1234567890123456 6543210987654321 >/dev/null 2>&1
    
    sleep 3
    kill $LOG_PID 2>/dev/null || true
    
    # Verificar que no hay números de cuenta en texto plano en la transmisión
    if grep -q "1234567890123456" server_logs.tmp; then
        log_warning "Posible exposición de datos sensibles en logs"
    else
        log_pass "Datos sensibles no expuestos en logs del servidor"
    fi
    
    rm -f server_logs.tmp
    ((TOTAL_TESTS++))
}

# Función para pruebas de rendimiento básicas
test_performance() {
    log_info "=== PRUEBAS DE RENDIMIENTO BÁSICAS ==="
    
    # Test 1: Tiempo de respuesta de transacciones
    log_test "Medición de tiempo de respuesta"
    
    local start_time end_time duration
    start_time=$(date +%s%3N)
    
    docker-compose exec -T cliente ./cliente servidor 8080 balance 1234567890123456 >/dev/null 2>&1
    
    end_time=$(date +%s%3N)
    duration=$((end_time - start_time))
    
    if [ $duration -lt 1000 ]; then  # Menos de 1 segundo
        log_pass "Tiempo de respuesta: ${duration}ms (< 1000ms objetivo)"
    else
        log_fail "Tiempo de respuesta: ${duration}ms (> 1000ms objetivo)"
    fi
    ((TOTAL_TESTS++))
    
    # Test 2: Prueba de carga básica (múltiples transacciones)
    log_test "Prueba de carga básica (10 transacciones concurrentes)"
    
    local pids=()
    start_time=$(date +%s)
    
    # Ejecutar 10 transacciones en paralelo
    for i in {1..10}; do
        (docker-compose exec -T cliente ./cliente servidor 8080 balance 1234567890123456 >/dev/null 2>&1) &
        pids+=($!)
    done
    
    # Esperar a que terminen todas
    for pid in "${pids[@]}"; do
        wait $pid
    done
    
    end_time=$(date +%s)
    duration=$((end_time - start_time))
    
    if [ $duration -lt 10 ]; then  # Menos de 10 segundos para 10 transacciones
        log_pass "Prueba de carga: 10 transacciones en ${duration}s"
    else
        log_fail "Prueba de carga: 10 transacciones en ${duration}s (demasiado lento)"
    fi
    ((TOTAL_TESTS++))
}

# Función para verificar estado del sistema antes de pruebas
verify_system_ready() {
    log_info "Verificando que el sistema esté listo para pruebas..."
    
    # Verificar que los contenedores están ejecutándose
    if ! docker-compose ps | grep servidor | grep -q Up; then
        log_info "Iniciando servidor..."
        docker-compose up -d servidor
        sleep 10
    fi
    
    if ! docker-compose ps | grep cliente | grep -q Up; then
        log_info "Iniciando cliente..."
        docker-compose up -d cliente
        sleep 5
    fi
    
    # Esperar a que el servidor esté listo
    local retries=0
    while ! docker exec servidor-transacciones nc -z localhost 8080 2>/dev/null; do
        if [ $retries -ge 30 ]; then
            log_fail "Servidor no responde después de 30 intentos"
            exit 1
        fi
        log_info "Esperando a que el servidor esté listo... (intento $((retries + 1))/30)"
        sleep 2
        ((retries++))
    done
    
    log_info "Sistema listo para pruebas"
    echo ""
}

# Función para generar reporte final
generate_report() {
    local end_time=$(date +%s)
    local duration=$((end_time - START_TIME))
    
    echo "" | tee -a "$TEST_LOG"
    echo -e "${CYAN}╔══════════════════════════════════════════════════════════════╗${NC}" | tee -a "$TEST_LOG"
    echo -e "${CYAN}║                     REPORTE FINAL DE PRUEBAS                 ║${NC}" | tee -a "$TEST_LOG"
    echo -e "${CYAN}╚══════════════════════════════════════════════════════════════╝${NC}" | tee -a "$TEST_LOG"
    echo "" | tee -a "$TEST_LOG"
    
    echo "📊 Estadísticas de Pruebas:" | tee -a "$TEST_LOG"
    echo "   • Total de pruebas ejecutadas: $TOTAL_TESTS" | tee -a "$TEST_LOG"
    echo "   • Pruebas exitosas: $PASSED_TESTS" | tee -a "$TEST_LOG"
    echo "   • Pruebas fallidas: $FAILED_TESTS" | tee -a "$TEST_LOG"
    echo "   • Tasa de éxito: $(( (PASSED_TESTS * 100) / TOTAL_TESTS ))%" | tee -a "$TEST_LOG"
    echo "   • Tiempo total: ${duration}s" | tee -a "$TEST_LOG"
    echo "" | tee -a "$TEST_LOG"
    
    if [ $FAILED_TESTS -eq 0 ]; then
        echo -e "${GREEN}🎉 ¡Todas las pruebas pasaron exitosamente!${NC}" | tee -a "$TEST_LOG"
        exit 0
    else
        echo -e "${RED}❌ $FAILED_TESTS pruebas fallaron${NC}" | tee -a "$TEST_LOG"
        echo "Revise los detalles arriba para más información" | tee -a "$TEST_LOG"
        exit 1
    fi
}

# Función principal
main() {
    # Inicializar log
    echo "=== PRUEBAS AUTOMATIZADAS - $(date) ===" > "$TEST_LOG"
    
    show_test_banner
    
    # Verificar que el sistema esté listo
    verify_system_ready
    
    # Ejecutar suites de pruebas
    test_connectivity
    test_valid_transactions
    test_invalid_transactions
    test_security_features
    test_performance
    
    # Generar reporte final
    generate_report
}

# Manejo de señales para limpieza
cleanup() {
    echo ""
    log_info "Limpiando recursos de prueba..."
    rm -f server_logs.tmp
    echo "Pruebas interrumpidas por el usuario"
    exit 130
}

trap cleanup INT TERM

# Ejecutar función principal
main "$@"