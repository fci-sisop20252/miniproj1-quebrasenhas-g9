[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/N_JmK8vD)
# 🔐 Mini-Projeto 1: Quebra-Senhas Paralelo

**Professor:** Lucas Figueiredo  
**Email:** lucas.figueiredo@mackenzie.br

Aprenda paralelização de processos implementando um quebra-senhas MD5 paralelo usando fork(), exec() e wait().

## 🚀 Quick Start

```bash
# 1. Compilar o projeto (vai ter warnings - TODOs não implementados ainda!)
make all

# 2. Testar a biblioteca MD5 fornecida (isso já funciona)
./test_hash abc
# Saída esperada: MD5: 900150983cd24fb0d6963f7d28e17f72

# 3. Tentar executar o coordinator (NÃO VAI FUNCIONAR ainda!)
./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 2
# ⚠️ Não funcionará até você implementar os TODOs em coordinator.c e worker.c

# 4. Após implementar todos os TODOs, teste com:
./tests/simple_test.sh
```

**📝 Importante:** Este é um projeto para VOCÊ implementar! Siga o `TUTORIAL.md` para o guia passo-a-passo.

### ⚠️ **Importante**
O **hash MD5 já está implementado** e funcional. Seu trabalho é implementar:
- 🔧 **Coordinator**: Reponsável por gerenciar os workers
- 🔧 **Worker**: Algoritmo de quebra de senhas por força bruta
- 📞 **Comunicação**: Via arquivo entre processos

## 📚 Documentação do Projeto

### 📖 Leia Primeiro
1. **[`TUTORIAL.md`](TUTORIAL.md)** - Guia passo-a-passo do miniprojeto
2. **[`docs/SYSCALLS.md`](docs/SYSCALLS.md)** - fork(), exec(), wait() detalhados
3. **[`docs/CONCEITOS_C.md`](docs/CONCEITOS_C.md)** - Conceitos de C necessários

### 🛠️ Para Debugging e Análise
4. **[`docs/DEBUGGING.md`](docs/DEBUGGING.md)** - Técnicas de debugging
5. **[`RELATORIO_TEMPLATE.md`](RELATORIO_TEMPLATE.md)** - Template do relatório final

## 📁 Estrutura do Projeto

```
miniproj1-quebrasenhas/
├── src/                          # Código fonte
│   ├── coordinator.c             # 🔧 TODO: Processo coordenador
│   ├── worker.c                  # 🔧 TODO: Processo trabalhador  
│   ├── hash_utils.c/h            # ✅ Utilitários MD5 (fornecidos)
│   └── test_hash.c               # ✅ Teste MD5 (fornecido)
├── docs/                         # Documentação
│   ├── SYSCALLS.md               # 📖 fork/exec/wait detalhados
│   ├── CONCEITOS_C.md            # 📖 Conceitos de C necessários
│   └── DEBUGGING.md              # 🐛 Guia de debugging
├── tests/                        # Testes automatizados
│   ├── simple_test.sh            # 🧪 Script de teste local
│   └── expected_output.txt       # 📋 Exemplos de saída
├── .github/classroom/            # 🤖 Autograding GitHub Classroom
├── TUTORIAL.md                   # 📚 Guia passo-a-passo (10 dias)
├── RELATORIO_TEMPLATE.md         # 📝 Template do relatório
└── Makefile                      # ⚙️ Compilação automatizada
```

## 🎯 Como Fazer o Projeto

### 🛣️ Roteiro de Desenvolvimento
1. **Leia o [`TUTORIAL.md`](TUTORIAL.md)** - Passo a passo
2. **Entenda o problema** - MD5 e paralelização
3. **Implemente o Worker** - Algoritmo de busca de senhas
4. **Implemente o Coordinator** - fork(), exec(), wait()
5. **Teste e Debug** - Use `./tests/simple_test.sh`
6. **Análise de Performance** - Diferentes números de workers
7. **Preencha o Relatório** - [`RELATORIO_TEMPLATE.md`](RELATORIO_TEMPLATE.md)

### 🧩 Componentes do Sistema
- **Coordinator** - Processo principal que divide o trabalho entre workers
- **Worker** - Processo que verifica senhas em um intervalo específico  
- **Hash MD5** - Biblioteca fornecida para calcular hashes (pronta)
- **Comunicação** - Coordenação via arquivo `password_found.txt`

## ⚡ Referência Rápida
### 🛠️ Compilação
```bash
make all                    # Compila coordinator, worker, test_hash
make clean                  # Remove binários
make help                   # Mostra ajuda do Makefile
```

### 🧪 Testes
```bash
# Teste automatizado completo
./tests/simple_test.sh

# Teste manual - senha "abc"  
./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 2

# Verificar hash (implementação fornecida)
./test_hash abc
echo -n "abc" | md5sum     # Comparar com sistema
```

### 🔍 Debugging
```bash
# Debug com GDB
gdb ./coordinator
(gdb) run "hash" 3 "abc" 2

# Monitorar processos
ps aux | grep coordinator
ps aux | grep worker

# Verificar processos zumbi
ps aux | grep defunct
```

## 🏆 Exemplos de Hash para Teste

| Senha    | Hash MD5 |
|----------|----------|
| "abc"    | `900150983cd24fb0d6963f7d28e17f72` |
| "123"    | `202cb962ac59075b964b07152d234b70` |
| "hello"  | `5d41402abc4b2a76b9719d911017c592` |
| "password" | `5f4dcc3b5aa765d61d8327deb882cf99` |

## 📤 Avaliação

### ✅ Checklist de Entrega
- [ ] Código compila sem warnings (`make all`)
- [ ] **TODOs implementados** em coordinator.c e worker.c
- [ ] **Testes passam** (`./tests/simple_test.sh`)  
- [ ] **Comunicação funciona** via password_found.txt
- [ ] **Performance melhora** com mais workers
- [ ] **Relatório preenchido** (`RELATORIO_TEMPLATE.md`)

### 📝 Submissão
```bash
git add .
git commit -m "Mini-Projeto 1: Quebra-senhas paralelo implementado"
git push
```

## ❓ Precisa de Ajuda?

1. **🗺️ Roteiro**: Siga o [`TUTORIAL.md`](TUTORIAL.md) passo-a-passo (10 dias)
2. **📖 System Calls**: Leia [`docs/SYSCALLS.md`](docs/SYSCALLS.md) para fork/exec/wait
3. **💻 Programação C**: Consulte [`docs/CONCEITOS_C.md`](docs/CONCEITOS_C.md)
4. **🐛 Debugging**: Use [`docs/DEBUGGING.md`](docs/DEBUGGING.md) para resolver problemas
5. **🧪 Testes**: Execute `./tests/simple_test.sh` para validação local

## 🎯 Objetivo de Aprendizado

Este mini-projeto ensina **paralelização de processos** na prática:
- ✅ Criação e gerenciamento de múltiplos processos
- ✅ Comunicação entre processos via arquivo
- ✅ Sincronização com wait()
- ✅ Análise de performance paralela
- ✅ Debugging de sistemas concorrentes

**MD5 é apenas o contexto** - o foco está na paralelização!