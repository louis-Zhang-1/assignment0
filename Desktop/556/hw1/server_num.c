#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

/**************************************************/
/* a few simple linked list functions             */
/**************************************************/

/* A linked list node data structure to maintain application
   information related to a connected socket */
int min_msg_size = 10;
int max_msg_size = 65535;
struct node
{
  int socket;
  struct sockaddr_in client_addr;
  int pending_data;
  char *buffer;
  int pointer;
  int buffer_size;
  struct node *next;
};

/* create the data structure associated with a connected socket */
void add(struct node *head, int socket, struct sockaddr_in addr)
{
  struct node *new_node;

  new_node = (struct node *)malloc(sizeof(struct node));
  new_node->socket = socket;
  new_node->client_addr = addr;
  //flag which indicates wether we still have data to send
  new_node->pending_data = 0;
  new_node->pointer = 0;
  new_node->buffer_size = 0;
  new_node->buffer = (char *)malloc(max_msg_size);
  new_node->next = head->next;
  head->next = new_node;
}

/* remove the data structure associated with a connected socket
   used when tearing down the connection */
void dump(struct node *head, int socket)
{
  struct node *current, *temp;

  current = head;

  while (current->next)
  {
    if (current->next->socket == socket)
    {
      /* remove */
      temp = current->next;
      current->next = temp->next;
      free(temp->buffer);
      free(temp); /* don't forget to free memory */
      return;
    }
    else
    {
      current = current->next;
    }
  }
}

/**************************************************/
/* Http mode to get http req and send file        */
/**************************************************/

void send_file(int client_sock, char *file_path)
{
  FILE *fp;
  char file_dir[1024];
  strcpy(file_dir, ".");
  strcat(file_dir, file_path);
  // open the file
  fp = fopen(file_dir, "rb");
  if (fp == NULL)
  {
    perror("File");
    return;
  }

  // get the size of file
  long indicator = 0;
  fseek(fp, indicator, SEEK_END);
  long pt = ftell(fp);
  rewind(fp);

  // file -> buffer
  char *buffer = calloc(1, pt + 1);
  fread(buffer, pt, 1, fp);
  int count = 0;
  while (count < pt)
  {
    count += send(client_sock, buffer + count, pt - count, 0);
  }
  fclose(fp);
  free(buffer);
}
void send_http_res(int client_sock, char *err_msg, int mode)
{
  char header[256] = "HTTP/1.1 ";
  char *content_type = "Content-Type: text/html \r\n";
  char *blank_char = "\r\n";
  printf("errmsg: %s\n", err_msg);
  int count = 0;
  if (mode == 0)
  {
    strcat(header, err_msg);
  }
  else
  {
    strcat(header, "200 OK");
  }
  strcat(header, " \r\n");
  int message_size = strlen(header);
  while (count < message_size)
  {
    count += send(client_sock, header + count, message_size - count, 0);
  }

  count = 0;
  message_size = strlen(content_type);
  while (count < message_size)
  {
    count += send(client_sock, content_type + count, message_size - count, 0);
  }

  count = 0;
  message_size = strlen("\r\n");
  while (count < message_size)
  {
    count += send(client_sock, blank_char + count, message_size - count, 0);
  }

  if (mode == 0)
  {
    char html_body[1024] = "<html><body><a>";
    strcat(html_body, err_msg);
    strcat(html_body, "</a></body></html>");
    count = 0;
    message_size = strlen(html_body);
    while (count < message_size)
    {
      count += send(client_sock, html_body + count, message_size - count, 0);
    }
  }
  else
  {
    char *file_path = err_msg;
    send_file(client_sock, file_path);
  }
}

