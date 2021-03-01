import xmpp
import string
from random import Random
from io import StringIO
from lxml.etree import ElementTree, Element, SubElement
import lxml.etree as etree
from xmpp.simplexml import *



class PubSubClient(object):


	def __init__(self, jid, password, resource='subscriptions'):
		"""Creates a new PubSubClient. jid can be a string or a JID,
		password and resource are strings."""
		self.password = password
		self.jid=JID(jid)
		self.resource=self.jid.resource
		self.user = self.jid.node
		self.server = self.jid.domain
		self.connection = xmpp.Client(self.server)
		# This stores the stanza ids used for this session, since ids
		# must be unique for their session
		self.callbacks = {}
		self.pending = {}		
		self.used_ids=[]

	def connect(self):
		"""Turn on the connection. Returns 1 for error, 0 for success."""
		connection_result = self.connection.connect()
		if not connection_result:
			print("Could not connect to server " + str(self.server))
			return 1
		if connection_result != 'tls':
			print("Warning: Could not use TLS")

		authorisation_result = self.connection.auth(self.user,self.password,self.resource)
		if not authorisation_result:
			print("Could not get authorized. Username/password problem?")
			return 1
		if authorisation_result != 'sasl':
			print("Warning: Could not use SASL")

		self.connection.sendInitPresence(1)

		self.connection.RegisterHandler('iq', self.iq_handler)

		print("Connected.")
		return 0

	def iq_handler(self, connection, iq):
		"""Looks at every incoming Jabber iq stanza and handles them."""
		stanza = ElementTree(file=StringIO(xmpp.simplexml.ustr(iq)))
		stanza_root = stanza.getroot()

		print(etree.tostring(stanza),"\n",etree.tostring(stanza_root))
		print(self.pending.keys(),stanza_root.get("id"))
		if 'id' in stanza_root.attrib.keys() and stanza_root.get('id') in self.pending.keys():
			self.pending[stanza_root.get('id')][0](
			    stanza_root, self.callbacks[stanza_root.get('id')][0])
			del(self.pending[stanza_root.get('id')])
			del(self.callbacks[stanza_root.get('id')])

	
	
	def send(self, stanza, reply_handler=None, callback=None):
		"""Sends the given stanza through the connection, giving it a
		random stanza id if it doesn't have one or if the current one
		is not unique. Also assigns the optional functions
		'reply_handler' and 'callback' to handle replies to this stanza."""
		if 'id' in stanza.attrib.keys() and stanza.get('id') not in self.used_ids:
			id = stanza.get('id')
		else:
			# Make a random ID which is not already used
			while True:
				id = ''.join(Random().sample(string.digits+string.ascii_letters, 8))
				if id not in self.used_ids: break
			stanza.set('id', id)
		self.used_ids.append(id)
		print(reply_handler,callback)
		self.pending[id] = [reply_handler]
		self.callbacks[id] = [callback]
		# self.connection.send(xmpp.simplexml.XML2Node(etree.tostring(stanza)))
		# self.connection.WaitForResponse(id)
		# while self.connection.Process(1): 
		# 	pass
		self.connection.SendAndWaitForResponse(xmpp.simplexml.XML2Node(etree.tostring(stanza)),2)
		# self.connection.SendAndWaitForResponse(xmpp.simplexml.XML2Node(etree.tostring(stanza)),2)

		
		# print(self.connection.Process())
		
		


	#Retrieve all top level nodes
	def get_nodes(self, server, node, return_function=None, stanza_id=None):
		"""Queries server (string or Server) for the top-level nodes it
		contains. If node is a string or Node then its child nodes are
		requested instead.
		Upon reply, return_function is called with a list of Nodes which
		were returned."""
		# This is the kind of XML we want to send
		# <iq type='get' from='us' to='them'>
		#  <query xmlns='http://jabber.org/protocol/disco#items'/>
		# </iq>

		# Make it as XML elements
		contents = Element(
		    'iq', attrib={'type': 'get', 'from': self.jid.jid, 'to': "pubsub."+str(server)})
		query = SubElement(contents, 'query', attrib={
		                   'xmlns': 'http://jabber.org/protocol/disco#items'})
		if node is not None:
			query.set('node', node.name)

		# This is run on any replies that are received (identified by
		# their stanza id)
		def handler(stanza, callback):
			# <iq type='result'
			#    from='pubsub.shakespeare.lit'
			#    to='francisco@denmark.lit/barracks'
			#    id='nodes2'>
			#  <query xmlns='http://jabber.org/protocol/disco#items'
			#         node='blogs'>
			#    <item jid='pubsub.shakespeare.lit'
			#          node='princely_musings'/>
			#    <item jid='pubsub.shakespeare.lit'
			#          node='kingly_ravings'/>
			#    <item jid='pubsub.shakespeare.lit'
			#          node='starcrossed_stories'/>
			#    <item jid='pubsub.shakespeare.lit'
			#          node='moorish_meanderings'/>
			#  </query>
			# </iq>
			if callback is not None:
				# reply = Element('reply')
				reply = []
				if stanza.attrib.get('type') == 'error':
					print("Error")
				elif stanza.attrib.get('type') == 'result':
					if stanza.find('.//{http://jabber.org/protocol/disco#items}query').get('node') is not None:
						node_parent = Node(name=stanza.find(
						    './/{http://jabber.org/protocol/disco#items}query').get('node'), server=Server(name=stanza.get('from')))
					else:
						node_parent = Server(name=stanza.get('from'))
					# Go through all of the 'item' elements in the stanza
					for item in stanza.findall('.//{http://jabber.org/protocol/disco#items}item'):
						# reply.append(Node(name=item.get('node'), jid=item.get('jid'),
						            #  server=Server(name=stanza.get('from')), parent=node_parent))
						reply.append(item)
				callback(reply)

		self.send(contents, handler, return_function)	


	def create_node(self, server, node, return_function=None, stanza_id=None):

		contents = Element(
		    'iq', attrib={'type': 'set', 'from': self.jid.jid, 'to': "pubsub."+str(server)})
		q1 = SubElement(contents, 'pubsub', attrib={
		                   'xmlns': 'http://jabber.org/protocol/pubsub'})

		q2 = SubElement(q1,'create')

		def handler(stanza, callback):

			if callback is not None:
				reply=[]
				reply.append(stanza)

				callback(reply)

		self.send(contents, handler, return_function)	


	
	def publish(self, server, node, title, content, return_function=None, stanza_id=None):

		entry=Element("entry")
		title1=SubElement(entry,"title")
		title1.text = title
		content1 = SubElement(entry,"content")
		content1.text = content
		print(etree.tostring(entry))
		
		stanza = Element('iq', attrib={'type':'set', 'from':self.jid.jid, 'to':"pubsub."+str(server)})
		pubsub = SubElement(stanza, 'pubsub', attrib={'xmlns':'http://jabber.org/protocol/pubsub'})
		publish = SubElement(pubsub, 'publish', attrib={'node':str(node)})
		item = SubElement(publish, 'item')		
		item.append(entry)


		def handler(stanza, callback):
			# <iq type='result'
			#    from='pubsub.shakespeare.lit'
			#    to='hamlet@denmark.lit/blogbot'
			#    id='publish1'>
			#  <pubsub xmlns='http://jabber.org/protocol/pubsub'>
			#    <publish node='princely_musings'>
			#      <item id='ae890ac52d0df67ed7cfdf51b644e901'/>
			#    </publish>
			#  </pubsub>

			# print (etree.tostring(stanza))
			if callback is not None:
				reply=[]
				if stanza.get("type") == "result":
					reply.append(stanza)
					callback(reply)
				else:
					print("error in publishing")

		self.send(stanza, handler, return_function)


	def get_items_from_a_node(self, server, node, return_function=None, stanza_id=None):

		stanza = Element("iq",attrib={"type":"get","from":self.jid.jid,"to":"pubsub."+str(server)})
		pubsub = SubElement(stanza,"pubsub",attrib={"xmlns":"http://jabber.org/protocol/pubsub"})
		items = SubElement(pubsub,"items",attrib={"node":str(node)})

		def handler(stanza, callback):
			if callback is not None:
				reply=[]
				if stanza.get("type") == "result":
					for item in stanza.findall('.//item'):
						reply.append(item)
						callback(reply)
				else:
					print("error in retreiving items from a node")

		self.send(stanza, handler, return_function)

	def delete_node(self, server, node, return_function=None, stanza_id=None):

		stanza = Element("iq",attrib={"type":"set","to":"pubsub."+str(server),"from":self.jid.jid})
		pubsub = SubElement(stanza,"pubsub",attrib={"xmlns":"http://jabber.org/protocol/pubsub#owner"})
		delete = SubElement(pubsub,"delete",attrib={"node":str(node)})

		def handler(stanza, callback):
			if callback is not None:
				reply=[]
				if stanza.get("type") == "result":
					reply.append(stanza)
					callback(reply)
				else:
					print("error in deleting node")

		self.send(stanza, handler, return_function)


	def delete_item_from_a_node(self,server,node,item_id,return_function=None,stanza_id=None):

		stanza = Element("iq",attrib={"type":"set","from":self.jid.jid,"to":"pubsub."+str(server)})
		pubsub = SubElement(stanza,"pubsub",attrib={"xmlns":"http://jabber.org/protocol/pubsub"})
		retract = SubElement(pubsub,"retract",attrib={"node":str(node)})
		item = SubElement(retract,"item",attrib={"id":str(item_id)})

		def handler(stanza, callback):
			if callback is not None:
				reply=[]
				if stanza.get("type") == "result":
					reply.append(stanza)
					callback(reply)
				else:
					print("error in deleting node")

		self.send(stanza, handler, return_function)

	
	def subscribe(self,server,node,return_function=None,stanza_id=None):

		stanza = Element("iq",attrib={"type":"set","from":self.jid.jid,"to":"pubsub."+str(server)})
		pubsub = SubElement(stanza,"pubsub",attrib={"xmlns":"http://jabber.org/protocol/pubsub"})
		subscribe = SubElement(pubsub,"subscribe",attrib={"node":str(node),"jid":str(self.jid.jid)})

		def handler(stanza, callback):
			if callback is not None:
				reply=[]
				if stanza.get("type") == "result":
					reply.append(stanza)
					callback(reply)
				else:
					print("error in deleting node")

		self.send(stanza, handler, return_function)


class JID:

	def __init__(self,jid="none@none"):
		jsplit=jid.partition("@")
		self.node=jsplit[0]
		self.jid=jid

		if jsplit[2].find("/"):
			self.domain=jsplit[2].partition("/")[0]
			self.resource=jsplit[2].partition("/")[2]
		else:
			self.domain=jsplit[2]
			self.resource="subscriptions"

class Server(object):

	def __init__(self, name=None):
		if name is not None:
			self.set_name(name)

	def set_name(self, name):
		self.name = name

	def __str__(self):
		return self.name

	def add_node(self, client, name, callback=None):
		client.request_node(self, name, None, None, return_function=callback)
