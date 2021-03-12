from mypubsubclient import *
import time
import random

Round = 10
RecvLoad=0 

"""def load(text):
    RecvLoad = len(text)"""




def callback(reply):
    print("hello Sj")
    global RecvLoad
    #load(reply)
    print("************************************", len(reply), "\n")
    a=RecvLoad
    RecvLoad=a+len(etree.tostring(reply[0]))
    
    # print(reply[0].__dict__)
    for stanza in reply:
        print(etree.tostring(stanza),"\n")

#connect
client = PubSubClient("killua@localhost","hello")
client.connect()

# #Benchmark
start_time = time.time()

for i in range(Round):
    #client.get_nodes(client.server,None,callback)
    #client.connection.sendPresence()
    # client.connection.getRoster()
    client.get_nodes(client.server,None,callback)
    print(time.time()-start_time)
    start_time=time.time()


print("Received Data/Payload: ",RecvLoad)
print("Turn arround time of", Round, "is =  %s seconds.\n" % (time.time() - start_time))


""" for i in range(Round):
    rand = random.randint(1,4)

    if(rand == 1):
        client.get_nodes(client.server,None,callback)
    
    if(rand == 2):
        client.publish(client.server,"princely_musings","Prophecy","All is Well",callback)

    if(rand == 3):
        client.get_items_from_a_node(client.server, "princely_musings", callback)

    if(rand == 4):
        client.subscribe(client.server,"princely_musings",callback)
 """

#client = PubSubClient("satanu@616.pub","12345")
#client = PubSubClient("satanu@xmpp.jp","123456")
#client = PubSubClient("test@localhost","12345")

#client.connect()


#client.get_nodes(client.server,None,callback)

#client.create_node(client.server,None,callback)

##-- Taking second argument which is node id ,third argument which is title and fourth argument which is content as input 

#client.publish(client.server,"princely_musings","Prophecy","All is Well",callback)

##-- After Publishing to check the item published in node princely musing run python test.py

#client.get_items_from_a_node(client.server, "princely_musings", callback)

# client.delete_node(client.server,"e464ea8d-7237-4ece-8aac-2186561571f2",callback)

# client.delete_item_from_a_node(client.server,"princely_musings","e92c6ec3-f2b7-4046-87ad-0cba933146fa")

# client.subscribe(client.server,"princely_musings",callback)



