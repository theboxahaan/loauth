// gcc -w  myclient.c -o test1 -I/usr/include/libxml2 -lxml2
// gcc -w  myclient.c -o test1 -L/usr/lib -lssl -lcrypto -I/usr/include/libxml2 -lxml2

// #include "authModule.h"
#include "present_cbc.h"

// #include<time.h> // time, ctime
#include<sys/types.h>
#include<sys/time.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netdb.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
// #include <time.h>

#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KWHT  "\x1B[37m"
#define KMAG  "\x1B[35m" 

struct stanza
{
    // char s_type[10];
    // char type[10];
    // char from[15];
    // char to[15];
    // int id;
    // char method[10];
    // char item[10];
    char* serverSign;
    char* challenge;
}p_stanza;

// Genaral Variable.
char client[100];
char nick[100];
char user[100];
char resourse[100];
char fulljid[200];
char pass[100];
char remotehost[100];
char remoteport[6];

char* key = "abcdefghij";

int msg_sequence_number;
char tempmessage[300000];
char presmessage[300000];
int sockbufflen;
char sockbuff[300000];

// For TCP Connection
int sock;
struct sockaddr_in client_addr;
struct sockaddr_in server_addr;
struct hostent *hostname;
int sin_size;

// char* clientnonce = "fyko+d2lbbFgONRv9qkxdawL";
char* clientnonce;
char* serversignature;

void DecodeXML();
static void XML_Parse(xmlNode*);

int Connect_TCP_Sock();
void TCP_Init_Stream_Header();
void TCP_SendTo(char *msg);
void TCP_GetIn();

void Init_Auth(char*, char*);
void Auth_Response(char*, char*, char*, char*);

void Bind();

void Start_PRESENT();
void PRE_Init_Stream_Header();
void PRE_SendTo();
void PRE_GetIn();
void PRE_presence();

// void SASL_Session(void);
// void SASL_Roster(void);

/* Main Funtion */
int main(int argc, char *argv[])
{
    printf("--- XMPP command-Line cLient ---\n");

    clientnonce = "bYNWijYPGGAALqRz3Ec1l5cEuG2aM/wOt+kOE31q2es=";

    strcpy(client,argv[1]);
    if (strstr(client,"@") != NULL){
        strcpy(nick,client);
        sscanf(nick,"%[^@]@%s",user,remotehost);
    }        
    strcpy(pass,argv[2]);
    strcpy(remoteport, "5222");

    printf("User: %s\n", user);
    printf("Remote Host: %s\n", remotehost);

    gethostname(resourse, 100);

    sprintf(fulljid, "%s@%s/%s", user, remotehost, resourse);

    if(Connect_TCP_Sock() != 0)
    {
        TCP_Init_Stream_Header();
    }

    return 0;
}

