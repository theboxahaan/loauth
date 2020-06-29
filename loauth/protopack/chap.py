import hmac
import requests

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
	try:
		# make challenge request
		a = requests.get(client.endpoint, params = {'uuid': client.uuid, 'op': 'get_challenge'})
		# generate response
		print('Generating Response')
		res = client.sig_alg(client.secret.encode('utf-8'), a.text.encode('utf-8'))
		ack = requests.get(client.endpoint, params = {'op': 'verify', 'payload': res})
		print(ack.text)
		return ack
	except: 
		print("Authentication Failed")


def CHAP_butler():
	pass	



if __name__ == '__main__':
	c = Client(endpoint = 'http://localhost:4000',
				secret = 'ahaan',
				uuid = 'ahaan',
				sig_alg = HMAC('sha256'))	
	print('Client Created')			
	print(CHAP_Client(c))			
	# Make a New Cijfer 
	# Run Test
