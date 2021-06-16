#include "srvheader.h"

#define VALID_PORT(x) ((x > 0 && x < 65535) ? 1 : 0) 

void handler(int signum){
    if(signum == SIGCHLD){
        int pid = waitpid(-1, NULL, WNOHANG);
        printf("Se cerro el hijo [%d]\n", pid);
    }
}

/**
 * Run with
 *         ./mysrv <SERVER_PORT>
 **/
int main (int argc, char *argv[]) {

    // arguments checking
    pid_t child_pid;

    if(argc == 2) {

        if(VALID_PORT(atoi(argv[1]))){
            // reserve sockets and variables space
            // slave: client    master: server
            int msd, ssd;
            struct sockaddr_in m_addr, s_addr;
            socklen_t s_addr_len;

            // create server socket and check errors
            msd = socket(AF_INET, SOCK_STREAM, 0);
            if (msd < 0) {
                errx(2, "Error en la creacion del socket.");
            }

            m_addr.sin_family = AF_INET;
            m_addr.sin_addr.s_addr = INADDR_ANY; 
            m_addr.sin_port = htons(atoi(argv[1]));

            // bind master socket and check errors
            if(bind(msd, (struct sockaddr *) &m_addr, sizeof(m_addr)) < 0){
                errx(3, "Error en bind.");
            }

            // se define el handler para la senial
            signal(SIGCHLD, handler);

            // make it listen
            if(listen(msd, 10) < 0){
                errx(4, "Error en listen");
            }

            // main loop
            while (true) {
                // accept connectiones sequentially and check errors
                s_addr_len = sizeof(s_addr);

                ssd = accept(msd, (struct sockaddr*) &s_addr, &s_addr_len);
                if(ssd < 0) errx(5, "Error en accept.");

                child_pid = fork();

                if (child_pid == 0){
                    // codigo del hijo
                    
                    // el hijo no necesita tener el socket del maestro
                    close(msd);

                    // send hello
                    send_ans(ssd, MSG_220);

                    // operate only if authenticate is true
                    if(authenticate(ssd)) {
                        operate(ssd);
                    } else {
                        warn("Conexion cerrada");
                        close(ssd);
                    }

                    exit(0);
                }

                // el padre cierra el socket del esclavo
                close(ssd);
            }
            close(msd);
            // close server socket
        }
        
    } else {
        errx(1, "Cantidad de argumentos incorrecta.\nUso: ./mysrv <SERVER_PORT>\n");
    }

    return 0;
}
