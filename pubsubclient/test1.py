from mypubsubclient import *

def callback(reply):
    print("hello Sj")
    # print(reply[0].__dict__)
    for stanza in reply:
        print(etree.tostring(stanza),"\n")

client = PubSubClient("killua@localhost","hello")
client.connect()

# client.get_nodes(client.server,None,callback)

# client.create_node(client.server,None,callback)

# Taking second argument which is node id ,third argument which is title and fourth argument which is content as input 

#client.publish(client.server,"princely_musings","Prophecy","All is Well",callback)

#After Publishing to check the item published in node princely musing run python test.py




