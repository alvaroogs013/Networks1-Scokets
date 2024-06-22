/*
 *			C L I E N T C P
 *
 *	This is an example program that demonstrates the use of
 *	stream sockets as an IPC mechanism.  This contains the client,
 *	and is intended to operate in conjunction with the server
 *	program.  Together, these two programs
 *	demonstrate many of the features of sockets, as well as good
 *	conventions for using these features.
 *
 *
 */
 

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PUERTO 18498


extern int errno;

#define ADDRNOTFOUND	0xffffffff	/* value returned for unknown host */
#define RETRIES	5		/* number of times to retry before givin up */
#define BUFFERSIZE	1024	/* maximum size of packets to be received */
#define TIMEOUT 6
#define MAXHOST 512
/*
 *			H A N D L E R
 *
 *	This routine is the signal handler for the alarm signal.
 */
 
void handler()
{
 printf("Alarma recibida \n");
}




int main(argc, argv)
int argc;
char *argv[];
{

	if (argc != 4) {
		fprintf(stderr, "Usage:  %s <remote host>\n", argv[0]);
		exit(1);
	}
	
	
	if ( strcmp(argv[2], "TCP") == 0 ) {
	
	
	    int s;				/* connected socket descriptor */
	    struct addrinfo hints, *res;
	    long timevar;			/* contains time returned by time() */
	    struct sockaddr_in myaddr_in;	/* for local socket address */
	    struct sockaddr_in servaddr_in;	/* for server socket address */
		int addrlen, i, j, errcode;

		char buf[BUFFERSIZE];
        	
			/* Create the socket. */
		s = socket (AF_INET, SOCK_STREAM, 0);
		if (s == -1) {
			perror(argv[0]);
			fprintf(stderr, "%s: unable to create socket\n", argv[0]);
			exit(1);
		}
		
		/* clear out address structures */
		memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
		memset ((char *)&servaddr_in, 0, sizeof(struct sockaddr_in));

		/* Set up the peer address to which we will connect. */
		servaddr_in.sin_family = AF_INET;
		
		/* Get the host information for the hostname that the
		 * user passed in. */
	      memset (&hints, 0, sizeof (hints));
	      hints.ai_family = AF_INET;
	 	 /* esta funci�n es la recomendada para la compatibilidad con IPv6 gethostbyname queda obsoleta*/
	    errcode = getaddrinfo (argv[1], NULL, &hints, &res); 
	    if (errcode != 0){
				/* Name was not found.  Return a
				 * special value signifying the error. */
			fprintf(stderr, "%s: No es posible resolver la IP de %s\n",
					argv[0], argv[1]);
			exit(1);
		}
	    else {
			/* Copy address of host */
			servaddr_in.sin_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;
		    }
	    freeaddrinfo(res);

	    /* puerto del servidor en orden de red*/
		servaddr_in.sin_port = htons(PUERTO);

			/* Try to connect to the remote server at the address
			 * which was just built into peeraddr.
			 */
			 
			 
		if (connect(s, (const struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1) {
			perror(argv[0]);
			fprintf(stderr, "%s: unable to connect to remote\n", argv[0]);
			exit(1);
		}
			/* Since the connect call assigns a free address
			 * to the local end of this connection, let's use
			 * getsockname to see what it assigned.  Note that
			 * addrlen needs to be passed in as a pointer,
			 * because getsockname returns the actual length
			 * of the address.
			 */
		addrlen = sizeof(struct sockaddr_in);
		if (getsockname(s, (struct sockaddr *)&myaddr_in, &addrlen) == -1) {
			perror(argv[0]);
			fprintf(stderr, "%s: unable to read socket address\n", argv[0]);
			exit(1);
		 }

		/* Print out a startup message for the user. */
		time(&timevar);
		/* The port number must be converted first to host byte
		 * order before printing.  On most hosts, this is not
		 * necessary, but the ntohs() call is included here so
		 * that this program could easily be ported to a host
		 * that does require it.
		 */
		printf("Connected to %s on port %u at %s",
				argv[1], ntohs(myaddr_in.sin_port), (char *) ctime(&timevar));
				

		char ip[INET_ADDRSTRLEN]; // Obtencion de la direccion ip del cliente en binario
		inet_ntop(AF_INET, &(myaddr_in.sin_addr), ip, INET_ADDRSTRLEN); // Traduccion de la ip a formato decimal
		FILE *log_file = fopen("peticiones.log", "a");
		fprintf(log_file, "\nFecha y hora de la conexion: %s", (char *) ctime(&timevar));
		fprintf(log_file, "Connected to %s whith ip %s on port %u using transport protocol TCP\n\n", argv[1], ip, ntohs(myaddr_in.sin_port));

				
				
		memset(buf, 0, BUFFERSIZE); //limpiamos la cadena
		
			
				
	//////////////////////////////////////////////////////////////////////////////////////////// SE ESTABLECE LA CONEXI�N
		int totalReceived = 0;

	    // Recibir datos del servidor
		i = recv(s, buf, BUFFERSIZE, 0);

		    // A�adir la terminaci�n nula
		    buf[i] = '\0';  

		    // Verificar si la cadena es igual a "220"
		    if ( atoi(buf) == 220 ) {
				printf("%d Servicio preparado\n", atoi(buf));
				// Limpiar el b�fer y salir del bucle
				memset(buf, 0, BUFFERSIZE); //limpiamos la cadena

		    } else {
		        printf("Error de conexi�n \n");

		    }

		
	//////////////////////////////////////////////////////////////////////////////////////////// SE ESTABLECE LA CONEXI�N

		

		while (1) { // EMPIEZA EL PROGRAMA
		
		
	//////////////////////////////////////////////////////////////////////////////////////////// ENVIO DE MENSAJES AL SERVIDOR

			if(argv[3] != NULL){

				FILE *file = fopen(argv[3], "r");
				char input[BUFFERSIZE];
				int continuar = 1;

				while (fgets(input, sizeof(input), file) != NULL && continuar==1){

					fprintf(log_file, "%s %s:%u | Mensaje: %s", argv[1], ip, ntohs(myaddr_in.sin_port), input);
					input[strcspn(input, "\n")] = '\0';
					strcat(input, "\r\n");

					//Enviar Mensaje
					if (send(s, input, strlen(input), 0) != strlen(input)) {
			    		fprintf(stderr, "%s: Connection aborted on error\n", argv[0]);
			    		exit(1);
					}

					//Recibir respuesta del servidor
					i = recv(s, buf, BUFFERSIZE, 0);
		    		// A�adir la terminaci�n nula
		    		buf[i] = '\0';
		    

		    		int codErr = atoi(buf);

					if (codErr == 500) {
						printf("%d Error de sintaxis \n", codErr);
						fprintf(log_file,"%s %s:%u TCP | Orden %d Error de sintaxis\n", argv[1], ip, ntohs(myaddr_in.sin_port), codErr);
		    
					} else if (strncmp(buf, "250", 3) == 0) {
					

						printf("%s", buf);
						fprintf(log_file,"%s %s:%u TCP | Pregunta %s", argv[1], ip, ntohs(myaddr_in.sin_port), buf); 
						
					} else if (codErr == 221) {
					
						if (shutdown(s, 1) == -1) {
						perror(argv[0]);
						fprintf(stderr, "%s: unable to shutdown socket\n", argv[0]);
						exit(1);
						}

						printf("%d Cerrando el servicio \n", codErr);
						fprintf(log_file, "%s %s:%u TCP | Orden %d Cerrando el servicio\n", argv[1], ip, ntohs(myaddr_in.sin_port), codErr);
						continuar = 0;
						break;

					}else if (strncmp(buf, "354", 3) == 0) {
					

						printf("%s", buf);
						fprintf(log_file, "%s %s:%u TCP | %s", argv[1], ip, ntohs(myaddr_in.sin_port), buf);
						
					} else if ( codErr == 350) {
					

						printf("%d ACIERTO \n", codErr);
						fprintf(log_file, "%s %s:%u TCP | Orden %d ACIERTO\n", argv[1], ip, ntohs(myaddr_in.sin_port), codErr);
						
					} else if ( codErr == 375) {
					

						printf("%d FALLO \n", codErr);
						fprintf(log_file, "%s %s:%u TCP | Orden %d FALLO\n", argv[1], ip, ntohs(myaddr_in.sin_port), codErr);
						
					} else {
						printf("ERROR EN LA CADENA %s \n", buf);
						fprintf(log_file, "%s %s:%u TCP | ERROR EN LA CADENA %s\n", argv[1], ip, ntohs(myaddr_in.sin_port), buf);

					}

		memset(buf, 0, BUFFERSIZE); //limpiamos la cadena
		}

		time(&timevar);
		printf("All done at %s", (char *)ctime(&timevar));
		fprintf(log_file, "\nSaliendo... %s %s:%u TCP | All done at %s", argv[1], ip, ntohs(myaddr_in.sin_port), (char *)ctime(&timevar));
		fclose(log_file);
		fclose(file);
		return 0;
	}

	}
			   	
	} else if ( strcmp(argv[2], "UDP") == 0 ){
		
		int i, errcode;
		    int retry = RETRIES;    /* holds the retry count */
		    int s;                  /* socket descriptor */
		    long timevar;           /* contains time returned by time() */
		    struct sockaddr_in myaddr_in;   /* for local socket address */
		    struct sockaddr_in servaddr_in; /* for server socket address */
		    struct in_addr reqaddr; /* for returned internet address */
		    int addrlen, n_retry;
		    struct sigaction vec;
		    char hostname[MAXHOST];
		    struct addrinfo hints, *res;

		    /* Create the socket. */
		    s = socket(AF_INET, SOCK_DGRAM, 0);
		    if (s == -1) {
			perror(argv[0]);
			fprintf(stderr, "%s: unable to create socket\n", argv[0]);
			exit(1);
		    }

		    /* clear out address structures */
		    memset((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
		    memset((char *)&servaddr_in, 0, sizeof(struct sockaddr_in));

		    /* Bind socket to some local address so that the
		     * server can send the reply back.  A port number
		     * of zero will be used so that the system will
		     * assign any available port number.  An address
		     * of INADDR_ANY will be used so we do not have to
		     * look up the internet address of the local host.
		     */
		    myaddr_in.sin_family = AF_INET;
		    myaddr_in.sin_port = 0;
		    myaddr_in.sin_addr.s_addr = INADDR_ANY;
		    if (bind(s, (const struct sockaddr *)&myaddr_in, sizeof(struct sockaddr_in)) == -1) {
			perror(argv[0]);
			fprintf(stderr, "%s: unable to bind socket\n", argv[0]);
			exit(1);
		    }
		    addrlen = sizeof(struct sockaddr_in);
		    if (getsockname(s, (struct sockaddr *)&myaddr_in, &addrlen) == -1) {
			perror(argv[0]);
			fprintf(stderr, "%s: unable to read socket address\n", argv[0]);
			exit(1);
		    }

		    /* Print out a startup message for the user. */
		    time(&timevar);
		    /* The port number must be converted first to host byte
		     * order before printing.  On most hosts, this is not
		     * necessary, but the ntohs() call is included here so
		     * that this program could easily be ported to a host
		     * that does require it.
		     */
		    printf("Connected to %s on port %u at %s", argv[1], ntohs(myaddr_in.sin_port), (char *)ctime(&timevar));


		    /* Set up the server address. */
		    servaddr_in.sin_family = AF_INET;
		    /* Get the host information for the server's hostname that the
		     * user passed in.
		     */
		    memset(&hints, 0, sizeof(hints));
		    hints.ai_family = AF_INET;
		    /* esta funci�n es la recomendada para la compatibilidad con IPv6 gethostbyname queda obsoleta*/
		    errcode = getaddrinfo(argv[1], NULL, &hints, &res);
		    if (errcode != 0) {
			/* Name was not found.  Return a
			 * special value signifying the error. */
			fprintf(stderr, "%s: No es posible resolver la IP de %s\n",
				argv[0], argv[1]);
			exit(1);
		    } else {
			/* Copy address of host */
			servaddr_in.sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
		    }
		    freeaddrinfo(res);
		    /* puerto del servidor en orden de red*/
		    servaddr_in.sin_port = htons(PUERTO);

		    /* Registrar SIGALRM para no quedar bloqueados en los recvfrom */
		    vec.sa_handler = (void *)handler;
		    vec.sa_flags = 0;
		    if (sigaction(SIGALRM, &vec, (struct sigaction *)0) == -1) {
			perror(" sigaction(SIGALRM)");
			fprintf(stderr, "%s: unable to register the SIGALRM signal\n", argv[0]);
			exit(1);
		    }
		    
		    /////////////////////////////////////////////////////////////////////////////// inicio
		    
			char message[] = "INICIO\r\n" ;


			int intentos = RETRIES;
			
			char chain[BUFFERSIZE];
			memset(chain, 0, BUFFERSIZE); //limpiamos la cadena
			

			while (intentos > 0) {
			    if (sendto(s, message, strlen(message), 0, (struct sockaddr *)&servaddr_in,
				       sizeof(struct sockaddr_in)) == -1) {
				perror(argv[0]);
				fprintf(stderr, "%s: unable to send request\n", argv[0]);
				exit(1);
			    }

			    alarm(TIMEOUT);
			    
			    

			    if (recvfrom(s, chain, BUFFERSIZE, 0, (struct sockaddr *)&servaddr_in, &addrlen) == -1) {
				if (errno == EINTR) {
				    printf("attempt %d (retries %d).\n", n_retry, RETRIES);
				    n_retry--;
				} else {
				    printf("Unable to get response from");
				    exit(1);
				}
			    } else {
				alarm(0);
				//printf("Response from server: %s\n", message);
				break;
			    }
			}
			
			chain[strlen(chain)] = '\0';    
			int codErr = atoi(chain);
			
			
			if (codErr == 220) {
			
				printf("%d Servicio preparado\n", codErr);
			    
			}

			char ip[INET_ADDRSTRLEN]; // Obtencion de la direccion ip del cliente en binario
			inet_ntop(AF_INET, &(myaddr_in.sin_addr), ip, INET_ADDRSTRLEN); // Traduccion de la ip a formato decimal
			FILE *log_file = fopen("peticiones.log", "a");
			fprintf(log_file, "\nFecha y hora de la conexion: %s", (char *) ctime(&timevar));
			fprintf(log_file, "Connected to %s whith ip %s on port %u using transport protocol UDP\n\n", argv[1], ip, ntohs(myaddr_in.sin_port));
			
		    ///////////////////////////////////////////////////////////////////////////////////
		    
		    
		    

		    while (1) {
		    
		    if(argv[3] != NULL){
				FILE *file = fopen(argv[3], "r");
				char message[BUFFERSIZE];
				int continuar = 1;

				while(fgets(message, sizeof(message), file) != NULL && continuar==1){
					fprintf(log_file, "%s %s:%u | Mensaje: %s", argv[1], ip, ntohs(myaddr_in.sin_port), message);
					message[strcspn(message, "\n")] = '\0';
					strcat(message, "\r\n");

					int n_retry = RETRIES;
			
					char chain[BUFFERSIZE];
					memset(chain, 0, BUFFERSIZE); //limpiamos la cadena
			
					//Enviar mensaje
					while (n_retry > 0) {
						if (sendto(s, message, strlen(message), 0, (struct sockaddr *)&servaddr_in,
							sizeof(struct sockaddr_in)) == -1) {
						perror(argv[0]);
						fprintf(stderr, "%s: unable to send request\n", argv[0]);
						exit(1);
						}

						alarm(TIMEOUT);
						
						if (recvfrom(s, chain, BUFFERSIZE, 0, (struct sockaddr *)&servaddr_in, &addrlen) == -1) {
						if (errno == EINTR) {
							printf("attempt %d (retries %d).\n", n_retry, RETRIES);
							n_retry--;
						} else {
							printf("Unable to get response from");
							exit(1);
						}
						} else {
						alarm(0);
						//printf("Response from server: %s\n", message);
						break;
						}
					}
			
					chain[strlen(chain)] = '\0';
			    
					int codErr = atoi(chain);
						
						
					if (codErr == 500) {
					
						printf("%d Error de sintaxis \n", codErr);
						fprintf(log_file,"%s %s:%u UDP | Orden %d Error de sintaxis\n", argv[1], ip, ntohs(myaddr_in.sin_port), codErr);

						
					} else if (strncmp(chain, "250", 3) == 0) {
					

						printf("%s", chain);
						fprintf(log_file,"%s %s:%u UDP | Pregunta %s", argv[1], ip, ntohs(myaddr_in.sin_port), chain); 

						
					} else if (codErr == 221) {

							printf("%d Cerrando el servicio \n", codErr);
							fprintf(log_file, "%s %s:%u UDP | Orden %d Cerrando el servicio\n", argv[1], ip, ntohs(myaddr_in.sin_port), codErr);
							continuar = 0;
							break;
						
					}else if (strncmp(chain, "354", 3) == 0) {

						printf("%s", chain);
						fprintf(log_file, "%s %s:%u UDP | %s", argv[1], ip, ntohs(myaddr_in.sin_port), chain);

						
					} else if ( codErr == 350) {
					

						printf("%d ACIERTO \n", codErr);
						fprintf(log_file, "%s %s:%u UDP | Orden %d ACIERTO\n", argv[1], ip, ntohs(myaddr_in.sin_port), codErr);

						
					} else if ( codErr == 375) {
					

						printf("%d FALLO \n", codErr);
						fprintf(log_file, "%s %s:%u UDP | Orden %d FALLO\n", argv[1], ip, ntohs(myaddr_in.sin_port), codErr);

						
					} else {
						printf("ERROR EN LA CADENA %s \n", chain);
						fprintf(log_file, "%s %s:%u UDP | ERROR EN LA CADENA %s\n", argv[1], ip, ntohs(myaddr_in.sin_port), chain);

					}
						
						
						memset(chain, 0, BUFFERSIZE); //limpiamos la cadena
					
					

					if (n_retry == 0) {
						printf("Unable to get response from");
						printf(" %s after %d attempts.\n", argv[1], RETRIES);
					}
				}

				/* Print message indicating completion of task. */
					time(&timevar);
					printf("All done at %s", (char *)ctime(&timevar));
					fprintf(log_file, "\nSaliendo... %s %s:%u UDP | All done at %s", argv[1], ip, ntohs(myaddr_in.sin_port), (char *)ctime(&timevar));
					fclose(log_file);
					close(s);
					return 0;
			}


	}
	
	} else {
		fprintf(stderr, "Usage: tcp or udp \n");
		exit(1);
	}
}