void DecodeXML()
{
    if(strstr(sockbuff, "<mechanism>SCRAM-SHA-1</mechanism>") != NULL)
    {
        Init_Auth(user, clientnonce);
    }
    else if(strstr(sockbuff, "<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>") != NULL)
    {
        xmlDoc *doc = NULL;
        xmlNode *root_element = NULL;
        doc = xmlParseDoc(sockbuff);
        root_element = xmlDocGetRootElement(doc);
        XML_Parse(root_element);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        Auth_Response(p_stanza.challenge, user, pass, clientnonce);
    }
    else if(strstr(sockbuff, "<success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>") != NULL)
    {
        TCP_Init_Stream_Header();
    }
    else if(strstr(sockbuff, "<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'><required/></bind>") != NULL)
    {
        Bind();
    }
    else if(strstr(sockbuff, "<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'><jid>") != NULL)
    {
        Start_PRESENT();
    }
    else if(strstr(sockbuff, "<proceed xmlns='urn:ietf:params:xml:ns:xmpp-present'/>") != NULL)
    {
        PRE_presence();
    }
    else
    {
        printf("%s-- The End --\n", KWHT);
        close(sock);
        exit(1);
    }
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
	server_addr.sin_port = htons((unsigned short int) atoi(remoteport));  /* Use specified port */
	//memset(server_addr.sin_zero, 0, 8);

    /* resolve DNS and connecting socket */
	printf("%sinfo: %sConnecting to: %s port: %s.\n", KWHT, KGRN, inet_ntoa(server_addr.sin_addr),remoteport);
	sin_size=sizeof(struct sockaddr_in);
	if(connect(sock,(struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) 
    {
		perror("connect");
		exit(1);
    }
    else
    {
        printf("%sinfo: %sConnecetion establishment successfull.\n", KWHT, KGRN);
        return 1;
    }
}

void TCP_Init_Stream_Header()
{
    sprintf(tempmessage, "<?xml version='1.0'?><stream:stream xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' xml:lang='en' version='1.0' to='%s'>", remotehost);
    TCP_SendTo(tempmessage);
    TCP_GetIn();
}

void TCP_SendTo(char *msg)
{
    if(send(sock,msg,strlen(msg),0) == -1)
    {
        perror("send");
        exit(1);
    }
    else
    {
        printf("%sinfo: %sClient Send:\n%s%s\n", KWHT, KGRN, KYEL, msg);
        TCP_GetIn();
    }
}

void TCP_GetIn(void)
{
    strncpy(sockbuff, "\0", sizeof(sockbuff));

    if((sockbufflen=recv(sock,sockbuff, sizeof(sockbuff),0)) == -1)	
    {
        printf("%sinfo: %sbrk  recv in - buflen = %i\n", KWHT, KRED, sockbuff);
        perror("recv");
        exit(1);
    }
    else
    {
        printf("%sinfo: %sServer Responce:\n%s%s\n", KWHT, KGRN, KMAG, sockbuff);
        DecodeXML();
    }
}

void Init_Auth(char* username, char* clientNonce)
{
    sprintf(tempmessage, "<auth xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\" mechanism=\"SCRAM-SHA-1\">%s</auth>", SCRAM_init(user, clientnonce));
    TCP_SendTo(tempmessage);
}

void Auth_Response(char* challange, char* username, char* password, char* clientNonce)
{
    sprintf(tempmessage, "<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>%s</response>", SCRAM_responce_create(challange, username, password, clientnonce, serversignature));
    TCP_SendTo(tempmessage);
}

void Bind()
{
    sprintf(tempmessage, "<iq type='set' id='bind_1'><bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'><resource>%s</resource></bind></iq>", resourse);
    TCP_SendTo(tempmessage);
}

void Start_PRESENT()
{
    sprintf(tempmessage, "<startpresent xmlns='http://jabber.org/protocol/startpresent'><method>present</method></startpresent>");
    TCP_SendTo(tempmessage);
}

void PRE_SendTo()
{
    strcpy(tempmessage, presentCBCencr(presmessage, key));
    TCP_SendTo(tempmessage);
}

void PRE_GetIn()
{
    
}

void PRE_presence()
{
    sprintf(presmessage, "<iq type='get' from='%s' to='pubsub.%s' id='feature1'><query xmlns='http://jabber.org/protocol/disco#items'/></iq>", fulljid, remotehost);
    PRE_SendTo();
}


static void XML_Parse(xmlNode * a_node)
{
    xmlNode *cur_node = NULL;
    for (cur_node = a_node; cur_node; cur_node = cur_node->next) 
    {
        if (cur_node->type == XML_ELEMENT_NODE) 
        {
            if((strcmp((char *)cur_node->name, "challenge") == 0))
            {
                xmlNode *cur_child_node = cur_node->children;
                if(xmlNodeIsText(cur_child_node))
                {
                    p_stanza.challenge = (char *)xmlNodeGetContent(cur_child_node);
                }
            }
            else if((strcmp((char *)cur_node->name, "success") == 0))
            {
                xmlNode *cur_child_node = cur_node->children;
                if(xmlNodeIsText(cur_child_node))
                {
                    p_stanza.serverSign = (char *)xmlNodeGetContent(cur_child_node);
                }
            }
        }

        XML_Parse(cur_node->children);
    }
}
