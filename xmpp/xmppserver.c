/* Version 3 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h> // time, ctime
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<libxml/parser.h>
#include<libxml/tree.h>
#include<sys/time.h>
#include<netdb.h>

#include <poll.h>

/* For coloring output */
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KWHT  "\x1B[37m"
#define KMAG  "\x1B[35m"

/* For polling purpose */
#define POLL_SIZE 1000
#define LISTEN_QUEUE 1000
struct pollfd poll_set[POLL_SIZE];
int numclient;

/* Topic */
#define ITEM "reverse"

/* Store the values decoded from xml stanza */
struct stanza
{
    char s_type[10];
    char type[10];
    char from[15];
    char to[15];
    int id;
    char method[10];
    char item[10];
    char* payload;
}p_stanza; // Structure instnat.


/* Variable used for genarel purpose */
char* IP;
int PORT;
char* selfaddress;
char fulladdress[25];

/* Variable used for TCP Connection. */
int selfsocket;
struct sockaddr_in serveraddress;
int AddrLen = sizeof(serveraddress);
int clientsocket;
struct sockaddr_in clientaddress;
struct hostent *hostname;
int sin_size;

/* Variable used for messaging purpose */
#define MessageLength 12048
unsigned short msg_sequence_number;
char message[MessageLength];
char socketbuffer[MessageLength];

/* Functions */
void Connect_TCP_Sock(void);     // Initial TCP connection.
void Polling(void);
static void XML_Parse(xmlNode*); // Parsing XML.
void Decode(int);               // Dicision making for what to do next.
void Presence_Sub_Reply(int);   // Reply to the presence_sub request.
void TCP_SendTo(int);               // Send message to client.
void Reverse_Str(char[]);        // Reverse string service.
void Result_Send_Back();     // Result send back.

/* Main Funtion */
int main(int argc, char *argv[])
{
    printf("--- XMPP command-Line server ---\n");

    // IP = argv[1];
    // IP = "127.0.0.1";
    IP = "172.19.0.2";
    
    PORT = atoi(argv[1]);
    
    selfaddress = IP;

    Connect_TCP_Sock();

    return 0;
}

void Decode(int clientindex)
{
    msg_sequence_number = p_stanza.id;

    if((strcmp(p_stanza.s_type, "presence") == 0) && (strcmp(p_stanza.type, "subscribe") == 0))
    {
        Presence_Sub_Reply(clientindex);
    }
    if((strcmp(p_stanza.s_type, "iq") == 0) && (strcmp(p_stanza.type, "set") == 0))
    {
        if((strcmp(p_stanza.method, "publish") == 0) && (strcmp(p_stanza.item, "reverse") == 0))
        {
            Reverse_Str(p_stanza.payload);
            Result_Send_Back();
        }
    }
    else
        printf("Error while decoding..\n");
}

void Result_Send_Back()
{
    sprintf(message, "<iq type='result' from='pubsub.%s' id='%d'><publish item='reverse'><payload>%s</payload></publish></iq>",selfaddress, rand(), p_stanza.payload);
    int i;
    for(i=1; i<numclient; i++)
    {
        TCP_SendTo(i);
    }

    printf("%sinfo: %sResult send back.\n", KWHT, KGRN);
    Polling();
}

void Presence_Sub_Reply(int clientindex)
{
    sprintf(message, "<presence from='pubsub.%s' to='%s' id='%d' type='subscribed' item='%s' />", selfaddress, p_stanza.from, (msg_sequence_number+1), ITEM);
    TCP_SendTo(clientindex);
    Polling();
}

void TCP_SendTo(int clientindex)
{
    if(send(poll_set[clientindex].fd,message,strlen(message),0) == 0)
    {
        perror("send");
        exit(1);
    }
    else
    {    
        printf("%sinfo: %sServer Send:  %s%s\n", KWHT, KGRN, KYEL, message);
    }
}

void Connect_TCP_Sock()
{
    //create server socket.
    selfsocket = socket(AF_INET, SOCK_STREAM, 0);
    
    //assign localhost/any ip address.
    serveraddress.sin_family = AF_INET;
    inet_pton(AF_INET, IP, &(serveraddress.sin_addr));
    serveraddress.sin_port = htons( PORT );     
       
    //bind address and port with server socket.
    bind(selfsocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress));

    sprintf(fulladdress, "%s:%d", inet_ntoa(serveraddress.sin_addr), serveraddress.sin_port);
    printf("%sServer Address: %s\n", KWHT, fulladdress);
    
    //ready for new connection.
    listen(selfsocket, LISTEN_QUEUE);

    numclient = 0;
    memset(poll_set, '\0', sizeof(poll_set));

    /* First add the server */
    poll_set[0].fd = selfsocket;
    poll_set[0].events = POLLIN;
    numclient++;
    
    Polling();
}

