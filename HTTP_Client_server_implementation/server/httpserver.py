from urllib import parse
import socket
import os
import sys
sys.path.append(os.path.abspath("/home/manas/Desktop/http"))
from socketfunc import *

import mimetypes




class Httpserver:

    statuscode = {
        200: "OK",
        404: "Not Found",
        501: "Not Implemented"
    }

    headers = "Server: MyServer\r\n"

    def responseline(self, status):
        rline = f"HTTP/1.1 {status} {self.statuscode[status]} \r\n"
        rline = rline.encode()
        return rline

    def start(self):
        host = "127.0.0.1"
        port = 8888
        s = CreateSocket()
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((host, port))
        s.listen(5)
        print("socket is listening for incoming connections at", s.getsockname)
        while True:
            con_s, add = s.accept()
            print("connected by", add)
            recieved = recserv(con_s)
            incom_req=recieved[1]
            print(incom_req)       
            resp = self.handleincom_req(incom_req, con_s,recieved[0])
            # print(resp)
            send_mess(resp, con_s)
            closeconn(con_s)

    def handleincom_req(self, req, sock, pra):
        if not req:
            # print("true")
            return req
        # parse request to know type of request
        parsedreq = self.parsereq(req)
        if parsedreq["Content-Length"] != -1:
            # print("hello",parsedreq["Content-Length"])      
            newlen=parsedreq["Content-Length"]-len(pra)
            if newlen != 0:
                payload = recpayload(newlen,sock,pra)
            else:
                payload=pra.decode()     
            # print(payload)
            parsedreq["reqb"] = payload

        try:
            handler = getattr(self, 'handle_%s' % parsedreq["method"])
        except AttributeError:
            handler = self.handle501()

        response = handler(parsedreq)

        # return string having complete response

        return response
# the below function will parse the incoming request and return a dictionary i have used partition because
# i know that all request headers are separated by colon also all request headers will lie between data[1:-2] as last two are blank and payload(post req) or blank and blank(get req)
    def parsereq(self, req):
        data = req.split("\r\n")
        mydic = {}
        requestline = data[0].split()

        mydic["method"] = requestline[0]
        mydic["path"] = requestline[1]
        mydic["httpv"] = requestline[2]
        if data[-1] != "":
            mydic["reqb"] = data[-1]

        mydic["Content-Length"] = -1

        data = data[1:-2]
        data1 = [x.partition(":") for x in data]

        for x in data1:
            if x[0] == "Content-Length":
                mydic[x[0]] = int(x[2])
            else:
                mydic[x[0]] = x[2]

        return mydic

    def handle_GET(self, req):
        filename = req["path"].strip("/")
        # print(req["path"])

        if not filename:
            filename = "index.html"

        if os.path.exists(filename) and not os.path.isdir(filename):
            rline = self.responseline(200)
            content_type = mimetypes.guess_type(filename)[0] or 'text/html'
            f = open(filename, "rb")
            resb = f.read()
            content_length=len(resb)
            header = self.headers+f"Content-Type: {content_type}\r\nContent-Length: {content_length}\r\n"
            header = header.encode()
            # resb=resb.decode()
        else:
            rline = self.responseline(404)
            header = self.headers+"Content-Type: text/html\r\n"
            header = header.encode()
            resb = b"<h1>Resource Not Found</h1>"
            resb = resb.encode()

        resp = rline+header+b"\r\n"+resb

        return resp

    def handle501(self):
        rline = self.responseline(501)
        header = self.headers+"Content-Type: text/html\r\n"
        resp = "<h1>Method Not Implemented</h1>"
        header = header.encode()
        resp = resp.encode()

        return rline+header+"\r\n"+resp

    def handle_POST(self, req):
         filename = req["path"].strip("/")
         if os.path.exists(filename) and not os.path.isdir(filename):
            rline = self.responseline(200)
            content_type = mimetypes.guess_type(filename)[0] or 'text/html'
            header = self.headers+f"Content-Type: {content_type}\r\n"
            header = header.encode()
            dump = parse.parse_qs(req["reqb"])

            dump = dict(dump)
            dump={k: v[0] for k, v in dump.items()}
            print(dump)
            output = ""
            for x in dump:
                output += x
                output = output+"="
                output = output+dump[x]
                output = output+"\r\n"

            f = open(filename, "w")
            f.write(output)
            rbody=b"<h1>request accepted and resource created</h1>"
         else:
            rline=self.responseline(404)
            header=self.headers+"Content-Type: text/html\r\n"
            header=header.encode()
            rbody=b"<h1>request denied as no such file exists</h1>"


         resp=rline+header+b"\r\n"+rbody
         return resp




if __name__=="__main__":
    server=Httpserver()
    server.start()








        









