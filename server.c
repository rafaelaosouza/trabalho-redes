#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define PORTA 8888
#define TAM_BUFFER 1024


struct Pacote {
    int seq;
    int ack;
    int rwnd;     
    int flags;     // 1=SYN, 2=ACK, 4=FIM, 8=DADOS
    int tam_dados;
    char dados[TAM_BUFFER];
};

int main() {
    int socket_fd;
    struct sockaddr_in end_servidor, end_cliente;
    socklen_t tam_end = sizeof(end_cliente);
    struct Pacote pkt_recebido, pkt_resposta;
    
    srand(time(NULL));

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        printf("Erro ao criar socket\n");
        return 1;
    }

    memset(&end_servidor, 0, sizeof(end_servidor));
    end_servidor.sin_family = AF_INET;
    end_servidor.sin_addr.s_addr = INADDR_ANY;
    end_servidor.sin_port = htons(PORTA);

    if (bind(socket_fd, (struct sockaddr*)&end_servidor, sizeof(end_servidor)) < 0) {
        printf("Erro no bind\n");
        return 1;
    }

    printf("Servidor rodando na porta %d...\n", PORTA);

    int esperado = 0; 

    while (1) {
        int n = recvfrom(socket_fd, &pkt_recebido, sizeof(struct Pacote), 0, (struct sockaddr*)&end_cliente, &tam_end);
        
        if (n > 0) {
            // SIMULAÇÃO DE PERDA (5% de chance)
            if ((pkt_recebido.flags == 8) && (rand() % 100 < 5)) {
                printf("[Simulacao] Pacote %d perdido propositalmente.\n", pkt_recebido.seq);
                continue;
            }

            // Se for SYN (Handshake)
            if (pkt_recebido.flags == 1) {
                printf("Cliente conectou (SYN).\n");
                pkt_resposta.flags = 3; // SYN + ACK
                pkt_resposta.ack = 0;
                pkt_resposta.rwnd = 100; // Minha janela fixa
                sendto(socket_fd, &pkt_resposta, sizeof(struct Pacote), 0, (struct sockaddr*)&end_cliente, tam_end);
                continue;
            }

            // Se for DADOS
            if (pkt_recebido.flags == 8) {
                for(int i=0; i < pkt_recebido.tam_dados; i++) {
                    pkt_recebido.dados[i] = pkt_recebido.dados[i] ^ 'A';
                }

                if (pkt_recebido.seq == esperado) {
                    esperado++;
                } else {
                    printf("Pacote fora de ordem. Esperava %d, veio %d\n", esperado, pkt_recebido.seq);
                }
            }
            
            // Se for FIM
            if (pkt_recebido.flags == 4) {
                printf("Fim da transmissao.\n");
                break;
            }

            pkt_resposta.flags = 2; // ACK
            pkt_resposta.ack = esperado;
            pkt_resposta.rwnd = 100;
            sendto(socket_fd, &pkt_resposta, sizeof(struct Pacote), 0, (struct sockaddr*)&end_cliente, tam_end);
        }
    }

    close(socket_fd);
    return 0;
}
