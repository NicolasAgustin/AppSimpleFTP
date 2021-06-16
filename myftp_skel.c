#include "clientheader.h"

#define VALID_PORT(x) ((x > 0 && x < 65535) ? 1 : 0) 

/**
 * Run with
 *         ./myftp <SERVER_IP> <SERVER_PORT>
 **/
int main (int argc, char *argv[]) {
    int sd, connection_port;
    struct sockaddr_in addr;

    // arguments checking
    if(argc == 3){
        
        connection_port = atoi(argv[2]);

        if(VALID_PORT(connection_port)){

            // create socket and check for errors
            sd = socket(AF_INET, SOCK_STREAM, 0);
            if(sd < 0){ 
                errx(3, "Error en creacion del socket"); 
            }

            // set socket data    
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet_addr(argv[1]);
            addr.sin_port = htons(connection_port);

            // connect and check for errors
            if(connect(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
                errx(4, "Error en la coneccion del socket");
            }


            // if receive hello proceed with authenticate and operate if not warning
            // recv_msg(sd, 220, text)
            // el servidor manda 220 si el cliente se pudo conectar con exito
            // el cliente siempre consulta al server con comandos de texto
            if(recv_msg(sd, 220, NULL)){
                // Comienza el procesamiento del programa

                // enviar comando de autenticacion
                authenticate(sd);

                operate(sd);

            } else {
                warn("Fallo en obtener respuesta hello del server.");
            }

            // close socket

        } else {
            // errx
            errx(2,"Puerto invalido.");
        }
    } else {
        // errx
        errx(1,"Cantidad de argumentos incorrecta\nUso: ./myftp <SERVER_IP> <PORT>");
    }
    

    return 0;
}