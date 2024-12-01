# TP Practico de SOPG

Trabajo Practico para la clase de Sistemas Operativos de Proposito General del 5to. Bimestre del 2024 de la Especialización de Sistemas Embedidos de la Universidad de Buenos Aires.
- Presentado por: Ricardo Roberto Palma Ventura

Este es un servidor simple implementado en C que maneja tres comandos básicos para almacenar, recuperar y eliminar datos:

- `SET <key> <value>`: Almacena un valor asociado a una clave.
- `GET <key>`: Recupera el valor asociado a una clave.
- `DEL <key>`: Elimina la clave y su valor asociado.

## Descripción

Este servidor escucha en el puerto 5000 y acepta conexiones de clientes. Los comandos enviados por los clientes son procesados y almacenados como archivos en el sistema de archivos. Cada clave se representa como un archivo, y el valor asociado a esa clave se guarda dentro del archivo.

### Comandos soportados:
- **SET <key> <value>**: Almacena un valor asociado a una clave. Si el archivo ya existe, sobrescribe su contenido.
- **GET <key>**: Recupera el valor asociado a la clave. Si la clave no existe, devuelve `NOTFOUND`.
- **DEL <key>**: Elimina el archivo asociado a la clave.

## Requisitos

- Un compilador de C (por ejemplo, `gcc`).
- Sistema operativo compatible con sockets TCP/IP (Linux recomendado).

## Instalación

1. Clona el repositorio:

    ```bash
    git clone https://github.com/rrpalmav/TP_SOPG.git
    cd TP_SOPG
    ```

2. Compila el código fuente:

    ```bash
    gcc -o servidor servidor.c
    ```

3. Ejecuta el servidor:

    ```bash
    ./servidor
    ```

El servidor comenzará a escuchar en el puerto 5000.

## Uso

El servidor espera comandos desde un cliente que se conecta a través de un socket TCP. Los comandos se envían como cadenas de texto, y el servidor responde con un mensaje de estado. Puedes probar el servidor usando herramientas como `telnet` o implementando un cliente en cualquier lenguaje.

### Ejemplo de uso:

1. Conectar al servidor con `telnet`:

    ```bash
    telnet 127.0.0.1 5000
    ```

2. Usar los comandos:

    - **SET**:

        ```text
        SET nombre "Juan Pérez"
        ```

        Respuesta esperada:

        ```text
        OK
        ```

    - **GET**:

        ```text
        GET nombre
        ```

        Respuesta esperada:

        ```text
        OK
        Juan Pérez
        ```

    - **DEL**:

        ```text
        DEL nombre
        ```

        Respuesta esperada:

        ```text
        OK
        ```

## Estructura del Proyecto

```plaintext
.
├── servidor.c            # Código fuente del servidor
├── README.md             # Este archivo


