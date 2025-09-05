#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include "hash_utils.h"

/**
 * PROCESSO COORDENADOR - Mini-Projeto 1: Quebra de Senhas Paralelo
 * 
 * Este programa coordena múltiplos workers para quebrar senhas MD5 em paralelo.
 * O MD5 JÁ ESTÁ IMPLEMENTADO - você deve focar na paralelização (fork/exec/wait).
 * 
 * Uso: ./coordinator <hash_md5> <tamanho> <charset> <num_workers>
 * 
 * Exemplo: ./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 4
 * 
 * SEU TRABALHO: Implementar os TODOs marcados abaixo
 */

#define MAX_WORKERS 16
#define RESULT_FILE "password_found.txt"

/**
 * Calcula o tamanho total do espaço de busca
 * 
 * @param charset_len Tamanho do conjunto de caracteres
 * @param password_len Comprimento da senha
 * @return Número total de combinações possíveis
 */
long long calculate_search_space(int charset_len, int password_len) {
    long long total = 1;
    for (int i = 0; i < password_len; i++) {
        total *= charset_len;
    }
    return total;
}

/**
 * Converte um índice numérico para uma senha
 * Usado para definir os limites de cada worker
 * 
 * @param index Índice numérico da senha
 * @param charset Conjunto de caracteres
 * @param charset_len Tamanho do conjunto
 * @param password_len Comprimento da senha
 * @param output Buffer para armazenar a senha gerada
 */
void index_to_password(long long index, const char *charset, int charset_len, 
                       int password_len, char *output) {
    for (int i = password_len - 1; i >= 0; i--) {
        output[i] = charset[index % charset_len];
        index /= charset_len;
    }
    output[password_len] = '\0';
}

/**
 * Função principal do coordenador
 */
