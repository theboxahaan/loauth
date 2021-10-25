import socket
from base64 import b64decode as bd
from base64 import b64encode as be
import string
import secrets

HOST = '127.0.0.1'
PORT = 5222

class SCRAM_SHA1:

	def __init__(self, user:str=None, pwd:str=None, client_nonce:str=None, server_nonce:str=None, seed:str=None, index:int=None, length:int=None):
		'''
		Ref - https://datatracker.ietf.org/doc/html/rfc5802#section-3
		'''
		self._gs2_header= 'n,,'
		self._a = ''	# N.S.
		self._n = user
		self._pwd = pwd
		self._m = None	# N.S.
		self._cr = client_nonce or self.ascii_nonce_generator(length)
		self._sr = server_nonce 
		self._i = index 
		self._s = seed


	def ascii_nonce_generator(self, length:int=None):
		'''
		Generates a string of printable ascii chars
		length := 5 is arbitrary...it needs to be rectified
		'''
		if length is None:
			length = 5
		printable_ascii = [i for i in string.printable[:-5] ]
		nonce = "".join([secrets.choice(printable_ascii) for _ in range(length)])
		nonce.replace(",", "0")
		return nonce

	def client_first_message_bare(self):
		return f"n={self._n},r={self._cr}"
	
	def client_first_message(self):
		return f"{self._gs2_header}{self.client_first_message_bare()}"

	def server_first_message(self):
		return f"r={self._sr},s={self._s},i={str(self._i)}"

	def client_final_message_without_proof(self):
		from base64 import b64encode as be 
		return f"c={be(self._gs2_header.encode('utf-8')).decode('utf-8')},r={self._sr}"

	def xor(self, a,b):
		import sys
		res = int.from_bytes(a, byteorder=sys.byteorder) ^ int.from_bytes(b, byteorder=sys.byteorder)
		return res.to_bytes(max(len(a), len(b)), byteorder=sys.byteorder)

	def generate_client_proof(self):
		import hashlib
		import hmac
		
		pwd_bytes = self._pwd.encode('utf-8')	#TODO normalise this
		salt_bytes = bd(self._s) 
		salted_password = hashlib.pbkdf2_hmac("sha1", pwd_bytes, salt_bytes, self._i)
		client_key = hmac.new(salted_password, b"Client Key", hashlib.sha1).digest()
		stored_key = hashlib.sha1(client_key).digest()
		auth_message = self.client_first_message_bare() + "," + self.server_first_message() + "," + self.client_final_message_without_proof()
		client_sig = hmac.new(stored_key, auth_message.encode('utf-8'), hashlib.sha1).digest()
		return self.xor(client_key,client_sig)
		
	def client_final_message(self):
		return f'{self.client_final_message_without_proof()},p={be(sasl.generate_client_proof()).decode()}'

if __name__ == '__main__':
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		s.connect((HOST,PORT))
		s.sendall(b'<?xml version="1.0"?><stream:stream to="localhost" xml:lang="en" version="1.0" xmlns="jabber:client" xmlns:stream="http://etherx.jabber.org/streams">')
		data = s.recv(2048)
		print(data)
		# disable encryption for now
		# and focus on getting SASL to work
		# s.sendall(b'<starttls xmlns="urn:ietf:params:xml:ns:xmpp-tls"/>')
		# print(s.recv(2048))

		# SASL Layer
		sasl = SCRAM_SHA1(user="admin", pwd="123")
		# send first message 
		client_first_msg = f'<auth mechanism="SCRAM-SHA-1" xmlns="urn:ietf:params:xml:ns:xmpp-sasl">{be(sasl.client_first_message().encode()).decode()}</auth>'
		s.sendall(client_first_msg.encode())
		# receive first server message
		import xml.etree.ElementTree as ET
		first_server_msg = s.recv(2048)
		resp_root = ET.fromstring(first_server_msg.decode())
		resp_dict = {k:v for (k,v) in [a.split('=') for a in bd(resp_root.text).decode().split(',')]}
		sasl._sr, sasl._s, sasl._i = resp_dict['r'], resp_dict['s'], int(resp_dict['i'])
		client_response = f'<response xmlns="urn:ietf:params:xml:ns:xmpp-sasl">{be(sasl.client_final_message().encode()).decode()}</response>'
		print(client_response)
		s.sendall(client_response.encode())
		
		# get server response
		print(s.recv(2048))
