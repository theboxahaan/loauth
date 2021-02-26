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
		self.connection.RegisterHandler('message', self.message_handler)
		self.connection.RegisterHandler('iq', self.iq_handler)
		

		# Tell the network we are online but don't ask for the roster
		self.connection.sendInitPresence(1)

		print("Connected.")
		return 0

	
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
		    'iq', attrib={'type': 'get', 'from': self.jid.jid, 'to': str(server)})
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
					# FIXME: Make this handle errors in a meaningful way
					# error = SubElement(reply, 'error')
					print("Error")
				elif stanza.attrib.get('type') == 'result':
					# This is run if the request has been successful
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



a=xmpp.JID("killua@localhost/xyz")

print(a.getNode(),a.getDomain(),a.getResource(),"hello",a.getStripped())
