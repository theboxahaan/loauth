#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/socket.h>
#include<sys/types.h>

#include<netinet/in.h>
#include<arpa/inet.h>

#include <unistd.h>

#include<time.h>

/* For coloring output */
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KWHT  "\x1B[37m"
#define KMAG  "\x1B[35m"

clock_t start, end;

int BENCHMARK_ROUND;
int PAYLOAD_SIZE;

#define BUFFERSIZE 4096
char request[BUFFERSIZE];

char postresponse[BUFFERSIZE];
char serveraddress[30];
char* mode;
char* Flag;   // G: GET | P:POST

int sendpayload;
int recvpayload;

void TCP_Recv(int);
void TCP_Send(int);
void GET();
void POST();
char *String_Gen(int);

struct http_response_parse //for storing the values of received msg.
{
    char* version;
    char* status_id;
    char* status;
    char* day;
    char* month;
    char* date;
    char* time;
    char* year;
    char* server;
    char* cache_control;
    char* content_type;
    char* content_length;
    char* content_language;
    char* connection;
    char* payload;
} http_response;

void HTTP_Parser(char req[]) //parsing received http request/response.
{
    char* line[30];
    char* token[100]; 
    char* load; 
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
        char* sent_token[2048];
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
    
    http_response.version = token[0];
    http_response.status_id = token[1];
    http_response.status = token[2];
    http_response.day = token[4];
    http_response.month = token[5];
    http_response.date = token[6];
    http_response.time = token[7];
    http_response.year = token[8];
    http_response.server = token[10];
    http_response.cache_control = token[12];
    http_response.content_type = token[14];

    http_response.content_length = token[17];
    http_response.content_language = token[19];
    http_response.connection = token[21];
    http_response.payload = token[22];

    // printf("[\n%s\n%s\n%s\n%s\n]", http_response.version, http_response.status_id, http_response.status, http_response.day);
}

int main(int argc, char *argv[])
{	
	//get the address.
	char *address;
	// address = argv[1];
    // address = "127.0.0.1";
    address = "172.18.0.2";
	
	//get port no and convert into integer.
	int port = atoi(argv[1]);

    mode = argv[2];

    if(argc == 5)
    {
        BENCHMARK_ROUND = atoi(argv[3]);
        PAYLOAD_SIZE = atoi(argv[4]);
    }
    
    if(argc == 4)
    {
        BENCHMARK_ROUND = atoi(argv[3]);
        PAYLOAD_SIZE = 1;
    }

    if(argc == 3)
    {
        BENCHMARK_ROUND = 1;
        PAYLOAD_SIZE = 1;
    }
	
	//create client side socket.
	int clientsocket;
	clientsocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientsocket < 0) // if can not create.
	{
		printf("%sError!%sSocket Createion Error!\n", KWHT, KRED);
		return -1;
	}

	//specify server address.
	struct sockaddr_in serveradd;
	serveradd.sin_family = AF_INET;
	serveradd.sin_port = htons(port);  //htons: convert the port to correct formation.
	if(inet_aton(address, &serveradd.sin_addr) < 0) //for format in IP address.
	{
		printf("%sError!%sInvalid IP Address!\n", KWHT, KRED);
		return -1;
	}
	serveradd.sin_addr.s_addr = inet_addr(address);

	//connect the client side socket to server.
	if(connect(clientsocket, (struct sockaddr*) &serveradd, sizeof(serveradd)) < 0)
	{
		printf("%sError!%sConnection Failed!\n", KWHT, KRED);
		return -1;
	}

    //store the server address and port in sending format.
    sprintf(serveraddress, "%s:%d", address, port);
    printf("\n%sinfo: %sServer Address: %s\n", KWHT, KGRN, serveraddress);

    sendpayload=0;
    recvpayload=0;
    BENCHMARK_ROUND = 2 * BENCHMARK_ROUND; // For GET + POST combine.

    start = clock();

    if(strcmp(mode, "L") == 0)
        TCP_Recv(clientsocket);
    
    if(strcmp(mode, "S") == 0)
    {
        GET(clientsocket);
    }

	//close the socket.
	close(clientsocket);
	
	return 0;
}

void TCP_Recv(int clientsocket)
{
    strncpy(postresponse, "\0", sizeof(postresponse));
    if(read(clientsocket, &postresponse, sizeof(postresponse)) > 0)
    {
        recvpayload = recvpayload + strlen(postresponse);

        printf("%sinfo: %sResponse came back from server.\n", KWHT, KGRN);
        printf("%sinfo: %sServer Responce:\n%s%s\n\n", KWHT, KGRN, KMAG, postresponse);

        BENCHMARK_ROUND = BENCHMARK_ROUND - 1;

        HTTP_Parser(postresponse);

        if((BENCHMARK_ROUND < 1) && (strcmp(mode, "S") == 0))
        {
            end = clock();
            printf("%sComplete!\nTurnaround Time: %f Sec.\n", KWHT, (((double) (end - start)) / CLOCKS_PER_SEC));
            printf("Send Payload= %d Byte\nReceived Payload= %d Byte\n", sendpayload, recvpayload);
            close(clientsocket);
            exit(1);
        }
        else
        {
            if(strcmp(mode, "L") == 0)
                TCP_Recv(clientsocket);
            
            if(strcmp(mode, "S") == 0)
            {
                if(strcmp(http_response.content_type, "text/html;") == 0)
                {
                    POST(clientsocket);
                }
                if(strcmp(http_response.content_type, "text/plain;") == 0)
                {
                    GET(clientsocket);
                }
            }
        }
    }
    else
    {
        end = clock();
        printf("%sComplete!\nTurnaround Time: %f Sec.\n", KWHT, (((double) (end - start)) / CLOCKS_PER_SEC));
        printf("Send Payload= %d Byte\nReceived Payload= %d Byte\n", sendpayload, recvpayload);
        close(clientsocket);
        exit(1);
    }
}

void TCP_Send(int clientsocket)
{
    printf("%sinfo: %sClient Request:\n%s%s\n", KWHT, KGRN, KYEL, request);

    //send the the POST request.
    int bytessent, totalbytessent, bytestosent; //for continous sending in loop
    bytestosent = strlen(request);
    sendpayload = sendpayload + bytestosent;
    totalbytessent = 0;
    while (1) 
    {
    	// sleep(1);
        bytessent = send(clientsocket, request, sizeof(request), 0);
        totalbytessent += bytessent;
        if (totalbytessent >= bytestosent) 
        {
            break;
        }
    }

    TCP_Recv(clientsocket);
}

void GET(int clientsocket)
{
    strncpy(request, "\0", sizeof(request));
    sprintf(request, "GET / HTTP/1.1\r\nHost: %s\r\n", serveraddress);
    printf("%sinfo: %sGET Request Send to the server.\n", KWHT, KGRN);
    TCP_Send(clientsocket);
}

void POST(int clientscoket)
{
    strncpy(request, "\0", sizeof(request));
    char *randomString = String_Gen(PAYLOAD_SIZE);
    int payloadsize = strlen(randomString);

    sprintf(request, "POST / HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s\r\n", serveraddress, payloadsize, randomString);
    printf("%sinfo: %sPOST Request Send to The Server.\n", KWHT, KGRN);
    free(randomString);

    TCP_Send(clientscoket);
}

char *String_Gen(int length) 
{ 
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!"; // could be const
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