void http_mode(int client_sock)
{

  char *request_msg;
  request_msg = (char *)malloc(max_msg_size);
  memset(request_msg, 0, max_msg_size);
  recv(client_sock, request_msg, max_msg_size, 0);

  const char tmp[2] = " ";
  char *url[2];

  url[0] = strtok(request_msg, tmp);
  url[1] = strtok(NULL, tmp);
  char *method = url[0];
  char *request_dir = url[1];
  char header[256];
  char file_dir[256];
  strcpy(file_dir, ".");
  strcat(file_dir, request_dir);

  if ((strcmp(method, "GET")) != 0)
  {
    strcpy(header, "501 Not Implemented");
    send_http_res(client_sock, header, 0);
    close(client_sock);
    free(request_msg);
    return;
  }
  if (strstr(request_dir, "../") != NULL)
  {
    strcpy(header, "400 Bad Request");
    send_http_res(client_sock, header, 0);
    close(client_sock);
    free(request_msg);
    return;
  }
  FILE *file;
  file = fopen(file_dir, "r");
  if (file == NULL)
  {
    strcpy(header, "404 Not Found");
    send_http_res(client_sock, header, 0);
    close(client_sock);
    free(request_msg);
    return;
  }
  else
  {
    fclose(file);
  }

  send_http_res(client_sock, request_dir, 1);
  close(client_sock);
  free(request_msg);
}

/*****************************************/
/* main program                          */
/*****************************************/

