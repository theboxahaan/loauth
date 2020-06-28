import requests
import hmac

from loauth.cijfer import HMAC
from loauth.protopack.protocol import Protocol
from loauth.client import Client 

class CHAP_client(Client):
	def __init__(self):
		pass


class CHAP(Protocol):
	def trigger(self, endpoint):
		ch = request.get(endpoint)
		return ch
	
	def response(self, ch, key):
		resp = hmac(ch, key, 'sha256') 
		
		
		
