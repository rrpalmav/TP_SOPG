#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#define PORT 5000
#define BUFFER_SIZE 1024

// Función para manejar el comando SET
void handle_set(int client_sock, char *key, char *value) {
    char filename[BUFFER_SIZE];
    snprintf(filename, sizeof(filename), "./%s", key);
    
    // Crear archivo con el contenido indicado
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error al crear archivo");
        write(client_sock, "ERROR\n", 6);
        return;
    }

    fprintf(file, "%s\n", value);
    fclose(file);

    // Responder OK
    write(client_sock, "OK\n", 3);
}

// Función para manejar el comando GET
/*
void handle_get(int client_sock, char *key) {
    char filename[BUFFER_SIZE];
    snprintf(filename, sizeof(filename), "./%s", key);

    // Intentar abrir el archivo
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        // Si no existe, responder NOTFOUND
        write(client_sock, "NOTFOUND\n", 9);
        return;
    }

    // Leer el contenido del archivo
    char value[BUFFER_SIZE];
    fread(value, sizeof(char), sizeof(value), file);
    fclose(file);

    // Responder OK seguido del valor
    write(client_sock, "OK\n", 3);
    write(client_sock, value, strlen(value));
    write(client_sock, "\n", 1);
}
*/

void handle_get(int client_sock, char *key) {
    char filename[BUFFER_SIZE];
    
    //printf("client_sock, %d\n", client_sock);
    //printf("client_sock, %s\n", key);
    // Sanitizar el nombre de archivo: eliminar cualquier barra diagonal
    // de la clave que pueda hacerla parecer una ruta
    if (strchr(key, '/') != NULL) {
        write(client_sock, "ERROR\n", 6);
        return;
    }

    snprintf(filename, sizeof(filename), "./%s", key);

    // Intentar abrir el archivo
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        // Si no existe, responder NOTFOUND
        write(client_sock, "NOTFOUND\n", 9);
        return;
    }

    // Leer el contenido del archivo
    char caracter;
    char value[BUFFER_SIZE];
    strcpy(value, "");
    while((caracter = fgetc(file)) != EOF)
    {
        size_t len = strlen(value);
        if(len < BUFFER_SIZE -1)
        {
           value[len] = caracter;
           value[len + 1] = '\0';
    	}     
    }

    //fread(value, sizeof(char), sizeof(value), file);
    fclose(file);

    // Responder OK seguido del valor
    write(client_sock, "OK\n", 3);
    write(client_sock, value, strlen(value));
    //write(client_sock, "\n", 1);
}


// Función para manejar el comando DEL
void handle_del(int client_sock, char *key) {
    char filename[BUFFER_SIZE];
    snprintf(filename, sizeof(filename), "./%s", key);

    // Intentar eliminar el archivo
    if (remove(filename) == 0) {
        // Responder OK
        write(client_sock, "OK\n", 3);
    } else {
        // Error al eliminar el archivo
        perror("Error al eliminar archivo");
        write(client_sock, "ERROR\n", 6);
    }
}

// Función para procesar el comando recibido
void process_command(int client_sock, char *command) {
    char *token;
    token = strtok(command, " \n");

    if (token == NULL) {
        write(client_sock, "ERROR\n", 6);
        return;
    }

    if (strcmp(token, "SET") == 0) {
        // Comando SET
        char *key = strtok(NULL, " \n");
        char *value = strtok(NULL, "\n");
        if (key != NULL && value != NULL) {
            handle_set(client_sock, key, value);
        } else {
            write(client_sock, "ERROR\n", 6);
        }
    } else if (strcmp(token, "GET") == 0) {
        // Comando GET
        char *key = strtok(NULL, " \n");
        if (key != NULL) {
            handle_get(client_sock, key);
        } else {
            write(client_sock, "ERROR\n", 6);
        }
    } else if (strcmp(token, "DEL") == 0) {
        // Comando DEL
        char *key = strtok(NULL, " \n");
        if (key != NULL) {
            handle_del(client_sock, key);
        } else {
            write(client_sock, "ERROR\n", 6);
        }
    } else {
        // Comando desconocido
        write(client_sock, "ERROR\n", 6);
    }
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    ssize_t n;

    // Crear socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Error al crear socket");
        exit(1);
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Enlazar el socket
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al enlazar");
        close(server_sock);
        exit(1);
    }

    // Escuchar en el puerto 5000
    if (listen(server_sock, 5) < 0) {
        perror("Error al escuchar");
        close(server_sock);
        exit(1);
    }

    printf("Servidor esperando conexiones en el puerto %d...\n", PORT);

    while (1) {
        // Aceptar una conexión entrante
        client_len = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Error al aceptar conexión");
            continue;
        }

        // Recibir el comando
        memset(buffer, 0, sizeof(buffer));
        n = read(client_sock, buffer, sizeof(buffer) - 1);
        if (n < 0) {
            perror("Error al leer del socket");
            close(client_sock);
            continue;
        }

        // Procesar el comando
        buffer[n] = '\0';  // Asegurarse de que el buffer es una cadena válida
        process_command(client_sock, buffer);

        // Cerrar la conexión con el cliente
        close(client_sock);
    }

    // Cerrar el socket del servidor
    close(server_sock);

    return 0;
}
