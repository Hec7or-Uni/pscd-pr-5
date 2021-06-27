# include <iostream>
# include <thread>
# include <stdlib.h>
# include <string.h>
# include "monitorServidor.hpp"
# include "Socket/Socket.hpp"

using namespace std;

const string MENSAJE_ALTA        = "SOLICITUD ALTA REGISTRO";
const string MENSAJE_BAJA        = "SOLICITUD BAJA REGISTRO";
const string REGISTRO_ACEPTADO   = "REGISTRO ACEPTADO";
const string REGISTRO_DECLINADO  = "REGISTRO RECHAZADO";
const string MENSAJE_RESERVAR    = "RESERVAR";
const string RECURSOS_CONCEDIDOS = "RECURSOS CONCEDIDOS";
const string RECURSOS_LIBERADOS  = "LIBERAR RECURSOS";
const string RECURSOS_OCUPADOS   = "RECURSOS OCUPADOS";
const string CLIENTE_ESPERA      = "OK, EN ESPERA";
const string CLIENTE_DECLINA     = "OFERTA DECLINADA";

const int N = 5;                    // Número de clientes que se permiten en el servidor
const int MAX_CONNECTIONS = 2 * N;  // Número de conexiones maximas.
const int LENGTH = 100;             // Longitud de los mensajes
const int MAX_RECURSOS = 4;         // Tamaño del vector que almacena los recursos
const int MAX_ATTEMPS = 10;         // Número de mensajes erroneos antes de ser expulsado por el servidor.
const int CANTIDAD_RECURSOS = 10;   // Cantidad de un <i> recurso

// Pre:  Servidor cerrado
// Post: Abre servidor
// Coms: ---
void launchServer(Socket& chan, MS_Clientes& MS_C, MS_Recursos& MS_R, thread cliente[], int client_fd[], int& socket_fd, int& error_code);

// Pre:  Servidor abierto
// Post: Cierra el server
// Coms: ---
void closeServer(Socket& chan, thread cliente[], int& socket_fd, int& error_code);

// Pre:  True
// Post: mensaje = mensaje enviado por el servidor
// Coms: Envia un mensaje
void enviarMSG(Socket& soc, const int client_fd, const string mensaje);

// Pre:  True
// Post: Buffer = mensaje recibido por el cliente
// Coms: Recibe un mensaje
void recibirMSG(Socket& soc, const int client_fd, string& buffer);


// Pre:  True
// Post: Gestion de la comunicación correcta de mensajes servidor cliente
// Coms: ---
void servCliente(Socket& soc, MS_Clientes& MS_C, MS_Recursos& MS_R, const int client_fd);

//-------------------------------------------------------------
int main(int numArg, char* args[]) {
    if (numArg == 2) {
        int client_fd[N];
        thread cliente[N];

        int SERVER_PORT = atoi(args[1]);    // Puerto donde escucha el proceso servidor
        Socket chan(SERVER_PORT);           // Creación del Socket que lleva la comunicación con el servidor.
        MS_Clientes MS_C(0);
        MS_Recursos MS_R(CANTIDAD_RECURSOS, CANTIDAD_RECURSOS, CANTIDAD_RECURSOS, CANTIDAD_RECURSOS);
        
        int socket_fd = chan.Bind();
        int error_code;

        launchServer(chan, MS_C, MS_R, cliente, client_fd, socket_fd, error_code);
        closeServer(chan, cliente, socket_fd, error_code);

        return error_code;
    } else {
        cerr << "ERROR -> Falta de parametros en la ejecución\n";
        cerr << "      -- Prueba a ejecutarlo asi: ./Servidor <puerto_servidor>\n";
        return 0;   
    }
}
//-------------------------------------------------------------
void launchServer(Socket& chan, MS_Clientes& MS_C, MS_Recursos& MS_R, thread cliente[], int client_fd[], int& socket_fd, int& error_code) {
    // Bind
    if (socket_fd == -1) {
        cerr << "Error en el bind: " + string(strerror(errno)) + "\n";
        exit(1);
    }

    // Listen
    error_code = chan.Listen(N);
    if (error_code == -1) {
        cerr << "Error en el listen: " + string(strerror(errno)) + "\n";
        chan.Close(socket_fd);  // Cerramos el socket
        exit(1);
    }

    for (int id = 0; id < N; id++) {
        // Accept
        client_fd[id] = chan.Accept();

        if(client_fd[id] == -1) {
            cerr << "Error en el accept: " + string(strerror(errno)) + "\n";
            chan.Close(socket_fd);  // Cerramos el socket
            exit(1);
        }

        cliente[id] = thread(&servCliente, ref(chan), ref(MS_C), ref(MS_R), client_fd[id]);
        cout << "Nuevo cliente " + to_string(id) + " aceptado" + "\n";
    }
}

