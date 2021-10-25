import socket
from base64 import b64decode as bd
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
		self._cr = client_nonce or ascii_nonce_generator(length)
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
		nonce = "".join([secrets.choice() for _ in range(length)])
		nonce.replace(",", "0")
		return nonce

	def client_first_message_bare(self):
		return f"n={self._n},r={self._cr}"

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

		s.sendall(b'<auth mechanism="SCRAM-SHA-1" xmlns="urn:ietf:params:xml:ns:xmpp-sasl">biwsbj1hZG1pbixyPUM0OTUzODUwREQ1QTM2M0RDMkVENEM1OENENTY4NkY=</auth>')
		print(s.recv(2048))
