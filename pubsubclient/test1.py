from mypubsubclient import *
import time

def callback(reply):
    print("hello Sj")
    # print(reply[0].__dict__)
    for stanza in reply:
        print(etree.tostring(stanza),"\n")
    print(time.time()-start_time,time.time(),start_time,"Hello5")

client = PubSubClient("killua@localhost","hello")
client.connect()

start_time=time.time()
print(start_time)
client.get_nodes(start_time,client.server,None,callback)
import traceback
print(traceback.format_exc())
print(time.time(),start_time,time.time()-start_time)

# client.create_node(client.server,None,callback)

# Taking second argument which is node id ,third argument which is title and fourth argument which is content as input 

#client.publish(client.server,"princely_musings","Prophecy","All is Well",callback)

#After Publishing to check the item published in node princely musing run python test.py

# client.get_items_from_a_node(client.server, "princely_musings", callback)

# client.delete_node(client.server,"e464ea8d-7237-4ece-8aac-2186561571f2",callback)

# client.delete_item_from_a_node(client.server,"princely_musings","e92c6ec3-f2b7-4046-87ad-0cba933146fa")

# client.subscribe(client.server,"princely_musings",callback)



# HTTP 10 requests GET  / 1.89 secs
# XMPP 10 requests get_nodes 18 secs