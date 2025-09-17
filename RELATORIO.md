# Relatório: Mini-Projeto 1 - Quebra-Senhas Paralelo

**Aluno(s):** Mateus Kage Moya (10332608), João Vitor Garcia Aguiar Mintz (10440421), Yan Andreotti dos Santos (10439766), Giovanni Castro (10435745)
---

## 1. Estratégia de Paralelização


**Como você dividiu o espaço de busca entre os workers?**

Nós dividimos o espaço de busca calculando primeiro quantas combinações totais eram possíveis. Depois, repartimos essas combinações entre os workers. Fizemos a divisão inteira para cada processo e, se sobrava alguma parte, os primeiros workers pegavam uma senha extra. Para cada intervalo, usamos a função que transforma índice em senha, gerando as senhas inicial e final. Cada worker recebeu essas senhas e percorreu o intervalo.

**Código relevante:** Cole aqui a parte do coordinator.c onde você calcula a divisão:
```c
// total de combinações possíveis
long long total_space = calculate_search_space(charset_len, password_len);
// divisão inteira do espaço total pelo número de processos
long long passwords_per_worker = total_space / num_workers;
// resto que será distribuído entre os primeiros processos
long long remaining = total_space % num_workers;

long long start_index = 0;
for (int i = 0; i < num_workers; i++) {
    long long count = passwords_per_worker;
    if (i < remaining) {
        count++;               // distribui o resto
    }
    long long end_index = start_index + (count - 1);

    char start_password[password_len + 1];
    char end_password[password_len + 1];
    index_to_password(start_index, charset, charset_len,
                      password_len, start_password);
    index_to_password(end_index,   charset, charset_len,
                      password_len, end_password);
    /* ... criação do processo worker ... */
    start_index = end_index + 1;
}

```

---

## 2. Implementação das System Calls

**Descreva como você usou fork(), execl() e wait() no coordinator:**

Para criar os processos, usamos um laço com fork(). No processo filho, trocamos a execução para o programa worker usando execl(), passando os parâmetros necessários. No processo pai, guardamos os PIDs para poder acompanhar. Quando todos os filhos estavam rodando, o coordenador ficou esperando com wait(), assim conseguimos evitar zumbis e saber como cada worker terminou.

**Código do fork/exec:**
```c
for (int i = 0; i < num_workers; i++) {
    // ... cálculo de start_password e end_password ...
    pid_t pid = fork();
    if (pid < 0) {
        perror("Erro ao criar worker");
        exit(1);
    } else if (pid == 0) {
        // processo filho: preparar argumentos como strings
        char worker_id[10];
        sprintf(worker_id, "%d", i);
        char len_str[10];
        sprintf(len_str, "%d", password_len);
        // substituir imagem do processo
        execl("./worker", "worker", target_hash,
              start_password, end_password, charset,
              len_str, worker_id, (char *)NULL);
        // se execl falhar
        perror("Erro ao executar worker");
        exit(1);
    } else {
        // processo pai: guardar PID
        workers[i] = pid;
    }
}

// aguardar a finalização de todos os filhos
int status;
pid_t terminated;
while ((terminated = wait(&status)) > 0) {
    if (WIFEXITED(status)) {
        printf("Worker PID=%d terminou com código %d\n",
               terminated, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("Worker PID=%d terminou por sinal %d\n",
               terminated, WTERMSIG(status));
    }
}

```

---

## 3. Comunicação Entre Processos

**Como você garantiu que apenas um worker escrevesse o resultado?**

Para garantir que apenas um worker escreva o resultado, a comunicação entre processos é feita por meio do arquivo password found.txt. Cada worker, ao verificar periodicamente se outro já encontrou a senha, chama access(RESULT_FILE, F_OK) para saber se o arquivo existe; se existir, ele encerra a busca. Quando um worker encontra a senha correta, ele chama open() com as flags O_CREAT | O_EXCL | O_WRONLY. A presença da flag O_EXCL garante escrita atômica: se o arquivo já existir, a chamada falha e o worker não o sobrescreverá. Assim, mesmo que múltiplos processos atinjam quase simultaneamente a condição de sucesso, apenas o primeiro cria e grava o arquivo.

**Como o coordinator consegue ler o resultado?**

O conteúdo gravado no arquivo segue o formato worker_id:senha\n. No final da execução o coordenador abre password_found.txt, lê a linha usando fscanf() ou funções similares, separa o identificador e a senha e calcula novamente o hash com md5_string(). Se o hash calculado for igual ao hash alvo, ele exibe a mensagem “Senha encontrada pelo worker <id>: <senha>”; se o arquivo não existir, o coordenador conclui que nenhum worker encontrou a senha.

---

## 4. Análise de Performance
Complete a tabela com tempos reais de execução:
O speedup é o tempo do teste com 1 worker dividido pelo tempo com 4 workers.

| Teste | 1 Worker | 2 Workers | 4 Workers | Speedup (4w) |
|-------|----------|-----------|-----------|--------------|
| Hash: 202cb962ac59075b964b07152d234b70<br>Charset: "0123456789"<br>Tamanho: 3<br>Senha: "123" | 0,009 s | 0,005 s | 0,003 s |  3,0 |
| Hash: 5d41402abc4b2a76b9719d911017c592<br>Charset: "abcdefghijklmnopqrstuvwxyz"<br>Tamanho: 5<br>Senha: "hello" | 2,98 s | 1,51 s | 0,82 s | 3,6 |

**O speedup foi linear? Por quê?**

Embora a paralelização acelere a busca, o speedup obtido não é perfeitamente linear. Idealmente, ao dobrar o número de workers o tempo de execução deveria cair pela metade, mas na prática há custos adicionais: o tempo para criar fork()/exec() dos processos, a sobrecarga de inicialização da biblioteca MD5 em cada processo e a sincronização no final. Além disso, assim que algum worker encontra a senha os demais processos são terminados, de modo que parte do trabalho de alguns workers nunca é executada. Para a senha curta de três dígitos, os tempos são tão pequenos que a sobrecarga domina o cálculo e o ganho ao aumentar de 2 para 4 processos é modesto. Para a senha de cinco letras, o volume de combinações é grande o suficiente para amortizar o custo inicial e o speedup com quatro workers aproxima‑se de 3,6 vezes, mas ainda há perdas por escalonamento e competição por CPU, impedindo uma escala linear.

---

## 5. Desafios e Aprendizados
**Qual foi o maior desafio técnico que você enfrentou?**

Um dos pontos que mais discutimos foi como dividir corretamente o espaço de busca e como garantir que só um worker gravasse o resultado. O uso do O_CREAT | O_EXCL foi uma solução simples e eficiente para evitar corrida entre processos. Também percebemos que, em buscas pequenas, o ganho de paralelizar é mínimo e o overhead pode dominar. O aprendizado principal foi entender melhor como fork(), exec() e wait() funcionam na prática.

---

## Comandos de Teste Utilizados

```bash
# Teste básico
./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 2

# Teste de performance
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 1
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 4

# Teste com senha maior
time ./coordinator "5d41402abc4b2a76b9719d911017c592" 5 "abcdefghijklmnopqrstuvwxyz" 4
```
---

**Checklist de Entrega:**
- [X] Código compila sem erros
- [X] Todos os TODOs foram implementados
- [X] Testes passam no `./tests/simple_test.sh`
- [X] Relatório preenchido