int main(int argc, char *argv[]) {
    // TODO 1: Validar argumentos de entrada
    // Verificar se argc == 5 (programa + 4 argumentos)
    // Se não, imprimir mensagem de uso e sair com código 1
    
    // IMPLEMENTE AQUI: verificação de argc e mensagem de erro

    if (argc != 5) {
        fprintf(stderr, "Uso: %s <hash_md5> <tamanho> <charset> <num_workers>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s \"900150983cd24fb0d6963f7d28e17f72\" 3 \"abc\" 4\n", argv[0]);
        exit(1);
    }

    // TODO 1 FEITO!
    
    // Parsing dos argumentos (após validação)
    const char *target_hash = argv[1];
    int password_len = atoi(argv[2]);
    const char *charset = argv[3];
    int num_workers = atoi(argv[4]);
    int charset_len = strlen(charset);
    
    // TODO: Adicionar validações dos parâmetros
    // - password_len deve estar entre 1 e 10
    // - num_workers deve estar entre 1 e MAX_WORKERS
    // - charset não pode ser vazio

    if (password_len < 1 || password_len > 10) {
        fprintf(stderr, "Erro!! O tamanho da senha deve ser entre 1 e 10 caracteres!!\n");
        exit(1);
    }

    if (num_workers < 1 || num_workers > MAX_WORKERS) {
        fprintf(stderr, "Erro!! O número de trabalhadores deve ser entre 1 e %d trabalhadores!\n" , MAX_WORKERS);
        exit(1);
    }

    if (charset == NULL || strlen(charset) == 0) {
        fprintf(stderr, "Erro!! Charset não pode ser vazio!\n");
        exit(1);
    }
    
    // TODO (acidionar validação dos parâmentros) FEITO!
    
    printf("=== Mini-Projeto 1: Quebra de Senhas Paralelo ===\n");
    printf("Hash MD5 alvo: %s\n", target_hash);
    printf("Tamanho da senha: %d\n", password_len);
    printf("Charset: %s (tamanho: %d)\n", charset, charset_len);
    printf("Número de workers: %d\n", num_workers);
    
    // Calcular espaço de busca total
    long long total_space = calculate_search_space(charset_len, password_len);
    printf("Espaço de busca total: %lld combinações\n\n", total_space);
    
    // Remover arquivo de resultado anterior se existir
    unlink(RESULT_FILE);
    
    // Registrar tempo de início
    time_t start_time = time(NULL);
    
    // TODO 2: Dividir o espaço de busca entre os workers
    // Calcular quantas senhas cada worker deve verificar
    // DICA: Use divisão inteira e distribua o resto entre os primeiros workers

    // IMPLEMENTE AQUI:
    long long passwords_per_worker = total_space / num_workers;
    long long remaining = total_space % num_workers;


    printf("Cada trabalhador processará aproximadamente %lld senhas!\n" , passwords_per_worker);
    if (remaining > 0) {
        printf("%lld workers processarão uma senha extra.\n", remaining);
    }

    
    // Arrays para armazenar PIDs dos workers
    pid_t workers[MAX_WORKERS];
    
    // TODO 3: Criar os processos workers usando fork()
    printf("Iniciando workers...\n");
    
    // IMPLEMENTE AQUI: Loop para criar workers

    long long start_index = 0;


    for (int i = 0; i < num_workers; i++) {
        // TODO: Calcular intervalo de senhas para este worker
        long long count = passwords_per_worker;
        if (i < remaining) {
            count ++; // Distribuir o resto
        }

        long long end_index = start_index + (count - 1);
        // TODO: Converter indices para senhas de inicio e fim
        char start_password[password_len + 1];
        char end_password[password_len + 1];
        index_to_password(start_index, charset, charset_len, password_len, start_password);
        index_to_password(end_index, charset, charset_len, password_len, end_password);

        // TODO 4: Usar fork() para criar processo filho
        // TODO 5: No processo pai: armazenar PID
        // TODO 6: No processo filho: usar execl() para executar worker
        // TODO 7: Tratar erros de fork() e execl()
        pid_t pid = fork();
        if (pid < 0) {
            perror("Erro ao criar worker");
            exit(1);
        } else if (pid == 0) {
            char worker_id[10];
            sprintf(worker_id, "%d", i);
            char len_str[10];
            sprintf(len_str, "%d", password_len);
            execl("./worker", "worker", target_hash, start_password, end_password, charset, len_str, worker_id, (char *)NULL);
            // Caso falhe
            perror("Erro ao executar worker");
            exit(1);
        } else {
            workers[i] = pid;
        }

        start_index = end_index + 1;
    }
    
    printf("\nTodos os workers foram iniciados. Aguardando conclusão...\n");
    
    // TODO 8: Aguardar todos os workers terminarem usando wait()
    // IMPORTANTE: O pai deve aguardar TODOS os filhos para evitar zumbis
    
    // IMPLEMENTE AQUI:
    // - Loop para aguardar cada worker terminar
    // - Usar wait() para capturar status de saída
    // - Identificar qual worker terminou
    // - Verificar se terminou normalmente ou com erro
    // - Contar quantos workers terminaram

    int status;
    int finished = 0;
    pid_t pid;

    while ((pid = wait(&status)) > 0) {
        finished++;
        if (WIFEXITED(status)) {
            printf("Worker PID=%d terminou com código %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Worker PID=%d terminou por sinal %d\n", pid, WTERMSIG(status));
        }
    }

    printf("Total de workers finalizados: %d\n", finished);
    
    // Registrar tempo de fim
    time_t end_time = time(NULL);
    double elapsed_time = difftime(end_time, start_time);
    
    printf("\n=== Resultado ===\n");
    
    // TODO 9: Verificar se algum worker encontrou a senha
    // Ler o arquivo password_found.txt se existir
    
    // IMPLEMENTE AQUI:
    // - Abrir arquivo RESULT_FILE para leitura
    // - Ler conteúdo do arquivo
    // - Fazer parse do formato "worker_id:password"
    // - Verificar o hash usando md5_string()
    // - Exibir resultado encontrado
    
    // Estatísticas finais (opcional)
    // TODO: Calcular e exibir estatísticas de performance
    
     FILE *f = fopen(RESULT_FILE, "r");
    if (f) {
        int worker_id;
        char password[11];
        if (fscanf(f, "%d:%10s", &worker_id, password) == 2) {
            char verify_hash[33];
            md5_string(password, verify_hash);
            if (strcmp(verify_hash, target_hash) == 0) {
                printf("Senha encontrada pelo worker %d: %s\n", worker_id, password);
            } else {
                printf("Arquivo de resultado encontrado, mas hash não confere!\n");
            }
        }
        fclose(f);
    } else {
        printf("Nenhum worker encontrou a senha.\n");
    }
    return 0;
}
