from urllib.parse import urlparse
import socket
from socketfunc import *
import ssl



class Httpclient:
    def __init__(self,url,port):
        self.url=url
        self.port=port
    
    def get(self):
        s=CreateSocket()
        if self.url.scheme=="https":
            context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
            s=context.wrap_socket(s, server_hostname=self.url.netloc)
            self.port=443
        client_connect((self.url.netloc,self.port),s)
        if not self.url.query:
            mess=f"GET {self.url.path} HTTP/1.1\r\nHost: {self.url.netloc}:{self.port}\r\nConnection: close\r\n\r\n"
        else: 
            mess=f"GET {self.url.path}?{self.url.query} HTTP/1.1\r\nHost: {self.url.netloc}:{self.port}\r\nConnection: close\r\n\r\n"


        mess=mess.encode()
        send_mess(mess,s)
        # print("hello")
        response=recieve_mess(s)
        return response

    def post(self):

        mess_body="user_name=asdas&user_email=asdasd%40gmail.com&user_message=adsasda"
        # print(mess_body)
        mess_head=f"POST /dump.txt HTTP/1.1\r\nHost: 127.0.0.1:8888\r\nConnection: close\r\nContent-Length: {len(mess_body)}\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n"
        mess=mess_head+mess_body
        s=CreateSocket()
        client_connect(("127.0.0.1",8888),s)
        mess=mess.encode()
        send_mess(mess,s)
        resp=recieve_mess(s)
        closeconn(s)
        print(resp)


    

