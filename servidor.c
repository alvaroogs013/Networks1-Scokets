/*
 *          		S E R V I D O R
 *
 *	This is an example program that demonstrates the use of
 *	sockets TCP and UDP as an IPC mechanism.  
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>




#define PUERTO 18498
#define ADDRNOTFOUND	0xffffffff	/* return address for unfound host */
#define BUFFERSIZE	1024	/* maximum size of packets to be received */
#define MAXHOST 128

extern int errno;

/*
 *			M A I N
 *
 *	This routine starts the server.  It forks, leaving the child
 *	to do all the work, so it does not have to be run in the
 *	background.  It sets up the sockets.  It
 *	will loop forever, until killed by a signal.
 *
 */
 
void serverTCP(int s, struct sockaddr_in peeraddr_in);
void serverUDP(int s, char * buffer, struct sockaddr_in clientaddr_in);
void errout(char *);		/* declare error out routine */

void sendChain(int s, char * input);


	
struct sockaddr_in myaddr_in;   /* for local socket address */
struct sockaddr_in auxaddr_in;   
struct sockaddr_in clientaddr_in;       /* for peer socket address */
int addrlen;
	
	

int FIN = 0;             /* Para el cierre ordenado */
void finalizar(){ FIN = 1; }

int main(argc, argv)
int argc;
char *argv[];
{
int s_TCP, s_UDP;           /* connected socket descriptor */
    int ls_TCP;
    int auxSocket_UDP;                 /* listen socket descriptor */

    int cc;                                 /* contains the number of bytes read */

    struct sigaction sa = {.sa_handler = SIG_IGN}; /* used to ignore SIGCHLD */

    fd_set readmask;
    int numfds,s_mayor;

    char buffer[BUFFERSIZE];    /* buffer for packets to be read into */

    struct sigaction vec;

                /* Create the listen socket. */
        ls_TCP = socket (AF_INET, SOCK_STREAM, 0);
        if (ls_TCP == -1) {
                perror(argv[0]);
                fprintf(stderr, "(S)%s: unable to create socket TCP\n", argv[0]);
                exit(1);
        }
        /* clear out address structures */
        memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
        memset ((char *)&clientaddr_in, 0, sizeof(struct sockaddr_in));

    	addrlen = sizeof(struct sockaddr_in);

        myaddr_in.sin_family = AF_INET;
        myaddr_in.sin_addr.s_addr = INADDR_ANY;
        myaddr_in.sin_port = htons(PUERTO);

        /* Bind the listen address to the socket. */
        if (bind(ls_TCP, (const struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
                perror(argv[0]);
                fprintf(stderr, "(S)%s: unable to bind address TCP\n", argv[0]);
                exit(1);
        }
                /* Initiate the listen on the socket so remote users
                 * can connect.  The listen backlog is set to 5, which
                 * is the largest currently supported.
                 */
        if (listen(ls_TCP, 5) == -1) {
                perror(argv[0]);
                fprintf(stderr, "(S)%s: unable to listen on socket\n", argv[0]);
                exit(1);
        }


        /* Create the socket UDP. */
        s_UDP = socket (AF_INET, SOCK_DGRAM, 0);
        if (s_UDP == -1) {
                perror(argv[0]);
                printf("(S)%s: unable to create socket UDP\n", argv[0]);
                exit(1);
           }
        /* Bind the server's address to the socket. */
        if (bind(s_UDP, (struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
                perror(argv[0]);
                printf("(S)%s: unable to bind address UDP\n", argv[0]);
                exit(1);
            }

                /* Now, all the initialization of the server is
                 * complete, and any user errors will have already
                 * been detected.  Now we can fork the daemon and
                 * return to the user.  We need to do a setpgrp
                 * so that the daemon will no longer be associated
                 * with the user's control terminal.  This is done
                 * before the fork, so that the child will not be
                 * a process group leader.  Otherwise, if the child
                 * were to open a terminal, it would become associated
                 * with that terminal as its control terminal.  It is
                 * always best for the parent to do the setpgrp.
                 */
        setpgrp();

        switch (fork()) {
        case -1:                /* Unable to fork, for some reason. */
                perror(argv[0]);
                fprintf(stderr, "(S)%s: unable to fork daemon\n", argv[0]);
                exit(1);

        case 0:     /* The child process (daemon) comes here. */

                        /* Close stdin and stderr so that they will not
                         * be kept open.  Stdout is assumed to have been
                         * redirected to some logging file, or /dev/null.
                         * From now on, the daemon will not report any
                         * error messages.  This daemon will loop forever,
                         * waiting for connections and forking a child
                         * server to handle each one.
                         */
                fclose(stdin);
                fclose(stderr);

                        /* Set SIGCLD to SIG_IGN, in order to prevent
                         * the accumulation of zombies as each child
                         * terminates.  This means the daemon does not
                         * have to make wait calls to clean them up.
                         */
                if ( sigaction(SIGCHLD, &sa, NULL) == -1) {
            perror(" sigaction(SIGCHLD)");
            fprintf(stderr,"(S)%s: unable to register the SIGCHLD signal\n", argv[0]);
            exit(1);
            }

                    /* Registrar SIGTERM para la finalizacion ordenada del programa servidor */
        vec.sa_handler = (void *) finalizar;
        vec.sa_flags = 0;
        if ( sigaction(SIGTERM, &vec, (struct sigaction *) 0) == -1) {
            perror(" sigaction(SIGTERM)");
            fprintf(stderr,"(S)%s: unable to register the SIGTERM signal\n", argv[0]);
            exit(1);
            }

                while (!FIN) {
            /* Meter en el conjunto de sockets los sockets UDP y TCP */
            FD_ZERO(&readmask);
            FD_SET(ls_TCP, &readmask);
            FD_SET(s_UDP, &readmask);
            /*
            Seleccionar el descriptor del socket que ha cambiado. Deja una marca en
            el conjunto de sockets (readmask)
            */
            if (ls_TCP > s_UDP)
                        s_mayor=ls_TCP;
                else
                        s_mayor=s_UDP;

            if ( (numfds = select(s_mayor+1, &readmask, (fd_set *)0, (fd_set *)0, NULL)) < 0) {
                if (errno == EINTR) {
                    FIN=1;
                            close (ls_TCP);
                            close (s_UDP);
                    perror("\n(S)Finalizando el servidor. SeÃƒal recibida en elect\n ");
                }
            }
           else {

                /* Comprobamos si el socket seleccionado es el socket TCP */
                if (FD_ISSET(ls_TCP, &readmask)) {
                    /* Note that addrlen is passed as a pointer
                     * so that the accept call can return the
                     * size of the returned address.
                     */
                                /* This call will block until a new
                                 * connection arrives.  Then, it will
                                 * return the address of the connecting
                                 * peer, and a new socket descriptor, s,
                                 * for that connection.
                                 */
                        s_TCP = accept(ls_TCP, (struct sockaddr *) &clientaddr_in, &addrlen);
                        if (s_TCP == -1) exit(1);
                        switch (fork()) {
                                case -1:        /* Can't fork, just exit. */
                                        exit(1);
                                case 0:         /* Child process comes here. */
                                        close(ls_TCP); /* Close the listen socket inherited from the daemon. */
                                        serverTCP(s_TCP, clientaddr_in);
                                        exit(0);
                                default:        /* Daemon process comes here. */
                                                /* The daemon needs to remember
                                                 * to close the new accept socket
                                                 * after forking the child.  This
                                                 * prevents the daemon from running
                                                 * out of file descriptor space.  It
                                                 * also means that when the server
                                                 * closes the socket, that it will
                                                 * allow the socket to be destroyed
                                                 * since it will be the last close.
                                                 */
                                        close(s_TCP);
                                }
             } /* De TCP*/
          /* Comprobamos si el socket seleccionado es el socket UDP */
          if (FD_ISSET(s_UDP, &readmask)) {
                /* This call will block until a new
                * request arrives.  Then, it will
                * return the address of the client,
                * and a buffer containing its request.
                * BUFFERSIZE - 1 bytes are read so that
                * room is left at the end of the buffer
                * for a null character.
                */
                cc = recvfrom(s_UDP, buffer, BUFFERSIZE - 1, 0,
                   (struct sockaddr *)&clientaddr_in, &addrlen);
                if ( cc == -1) {
                    perror(argv[0]);
                    printf("(S)%s: recvfrom error\n", argv[0]);
                    exit (1);
                    }
                /* Make sure the message received is
                * null terminated.
                */
                //buffer[cc]='\0';
                
                memset(buffer, 0, BUFFERSIZE);
                	
                auxSocket_UDP=socket(AF_INET,SOCK_DGRAM,0);

                if(auxSocket_UDP == -1) exit(-1);
                 
		auxaddr_in.sin_family = AF_INET;
        	auxaddr_in.sin_addr.s_addr = INADDR_ANY;
        	auxaddr_in.sin_port = 0;
        	
        	
                if (bind(auxSocket_UDP, (struct sockaddr *) &auxaddr_in, sizeof(struct sockaddr_in)) == -1) {
                perror(argv[0]);
                printf("(S)%s: unable to bind address UDP\n", argv[0]);
                exit(1);
            }
            
            
		pid_t child_pid = fork();

		if (child_pid == -1) {
			close(s_UDP);
			exit(-1);
		} else if (child_pid == 0) {
			close(s_UDP);
			serverUDP(auxSocket_UDP, buffer, clientaddr_in);
			exit(0);
		} else {
			close(auxSocket_UDP);
		}
                
                
                }
          }
          
                }   /* Fin del bucle infinito de atenciÃ³n a clientes */
                
                
        /* Cerramos los sockets UDP y TCP */
        close(ls_TCP);
        close(s_UDP);

        printf("\n(S)Fin de programa servidor!\n");

        default:                /* Parent process comes here. */
                exit(0);
        }

}

/*
 *				S E R V E R T C P
 *
 *	This is the actual server routine that the daemon forks to
 *	handle each individual connection.  Its purpose is to receive
 *	the request packets from the remote client, process them,
 *	and return the results to the client.  It will also write some
 *	logging information to stdout.
 *
 */
 
 	struct preguntaServidor {
	    char pregunta[500];
	    int respuesta;
	};
 
 
void serverTCP(int s, struct sockaddr_in clientaddr_in)
{

	struct preguntaServidor preguntas[] = {
		
		{"¿Cuantos continentes hay en el mundo?", 7},
		{"¿Cuantos millones de habitantes tiene Espania?", 47},
		{"¿Cuantos lados tiene un triángulo?", 3},
		{"¿En que anio comenzo la Segunda Guerra Mundial?", 1939},
		{"¿Cuantos mundiales de F1 tiene Fernando Alonso?", 2},
		{"¿En que anio gano Espania el mundial de futbol?", 2010}

	};


	int reqcnt = 0;		/* keeps count of number of requests */
	char buf[BUFFERSIZE];		/* This example uses BUFFERSIZE byte messages. */
	char hostname[MAXHOST];		/* remote host's name string */

	int len, len1, status;
    struct hostent *hp;		/* pointer to host info for remote host */
    long timevar;			/* contains time returned by time() */
    
    struct linger linger;		/* allow a lingering, graceful close; */
    				            /* used when setting SO_LINGER */
    				
	/* Look up the host information for the remote host
	 * that we have connected with.  Its internet address
	 * was returned by the accept call, in the main
	 * daemon loop above.
	 */
	 
     status = getnameinfo((struct sockaddr *)&clientaddr_in,sizeof(clientaddr_in),
                           hostname,MAXHOST,NULL,0,0);
     if(status){
           	/* The information is unavailable for the remote
			 * host.  Just format its internet address to be
			 * printed out in the logging information.  The
			 * address will be shown in "internet dot format".
			 */
			 /* inet_ntop para interoperatividad con IPv6 */
            if (inet_ntop(AF_INET, &(clientaddr_in.sin_addr), hostname, MAXHOST) == NULL)
            	perror(" inet_ntop \n");
             }
    /* Log a startup message. */
    time (&timevar);
		/* The port number must be converted first to host byte
		 * order before printing.  On most hosts, this is not
		 * necessary, but the ntohs() call is included here so
		 * that this program could easily be ported to a host
		 * that does require it.
		 */
	printf("Startup from %s port %u at %s",
		hostname, ntohs(clientaddr_in.sin_port), (char *) ctime(&timevar));

		/* Set the socket for a lingering, graceful close.
		 * This will cause a final close of this socket to wait until all of the
		 * data sent on it has been received by the remote host.
		 */
	linger.l_onoff  =1;
	linger.l_linger =1;
	if (setsockopt(s, SOL_SOCKET, SO_LINGER, &linger,
					sizeof(linger)) == -1) {
		errout(hostname);
	}

		/* Go into a loop, receiving requests from the remote
		 * client.  After the client has sent the last request,
		 * it will do a shutdown for sending, which will cause
		 * an end-of-file condition to appear on this end of the
		 * connection.  After all of the client's requests have
		 * been received, the next recv call will return zero
		 * bytes, signalling an end-of-file condition.  This is
		 * how the server will know that no more requests will
		 * follow, and the loop will be exited.
		 */
	
	char chain[BUFFERSIZE];
	memset(chain, 0, BUFFERSIZE); //limpiamos la cadena
	
	char buffTemp[BUFFERSIZE];
	memset(buffTemp, 0, sizeof(buffTemp));
	
	int nivel = 1;
	int exitVar = 0;
	int intentos = 5;
	int preguntasCont = (sizeof(preguntas) / sizeof(preguntas[0]))-1;
	int cont = preguntasCont;
	
	
	char cad[] = "220\r\n";
	
	if (send(s, cad, strlen(cad), 0) != strlen(cad)) {
		    
		    exit(1);
		}
	
			//IMPLEMENTAR NIVELES: 
			//nivel 0 sin conexión
			//nivel 1 que solo se pueda recibir HOLA y ADIOS, el resto 500
			//nivel 2 que solo se pueda recibir un numero y ADIOS, el resto 500
			//nivel 3 si se acaban los reintentos que solo se pueda recibir +(volver al nivel 2) y ADIOS, el resto 500
			
	while(1){
	
	memset(buf, 0, sizeof(buf));
	memset(buffTemp, 0, sizeof(buffTemp));
	
			
			while (len = recv(s, buf, BUFFERSIZE, 0)) {
				if (len == -1) errout(hostname); 

				reqcnt++;
				


				for (int i = 0; i < strlen(buf); i++) {
					if (buf[i] == '\r' && buf[i+1] == '\n') {
						buffTemp[i-1] = '\0';
						//printf("El buff es %s", buffTemp);
						break;
					}
					buffTemp[i] = buf[i];
				}
				
					 			
				strcat(chain, buffTemp);
				
				switch(nivel) {
				
					case 1: 
					
						//printf("Estamos en el nivel 1");
						
						if (strcmp(chain, "ADIOS") == 0) {
						
					
							char input[] = "221\r\n";

							
						    	if (send(s, input, strlen(input), 0) != strlen(input)) errout(hostname);
						    	memset(chain, 0, 500); //limpiamos la cadena
						    	
						    	exitVar = 1;
					    	
					    	
						} else if(strcmp(chain, "HOLA") == 0) {
						

							char input[BUFFERSIZE];
							
							strcpy(input, "250 ");
							
							input[strcspn(input, "\n")] = '\0';
						    	strcat(input, preguntas[cont].pregunta);
							
							input[strcspn(input, "\n")] = '\0';
						    	strcat(input, "\r\n");

							if (send(s, input, strlen(input), 0) != strlen(input)) errout(hostname);
							memset(chain, 0, BUFFERSIZE); //limpiamos la cadena
							
							nivel = 2;
							
						    	
						} else { //si el comando no es válido que se quede en el while
						
							char input[] = "500\r\n";
							
						    	if (send(s, input, strlen(input), 0) != strlen(input)) errout(hostname);
						    	memset(chain, 0, BUFFERSIZE); //limpiamos la cadena
						}

						
					
					break; //salimos del switch, si no se ha cambiado de nivel se volverá automáticamente a este case
					
					case 2: 
					
						//printf("Estamos en el nivel 2");
						
						if (strcmp(chain, "ADIOS") == 0) {
					
							char input[] = "221\r\n";

						    	if (send(s, input, strlen(input), 0) != BUFFERSIZE) errout(hostname);
						    	memset(chain, 0, BUFFERSIZE); //limpiamos la cadena
						    	
						    	exitVar = 1;
					    	
					    	
						} else if (strncmp(chain, "RESPUESTA", 9) == 0) { 
						
						int numero;

						    // Intentar extraer el número de la cadena
							if (sscanf(chain, "RESPUESTA %d", &numero) == 1) {
							
								// Imprimir el número si se pudo extraer correctamente
								//printf("Numero: %d y la respuesta es %d \n", numero, preguntas[0].respuesta);
								
								if(numero > preguntas[cont].respuesta ){
								
									
									if(intentos > 1){
									
										intentos--;
									
										char nIntentos[20];  // Asegúrate de tener suficiente espacio
										sprintf(nIntentos, "%d", intentos);
									
										char input[30];
							
										strcpy(input, "354 MAYOR#");
										
										input[strcspn(input, "\n")] = '\0';
									    	strcat(input, nIntentos);
										
										input[strcspn(input, "\n")] = '\0';
									    	strcat(input, "\r\n");

										if (send(s, input, strlen(input), 0) != strlen(input)) errout(hostname);

								    	
								    	} else {
								    	
								    		intentos--;
								    	
								    		char input[] = "375\r\n";

										if (send(s, input, strlen(input), 0) != strlen(input)) errout(hostname);
								    	
								    		intentos = 5;
								    		nivel = 3;
								    	
								    	}
								    	
								    	
								    	
								    	memset(chain, 0, BUFFERSIZE); //limpiamos la cadena
									
								} else if (numero < preguntas[cont].respuesta){
								
									
									if(intentos > 1){
									
								    		intentos--;
									
										char nIntentos[20];  // Asegúrate de tener suficiente espacio
										sprintf(nIntentos, "%d", intentos);
									
										char input[500];
							
										strcpy(input, "354 MENOR#");
										
										input[strcspn(input, "\n")] = '\0';
									    	strcat(input, nIntentos);
										
										input[strcspn(input, "\n")] = '\0';
									    	strcat(input, "\r\n");

										if (send(s, input, strlen(input), 0) != strlen(input)) errout(hostname);
									    	
								    	} else {
								    	
								    		intentos--;
								    	
								    		char input[] = "375\r\n";

										if (send(s, input, strlen(input), 0) != strlen(input)) errout(hostname);
								    	
								    		intentos = 5;
								    		nivel = 3;
								    	
								    	}
								    	
								    	
								    	memset(chain, 0, 500); //limpiamos la cadena
									
								} else {
								
									if( cont > 0 ){
										cont--;
									} else {
										cont = preguntasCont;
									}
									
								
									intentos = 5;
								
									char input[] = "350\r\n";
									
								    	if (send(s, input, strlen(input), 0) != strlen(input)) errout(hostname);
								    	memset(chain, 0, BUFFERSIZE); //limpiamos la cadena
									
									nivel = 3;
									
								}
								
								
							} else {
								// Si no se pudo extraer el número
								char input[] = "500\r\n";
								
							    	if (send(s, input, strlen(input), 0) != strlen(input)) errout(hostname);
							    	memset(chain, 0, BUFFERSIZE); //limpiamos la cadena
							    	
								}
						
							
						    	
						} else { //si el comando no es válido que se quede en el nivel
						
							char input[] = "500\r\n";
							
						    	if (send(s, input, strlen(input), 0) != strlen(input)) errout(hostname);
						    	memset(chain, 0, BUFFERSIZE); //limpiamos la cadena
						}

						
					break; //salimos del switch, si no se ha cambiado de nivel se volverá automáticamente a este case
					
					case 3: 
						
						//printf("Estamos en el nivel 3");
					
						if (strcmp(chain, "ADIOS") == 0) {
					
							char input[] = "221\r\n";
							
						    	if (send(s, input, strlen(input), 0) != strlen(input)) errout(hostname);
						    	memset(chain, 0, BUFFERSIZE); //limpiamos la cadena despues de mandarla
						    	
						    	exitVar = 1;
					    	
					    	
						} else if(strcmp(chain, "+") == 0) {
						
							char input[BUFFERSIZE];
							
							strcpy(input, "250 ");
							
							input[strcspn(input, "\n")] = '\0';
						    	strcat(input, preguntas[cont].pregunta);
							
							input[strcspn(input, "\n")] = '\0';
						    	strcat(input, "\r\n");

							if (send(s, input, strlen(input), 0) != strlen(input)) errout(hostname);
							memset(chain, 0, BUFFERSIZE); //limpiamos la cadena
							
							nivel = 2;
							
						    	
						} else { //si el comando no es válido que se quede en el nivel
						
							char input[] = "500\r\n";
							
						    	if (send(s, input, strlen(input), 0) != strlen(input)) errout(hostname);
						    	memset(chain, 0, BUFFERSIZE); //limpiamos la cadena
						}

						
						
					break; //salimos del switch, si no se ha cambiado de nivel se volverá automáticamente a este case

					default: printf("Error \n");
				}

				
				break; // una vez que ha encontrado el \r\n que deje de leer el buffer
				
				//} // aqui tiene que acabar toda la lógica del programa
				
			} //WHILE lectura buffer 
			
			if(exitVar == 1){ //Si se ha seleccionado la opción, salimos del loop principal
				break; 
			}
			
		} //FIN LOOP
		
		
		//**************************************************XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX**********************************************
			
			
			
	
	
	
		
///////////////////**********************************************FIN PROGRAMA****************************************/////////////////// 

		/* The loop has terminated, because there are no
		 * more requests to be serviced.  As mentioned above,
		 * this close will block until all of the sent replies
		 * have been received by the remote host.  The reason
		 * for lingering on the close is so that the server will
		 * have a better idea of when the remote has picked up
		 * all of the data.  This will allow the start and finish
		 * times printed in the log file to reflect more accurately
		 * the length of time this connection was used.
		 */
	close(s);

		/* Log a finishing message. */
	time (&timevar);
		/* The port number must be converted first to host byte
		 * order before printing.  On most hosts, this is not
		 * necessary, but the ntohs() call is included here so
		 * that this program could easily be ported to a host
		 * that does require it.
		 */
	printf("Completed %s port %u, %d requests, at %s\n",
		hostname, ntohs(clientaddr_in.sin_port), reqcnt, (char *) ctime(&timevar));
}

/*
 *	This routine aborts the child process attending the client.
 */
void errout(char *hostname)
{
	printf("Connection with %s aborted on error\n", hostname);
	exit(1);     
}



/*
 *				S E R V E R U D P
 *
 *	This is the actual server routine that the daemon forks to
 *	handle each individual connection.  Its purpose is to receive
 *	the request packets from the remote client, process them,
 *	and return the results to the client.  It will also write some
 *	logging information to stdout.
 *
 */

 
 
void serverUDP(int s, char * buffer, struct sockaddr_in clientaddr_in)
{


	 int nivel = 1;
	 int intentos = 5;
	 int preguntasCont = 5;
	 int cont = 5;
	 
	 
	 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////



        struct addrinfo hints;

	int addrlen;
	
	char hostname[MAXHOST];
        
                           
        if(getnameinfo((struct sockaddr *)&clientaddr_in,sizeof(clientaddr_in),
                           hostname,MAXHOST,NULL,0,0) ) {
                           
		if (inet_ntop(AF_INET, &(clientaddr_in.sin_addr), hostname, MAXHOST) == NULL)
			perror(" inet_ntop \n");
		
	}

        addrlen = sizeof(struct sockaddr_in);

        memset (&hints, 0, sizeof (hints));
        hints.ai_family = AF_INET;

        char inicio[] = "220\r\n";
        
        
	sendto(s, inicio, strlen(inicio), 0,
		(struct sockaddr *)&clientaddr_in, sizeof(struct sockaddr_in));

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
 
	while(1){
	

 	char buf[BUFFERSIZE];
 	memset(buf, 0, BUFFERSIZE); //limpiamos la cadena
 	
 	char chain[BUFFERSIZE];
 	memset(chain, 0, BUFFERSIZE); //limpiamos la cadena
 	
 	int i = 0;

	i = recvfrom(s, buf, BUFFERSIZE - 1, 0,
                   (struct sockaddr *)&clientaddr_in, &addrlen);
                   
        buf[i] = '\0' ;

	
	struct preguntaServidor preguntas[] = {
		
		{"¿Cuantos continentes hay en el mundo?", 7},
		{"¿Cuantos millones de habitantes tiene Espania?", 47},
		{"¿Cuantos lados tiene un triángulo?", 3},
		{"¿En que anio comenzo la Segunda Guerra Mundial?", 1939},
		{"¿Cuantos mundiales de F1 tiene Fernando Alonso?", 2},
		{"¿En que anio gano Espania el mundial de futbol?", 2010}

	};

        
	
	for (int i = 0; i < strlen(buf); i++) {
	
                if (buf[i] == '\r' && buf[i+1] == '\n') {
                        chain[i-1] = '\0';
                        //printf("se encontraron \n");
                        break;
                } 
                
                chain[i] = buf[i];

        }
        
        //printf("En el buffer hay: %s \n", chain);
        //printf("Estamos en el nivel: %d \n", nivel);
        
        
           switch(nivel) {
			case 1: 
						
				if (strcmp(chain, "ADIOS") == 0) {
					
				char input[] = "221\r\n";
							
				sendto(s, input, strlen(input), 0,
           				(struct sockaddr *)&clientaddr_in, sizeof(struct sockaddr_in));
           
						    	
				nivel = 1;
				intentos = 5;
				preguntasCont = 5;
				cont = 5;
					    	
					    	
						} else if(strcmp(chain, "HOLA") == 0) {
						

							char input[500];
							
							strcpy(input, "250 ");
							
							input[strcspn(input, "\n")] = '\0';
						    	strcat(input, preguntas[cont].pregunta);
							
							input[strcspn(input, "\n")] = '\0';
						    	strcat(input, "\r\n");

							sendto(s, input, strlen(input), 0,
           							(struct sockaddr *)&clientaddr_in, sizeof(struct sockaddr_in));
							
							nivel = 2;
							
						    	
						} else { //si el comando no es válido que se quede en el while
						
							char input[] = "500\r\n";
							
							sendto(s, input, strlen(input), 0,
           							(struct sockaddr *)&clientaddr_in, sizeof(struct sockaddr_in));
           							
						}

						
					
					break;
					
			case 2: 
					
						//printf("Estamos en el nivel 2");
						
						if (strcmp(chain, "ADIOS") == 0) {
					
							char input[] = "221\r\n";
							
							sendto(s, input, strlen(input), 0,
           							(struct sockaddr *)&clientaddr_in, sizeof(struct sockaddr_in));
						    	
						    	nivel = 1;
							intentos = 5;
							preguntasCont = 5;
							cont = 5;
					    	
					    	
						} else if (strncmp(chain, "RESPUESTA", 9) == 0) { 
						
						
						    int numero = 0;
						    int digitFound = 0;

						    for (int x = 9; x < strlen(chain); x++) {
							if (isdigit(chain[x])) {
							    // Si encuentra un dígito, lo acumula en el número
							    numero = numero * 10 + (chain[x] - '0');
							    digitFound = 1;
							} else if (digitFound) {
							    // Si había encontrado dígitos y se encuentra otro tipo de caracter, termina la búsqueda
							    break;
							}
						    }
						    
						    /*if (digitFound) {
							printf("Número encontrado: %d\n", numero);
						    } else {
							printf("No se encontraron números en la cadena.\n");
						    }*/
						    

						    // Intentar extraer el número de la cadena
							if (digitFound == 1) {
							
								// Imprimir el número si se pudo extraer correctamente
								//printf("Numero: %d y la respuesta es %d \n", numero, preguntas[0].respuesta);
								
								if(numero > preguntas[cont].respuesta ){
								
									
									if(intentos > 1){
									
										intentos--;
									
										char nIntentos[20];  // Asegúrate de tener suficiente espacio
										sprintf(nIntentos, "%d", intentos);
									
										char input[500];
							
										strcpy(input, "354 MAYOR#");
										
										input[strcspn(input, "\n")] = '\0';
									    	strcat(input, nIntentos);
										
										input[strcspn(input, "\n")] = '\0';
									    	strcat(input, "\r\n");

										sendto(s, input, strlen(input), 0,
				   							(struct sockaddr *)&clientaddr_in, sizeof(struct sockaddr_in));

								    	
								    	} else {
								    	
								    		intentos--;
								    	
								    		char input[] = "375\r\n";

										sendto(s, input, strlen(input), 0,
				   							(struct sockaddr *)&clientaddr_in, sizeof(struct sockaddr_in));
								    	
								    		intentos = 5;
								    		nivel = 3;
								    	
								    	}
								    	
									
								} else if (numero < preguntas[cont].respuesta){
								
									
									if(intentos > 1){
									
								    		intentos--;
									
										char nIntentos[20];  // Asegúrate de tener suficiente espacio
										sprintf(nIntentos, "%d", intentos);
									
										char input[500];
							
										strcpy(input, "354 MENOR#");
										
										input[strcspn(input, "\n")] = '\0';
									    	strcat(input, nIntentos);
										
										input[strcspn(input, "\n")] = '\0';
									    	strcat(input, "\r\n");

										sendto(s, input, strlen(input), 0,
           										(struct sockaddr *)&clientaddr_in, sizeof(struct sockaddr_in));
									    	
								    	} else {
								    	
								    		intentos--;
								    	
								    		char input[] = "375\r\n";

										sendto(s, input, strlen(input), 0,
				   							(struct sockaddr *)&clientaddr_in, sizeof(struct sockaddr_in));
								    	
								    		intentos = 5;
								    		nivel = 3;
								    	
								    	}
								    	
								    	
									
								} else {
								
									if( cont > 0 ){
										cont--;
									} else {
										cont = preguntasCont;
									}
									
								
									intentos = 5;
								
									char input[] = "350\r\n";
									//input[strcspn(input, "\n")] = '\0';
								    	//strcat(input, "\r\n");
									
								    	sendto(s, input, strlen(input), 0,
           									(struct sockaddr *)&clientaddr_in, sizeof(struct sockaddr_in));

									
									nivel = 3;
									
								}
								
								
							} else {
								// Si no se pudo extraer el número
								char input[] = "500\r\n";
								
								
								/*for( int x = 0;  x < strlen(chain); x++){
									printf("--> %c \n", chain[x]); 
								}*/
							    	
								
							    	sendto(s, input, strlen(input), 0,
           								(struct sockaddr *)&clientaddr_in, sizeof(struct sockaddr_in));
								}
						
							
						    	
						} else { //si el comando no es válido que se quede en el nivel
						
							char input[] = "500\r\n";
							

							/*for( int x = 0;  x < strlen(chain); x++){
								printf("-> %c \n", chain[x]); 
							}*/
							
						    	sendto(s, input, strlen(input), 0,
           							(struct sockaddr *)&clientaddr_in, sizeof(struct sockaddr_in));
						}
			
			
			break;
			
			
			case 3: 
						
						//printf("Estamos en el nivel 3");
					
						if (strcmp(chain, "ADIOS") == 0) {
					
							char input[] = "221\r\n";
							
						    	sendto(s, input, strlen(input), 0,
           							(struct sockaddr *)&clientaddr_in, sizeof(struct sockaddr_in));
           							
						    	nivel = 1;
							intentos = 5;
							preguntasCont = 5;
							cont = 5;
					    	
					    	
						} else if(strcmp(chain, "+") == 0) {
						
							char input[500];
							
							strcpy(input, "250 ");
							
							input[strcspn(input, "\n")] = '\0';
						    	strcat(input, preguntas[cont].pregunta);
							
							input[strcspn(input, "\n")] = '\0';
						    	strcat(input, "\r\n");

							sendto(s, input, strlen(input), 0,
           							(struct sockaddr *)&clientaddr_in, sizeof(struct sockaddr_in));
							
							nivel = 2;
							
						    	
						} else { //si el comando no es válido que se quede en el nivel
						
							char input[] = "500\r\n";
							//input[strcspn(input, "\n")] = '\0';
						    	//strcat(input, "\r\n");
							
						    	sendto(s, input, strlen(input), 0,
           							(struct sockaddr *)&clientaddr_in, sizeof(struct sockaddr_in));
						}

						
						
			break; //salimos del switch, si no se ha cambiado de nivel se volverá automáticamente a este case
			
			
			default: printf("ERROR");
				
		}
           	
           	}
              
 }
 
 
 
 

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
