#!/bin/bash

# Script de Teste Local para o Mini-Projeto 1: Quebra-Senhas Paralelo
# Este script ajuda os alunos a validarem suas implementações localmente
# Foca nos aspectos de paralelização - MD5 já está implementado

echo "=== Teste do Mini-Projeto 1: Quebra-Senhas Paralelo ==="
echo "Este script testa sua implementação com casos conhecidos."
echo "LEMBRE-SE: O MD5 já está pronto - foque na paralelização!"
echo ""

# Cores para output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Contadores
TESTS_PASSED=0
TESTS_FAILED=0

# Função para verificar se arquivo existe
check_file() {
    if [ ! -f "$1" ]; then
        echo -e "${RED}✗ Arquivo $1 não encontrado!${NC}"
        return 1
    fi
    return 0
}

# Função para executar teste
run_test() {
    local test_name="$1"
    local hash="$2"
    local length="$3"
    local charset="$4"
    local workers="$5"
    local expected_password="$6"
    
    echo -e "\n${YELLOW}[Teste] $test_name${NC}"
    echo "Hash: $hash"
    echo "Parâmetros: tamanho=$length, charset='$charset', workers=$workers"
    echo "Senha esperada: '$expected_password'"
    
    # Limpar arquivo anterior
    rm -f password_found.txt
    
    # Executar com timeout de 30 segundos
    timeout 30s ./coordinator "$hash" "$length" "$charset" "$workers" > test_output.tmp 2>&1
    local exit_code=$?
    
    if [ $exit_code -eq 124 ]; then
        echo -e "${RED}✗ FALHOU: Timeout (>30s)${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return
    elif [ $exit_code -ne 0 ]; then
        echo -e "${RED}✗ FALHOU: Coordinator retornou código $exit_code${NC}"
        echo "Output:"
        cat test_output.tmp
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return
    fi
    
    # Verificar se arquivo foi criado
    if [ ! -f "password_found.txt" ]; then
        echo -e "${RED}✗ FALHOU: Arquivo password_found.txt não foi criado${NC}"
        echo "Output do coordinator:"
        cat test_output.tmp
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return
    fi
    
    # Verificar conteúdo do arquivo
    local found_password=$(cut -d':' -f2 password_found.txt | tr -d '\n\r ')
    if [ "$found_password" = "$expected_password" ]; then
        echo -e "${GREEN}✓ PASSOU: Senha '$found_password' encontrada corretamente${NC}"
        
        # Verificar se o hash está correto
        local computed_hash=$(./test_hash "$found_password" | grep "MD5:" | awk '{print $2}')
        if [ "$computed_hash" = "$hash" ]; then
            echo -e "${GREEN}✓ Hash verificado corretamente${NC}"
            TESTS_PASSED=$((TESTS_PASSED + 1))
        else
            echo -e "${RED}✗ FALHOU: Hash incorreto. Esperado: $hash, Obtido: $computed_hash${NC}"
            TESTS_FAILED=$((TESTS_FAILED + 1))
        fi
    else
        echo -e "${RED}✗ FALHOU: Senha incorreta. Esperada: '$expected_password', Encontrada: '$found_password'${NC}"
        echo "Conteúdo do arquivo:"
        cat password_found.txt
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    
    # Mostrar tempo de execução
    local tempo=$(grep "Tempo total:" test_output.tmp | awk '{print $3}')
    if [ -n "$tempo" ]; then
        echo "Tempo de execução: ${tempo}s"
    fi
}

# Função para teste de performance
performance_test() {
    local test_name="$1"
    local hash="$2"
    local length="$3"
    local charset="$4"
    local expected_password="$5"
    
    echo -e "\n${YELLOW}[Teste de Performance] $test_name${NC}"
    echo "Testando com diferentes números de workers..."
    
    for workers in 1 2 4; do
        echo -n "  $workers worker(s): "
        rm -f password_found.txt
        
        start_time=$(date +%s.%N)
        timeout 60s ./coordinator "$hash" "$length" "$charset" "$workers" >/dev/null 2>&1
        end_time=$(date +%s.%N)
        
        if [ -f "password_found.txt" ]; then
            local found_password=$(cut -d':' -f2 password_found.txt | tr -d '\n\r ')
            if [ "$found_password" = "$expected_password" ]; then
                local elapsed=$(echo "$end_time - $start_time" | bc -l)
                echo -e "${GREEN}$(printf "%.2f" $elapsed)s${NC}"
            else
                echo -e "${RED}senha incorreta${NC}"
            fi
        else
            echo -e "${RED}falhou${NC}"
        fi
    done
}

