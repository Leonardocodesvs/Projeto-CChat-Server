
#define WIN32_LEAN_AND_MEAN

#include "server.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 5555
#define MAX_CLIENTS 16
#define USERNAME_SIZE 32
#define MESSAGE_SIZE 512

int i;

typedef struct {
    SOCKET socket;
    int active;
    char username[USERNAME_SIZE];
} Client;

static Client clients[MAX_CLIENTS];
static CRITICAL_SECTION clients_lock;

static int send_all(SOCKET socket, const char *data, int length)
{
    int sent = 0;

    while (sent < length) {
        int result = send(
            socket,
            data + sent,
            length - sent,
            0
        );

        if (result == SOCKET_ERROR || result == 0)
            return -1;

        sent += result;
    }

    return 0;
}

/*
 * Retorna:
 *  1: linha recebida
 *  0: cliente desconectado
 * -1: erro de socket
 * -2: linha longa demais
 */
static int read_line(
    SOCKET socket,
    char *buffer,
    int capacity
)
{
    int used = 0;
    int too_long = 0;

    for (;;) {
        char ch;
        int result = recv(socket, &ch, 1, 0);

        if (result == 0)
            return 0;

        if (result == SOCKET_ERROR)
            return -1;

        if (ch == '\n')
            break;

        if (ch == '\r')
            continue;

        if (used < capacity - 1)
            buffer[used++] = ch;
        else
            too_long = 1;
    }

    buffer[used] = '\0';

    return too_long ? -2 : 1;
}

static void broadcast(const char *message)
{
    int length = (int)strlen(message);

    EnterCriticalSection(&clients_lock);
	
	int i;
    for ( i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            /*
             * Implementação inicial: o envio ocorre sob lock.
             * Para produção, use filas por cliente e timeouts.
             */
            (void)send_all(
                clients[i].socket,
                message,
                length
            );
        }
    }

    LeaveCriticalSection(&clients_lock);
}

static void remove_client(Client *client)
{
    EnterCriticalSection(&clients_lock);

    if (client->active) {
        shutdown(client->socket, SD_BOTH);
        closesocket(client->socket);

        client->socket = INVALID_SOCKET;
        client->active = 0;
        client->username[0] = '\0';
    }

    LeaveCriticalSection(&clients_lock);
}

static DWORD WINAPI handle_client(LPVOID argument)
{
    Client *client = (Client *)argument;
    SOCKET socket = client->socket;

    char username[USERNAME_SIZE];
    char message[MESSAGE_SIZE];
    char output[MESSAGE_SIZE + USERNAME_SIZE + 64];

    const char *prompt = "Digite seu nome:\n";

    (void)send_all(
        socket,
        prompt,
        (int)strlen(prompt)
    );

    int result = read_line(
        socket,
        username,
        sizeof(username)
    );

    if (result != 1 || username[0] == '\0') {
        remove_client(client);
        return 0;
    }
	
	int i;
    for ( i = 0; username[i] != '\0'; i++) {
        unsigned char ch = (unsigned char)username[i];

        if (ch < 32 || ch == 127) {
            remove_client(client);
            return 0;
        }
    }

    EnterCriticalSection(&clients_lock);

    snprintf(
        client->username,
        sizeof(client->username),
        "%s",
        username
    );

    LeaveCriticalSection(&clients_lock);

    snprintf(
        output,
        sizeof(output),
        "[SERVIDOR] %s entrou no chat.\n",
        username
    );

    broadcast(output);

    const char *help =
        "Comandos: /users e /quit\n";

    (void)send_all(socket, help, (int)strlen(help));

    for (;;) {
        result = read_line(
            socket,
            message,
            sizeof(message)
        );

        if (result == 0 || result == -1)
            break;

        if (result == -2) {
            const char *warning =
                "[SERVIDOR] Mensagem muito longa.\n";

            (void)send_all(
                socket,
                warning,
                (int)strlen(warning)
            );

            continue;
        }

        if (strcmp(message, "/quit") == 0)
            break;

        if (strcmp(message, "/users") == 0) {
            char list[MESSAGE_SIZE];
            int used = snprintf(
                list,
                sizeof(list),
                "Usuarios online:\n"
            );

            EnterCriticalSection(&clients_lock);
			
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active &&
                    clients[i].username[0] != '\0') {

                    int written = snprintf(
                        list + used,
                        sizeof(list) - (size_t)used,
                        "- %s\n",
                        clients[i].username
                    );

                    if (written < 0 ||
                        written >= (int)sizeof(list) - used)
                        break;

                    used += written;
                }
            }

            LeaveCriticalSection(&clients_lock);

            (void)send_all(socket, list, used);
            continue;
        }

        if (message[0] == '\0')
            continue;

        snprintf(
            output,
            sizeof(output),
            "[%s] %s\n",
            username,
            message
        );

        broadcast(output);
    }

    snprintf(
        output,
        sizeof(output),
        "[SERVIDOR] %s saiu do chat.\n",
        username
    );

    remove_client(client);
    broadcast(output);

    return 0;
}

