# include "monitorServidor.hpp"

MS_Clientes::MS_Clientes(int numClientes) {
    this->numClientes = numClientes;
}

MS_Clientes::~MS_Clientes() {}

void MS_Clientes::altaCliente() {
    unique_lock<mutex> lck(mtxC);
    numClientes++;  // Actualizamos clientes::alta en el servidor
}

void MS_Clientes::bajaCliente() {
    unique_lock<mutex> lck(mtxC);
    numClientes--;  // Actualizamos clientes::Baja del servidor
}

MS_Recursos::MS_Recursos(int n1, int n2, int n3, int n4) {
    R0 = n1;
    R1 = n2;
    R2 = n3;
    R3 = n4;
}

MS_Recursos::~MS_Recursos() {}

bool MS_Recursos::reservaRecursos(const int recursos[]) {
    unique_lock<mutex> lck(mtxR);
    // Comprobamos si tenemos los recursos disponibles.
    if (R0 < recursos[0] || R1 < recursos[1] || R2 < recursos[2] || R3 < recursos[3]) {
        return false;  // Recursos ocupados
    } else {
        // Actualizamos recursos::Adquisición
        R0 -= recursos[0];  // Al recurso <ID_recurso> le quitamos los que quiere usar un usuario[i].
        R1 -= recursos[1];
        R2 -= recursos[2];
        R3 -= recursos[3];
        return true;  // Recursos disponibles
    }
}

void MS_Recursos::esperaRecursos() {
    unique_lock<mutex> lck(mtxR);
    resources.wait(lck);
}

void MS_Recursos::liberarRecursos(const int recursos[]) {
    unique_lock<mutex> lck(mtxR);
    // Actualizamos recursos::Devolución
    R0 += recursos[0];  // Al recurso <ID_recurso> le devolvemos los que uso un usuario[i].
    R1 += recursos[1];
    R2 += recursos[2];
    R3 += recursos[3];

    resources.notify_all();
}
