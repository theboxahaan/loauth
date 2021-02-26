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

		# Then try to log in
		authorisation_result = self.connection.auth(self.user,self.password,self.resource)
		if not authorisation_result:
			print("Could not get authorized. Username/password problem?")
			return 1
		if authorisation_result != 'sasl':
			print("Warning: Could not use SASL")

		# Here we specify which methods to run to process messages and
		# queries
		# self.connection.RegisterHandler('message', self.message_handler)
		
		

		# Tell the network we are online but don't ask for the roster
		self.connection.sendInitPresence(1)

		self.connection.RegisterHandler('iq', self.iq_handler)

		print("Connected.")
		return 0

	def iq_handler(self, connection, iq):
		"""Looks at every incoming Jabber iq stanza and handles them."""
		# This creates an XML object out of the stanza, making it more
		# manageable
		print("hello12")
		# print(type(xmpp.simplexml.ustr(iq)),xmpp.simplexml.ustr(iq))
		stanza = ElementTree(file=StringIO(xmpp.simplexml.ustr(iq)))
		# Gets the top-level XML element of the stanza (the 'iq' one)
		stanza_root = stanza.getroot()

		print(etree.tostring(stanza),"\n",etree.tostring(stanza_root))

		# See if there is a stanza id and if so whether it is in the
		# dictionary of stanzas awaiting replies.
		print(self.pending.keys(),stanza_root.get("id"))
		if 'id' in stanza_root.attrib.keys() and stanza_root.get('id') in self.pending.keys():
			# This stanza must be a reply, therefore run the function
			# which is assigned to handle it
			print("hello12")
			self.pending[stanza_root.get('id')][0](
			    stanza_root, self.callbacks[stanza_root.get('id')][0])
			# These won't be run again, so collect the garbage
			del(self.pending[stanza_root.get('id')])
			del(self.callbacks[stanza_root.get('id')])

	
	
	def send(self, stanza, reply_handler=None, callback=None):
		"""Sends the given stanza through the connection, giving it a
		random stanza id if it doesn't have one or if the current one
		is not unique. Also assigns the optional functions
		'reply_handler' and 'callback' to handle replies to this stanza."""
		# Get the id of this stanza if it has one,
		# or else make a new random one
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
						reply.append(Node(name=item.get('node'), jid=item.get('jid'),
						             server=Server(name=stanza.get('from')), parent=node_parent))
				callback(reply)

		self.send(contents, handler, return_function)	








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

class Node(object):
	"""Pointer to a PubSub Node."""

	def __init__(self, server=None, name=None, jid=None, type=None, parent=None):
		self.set_server(server)
		self.set_name(name)
		self.set_jid(jid)
		self.set_type(type)
		self.set_parent(parent)

	def __str__(self):
		return self.name

	def set_server(self, server):
		"""Sets the server which this Node object points to (does NOT
		edit any actual nodes, only this pointer!)"""
		if type(server) == type("string"):
			self.server = Server(server)
		elif type(server) == type(Server()):
			self.server = server
		else:
			print ("Error: server must be a string or a Server.")

	def set_name(self, name):
		"""Sets the node name which this Node object points to (does NOT
		edit any actual nodes, only this pointer!)"""
		self.name = str(name)

	def set_jid(self, jid):
		self.jid = jid

	def set_type(self, type):
		"""Sets the type of this Node object. Does not edit the actual
		node."""
		self.type = type

	def set_parent(self, parent):
		"""Sets the parent collection node of this Node object. Does
		not edit the actual node."""
		self.parent = parent

	def get_sub_nodes(self, client, callback=None):
		"""Queries this node for its children. Passes a list of Nodes
		it finds to the return_function when a reply is received."""
		client.get_nodes(self.server, self, return_function=callback)

	def get_items(self, client, callback=None):
		"""TODO: Queries this node for the items it contains. Returns a list
		of the strings contained in the items."""
		client.get_items(self.server, self.name, return_function=callback)

	def get_information(self, client, callback=None):
		client.get_node_information(self.server, self, return_function=callback)

	def make_sub_node(self, client, name, type, callback=None):
		if self.type is "leaf":
			raise TypeError('Leaf nodes cannot contain child nodes')
		else:
			if self.type is None:
				print ("Warning: Node type is not known, yet child node requested. This will fail for leaf nodes.")
			if type == 'leaf':
				client.get_new_leaf_node()
			elif type == 'collection':
				client.get_new_collection_node()

	def request_all_affiliated_entities(self, client, return_function=None):
		client.request_all_affiliated_entities(self.server, self, return_function)

	def modify_affiliations(self, client, affiliation_dictionary, return_function=None):
		client.modify_affiliation(self.server, self, affiliation_dictionary, return_function)

	def publish(self, client, body, id=None, return_function=None):
		client.publish(self.server, self, body, id, None, return_function)

	def subscribe(self, client, jid, return_function=None):
		client.subscribe(self.server, self, jid, return_function)




a=xmpp.JID("killua@localhost/xyz")

print(a.getNode(),a.getDomain(),a.getResource(),"hello",a.getStripped())
