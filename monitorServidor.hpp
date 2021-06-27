# pragma once

# include <mutex>
# include <condition_variable>

using namespace std;
class MS_Clientes {
    private:
        int numClientes;    // Número de clientes del servidor
        mutex mtxC;         // Ejecución de procesos en exclusión mutua
    public:
        //------------------------- Constructor
        // Pre:  True
        // Post: El dato ha sido creado
        // Coms: Constructor con información para MS_Clientes
        MS_Clientes(int numClientes);

        //------------------------- Destructor
        // Pre:  True
        // Post: El dato ha sido destruído
        // Coms: Se invoca de manera automática al cerrarse el bloque donde ha sido declarado
        ~MS_Clientes();

        //------------------------- Cliente
        // Pre:  True
        // Post: numClientes++
        // Coms: Incrementa el número de clientes conectados.
        void altaCliente();

        // Pre:  numClientes > 0
        // Post: numClientes--
        // Coms: Decrementa el número de clientes conectados.
        void bajaCliente();
};

class MS_Recursos {
    private:
        int R0;     // Número de recursos del tipo R<i>
        int R1;
        int R2;
        int R3;
        mutex mtxR; // Ejecución de procesos en exclusión mutua
        condition_variable resources;   // Comprueba si se han liberado recursos
    public:
        //------------------------- Constructor
        // Pre:  True
        // Post: El dato ha sido creado
        // Coms: Constructor con información para MS_Recursos
        MS_Recursos(int n1, int n2, int n3, int n4);

        //------------------------- Destructor
        // Pre:  True
        // Post: El dato ha sido destruído
        // Coms: Se invoca de manera automática al cerrarse el bloque donde ha sido declarado
        ~MS_Recursos();

        //------------------------- Cliente
        // Pre:  
        // Post: Si posible: R[i] -= recursos[i]
        // Coms: Devuelve TRUE si y solo si puede reservar los recursos y los resta al número total de recursos.
        //       Devuelve FALSE en caso de no poder reservarlos.
        bool reservaRecursos(const int recursos[]);

        // Pre:  
        // Post: <await resources>
        // Coms: Espera a que un cliente haya liberado recursos.
        void esperaRecursos();

        // Pre:  
        // Post: R[i] += recursos[i]
        // Coms: Devuelve el número de recursos que previamente se llevo.
        void liberarRecursos(const int recursos[]);
};
