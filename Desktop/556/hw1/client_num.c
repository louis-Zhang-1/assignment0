#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <math.h>
#include "util.h"
/* simple client, takes two parameters, the server domain name,
   and the server port number */
int Client_arg = 5;
unsigned short Server_port_max = 18200;
unsigned short Server_port_min = 18000;
int Msg_size_min = 10;
int Msg_size_max = 65535;
int Block_size_min = 1;
int Block_size_max = 10000;
int BACKLOG = 5;


/***************CLIENT UTIL******************/
int validateUserInput(int argc, unsigned short server_port, unsigned short size_msg, int count){

	if ((argc != Client_arg) ||(server_port < Server_port_min || server_port > Server_port_max) ||(size_msg < Msg_size_min || size_msg > Msg_size_max) || (count < Block_size_min || count > Block_size_max)) {
		return 0;
	}
	return 1;
}

void initClient(char** pbuffer, char** psendbuffer, unsigned short size_msg, int* psock, struct sockaddr_in* psin, unsigned short  server_port, unsigned int server_addr)
{
	*pbuffer = (char *) malloc(size_msg);
	if (!(*pbuffer))
	{
		perror("failed to allocated buffer");
		abort();
	}

	*psendbuffer = (char *) malloc(size_msg);
	if (!(*psendbuffer))
	{
		perror("failed to allocated sendbuffer");
		abort();
	}
	/* create a socket */
	if ((*psock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror ("opening TCP socket");
		abort ();
	}

	/* fill in the server's address */
	memset (psin, 0, sizeof (*psin));
	psin->sin_family = AF_INET;
	psin->sin_addr.s_addr = server_addr;
	psin->sin_port = htons(server_port);

	/* connect to the server */
	if (connect(*psock, (struct sockaddr *)psin, sizeof (*psin)) < 0)
	{
		perror("connect to server failed");
		abort();
	}

}

void setTimeStamp(char* sendbuffer, struct timeval* tv){
	gettimeofday(tv,NULL);
	*(int*)(sendbuffer + 2) = (int)htons(tv->tv_sec);
	*(int*)(sendbuffer + 2 + 4) = (int)htons(tv->tv_usec);
}

int main(int argc, char** argv) {
	printf ("Hello\n");

	/* our client socket */
	int sock;

	/* address structure for identifying the server */
	struct sockaddr_in sin;

	/* convert server domain name to IP address */
	struct hostent *host = gethostbyname(argv[1]);
	printf ("Name: %s\n", host->h_name);
	unsigned int server_addr = *(unsigned int *) host->h_addr_list[0];

	/* server port number */
	unsigned short server_port = atoi (argv[2]);
	

	/*message size*/
	unsigned short size_msg = atoi(argv[3]);
	/* num of msgs*/
	int count = atoi(argv[4]);

	if (!validateUserInput(argc, server_port, size_msg, count)){
		perror("Invalid input arguments!");
		abort();
	}

	char *buffer = NULL;
	char *sendbuffer = NULL;

	/* allocate a memory buffer in the heap */
	/* putting a buffer on the stack like:

	   char buffer[500];

	   leaves the potential for
	   buffer overflow vulnerability */
	initClient(&buffer, &sendbuffer, size_msg, &sock, &sin, server_port, server_addr); 

	if (!sendbuffer)
	{
		perror("failed to allocated sendbuffer");
		abort();
	}

	/* everything looks good, since we are expecting a
	   message from the server in this example, let's try receiving a
	   message from the socket. this call will block until some data
	   has been received */

	long double total_latency, avr_latency;
	total_latency = 0.;
	avr_latency = 0.;
	struct timeval start, end;
	*(short*) sendbuffer  = (short)htons(size_msg);/*set the length of msg to be sent*/
	int num_tests = 0;
	long double cur_latency = 0.;
    while(num_tests < count){
		setTimeStamp(sendbuffer, &start);
		int cnt = 0;
		while(cnt < size_msg){
			int sendnum = send(sock, sendbuffer+cnt, size_msg-cnt, 0);
			cnt = cnt + sendnum;
		}
		sendbuffer[size_msg] = 0;
		// printf("sendbuffer %s",sendbuffer);

		cnt = 0;
		while(cnt < size_msg){
            cnt += recv(sock, buffer+cnt, size_msg-cnt,0);
		}
		gettimeofday(&end,NULL);
		
        cur_latency = (end.tv_sec - start.tv_sec)*pow(10.,6) + end.tv_usec-start.tv_usec; 
        total_latency += cur_latency;
		num_tests += 1;
    }
	
	avr_latency = total_latency/(count*pow(10.,3));

	printf("The average latency is: %Lf ms\n", avr_latency);    

	/* free the resources, generally important! */
	close(sock);
	free(buffer);
	free(sendbuffer);
	return 0;
}















// #include <stdlib.h>
// #include <stdio.h>
// #include <unistd.h>
// #include <string.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <netdb.h>

// /* simple client, takes two parameters, the server domain name,
//    and the server port number */

// int main(int argc, char** argv) {

//   /* our client socket */
//   int sock;

//   /* variables for identifying the server */
//   unsigned int server_addr;
//   struct sockaddr_in sin;
//   struct addrinfo *getaddrinfo_result, hints;

//   /* convert server domain name to IP address */
//   memset(&hints, 0, sizeof(struct addrinfo));
//   hints.ai_family = AF_INET; /* indicates we want IPv4 */

//   if (getaddrinfo(argv[1], NULL, &hints, &getaddrinfo_result) == 0) {
//     server_addr = (unsigned int) ((struct sockaddr_in *) (getaddrinfo_result->ai_addr))->sin_addr.s_addr;
//     freeaddrinfo(getaddrinfo_result);
//   }

//   /* server port number */
//   unsigned short server_port = atoi (argv[2]);

//   char *buffer, *sendbuffer;
//   int size = 500;
//   int count;
//   int num;

//   /* allocate a memory buffer in the heap */
//   /* putting a buffer on the stack like:

//          char buffer[500];

//      leaves the potential for
//      buffer overflow vulnerability */
//   buffer = (char *) malloc(size);
//   if (!buffer)
//     {
//       perror("failed to allocated buffer");
//       abort();
//     }

//   sendbuffer = (char *) malloc(size);
//   if (!sendbuffer)
//     {
//       perror("failed to allocated sendbuffer");
//       abort();
//     }


//   /* create a socket */
//   if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
//     {
//       perror ("opening TCP socket");
//       abort ();
//     }

//   /* fill in the server's address */
//   memset (&sin, 0, sizeof (sin));
//   sin.sin_family = AF_INET;
//   sin.sin_addr.s_addr = server_addr;
//   sin.sin_port = htons(server_port);

//   /* connect to the server */
//   if (connect(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
//     {
//       perror("connect to server failed");
//       abort();
//     }

//   /* everything looks good, since we are expecting a
//      message from the server in this example, let's try receiving a
//      message from the socket. this call will block until some data
//      has been received */
//   count = recv(sock, buffer, size, 0);
//   if (count < 0)
//     {
//       perror("receive failure");
//       abort();
//     }

//   /* in this simple example, the message is a string, 
//      we expect the last byte of the string to be 0, i.e. end of string */
//   if (buffer[count-1] != 0)
//     {
//       /* In general, TCP recv can return any number of bytes, not
// 	 necessarily forming a complete message, so you need to
// 	 parse the input to see if a complete message has been received.
//          if not, more calls to recv is needed to get a complete message.
//       */
//       printf("Message incomplete, something is still being transmitted\n");
//     } 
//   else
//     {
//       printf("Here is what we got: %s", buffer);
//     }



//   long double total_latency, avr_latency;
// 	total_latency = 0.;
// 	avr_latency = 0.;
// 	struct timeval start, end;
// 	*(short*) sendbuffer  = (short)htons(size_msg);/*set the length of msg to be sent*/
// 	int num_tests = 0;
// 	long double cur_latency = 0.;
//     while(num_tests < count){
// 		setTimeStamp(sendbuffer, &start);
// 		int cnt = 0;
// 		while(cnt < size_msg){
// 			int sendnum = send(sock, sendbuffer+cnt, size_msg-cnt, 0);
// 			cnt = cnt + sendnum;
// 		}
// 		sendbuffer[size_msg] = 0;
// 		// printf("sendbuffer %s",sendbuffer);

// 		cnt = 0;
// 		while(cnt < size_msg){
//             cnt += recv(sock, buffer+cnt, size_msg-cnt,0);
// 		}
// 		gettimeofday(&end,NULL);
		
//         cur_latency = (end.tv_sec - start.tv_sec)*pow(10.,6) + end.tv_usec-start.tv_usec; 
//         total_latency += cur_latency;
// 		num_tests += 1;
//     }
	
// 	avr_latency = total_latency/(count*pow(10.,3));

// 	printf("The average latency is: %Lf ms\n", avr_latency);    

// 	/* free the resources, generally important! */
// 	close(sock);
// 	free(buffer);
// 	free(sendbuffer);
// 	return 0;

//   // while (1){ 
//   //   printf("\nEnter the type of the number to send (options are char, short, int, or bye to quit): ");
//   //   fgets(buffer, size, stdin);
//   //   if (strncmp(buffer, "bye", 3) == 0) {
//   //     /* free the resources, generally important! */
//   //     close(sock);
//   //     free(buffer);
//   //     free(sendbuffer);
//   //     return 0;
//   //   }

//   //   /* first byte of the sendbuffer is used to describe the number of
//   //      bytes used to encode a number, the number value follows the first 
//   //      byte */
//   //   if (strncmp(buffer, "char", 4) == 0) {
//   //     sendbuffer[0] = 1;
//   //   } else if (strncmp(buffer, "short", 5) == 0) {
//   //     sendbuffer[0] = 2;
//   //   } else if (strncmp(buffer, "int", 3) == 0) {
//   //     sendbuffer[0] = 4;
//   //   } else {
//   //     printf("Invalid number type entered, %s\n", buffer);
//   //     continue;
//   //   }

//   //   printf("Enter the value of the number to send: ");
//   //   fgets(buffer, size, stdin);
//   //   num = atol(buffer);

//   //   switch(sendbuffer[0]) {
//   //   case 1:
//   //     *(char *) (sendbuffer+1) = (char) num;
//   //     break;
//   //   case 2:
//   //     /* for 16 bit integer type, byte ordering matters */
//   //     *(short *) (sendbuffer+1) = (short) htons(num);
//   //     break;
//   //   case 4:
//   //     /* for 32 bit integer type, byte ordering matters */
//   //     *(int *) (sendbuffer+1) = (int) htonl(num);
//   //     break;
//   //   default:
//   //     break;
//   //   }
//   //   send(sock, sendbuffer, sendbuffer[0]+1, 0);
//   // }

//   return 0;
// }
