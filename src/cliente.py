
import socket
import threading

HOST = "127.0.0.1"
PORT = 5555

def receber(sock):
    arquivo = sock.makefile("r", encoding="utf-8", errors="replace")
    try:
        for linha in arquivo:
            print(linha, end="", flush=True)
    except OSError:
        pass

cliente = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    cliente.connect((HOST, PORT))
    threading.Thread(
        target=receber,
        args=(cliente,),
        daemon=True
    ).start()

    while True:
        mensagem = input()
        cliente.sendall((mensagem + "\n").encode("utf-8"))

        if mensagem == "/quit":
            break
except (ConnectionError, OSError) as erro:
    print("Erro de conexão:", erro)
finally:
    cliente.close()
