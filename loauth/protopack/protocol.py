from loauth import cijfer

class Protocol:
	def __init__(self, name):
		self.name = name
	
	def trigger(self):
		raise NotImplementedError
	
	def response(self): 
		raise NotImplementedError