int run_server(void)
{
    WSADATA wsa;
    SOCKET server_socket;
    struct sockaddr_in address;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "Falha ao iniciar Winsock.\n");
        return EXIT_FAILURE;
    }

    InitializeCriticalSection(&clients_lock);

    for ( i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = INVALID_SOCKET;
        clients[i].active = 0;
        clients[i].username[0] = '\0';
    }

    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (server_socket == INVALID_SOCKET) {
        fprintf(stderr, "Falha ao criar socket.\n");
        DeleteCriticalSection(&clients_lock);
        WSACleanup();
        return EXIT_FAILURE;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(
        server_socket,
        (struct sockaddr *)&address,
        sizeof(address)
    ) == SOCKET_ERROR) {
        fprintf(stderr, "Falha no bind: %d\n", WSAGetLastError());
        closesocket(server_socket);
        DeleteCriticalSection(&clients_lock);
        WSACleanup();
        return EXIT_FAILURE;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "Falha no listen.\n");
        closesocket(server_socket);
        DeleteCriticalSection(&clients_lock);
        WSACleanup();
        return EXIT_FAILURE;
    }

    printf("CChat Server iniciado em 127.0.0.1:%d\n", PORT);
    printf("Pressione Ctrl+C para encerrar.\n");

    for (;;) {
        SOCKET client_socket = accept(
            server_socket,
            NULL,
            NULL
        );

        if (client_socket == INVALID_SOCKET) {
            fprintf(stderr, "Falha no accept.\n");
            continue;
        }

        EnterCriticalSection(&clients_lock);

        int slot = -1;

        for (i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i].active) {
                slot = i;
                break;
            }
        }

        if (slot < 0) {
            LeaveCriticalSection(&clients_lock);

            const char *full =
                "Servidor cheio. Tente novamente mais tarde.\n";

            (void)send_all(
                client_socket,
                full,
                (int)strlen(full)
            );

            closesocket(client_socket);
            continue;
        }

        clients[slot].socket = client_socket;
        clients[slot].active = 1;
        clients[slot].username[0] = '\0';

        HANDLE thread = CreateThread(
            NULL,
            0,
            handle_client,
            &clients[slot],
            0,
            NULL
        );

        if (thread == NULL) {
            clients[slot].active = 0;
            clients[slot].socket = INVALID_SOCKET;

            LeaveCriticalSection(&clients_lock);
            closesocket(client_socket);
            continue;
        }

        CloseHandle(thread);
        LeaveCriticalSection(&clients_lock);
    }

    /* O loop é contínuo; o encerramento gracioso será implementado depois. */
    closesocket(server_socket);
    DeleteCriticalSection(&clients_lock);
    WSACleanup();

    return EXIT_SUCCESS;
}
