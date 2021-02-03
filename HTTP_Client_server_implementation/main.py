import client.httpclient as httpclient
from urllib.parse import urlparse



def main():
    url=input("enter url with proper scheme like https://www.google.com/\n")
    parsedurl=urlparse(url)
    print(parsedurl)
    port=input("port")
    client=httpclient.Httpclient(parsedurl,int(port))
    resp=client.get()
    print(resp)
    client.post()

    


if __name__=="__main__":
    main()
