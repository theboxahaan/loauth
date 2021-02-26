from mypubsubclient import *

def callback(reply):
    print("hello Sj")
    print(reply[0].__dict__)

client = PubSubClient("killua@localhost","hello")
client.connect()
client.get_nodes(client.server,None,callback)





