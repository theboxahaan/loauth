import socket

# Creating AF_INET Sockets

def CreateSocket():
    s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print("socket created.")

    return s

# Connect to address client

def client_connect(address,s):
    print(address)
    #address is a tuple (host,port)
    s.connect(address)
    print("socket connected to the mentioned address")


# send message via connected socket

def send_mess(mess,s):
    # mess=mess.encode("UTF-8")
    s.sendall(mess)
    # s.close()
    print("message sent to the connected socket")

# recieve a message from connected socket returns recieved message as a string

def recieve_mess(s):
    data=b""
    while True:
        buf=s.recv(1024)      
        if buf == b"":
            break
        data+=buf
    print("message recieved from the connected socket")

    try:
        return data.decode("utf-8")
    except UnicodeDecodeError:
        print("error")
    
    return data.decode("cp1252")

   

#close socket connection
def closeconn(s):
    s.close()
    print("connection closed")

#this function recserv is made for recieving client request irrespective of get or post,it returns a tuple
#the first member is the payload that might have been read by the buffer if it were a post request otherwise if is 
#is a get request then pra will be empty and the other tuple member is the request without payload ending with \r\n\r\n
#this function breaks(to stop recv blocking)when we encounter \r\n\r\n in the buffer at any time

def recserv(s):
    data=b""
    while True:
        pra=b""
        buf=s.recv(1024) 
        if buf.find(b"\r\n\r\n") != -1:
            temp=buf.find(b"\r\n\r\n")
            #pra is payload ready already
            pra=buf[temp+4:]
            # print(pra)
            buf=buf[:temp+4]
            data+=buf
            break     
        if buf == b"":
            break
        data+=buf
    print("message recieved from the connected socket")

    try:
        return (pra,data.decode("utf-8"))
    except UnicodeDecodeError:
        print("error")
    
    return (pra,data.decode("cp1252"))

#this function below is used to catch the payload that was remaining in the above function recserv all the payload
# might not have been captured so to catch the remaining we require this function and it breaks when data length equals payload remaining.


def recpayload(length,s,pra):
    data=b""
    while True:
        buf=s.recv(1024)
        if len(buf) == length:
            data=data+buf
            break
        # print(buf)
        if buf == b"":
            break
        data=data+buf
        if len(data)==length:
            break
    
    data=data+pra 
    try:
        return data.decode("utf-8")
    except UnicodeDecodeError:
        print("error")
    
    return data.decode("cp1252")
    











