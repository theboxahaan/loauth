import hashlib
import hmac as hm

class Cijfer:
	'''Base Class for Signature ALgorithms
	Methods - prepare_key(key), sign(key,msg), verify(key, msg, sign)
	Usage-
	Override Methods in inheriting class ot implement new signature algorithm
	Reigster the new cipher to the cijferpack using cijferpack.register('name', 'object') function

	class newCipher(Cijfer):
		def prepare_key(self, key):
			pass
		def sign(self, key, msg);
			pass
		def verify(self, key, msg, sign):
			pass
	'''

	def prepare_key(self, key):
		raise NotImplementedError
	def sign(self, key, msg):
		raise NotImplementedError
	def verify(self, key, msg, sign):
		raise NotImplementedError

class HMAC(Cijfer):
	__hash_list = {'sha256': hashlib.sha256, 'sha512':hashlib.sha512 }

	def __init__(self, hash_alg):
		self.__hash_alg = self.__hash_list[hash_alg.lower()]
	
	@staticmethod
	def hash_list():
		return ['sha256', 'sha512']

	def sign(self, key, msg):
		if not isinstance(key, bytes) or not isinstance(msg, bytes):
			raise TypeError
		else:
			return hm.new(key, msg, self.__hash_alg).digest()
	
	def prepare_key(self, key):
		if isinstance(key, bytes):
			return key
		if isinstance(key, string):
			return key.encode('utf-8')
		else:
			raise TypeError

	def verify(self, key, msg, sign):
		return hm.compare_digest(sign, hm.new(key, msg, self.__hash_alg).digest() )


class Cijferpack:
	cijfer_list = {'HS256': HMAC('sha256'), 'HS512': HMAC('sha512') }

	def register(self, tag, new_cijfer):
		self.cijfer_list[tag] = new_cijfer
		return self.cijfer_list
	

