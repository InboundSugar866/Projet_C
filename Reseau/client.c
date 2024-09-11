#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define LEPORT 2020
#define BUFFER_SIZE 1000024

int main() {
    int s = 0, bd;
    struct sockaddr_in padin;
    socklen_t p_lgexp = sizeof(struct sockaddr_in);
    char *name = "localhost";
    char *adr_serv = "127.0.0.1";
    char msg[40];
    char msg1[40];
    struct hostent *host;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == -1) {
        perror("Erreur création socket");
        exit(EXIT_FAILURE);
    }

    bzero((char *)&padin, sizeof(struct sockaddr_in));

    if ((host = gethostbyname(name)) == NULL) {
        perror("Nom de machine");
        exit(EXIT_FAILURE);
    }
    bcopy(host->h_addr_list[0], &padin.sin_addr, host->h_length);

    padin.sin_family = AF_INET;
    padin.sin_port = htons(LEPORT);

    while (1) {
        printf("Nom du fichier source :  ");
        fgets(msg, sizeof(msg), stdin);
        msg[strcspn(msg, "\n")] = '\0';

        printf("Nom du fichier : '%s'\n", msg);

        char filename[50];
        sprintf(filename, "%s.txt", msg);
        FILE *file = fopen(filename, "rb");
        if (file == NULL) {
            perror("Erreur lors de l'ouverture du fichier");
            continue;  // Continue to the next iteration of the loop
        }

        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);

        char *file_content = (char *)malloc(file_size + 1);
        if (file_content == NULL) {
            perror("Erreur allocation mémoire");
            fclose(file);
            continue;
        }

        fread(file_content, 1, file_size, file);
        file_content[file_size] = '\0';  // Null-terminate the string
        printf("contenu : \n%s\n", file_content);
        fclose(file);

        bd = sendto(s, file_content, file_size, 0, (struct sockaddr *)&padin, sizeof(padin));
        if (bd == -1) {
            perror("Erreur send");
            free(file_content);
            exit(EXIT_FAILURE);
        }
        
      
		char buffer[BUFFER_SIZE];
		bd = recvfrom(s, buffer, sizeof(buffer), 0, (struct sockaddr *)&padin, &p_lgexp);
        if (bd == -1) {
            perror("Erreur receive");
            exit(EXIT_FAILURE);
        }

        printf("Serveur : \n%s\n", buffer);
        free(file_content);
        
    }
    
    close(s);

    return 0;
}
