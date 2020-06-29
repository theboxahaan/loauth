import hmac
import requests
from base64 import urlsafe_b64encode as be
from base64 import urlsafe_b64decode as bd
from loauth.cijfer import HMAC
from loauth.client import Client 
from loauth.cijfer import Cijfer



def CHAP_Client(client):
	'''Function To execute client side of the CHAP protocol
	Parameters: <loauth.client Object> with class attributes
		- endpoint <string>
		- secret <string>
		- sig_alg <loauth.cijfer.Cijfer Object>
		- uuid <string>
	Return Value: <requests.get() Object> Response object containing Auth Status	
	'''

	if not isinstance(client, Client):
		raise TypeError
	# make challenge request
	a = requests.get(client.endpoint, params = {'uuid': client.uuid, 'op': 'get_challenge'})
	print(a.text)
	# generate response
	print('Generating Response')
	res = client.sig_alg.sign(client.secret.encode('utf-8'), a.text.encode('utf-8'))
	ack = requests.get(client.endpoint, params = { 'uuid': client.uuid, 'op': 'verify', 'payload': be(res)})
	#print(ack.text)
	#return ack



if __name__ == '__main__':
	c = Client(endpoint = 'http://localhost:4000',
				secret = 'secret',
				uuid = 'ahaan',
				sig_alg = HMAC('sha256'))	
	print('Client Created')			
	print(CHAP_Client(c))			
	# Make a New Cijfer 
	# Run Test
