#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <memory.h>
#include <time.h>

#include <poll.h>

/* For coloring output */
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KWHT  "\x1B[37m"
#define KMAG  "\x1B[35m"

//global variable
#define BUFFERSIZE 4096
char response[BUFFERSIZE];
char selfaddress[30];
char load[2048];
char header[512];
char head[3072]; 
char html[] = "<html><title>Reverse Server</title><body>Server Can Reverse A String.</body></html>";
                            

/* For polling purpose */
#define POLL_SIZE 1000
#define LISTEN_QUEUE 1000
struct pollfd poll_set[POLL_SIZE];
int numclient;

int serversocket;
struct sockaddr_in serveraddress;
int AddrLen;
int clientsocket;

struct http_request_parse //for storing the values of received msg.
{
    char* method;
    char* location;
    char* version;
    char* host;
    char* content_type;
    char* content_length;
    char* payload;
} http_request;

void HTTP_Parser(char req[]) //parsing received http request/response.
{
    char* line[30];
    char* token[100]; 
    
    int count = 0;
    int count2;
    int count1 = 0;

    //devided by \r\n.
    line[count] = strtok(req, "\r\n");
    for (;(line[++count] = strtok(NULL, "\r\n")) != NULL;);

    //devided by space
    count = 0;
    while(line[count] != NULL)
    {
        char* sent_token[100];
        count2 = 0;
        sent_token[count2] = strtok(line[count], " ");

        while(sent_token[count2] != NULL)
        {
            token[count1] = sent_token[count2];
            sent_token[++count2] = strtok(NULL, " ");
            count1++;
        }
        count++;
    }
    
    http_request.method = token[0];
    http_request.location = token[1];
    http_request.version = token[2];
    
    if(strcmp(http_request.method, "POST") == 0)
    {
        count = 3;
        if(strcmp(token[count], "Host:") == 0)
        {
            http_request.host = token[count+1];
            count = count + 2;
        }
        if(strcmp(token[count], "Content-Type:") == 0)
        {
            http_request.content_type = token[count+1];
            count = count + 2;
        }
        if(strcmp(token[count], "Content-Length:") == 0)
        {
            http_request.content_length = token[count+1];
            count = count + 2;
        }
        http_request.payload = token[count];
    }  

    // printf("[\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n]", http_request.method, http_request.location, http_request.version, http_request.host,http_request.content_type, http_request.content_length, http_request.payload);  
}

void Reverse_Str(char str[]) //Reverse String function
{
  int n = strlen(str);
  int i;
  for(i=0; i<(n/2); i++)
  {
    char ch = str[i];
    str[i] = str[n-i-1];
    str[n-i-1] = ch;
  }
}

void TCP_Send(int clientindex)
{
    printf("\n%sinfo: %sServer Response:", KWHT, KGRN);
    printf("\n%s%s\n\n", KMAG, response);

    int bytes_sent, total_bytes_sent, bytes_to_send;
    total_bytes_sent = 0;
    bytes_to_send = strlen(response);
    while(total_bytes_sent < bytes_to_send) 
    {
        // sleep(1);
        bytes_sent = send(poll_set[clientindex].fd, response, strlen(response), 0);
        total_bytes_sent += bytes_sent;
    }
}

void Polling()
{
    while(1)
    {
        printf("\n%s--------------------------------------------\n(Currently No. of Client: %d) Waiting for new connection...\n--------------------------------------------\n", KWHT, (numclient-1));

        int clientindex;
        poll(poll_set, numclient, -1);

        for(clientindex = 0; clientindex < numclient; clientindex++)
        {
            if( poll_set[clientindex].revents & POLLIN ) 
            {
                if(poll_set[clientindex].fd == serversocket)
                {
                    clientsocket = accept(serversocket, (struct sockaddr *)&serveraddress, (socklen_t*)&AddrLen);

                    poll_set[numclient].fd = clientsocket;
                    poll_set[numclient].events = POLLIN;
                    numclient++;
 
                    printf("%sinfo: %sAdding client at %d in poll.\n", KWHT, KGRN, clientsocket);
                }
                else 
                {   
                    strncpy(response, "\0", sizeof(response));
                    // sleep(1);
                    if(recv(poll_set[clientindex].fd, response, sizeof(response), 0) == 0)	
                    {
                        printf("%sinfo: %sConnecttion End!\n", KWHT, KRED);

                        close(poll_set[clientindex].fd);
                        poll_set[clientindex].events = 0;
                        printf("%sinfo: %sRemoving client %d from poll.\n", KWHT, KRED, poll_set[clientindex].fd);
                        int i = clientindex;
                        for( i = clientindex; i<numclient; i++)
                        {
                            poll_set[i] = poll_set[i + 1];
                        }
                        numclient--;
                    }
                    else
                    {
                        printf("%sinfo: %sServing client %d.\n", KWHT, KGRN, poll_set[clientindex].fd);
                        printf("%sinfo: %sClient Request:\n%s%s\n", KWHT, KGRN, KYEL, response);
                        time_t t;
                        time(&t);

                        HTTP_Parser(response);

                        if(strcmp(http_request.method, "GET") ==  0)
                        {
                            strncpy(header, "\0", sizeof(header));
                            sprintf(header, "HTTP/1.1 200 OK \nDate: %sServer: %s\nCache-control: no-cache\nContent-Type: text/html; charset=utf-8\nContent-Length: %ld\nContent-Language: en\nConnection: keep-alive\n\n", ctime(&t), selfaddress, strlen(html));  

                            strncpy(response, "\0", sizeof(response));
                            strcpy(response, header);
                            strcat(response, html);

                            TCP_Send(clientindex);
                        }
                        else // if(strcmp(http_request.method, "POST") ==  0)
                        {
                            strncpy(load, "\0", sizeof(load));
                            sprintf(load, "%s", http_request.payload);
                            Reverse_Str(load);

                            strncpy(head, "\0", sizeof(head));
                            sprintf(head, "HTTP/1.1 200 OK \nDate: %sServer: %s\nCache-control: no-cache\nContent-Type: text/plain; charset=utf-8\nContent-Length: %s\nContent-Language: en\nConnection: keep-alive\n\n", ctime(&t), selfaddress, http_request.content_length); 

                            strncpy(response, "\0", sizeof(response));
                            strcpy(response, head);
                            strcat(response, load);

                            int i;
                            for(i=1; i<numclient; i++)
                            {
                                TCP_Send(i);
                            }
                        }                      
                    }
                }
            }
        }
    }
    
}

int main(int argc, char const *argv[])
{
    // char* IP = "127.0.0.1";
    char* IP = "172.18.0.2";
    int PORT = atoi(argv[1]);

    sprintf(selfaddress, "%s:%d", IP, PORT);
    printf("%sServer Address: %s\n", KWHT, selfaddress);
    
    //create server socket.
    serversocket = socket(AF_INET, SOCK_STREAM, 0);
    
    //assign localhost/any ip address.
    AddrLen = sizeof(serveraddress);
    serveraddress.sin_family = AF_INET;
    inet_pton(AF_INET, IP, &(serveraddress.sin_addr));
    serveraddress.sin_port = htons( PORT );    
       
    //bind address and port with server socket.
    bind(serversocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress));
    
    //ready for new connection.
    listen(serversocket, LISTEN_QUEUE);

    numclient = 0;
    memset(poll_set, '\0', sizeof(poll_set));

    /* First add the server */
    poll_set[0].fd = serversocket;
    poll_set[0].events = POLLIN;
    numclient++;

    Polling(); 

    return 0;
}

