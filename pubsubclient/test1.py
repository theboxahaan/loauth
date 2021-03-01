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

# client.get_items_from_a_node(client.server, "princely_musings", callback)

# client.delete_node(client.server,"e464ea8d-7237-4ece-8aac-2186561571f2",callback)

# client.delete_item_from_a_node(client.server,"princely_musings","e92c6ec3-f2b7-4046-87ad-0cba933146fa")

# client.subscribe(client.server,"princely_musings",callback)