/* simple server, takes one parameter, the server port number */
int main(int argc, char **argv)
{
  /* socket and option variables */
  int sock, new_sock, max;
  int optval = 1;

  /* server socket address variables */
  struct sockaddr_in sin, addr;
  unsigned short server_port = atoi(argv[1]);

  /* socket address variables for a connected client */
  socklen_t addr_len = sizeof(struct sockaddr_in);

  /* maximum number of pending connection requests */
  int BACKLOG = 5;

  /* variables for select */
  fd_set read_set, write_set;
  struct timeval time_out;
  int select_retval;

  /* a silly message */
  char *message = "Welcome! COMP/ELEC 429 Students!\n";

  /* number of bytes sent/received */
  int count;

  /* linked list for keeping track of connected sockets */
  struct node head;
  struct node *current, *next;

  /* a buffer to read data */
  char *buf;
  int BUF_LEN = 1000;

  buf = (char *)malloc(BUF_LEN);

  /* initialize dummy head node of linked list */
  head.socket = -1;
  head.next = 0;
  char *req_dir;

  int www_mode = 0; // www_mode == 0 --> pingpong mode; 1 for www mode.
  if ((argc == 4) && (strcmp(argv[2], "www") == 0))
  {
    req_dir = argv[3];
    www_mode = 1;
    printf("%s", "you are in web mode\n");
  }

  /* create a server socket to listen for TCP connection requests */
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  {
    perror("opening TCP socket");
    abort();
  }

  /* set option so we can reuse the port number quickly after a restart */
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
  {
    perror("setting TCP socket option");
    abort();
  }

  /* fill in the address of the server socket */
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(server_port);

  /* bind server socket to the address */
  if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
  {
    perror("binding socket to address");
    abort();
  }

  /* put the server socket in listen mode */
  if (listen(sock, BACKLOG) < 0)
  {
    perror("listen on socket failed");
    abort();
  }

  /* now we keep waiting for incoming connections,
     check for incoming data to receive,
     check for ready socket to send more data */
  while (1)
  {

    /* set up the file descriptor bit map that select should be watching */
    FD_ZERO(&read_set);  /* clear everything */
    FD_ZERO(&write_set); /* clear everything */

    FD_SET(sock, &read_set); /* put the listening socket in */
    max = sock;              /* initialize max */

    /* put connected sockets into the read and write sets to monitor them */
    for (current = head.next; current; current = current->next)
    {
      FD_SET(current->socket, &read_set);

      if (current->pending_data)
      {
        /* there is data pending to be sent, monitor the socket
             in the write set so we know when it is ready to take more
             data */
        FD_SET(current->socket, &write_set);
      }

      if (current->socket > max)
      {
        /* update max if necessary */
        max = current->socket;
      }
    }

    time_out.tv_usec = 100000; /* 1-tenth of a second timeout */
    time_out.tv_sec = 0;

    /* invoke select, make sure to pass max+1 !!! */
    select_retval = select(max + 1, &read_set, &write_set, NULL, &time_out);
    if (select_retval < 0)
    {
      perror("select failed");
      abort();
    }

    if (select_retval == 0)
    {
      /* no descriptor ready, timeout happened */
      continue;
    }

    if (select_retval > 0) /* at least one file descriptor is ready */
    {
      if (FD_ISSET(sock, &read_set)) /* check the server socket */
      {
        /* there is an incoming connection, try to accept it */
        new_sock = accept(sock, (struct sockaddr *)&addr, &addr_len);

        if (new_sock < 0)
        {
          perror("error accepting connection");
          abort();
        }

        if (www_mode)
        {
          http_mode(new_sock);
          close(new_sock);
          continue;
        }

        /* make the socket non-blocking so send and recv will
                 return immediately if the socket is not ready.
                 this is important to ensure the server does not get
                 stuck when trying to send data to a socket that
                 has too much data to send already.
               */
        if (fcntl(new_sock, F_SETFL, O_NONBLOCK) < 0)
        {
          perror("making socket non-blocking");
          abort();
        }

        /* the connection is made, everything is ready */
        /* let's see who's connecting to us */
        printf("Accepted connection. Client IP address is: %s\n",
               inet_ntoa(addr.sin_addr));

        /* remember this client connection in our linked list */
        add(&head, new_sock, addr);

        /* let's send a message to the client just for fun */
        count = send(new_sock, message, strlen(message) + 1, 0);
        if (count < 0)
        {
          perror("error sending message to client");
          abort();
        }
      }

      /* check other connected sockets, see if there is
             anything to read or some socket is ready to send
             more pending data */
      for (current = head.next; current; current = next)
      {
        next = current->next;
        /* see if we can now do some previously unsuccessful writes */
        if (FD_ISSET(current->socket, &write_set))
        {
          /* the socket is now ready to take more data */
          int count;
          count = send(current->socket, current->buffer + current->pointer, current->buffer_size - current->pointer, MSG_DONTWAIT);
          if (count <= 0)
          {
            /* something is wrong */
            if (count == 0)
            {
              printf("Client closed connection. Client IP address is: %s\n", inet_ntoa(current->client_addr.sin_addr));
            }
            else
            {
              perror("error receiving from a client");
            }
            /* connection is closed, clean up */
            close(current->socket);
            dump(&head, current->socket);
          }
          else
          {
            // change the state of the connection
            current->pointer += count;
            // all data has been sent out, this connection can get out of the write set
            if (current->pointer == current->buffer_size)
            {
              current->pointer = 0;
              current->pending_data = 0;
              current->buffer_size = 0;
            }
          }
        }

        if (FD_ISSET(current->socket, &read_set))
        {
          int count;
          /*use non blocking mechanism*/
          count = recv(current->socket, current->buffer + current->pointer, max_msg_size - current->pointer, MSG_DONTWAIT);
          if (count <= 0)
          {
            /* something is wrong */
            if (count == 0)
            {
              printf("Client closed connection. Client IP address is: %s\n", inet_ntoa(current->client_addr.sin_addr));
            }
            else
            {
              perror("error receiving from a client");
            }
            /* connection is closed, clean up */
            close(current->socket);
            dump(&head, current->socket);
          }
          else
          {
            if (current->buffer_size == 0)
            {
              current->buffer_size = ntohs(*(unsigned short *)current->buffer); /*get the length of message to be received*/
            }
            current->pointer += count;
            /*if the total msg has been received, enable the write flag of this node*/
            if (current->pointer == current->buffer_size)
            {
              current->pending_data = 1;
              current->pointer = 0;
            }
          }
        }
      }
    }
  }
}
