#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define PORTA 8888
#define IP_SERV "127.0.0.1"
#define TAM_BUFFER 1024
#define TOTAL_PACOTES 10000

struct Pacote {
    int seq;
    int ack;
    int rwnd;
    int flags; // 1=SYN, 2=ACK, 4=FIM, 8=DADOS
    int tam_dados;
    char dados[TAM_BUFFER];
};

double tempo_atual() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}

int main() {
    int socket_fd;
    struct sockaddr_in end_servidor;
    struct Pacote pkt, resposta;
    socklen_t tam_end = sizeof(end_servidor);

    FILE *arquivo_log = fopen("dados.csv", "w");
    fprintf(arquivo_log, "Tempo,Cwnd\n");

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        printf("Erro socket\n");
        return 1;
    }

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000; 
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    memset(&end_servidor, 0, sizeof(end_servidor));
    end_servidor.sin_family = AF_INET;
    end_servidor.sin_port = htons(PORTA);
    inet_pton(AF_INET, IP_SERV, &end_servidor.sin_addr);

    printf("Conectando...\n");
    pkt.flags = 1; // SYN
    pkt.seq = 0;
    sendto(socket_fd, &pkt, sizeof(struct Pacote), 0, (struct sockaddr*)&end_servidor, tam_end);
    
    recvfrom(socket_fd, &resposta, sizeof(struct Pacote), 0, NULL, NULL);
    printf("Conectado! Janela do servidor: %d\n", resposta.rwnd);

    // VARIAVEIS DE CONTROLE
    int base = 0;
    int prox_seq = 0;
    float cwnd = 1.0;     
    int ssthresh = 64;  
    int rwnd_servidor = resposta.rwnd;
    int estado_slow_start = 1; // 1 = Sim, 0 = Congestion Avoidance
    
    double inicio = tempo_atual();

    while (base < TOTAL_PACOTES) {
        
        int limite_janela = (int)cwnd;
        if (limite_janela > rwnd_servidor) {
            limite_janela = rwnd_servidor;
        }

        while (prox_seq < base + limite_janela && prox_seq < TOTAL_PACOTES) {
            pkt.seq = prox_seq;
            pkt.flags = 8;
            sprintf(pkt.dados, "Dados %d", prox_seq);
            pkt.tam_dados = strlen(pkt.dados);

            for(int i=0; i < pkt.tam_dados; i++) {
                pkt.dados[i] = pkt.dados[i] ^ 'A';
            }

            sendto(socket_fd, &pkt, sizeof(struct Pacote), 0, (struct sockaddr*)&end_servidor, tam_end);
            prox_seq++;
        }

        int n = recvfrom(socket_fd, &resposta, sizeof(struct Pacote), 0, NULL, NULL);

        if (n < 0) {
            ssthresh = (int)(cwnd / 2);
            if (ssthresh < 2) ssthresh = 2;
            cwnd = 1.0;
            estado_slow_start = 1;

            prox_seq = base;

            fprintf(arquivo_log, "%f,%.2f\n", tempo_atual() - inicio, cwnd);

        } else {
            if (resposta.ack > base) {
                base = resposta.ack;
                rwnd_servidor = resposta.rwnd; 

                if (estado_slow_start) {
                    cwnd = cwnd + 1.0;
                    if (cwnd >= ssthresh) {
                        estado_slow_start = 0; 
                    }
                } else {
                    cwnd = cwnd + (1.0 / cwnd); 
                }

                fprintf(arquivo_log, "%f,%.2f\n", tempo_atual() - inicio, cwnd);
            }
        }
    }

    printf("Terminou envio. Mandando FIN.\n");
    pkt.flags = 4;
    sendto(socket_fd, &pkt, sizeof(struct Pacote), 0, (struct sockaddr*)&end_servidor, tam_end);

    fclose(arquivo_log);
    close(socket_fd);
    return 0;
}
