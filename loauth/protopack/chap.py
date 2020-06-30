import hmac
import requests
from base64 import urlsafe_b64encode as be
from base64 import urlsafe_b64decode as bd
from loauth.cijfer import HMAC
from loauth.client import Client 
from loauth.cijfer import Cijfer
import random

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
	
	print("CHALLENGE: ", a.text)
	# generate response
	res = client.sig_alg.sign(client.secret.encode('utf-8'), a.text.encode('utf-8'))
	ack = requests.get(client.endpoint, params = { 'uuid': client.uuid, 'op': 'verify', 'challenge': a.text, 'payload': be(res)})
	print("ACK: ",ack.text)
	return ack

def CHAP_Butler(query_args):
	#currently only HMAC
	op = query_args['op'][0]
	if op  == 'get_challenge':
		return str(random.randint(0,99999999))
	if op ==  'verify':
		check = query_args['payload'][0]
		tmp = be(HMAC('sha256').sign( b'secret', bytes(query_args['challenge'][0], 'utf-8')))
		print("CHECK:", tmp, check)
		if tmp.decode('utf-8') == check:
			return 'Authenticated'
		else:
			return 'NA'
	else:		
		return 'NULLBABA'
		


if __name__ == '__main__':
	c = Client(endpoint = 'http://localhost:4000',
				secret = 'secret',
				uuid = 'ahaan',
				sig_alg = HMAC('sha256'))	
	print(CHAP_Client(c))			
	# Make a New Cijfer 
	# Run Test
