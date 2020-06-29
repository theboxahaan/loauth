import requests

class Client:
	def __init__(self, endpoint,  **kwargs):
		self.endpoint = endpoint
		for x in kwargs:
			setattr(self, x, kwargs[x])
	
	def __repr__(self):
		print(self.__dict__)

