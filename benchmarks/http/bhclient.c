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

int BENCHMARK_ROUND=1000;
int PAYLOAD_SIZE;

#define BUFFERSIZE 2048
char request[BUFFERSIZE];
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

int main(int argc, char *argv[])
{	
	//get the address.
	char *address;
	// address = argv[1];
    address = "127.0.0.1";
	
	//get port no and convert into integer.
	int port = atoi(argv[1]);

    mode = argv[2];
	
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
    printf("\n%sinfo: %sServer Address: %s", KWHT, KGRN, serveraddress);

    sendpayload=0;
    recvpayload=0;
    BENCHMARK_ROUND = 2 * BENCHMARK_ROUND; // For GET + POST combine.

    start = clock();

    if(strcmp(mode, "L") == 0)
        TCP_Recv(clientsocket);
    
    if(strcmp(mode, "S") == 0)
    {
        Flag = "P";
        PAYLOAD_SIZE = atoi(argv[3]);
        GET(clientsocket);
    }

	//close the socket.
	close(clientsocket);
	
	return 0;
}

void TCP_Recv(int clientsocket)
{
    char postresponse[BUFFERSIZE];

    strncpy(postresponse, "\0", sizeof(postresponse));
    recv(clientsocket, &postresponse, sizeof(postresponse), 0);
    printf("%sinfo: %s Response came back from server.\n", KWHT, KGRN);
    printf("%sinfo: %sServer Responce:\n%s%s\n\n", KWHT, KGRN, KMAG, postresponse);

    recvpayload = recvpayload + strlen(postresponse);
    BENCHMARK_ROUND = BENCHMARK_ROUND - 1;

    if(BENCHMARK_ROUND < 1)
    {
        end = clock();
        printf("%sComplete!\nTurnaround Time: %f Sec.\n", KWHT, (((double) (end - start)) / CLOCKS_PER_SEC));
        printf("Send Payload= %d\nReceived Payload= %d\n", sendpayload, recvpayload);
    }
    else
    {
        if(strcmp(mode, "L") == 0)
            TCP_Recv(clientsocket);
        
        if(strcmp(mode, "S") == 0)
        {
            if(strcmp(Flag, "P") == 0)
            {
                Flag = "G";
                POST(clientsocket);
            }
            else
            {
                Flag = "P";
                GET(clientsocket);
            }
        }
    }
}

void TCP_Send(int clientsocket)
{
    printf("%sinfo: %sClient Request:\n%s%s\n", KWHT, KGRN, KYEL, request);

    //send the the POST request.
    int bytessent, totalbytessent, bytestosent; //for continous sending in loop
    bytestosent = strlen(request);
    totalbytessent = 0;
    while (1) 
    {
        bytessent = send(clientsocket, request, sizeof(request), 0);
        totalbytessent += bytessent;
        if (totalbytessent >= bytestosent) 
        {
            sendpayload = sendpayload + totalbytessent;
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
    char *payload = String_Gen(PAYLOAD_SIZE);
    // char *payload = "satanu";
    int payloadsize = strlen(payload);

    sprintf(request, "POST / HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s\r\n", serveraddress, payloadsize, payload);
    printf("%sinfo: %sPOST Request Send to The Server.\n", KWHT, KGRN);

    TCP_Send(clientscoket);
}

char *String_Gen(int length) 
{ 
    // const size_t length, supra
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!"; // could be const

    char *randomString = NULL;

    if (length) 
    {
        randomString = malloc(length +1); // sizeof(char) == 1, cf. C99

        if (randomString) 
        {
            int l = (int) (sizeof(charset) -1); // (static/global, could be const or #define SZ, would be even better)
            int key;  // one-time instantiation (static/global would be even better)
            for (int n = 0;n < length;n++) 
            {        
                key = rand() % l;   // no instantiation, just assignment, no overhead from sizeof
                randomString[n] = charset[key];
            }

            randomString[length] = '\0';
        }
    }
	else *randomString = '\0';
    return randomString;
}

