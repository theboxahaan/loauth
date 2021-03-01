import socket
from xmpp import *
from lxml.etree import ElementTree, Element, SubElement
import lxml.etree as etree
from io import StringIO
import string
from random import Random








# # import xmpp

# # username = 'jutsu@localhost'
# # passwd = '123'
# # to='pubsub.localhost'
# # msg=''


# # client = xmpp.Client('localhost',5222)
# # client.connect(server=('localhost',5222))
# # client.auth(username, passwd)
# # client.sendInitPresence()
# # message = xmpp.Message(to, msg)
# # message.setAttr('type', 'chat')
# # client.send(message)
# # print(client.getRoster())



# START
 
user="killua@localhost"
password="hello"
server="localhost"
 
jid = JID(user) 
connection = Client(server) 
connection.connect()

#END



# print(connection.connected,"\n",connection.debug_flags,"\n",connection.disconnect_handlers,"\n",connection.defaultNamespace,"\n",connection.Namespace)

#start
result = connection.auth(jid.getNode(),password) 
#end



# print(result)
# print(connection._owner,"\n",connection.__dict__,"\n",connection.__module__)
# print(connection.send())

#start 
connection.sendInitPresence()
#end



#start
def iq_handler(connection,iq):
		"""Looks at every incoming Jabber iq stanza and handles them."""
		# This creates an XML object out of the stanza, making it more
		# manageable
		print(type(iq))
		# stanza = ElementTree(file=StringIO(iq))
		# # Gets the top-level XML element of the stanza (the 'iq' one)
		# stanza_root = stanza.getroot()
		# print(etree.tostring(stanza))
		# print(a,connection,iq)

		# See if there is a stanza id and if so whether it is in the
		# dictionary of stanzas awaiting replies.
		# if 'id' in stanza_root.attrib.keys() and stanza_root.get('id') in self.pending.keys():
		# 	# This stanza must be a reply, therefore run the function
		# 	# which is assigned to handle it
		# 	self.pending[stanza_root.get('id')][0](
		# 	    stanza_root, self.callbacks[stanza_root.get('id')][0])
		# 	# These won't be run again, so collect the garbage
		# 	del(self.pending[stanza_root.get('id')])
		# 	del(self.callbacks[stanza_root.get('id')])
		print("hello1")

        
 

connection.RegisterHandler("iq",iq_handler) 

#end

#start

stanza = Element('iq', attrib={'type': 'get','from': user, 'to': "pubsub."+str(server),'id':'node1'})
stanza.append(Element('query', attrib={'xmlns': 'http://jabber.org/protocol/disco#info'}))
print(stanza.attrib.get("type"))
a=simplexml.XML2Node(etree.tostring(stanza))
print(a,"hello")

print(a.getAttrs())
print(a.getName())

#end

# print(etree.tostring(stanza),"\n")
# Iq.__init__(typ=None, queryNS=None, attrs={}, to=None, frm=None, payload=[], xmlns='jabber:client', node=None)
# node=Node('http://jabber.org/protocol/pubsub pubsub', attrs={},payload=[Node('items',attrs={"node":"princely_musings"},payload=[])])
# print(node)


#start
iq=Iq('get',"http://jabber.org/protocol/pubsub",attrs={"id":"node2"},to="pubsub.localhost",frm="killua@localhost/manas-Inspiron-7570",payload=[Node('items',attrs={"node":"princely_musings"},payload=[])],xmlns=None)
iq.setQuery("pubsub")
print(iq)
#end


# print(find('.//{http://jabber.org/protocol/disco#info}query'))



# node=Node('http://jabber.org/protocol/pubsub pubsub', attrs={},payload=[Node('items',attrs={"node":"princely_musings"},payload=[])])
# print(node)
# print(connection.__dict__)

#start
# connection.send(simplexml.XML2Node(etree.tostring(stanza)))
connection.send(iq)
#end

if connection.Process(1):
    pass

# connection.send(simplexml.XML2Node(etree.tostring(stanza)))
# # # print(connection.Process())
connection.WaitForResponse("node1")
# print(connection.Process())






    







# <iq type='get'
#     from='killua@localhost'
#     to='pubsub.localhost'
#     id='items1'>
#   <pubsub xmlns='http://jabber.org/protocol/pubsub'>
#     <items node='princely_musings'/>
#   </pubsub>
# </iq>

# import sys
# import logging
# import getpass
# from optparse import OptionParser

# import sleekxmpp

#616.pubsub