# Verificar se os binários existem
echo "Verificando binários do mini-projeto..."
for binary in coordinator worker test_hash; do
    if ! check_file "$binary"; then
        echo -e "${RED}Execute 'make all' primeiro!${NC}"
        echo "Binários necessários: coordinator, worker, test_hash"
        exit 1
    fi
done
echo -e "${GREEN}✓ Todos os binários encontrados${NC}"

# Testar MD5 fornecido
echo -n "Testando biblioteca MD5 fornecida: "
MD5_OUTPUT=$(./test_hash abc 2>/dev/null | grep "MD5:" | awk '{print $2}')
if [ "$MD5_OUTPUT" = "900150983cd24fb0d6963f7d28e17f72" ]; then
    echo -e "${GREEN}✓ MD5 funcionando${NC}"
else
    echo -e "${RED}✗ Problema com MD5${NC}"
    echo "Execute: ./test_hash para diagnosticar"
fi

# Teste 1: Hash MD5 básico - senha "abc"
run_test "Hash Simples (abc)" \
    "900150983cd24fb0d6963f7d28e17f72" \
    "3" \
    "abc" \
    "2" \
    "abc"

# Teste 2: Senha numérica "123" 
run_test "Senha Numérica (123)" \
    "202cb962ac59075b964b07152d234b70" \
    "3" \
    "0123456789" \
    "4" \
    "123"

# Teste 3: Senha no final do espaço
run_test "Senha no Final (zzz)" \
    "f3abb86bd34cf4d52698f14c0da1dc60" \
    "3" \
    "xyz" \
    "3" \
    "zzz"

# Teste 4: Apenas um worker
run_test "Um Worker Apenas" \
    "900150983cd24fb0d6963f7d28e17f72" \
    "3" \
    "abc" \
    "1" \
    "abc"

# Teste 5: Charset maior
run_test "Charset Maior" \
    "5d41402abc4b2a76b9719d911017c592" \
    "5" \
    "abcdefghijklmnopqrstuvwxyz" \
    "4" \
    "hello"

# Teste de Performance
echo -e "\n${YELLOW}[Teste de Performance] Speedup com Múltiplos Workers${NC}"
echo "Testando se paralelização realmente acelera a busca:"
performance_test "Speedup Test" \
    "202cb962ac59075b964b07152d234b70" \
    "3" \
    "0123456789" \
    "123"

# Teste de Edge Cases
echo -e "\n${YELLOW}[Testes de Edge Cases]${NC}"

echo -n "Argumentos insuficientes: "
./coordinator 2>/dev/null
if [ $? -ne 0 ]; then
    echo -e "${GREEN}✓ Erro tratado corretamente${NC}"
else
    echo -e "${RED}✗ Deveria retornar erro${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo -n "Hash inválido (não encontrado): "
rm -f password_found.txt
timeout 10s ./coordinator "hash_inexistente" "2" "ab" "2" >/dev/null 2>&1
if [ ! -f "password_found.txt" ]; then
    echo -e "${GREEN}✓ Nenhuma senha encontrada (correto)${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}✗ Não deveria encontrar senha${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Verificar processos zumbi
echo -n "Verificando processos zumbi: "
./coordinator "900150983cd24fb0d6963f7d28e17f72" "3" "abc" "4" >/dev/null 2>&1
sleep 1
ZOMBIES=$(ps aux | grep -v grep | grep defunct | wc -l)
if [ $ZOMBIES -eq 0 ]; then
    echo -e "${GREEN}✓ Nenhum processo zumbi${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}✗ $ZOMBIES processo(s) zumbi encontrado(s)${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Limpeza
rm -f test_output.tmp password_found.txt

# Resultado final
echo -e "\n=== Resultado Final ==="
TOTAL_TESTS=$((TESTS_PASSED + TESTS_FAILED))
echo "Total de testes: $TOTAL_TESTS"
echo -e "Passou: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Falhou: ${RED}$TESTS_FAILED${NC}"

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "\n${GREEN}🎉 Todos os testes passaram! Sua implementação está funcionando.${NC}"
    echo -e "✅ Coordinator, Worker e comunicação estão corretos"
    echo -e "✅ Paralelização com fork/exec/wait implementada"
    echo -e "✅ Ready para submissão!"
    exit 0
else
    echo -e "\n${YELLOW}⚠️  Alguns testes falharam. Verifique os erros acima.${NC}"
    echo -e "📖 Consulte docs/DEBUGGING.md para dicas de debugging"
    echo -e "📚 Revise docs/SYSCALLS.md para fork/exec/wait"
    echo -e "🗺️ Siga o TUTORIAL.md passo-a-passo"
    exit 1
fi
