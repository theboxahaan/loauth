import json
import re
from base64 import urlsafe_b64encode, urlsafe_b64decode
from exceptions import InvalidTokenError
import cijfer

class JWT:
	
	def __init__(self, header=None, payload=None, jwt=None):
		if jwt is not None and header is None and payload is None:
			self.token = jwt
			self.header,self.payload, self.signature = [urlsafe_b64decode(x + '===') for x in jwt.split('.')]
			__algo = json.loads(self.header.decode('utf-8'))['alg']  
			self.cijfer = cijfer.Cijferpack.cijfer_list[__algo]

		elif jwt is None and header is not None and payload is not None:
			try:
				self.header, self.payload = header, payload
				if "alg" not in header:
					raise KeyError("'alg' not present in header")
				__algo = header["alg"]
				self.cijfer = cijfer.Cijferpack.cijfer_list[__algo]
			except TypeError as e:
				raise TypeError("Header and payload should be JSON objects")

		else:
			raise Exception("Provide either header and payload, or a jwt token")

	def verify(self, key):
		sig_pt = self.token.rsplit('.', 1)[0].encode('utf-8')
		return self.cijfer.verify(key, sig_pt , self.signature)


	def Generate(self, key):
		self.header, self.payload = [json.dumps(x).encode('utf-8') for x in [self.header, self.payload]]
		self.header, self.payload = [(urlsafe_b64encode(x)).decode('utf-8') for x in [self.header, self.payload]]
		token = self.header + '.' + self.payload

		self.signature = (self.cijfer.sign(key, token.encode('utf-8')))
		self.token = token + '.' + urlsafe_b64encode(self.signature).decode('utf-8')
		return self.token

	def Header(self, key):
		if not self.verify(key):
			raise InvalidTokenError("Invalid token signature")

		return urlsafe_b64decode(self.header + '===').decode("utf-8")

	def Payload(self, key):
		if not self.verify(key):
			raise InvalidTokenError("Invalid token signature")
		return urlsafe_b64decode(self.payload + '===').decode("utf-8")

	def HeaderAndPayload(self, key):
		if not self.verify(key):
			raise InvalidTokenError("Invalid token signature")
		return urlsafe_b64decode(self.header + '===').decode("utf-8"), urlsafe_b64decode(self.payload + '===').decode("utf-8")


# if __name__ == '__main__':
# 	jwt = JWT(header={"alg": "HS256"}, payload={"body":"hi"})
# 	print(jwt.Generate(b"WHAFJGIHJTPJINJHE"))
# 	print(jwt.Header(b"WHAFJGIHJTPJINJHE"))



