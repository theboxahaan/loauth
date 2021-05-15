/* Version 3 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h> // time, ctime
#include<sys/types.h>
#include<sys/time.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netdb.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KWHT  "\x1B[37m"
#define KMAG  "\x1B[35m"

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
}p_stanza;

// Genaral Variable.
char client[20];
char nick[20];
char user[20];
char remotehost[20];
char method[10];
char text[10];
int remoteport = 7002;

/* Variable used for messaging purpose */
#define MessageLength 4096
unsigned short msg_sequence_number;
char message[MessageLength];
char socketbuffer[MessageLength];

/* Used for benchmark purpose */
int BenchMark;           // Chage as per your requirement
int PAYLOAD_SIZE;
int sent_total_payload;
int recv_total_payload;

// For TCP Connection
int sock;
struct sockaddr_in client_addr;
struct sockaddr_in server_addr;
struct hostent *hostname;
int sin_size;

//clock
clock_t start, end;

static void XML_Parse(xmlNode*);
int Connect_TCP_Sock(void);
void Presence(void);
void Publish(void);
void TCP_SendTo();
void TCP_GetIn(void);
void Decode(void);
char *String_Gen(int);

/* Main Funtion */
int main(int argc, char *argv[])
{
    printf("--- XMPP command-Line cLient ---\n");

    strcpy(client,argv[1]);
    
    if (strstr(client,"@") != NULL)
    {
        strcpy(nick,client);
        sscanf(nick,"%[^@]@%s",user,remotehost);
    }        

    strcpy(method,argv[2]);

    if(argc == 5)
    {
        BenchMark = atoi(argv[3]);
        PAYLOAD_SIZE = atoi(argv[4]);
    }
    
    if(argc == 4)
    {
        BenchMark = atoi(argv[3]);
        PAYLOAD_SIZE = 1;
    }

    if(argc == 3)
    {
        BenchMark = 1;
        PAYLOAD_SIZE = 1;
    }

    printf("User: %s\n", user);
    printf("RemoteHost: %s\n", remotehost);


    if(Connect_TCP_Sock() != 0)
    {
        msg_sequence_number = 0; //start message sequence id;
        sent_total_payload = 0;
        recv_total_payload = 0;
        start = clock();
        Presence();
    }

    return 0;
}

void Decode()
{
    msg_sequence_number = p_stanza.id;

    if(strcmp(method, "sub") == 0)
    {
        if((strcmp(p_stanza.s_type, "presence") == 0)&&(strcmp(p_stanza.type, "subscribed") == 0))
        {
            TCP_GetIn();
        }
        if(strcmp(p_stanza.type, "result") == 0)
        {
            Presence();
        }
    }
    else
    {
        if((strcmp(p_stanza.s_type, "presence") == 0)&&(strcmp(p_stanza.type, "subscribed") == 0))
        {
            Publish();
        }
        if(strcmp(p_stanza.type, "result") == 0)
        {
            BenchMark = BenchMark - 1;
            if(BenchMark == 0)
            {
                end = clock();
                printf("%sComplete!\n", KWHT);
                printf("Turnaround Time: %f Sec.\n", (((double) (end - start)) / CLOCKS_PER_SEC));
                printf("Send Data: %d Bytes\nRecv Data: %d Bytes\n", sent_total_payload, recv_total_payload);
                close(sock);
                exit(1);
            }
            else
                Presence();
        }
    }
    
}

void Publish()
{
    char *randomString = String_Gen(PAYLOAD_SIZE);
    sprintf(message, "<iq type='set' to='pubsub.%s' from='%s' id='%d'><publish item='%s'><payload>%s</payload></publish></iq>", remotehost, client, (msg_sequence_number+1), p_stanza.item, randomString);
    TCP_SendTo();
    free(randomString);
    printf("%sinfo: %sPublish an item.\n", KWHT, KGRN);
    TCP_GetIn();
}

void TCP_GetIn(void)
{
    strncpy(socketbuffer, "\0", sizeof(socketbuffer));   //Reset the buffer.
    
    if(recv(sock, socketbuffer, sizeof(socketbuffer),0) < 1)	
    {
        close(sock);
        exit(1);
    }
    else
    {
        printf("%sinfo: %sServer Responce:  %s%s\n", KWHT, KGRN, KMAG, socketbuffer);
        recv_total_payload = recv_total_payload + strlen(socketbuffer);
        xmlDoc *doc = NULL;
        xmlNode *root_element = NULL;
        doc = xmlParseDoc(socketbuffer);
        root_element = xmlDocGetRootElement(doc);
        XML_Parse(root_element);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        Decode();
    }
}

void TCP_SendTo()
{
    if(send(sock,message,strlen(message),0) == 0)
    {
        close(sock);
        exit(1);
    }
    else
    {    
        printf("%sinfo: %sClient Send:  %s%s\n", KWHT, KGRN, KYEL, message);
        sent_total_payload = sent_total_payload + strlen(message);
    }
}

void Presence()
{
    sprintf(message, "<presence id='%d' to='pubsub.%s' from='%s' type='subscribe'/>", (msg_sequence_number+1), remotehost, client);
    TCP_SendTo();
    printf("%sinfo: %sPresence with subscribtion send.\n", KWHT, KGRN);
    TCP_GetIn();
}

int Connect_TCP_Sock(void)
{
    /*  Create socket  */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{	perror("opening steam socket in");
		exit(1);
	}

    /* Create server address */
	hostname=gethostbyname(remotehost);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr( inet_ntoa(*((struct in_addr *)hostname->h_addr)) );
    server_addr.sin_port = htons(remoteport);
	//memset(server_addr.sin_zero, 0, 8);

    /* resolve DNS and connecting socket */
	printf("%sinfo: %sConnecting to: %s port: %d.\n", KWHT, KGRN, inet_ntoa(server_addr.sin_addr),server_addr.sin_port);
	
	sin_size=sizeof(struct sockaddr_in);
	if(connect(sock,(struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) 
    {
		perror("connect");
		exit(1);
    }
    else
    {
        printf("%sinfo: %sTCP connecetion establishment successfull.\n", KWHT, KGRN);
        return 1;
    }

    printf("%sinfo: %sTCP connecetion establishment successfull.\n", KWHT, KGRN);
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

char *String_Gen(int length) 
{ 
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; // could be const
    char *randomString;

    if (length) 
    {
        randomString = malloc(length +1);

        if (randomString) 
        {
            int l = (int) (sizeof(charset) -1);
            int key;
            for (int n = 0;n < length;n++) 
            {        
                key = rand() % l;
                randomString[n] = charset[key];
            }

            randomString[length] = '\0';
        }
    }

    return randomString;
}