void closeServer(Socket& chan, thread cliente[], int& socket_fd, int& error_code) {
    for (int id = 0; id < N; id++) {
        cliente[id].join();
    }

    error_code = chan.Close(socket_fd); // Cerramos el socket del servidor
    if (error_code == -1) {
        cerr << "Error cerrando el socket del servidor: " + string(strerror(errno)) + " aceptado" + "\n";
    }

    cout << "Bye bye" << endl;
}

void enviarMSG(Socket& soc, const int client_fd, const string mensaje) {
    int send_bytes;
    send_bytes = soc.Send(client_fd, mensaje);
    if(send_bytes == -1) {
        cerr << "Error al enviar datos: " + string(strerror(errno)) + "\n";    
        soc.Close(client_fd);   // Cerramos los sockets.
        exit(1);
    }
}

void recibirMSG(Socket& soc, const int client_fd, string& buffer) {
    int rcv_bytes;
    rcv_bytes = soc.Recv(client_fd, buffer, LENGTH);    // Recibimos el mensaje del cliente
    if (rcv_bytes == -1) {
        cerr << "Error al recibir datos: " + string(strerror(errno)) + "\n";
        soc.Close(client_fd);   // Cerramos los sockets.
    }
}

void servCliente(Socket& soc, MS_Clientes& MS_C, MS_Recursos& MS_R, const int client_fd) {
    int recursos[MAX_RECURSOS];
    int num_errores = 0;
    string buffer;
    char* token;
    char* copia;
    bool out = false;   // Inicialmente no salir del bucle

    do {
        recibirMSG(soc, client_fd, buffer);
        // Separamos el RESERVAR de los RECURSOS a reservar
        copia = strdup(buffer.c_str()); // Trabajaremos sobre una copia
        token = strtok(copia, " ");

        if (buffer == MENSAJE_ALTA) {           // Cliente se da de alta
            MS_C.altaCliente();
            enviarMSG(soc, client_fd, REGISTRO_ACEPTADO);
        } else if (token == MENSAJE_RESERVAR) { // Cliente intenta reservar recursos
            sscanf(buffer.c_str(),"RESERVAR %d,%d,%d,%d", &recursos[0], &recursos[1], &recursos[2], &recursos[3]); // Trocea el mensaje para obtener los recursos
            if (MS_R.reservaRecursos(recursos)) {       // Si hay recursos disponibles...
                enviarMSG(soc, client_fd, RECURSOS_CONCEDIDOS);
                // Espera a recibir recursos liberados
                recibirMSG(soc, client_fd, buffer);
                MS_R.liberarRecursos(recursos);
            } else {                                    // Si no hay recursos disponibles...
                enviarMSG(soc, client_fd, RECURSOS_OCUPADOS);
                recibirMSG(soc, client_fd, buffer);
                if (buffer == CLIENTE_ESPERA) {             // Cliente decide esperar a recursos...
                    while (!MS_R.reservaRecursos(recursos)) MS_R.esperaRecursos();
                    enviarMSG(soc, client_fd, RECURSOS_CONCEDIDOS);
                } else if (buffer == CLIENTE_DECLINA) {     // Cliente no espera los recursos
                } else if (buffer == MENSAJE_BAJA) {        // Cliente abandona el servidor
                    out = true;
                } else {    // ERROR -> Mensaje no esperado
                    num_errores++;
                    out = (num_errores == MAX_ATTEMPS);
                }
            }
        } else if (buffer == RECURSOS_LIBERADOS) {  // Cliente devuelve los recursos
            MS_R.liberarRecursos(recursos);
        } else if (buffer == MENSAJE_BAJA) {        // Cliente abandona el servidor
            out = true;
        } else {        // ERROR -> Mensaje no esperado
            num_errores++;
            out = (num_errores == MAX_ATTEMPS);
        }
    } while (!out);
    
    MS_C.bajaCliente(); //Actualiza un cliente menos.
    soc.Close(client_fd);
}
