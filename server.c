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
#define LOG_FILE "servidor.log"

FILE *log_file;

// Función para registrar mensajes en el log
void log_message(const char *message) {
    if (log_file != NULL) {
        fprintf(log_file, "%s\n", message);
        fflush(log_file);  // Asegura que el mensaje se escriba inmediatamente
    }
}

// Función para manejar el comando SET
void handle_set(int client_sock, char *key, char *value) {
    char filename[BUFFER_SIZE];
    snprintf(filename, sizeof(filename), "./%s", key);

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error al crear archivo");
        log_message("ERROR: No se pudo crear el archivo para SET");
        if (write(client_sock, "ERROR\n", 6) < 0) {
            perror("Error al enviar respuesta de ERROR");
        }
        return;
    }

    fprintf(file, "%s\n", value);
    fclose(file);

    log_message("SET: Comando ejecutado correctamente");
    if (write(client_sock, "OK\n", 3) < 0) {
        perror("Error al enviar respuesta de OK");
        log_message("ERROR: No se pudo enviar respuesta de OK después de SET");
    }
}

// Función para manejar el comando GET
void handle_get(int client_sock, char *key) {
    char filename[BUFFER_SIZE];
    
    if (strchr(key, '/') != NULL) {
        log_message("ERROR: El nombre de la clave no puede contener '/' en GET");
        if (write(client_sock, "ERROR\n", 6) < 0) {
            perror("Error al enviar respuesta de ERROR");
        }
        return;
    }

    snprintf(filename, sizeof(filename), "./%s", key);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        log_message("GET: La clave no se encontró");
        if (write(client_sock, "NOTFOUND\n", 9) < 0) {
            perror("Error al enviar respuesta de NOTFOUND");
        }
        return;
    }

    char caracter;
    char value[BUFFER_SIZE];
    strcpy(value, "");
    while((caracter = fgetc(file)) != EOF) {
        size_t len = strlen(value);
        if(len < BUFFER_SIZE -1) {
           value[len] = caracter;
           value[len + 1] = '\0';
        }     
    }
    fclose(file);

    log_message("GET: Comando ejecutado correctamente");
    if (write(client_sock, "OK\n", 3) < 0) {
        perror("Error al enviar respuesta de OK");
        log_message("ERROR: No se pudo enviar respuesta de OK después de GET");
    }
    if (write(client_sock, value, strlen(value)) < 0) {
        perror("Error al enviar el valor");
        log_message("ERROR: No se pudo enviar el valor después de GET");
    }
}

// Función para manejar el comando DEL
void handle_del(int client_sock, char *key) {
    char filename[BUFFER_SIZE];
    snprintf(filename, sizeof(filename), "./%s", key);

    if (remove(filename) == 0) {
        log_message("DEL: Comando ejecutado correctamente");
        if (write(client_sock, "OK\n", 3) < 0) {
            perror("Error al enviar respuesta de OK");
            log_message("ERROR: No se pudo enviar respuesta de OK después de DEL");
        }
    } else {
        perror("Error al eliminar archivo");
        log_message("ERROR: No se pudo eliminar el archivo en DEL");
        if (write(client_sock, "ERROR\n", 6) < 0) {
            perror("Error al enviar respuesta de ERROR");
        }
    }
}

// Función para procesar el comando recibido
void process_command(int client_sock, char *command) {
    char *token;
    token = strtok(command, " \n");

    if (token == NULL) {
        log_message("ERROR: Comando vacío recibido");
        if (write(client_sock, "ERROR\n", 6) < 0) {
            perror("Error al enviar respuesta de ERROR");
        }
        return;
    }

    if (strcmp(token, "SET") == 0) {
        char *key = strtok(NULL, " \n");
        char *value = strtok(NULL, "\n");
        if (key != NULL && value != NULL) {
            handle_set(client_sock, key, value);
        } else {
            log_message("ERROR: Parámetros inválidos en SET");
            if (write(client_sock, "ERROR\n", 6) < 0) {
                perror("Error al enviar respuesta de ERROR");
            }
        }
    } else if (strcmp(token, "GET") == 0) {
        char *key = strtok(NULL, " \n");
        if (key != NULL) {
            handle_get(client_sock, key);
        } else {
            log_message("ERROR: Parámetros inválidos en GET");
            if (write(client_sock, "ERROR\n", 6) < 0) {
                perror("Error al enviar respuesta de ERROR");
            }
        }
    } else if (strcmp(token, "DEL") == 0) {
        char *key = strtok(NULL, " \n");
        if (key != NULL) {
            handle_del(client_sock, key);
        } else {
            log_message("ERROR: Parámetros inválidos en DEL");
            if (write(client_sock, "ERROR\n", 6) < 0) {
                perror("Error al enviar respuesta de ERROR");
            }
        }
    } else {
        log_message("ERROR: Comando desconocido");
        if (write(client_sock, "ERROR\n", 6) < 0) {
            perror("Error al enviar respuesta de ERROR");
        }
    }
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    ssize_t n;

    log_file = fopen(LOG_FILE, "a"); // Abrir el archivo de log en modo append
    if (log_file == NULL) {
        perror("No se pudo abrir el archivo de log");
        exit(1);
    }

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Error al crear socket");
        log_message("ERROR: No se pudo crear el socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al enlazar");
        log_message("ERROR: No se pudo enlazar el socket");
        close(server_sock);
        exit(1);
    }

    if (listen(server_sock, 5) < 0) {
        perror("Error al escuchar");
        log_message("ERROR: No se pudo escuchar en el socket");
        close(server_sock);
        exit(1);
    }
    printf("Servidor esperando conexiones en el puerto %d...\n", PORT);
    log_message("Servidor esperando conexiones en el puerto 5000...");

    while (1) {
        client_len = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Error al aceptar conexión");
            log_message("ERROR: No se pudo aceptar la conexión");
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        n = read(client_sock, buffer, sizeof(buffer) - 1);
        if (n < 0) {
            perror("Error al leer del socket");
            log_message("ERROR: No se pudo leer del socket");
            close(client_sock);
            continue;
        }

        buffer[n] = '\0';  // Asegurarse de que el buffer es una cadena válida
        log_message("Comando recibido: ");
        log_message(buffer);
        process_command(client_sock, buffer);

        close(client_sock);
    }

    close(server_sock);
    fclose(log_file);  // Cerrar el archivo de log

    return 0;
}
