# Trabalho Final de Redes - Protocolo Confiável sobre UDP

**Aluno:** Rafaela Oliveira de Souza

## Descrição
Implementação de um protocolo de transporte confiável (RDT) sobre UDP em linguagem C. O protocolo implementa mecanismos de confiabilidade, controle de fluxo e controle de congestionamento similares ao TCP.

## Funcionalidades Implementadas
1.  **Entrega Ordenada:** Utilização de números de sequência para garantir a ordem dos pacotes.
2.  **ACK Acumulativo:** Confirmação de recebimento baseada no último pacote recebido em ordem.
3.  **Controle de Fluxo:** O remetente respeita a janela do receptor (`rwnd`) informada nos ACKs.
4.  **Controle de Congestionamento:** Implementação de máquina de estados com fases *Slow Start* e *Congestion Avoidance*, utilizando as variáveis `cwnd` e `ssthresh`.
5.  **Criptografia:** Cifra XOR simples aplicada ao payload dos pacotes de dados.
6.  **Simulação de Falhas:** O servidor descarta aleatoriamente 5% dos pacotes para testar a robustez do protocolo.

## Como Compilar e Executar
1. **Compilar:**
   ```bash
   gcc server.c -o server
   gcc client.c -o client

2. **Executar o Servidor:**
   ./server

2. **Executar o Cliente (em outro terminal):**
   ./client
