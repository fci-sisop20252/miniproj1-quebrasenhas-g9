#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include "hash_utils.h"

#define MAX_WORKERS 16
#define RESULT_FILE "password_found.txt"

long long calculate_search_space(int charset_len, int password_len) {
    long long total = 1;
    for (int i = 0; i < password_len; i++) total *= charset_len;
    return total;
}

void index_to_password(long long index, const char *charset, int charset_len, 
                       int password_len, char *output) {
    for (int i = password_len - 1; i >= 0; i--) {
        output[i] = charset[index % charset_len];
        index /= charset_len;
    }
    output[password_len] = '\0';
}

int main(int argc, char *argv[]) {

    if(argc != 5){
        printf("Ocorreu um erro porque o comando deve ter 5 argumentos. Exemplo de uso: ./coordinator <hash> <tamanho> <charset> <workers>\n");
        return 1;
    }

    const char *target_hash = argv[1];
    int password_len = atoi(argv[2]);
    const char *charset = argv[3];
    int num_workers = atoi(argv[4]);
    int charset_len = strlen(charset);

    if(password_len < 1 || password_len > 10){
        printf("erro, a senha deve ter entre 1 e 10 caracteres.\n");
        return 1;
    }
    if(num_workers < 1 || num_workers > MAX_WORKERS){
        printf("tem que ter mais de 1 e menos de %d workers\n", MAX_WORKERS);
        return 1;
    }
    if (charset_len == 0) {
        fprintf(stderr, "Erro: O charset não pode ser vazio.\n");
        return 1;
    }

    printf("=== Mini-Projeto 1: Quebra de Senhas Paralelo ===\n");
    printf("Hash MD5 alvo: %s\n", target_hash);
    printf("Tamanho da senha: %d\n", password_len);
    printf("Charset: %s (tamanho: %d)\n", charset, charset_len);
    printf("Número de workers: %d\n", num_workers);

    long long total_space = calculate_search_space(charset_len, password_len);
    printf("Espaço de busca total: %lld combinações\n\n", total_space);

    unlink(RESULT_FILE);

    time_t start_time = time(NULL);

    long long passwords_per_worker = total_space / num_workers;
    long long remaining = total_space % num_workers;
    long long start = 0;
    pid_t workers[MAX_WORKERS];

    printf("Iniciando workers...\n");

    for (int i = 0; i < num_workers; i++) {
        long long piece_size = passwords_per_worker;
        if (i < remaining) piece_size++;
        long long end = start + piece_size;

        char start_pass[20], end_pass[20];
        index_to_password(start, charset, charset_len, password_len, start_pass);
        index_to_password(end - 1, charset, charset_len, password_len, end_pass);

        // Criar worker
        pid_t pid = fork();
        if(pid < 0){
            perror("Erro no fork");
            exit(1);
        }

        if(pid == 0){
            // processo filho executa worker
            char worker_id_str[10];
            sprintf(worker_id_str, "%d", i); // worker_id correto
            execl("./worker", "./worker", 
                  target_hash, 
                  start_pass, 
                  end_pass, 
                  charset, 
                  argv[2], // tamanho da senha
                  worker_id_str, // worker id
                  NULL);
            perror("Erro no execl");
            exit(1);
        } else {
            // processo pai armazena PID
            workers[i] = pid;
        }

        start = end;
    }

    printf("\nTodos os workers foram iniciados. Aguardando conclusão...\n");

    // Esperar todos os workers
    for(int i = 0; i < num_workers; i++){
        int status;
        pid_t wpid = waitpid(workers[i], &status, 0);
        if(WIFEXITED(status)){
            printf("[Coordinator] Worker %d terminou com código %d\n", i, WEXITSTATUS(status));
        } else {
            printf("[Coordinator] Worker %d terminou de forma anormal.\n", i);
        }
    }

    time_t end_time = time(NULL);
    double elapsed_time = difftime(end_time, start_time);

    printf("\n=== Resultado ===\n");

    FILE *fp = fopen(RESULT_FILE, "r");
    if(fp){
        char line[256];
        while(fgets(line, sizeof(line), fp)){
            int worker_id;
            char found_password[100];
            if(sscanf(line, "%d:%s", &worker_id, found_password) == 2){
                printf("[RESULTADO] Worker %d: %s\n", worker_id, found_password);
            }
        }
        fclose(fp);
    } else {
        printf("Nenhum worker encontrou a senha.\n");
    }

    printf("\nTempo total: %.2f segundos\n", elapsed_time);

    return 0;
}