void Polling()
{
    while(1)
    {
        printf("\n%s--------------------------------------------\n(Currently No. of Client: %d) Waiting for new connection...\n--------------------------------------------\n", KWHT, numclient-1);

        int clientindex;
        poll(poll_set, numclient, -1);

        for(clientindex = 0; clientindex < numclient; clientindex++)
        {
            if( poll_set[clientindex].revents & POLLIN ) 
            {
                if(poll_set[clientindex].fd == selfsocket)
                {
                    clientsocket = accept(selfsocket, (struct sockaddr *)&serveraddress, (socklen_t*)&AddrLen);

                    poll_set[numclient].fd = clientsocket;
                    poll_set[numclient].events = POLLIN;
                    numclient++;
 
                    printf("%sinfo: %sAdding client at %d in poll.\n", KWHT, KGRN, clientsocket);
                }
                else 
                {
                    strncpy(socketbuffer, "\0", sizeof(socketbuffer));   //Reset the buffer.
		            // sleep(1);
                    if(recv(poll_set[clientindex].fd,socketbuffer, sizeof(socketbuffer),0) == 0)	
                    {
                        printf("%sinfo: %sConnecttion End.\n", KWHT, KRED);

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
                        printf("%sinfo: %sClient Request:  %s%s\n", KWHT, KGRN, KMAG, socketbuffer);
                        xmlDoc *doc = NULL;
                        xmlNode *root_element = NULL;
                        doc = xmlParseDoc(socketbuffer);
                        root_element = xmlDocGetRootElement(doc);
                        XML_Parse(root_element);
                        xmlFreeDoc(doc);
                        xmlCleanupParser();
                        Decode(clientindex);
                    }
                }
            }
        }
    }
    
}

static void XML_Parse(xmlNode * a_node)
{
    xmlNode *cur_node = NULL;
    for (cur_node = a_node; cur_node; cur_node = cur_node->next) 
    {
        if (cur_node->type == XML_ELEMENT_NODE) 
        {
            if((strcmp((char *)cur_node->name, "iq") == 0) || (strcmp((char *)cur_node->name, "presence") == 0))
            {
                strcpy(p_stanza.s_type, (char *)cur_node->name);
            }
            if(strcmp(cur_node->name, "publish") == 0)
            {
                strcpy(p_stanza.method, (char *)cur_node->name);
            }

            int count = 0;
            xmlAttr *cur_attr_node = cur_node->properties;
            while(cur_attr_node != NULL)
            {
                count++;
                if(cur_attr_node->name != NULL)

                    if(strcmp(cur_attr_node->name, "type") == 0)
                    {
                        strcpy(p_stanza.type, (char *)cur_attr_node->children->content);
                    }
                    if(strcmp(cur_attr_node->name, "from") == 0)
                    {
                        strcpy(p_stanza.from, (char *)cur_attr_node->children->content);
                    }
                    if(strcmp(cur_attr_node->name, "to") == 0)
                    {
                        strcpy(p_stanza.to, (char *)cur_attr_node->children->content);
                    }
                    if(strcmp(cur_attr_node->name, "item") == 0)
                    {
                        strcpy(p_stanza.item, (char *)cur_attr_node->children->content);
                    }
                    if(strcmp(cur_attr_node->name, "id") == 0)
                    {
                        p_stanza.id = atoi(cur_attr_node->children->content);
                    }

                cur_attr_node = cur_attr_node->next;
            }
            xmlNode *cur_child_node = cur_node->children;
            if(xmlNodeIsText(cur_child_node))
            {
                p_stanza.payload = (char *)xmlNodeGetContent(cur_child_node);
            }
        }

        XML_Parse(cur_node->children);
    }
}

void Reverse_Str(char str[]) //Reverse String function
{
  int n = strlen(str);
  int i;
  for (i=0; i<(n/2); i++)
  {
    char ch = str[i];
    str[i] = str[n-i-1];
    str[n-i-1] = ch;
  }
}

