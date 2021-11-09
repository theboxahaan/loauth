from pypresent import Present
import sys
import binascii

def xor(a,b):
	res = int.from_bytes(a, byteorder=sys.byteorder) ^ int.from_bytes(b, byteorder=sys.byteorder)
	return res.to_bytes(max(len(a), len(b)), byteorder=sys.byteorder)


class PRESENT_CBC(Present):
	
	def __init__(self, IV:bytes=None, key:bytes=None):
		super().__init__(key)
		self._IV = IV
		self._msg_list = None
		self._key = None
	
	def encrypt(self, msg_list:bytes=None):
		if isinstance(msg_list, bytes):
			msg_list = self.msg_to_blocks(msg_list)
		cipher_list = []
		for i in range(len(msg_list)):
			if i == 0:
				_t = xor(msg_list[i], self._IV)
			else:
				_t = xor(msg_list[i], cipher_list[i-1])

			cipher_list.append(bytes.fromhex(super().encrypt(_t)))
		#for i in range(len(cipher_list)):
		#	cipher_list[i] = binascii.hexlify(cipher_list[i]).decode('utf-8') 
		return cipher_list
	
	def decrypt(self, msg_list:bytes=None):
		if isinstance(msg_list, bytes):
			msg_list = self.msg_to_blocks(msg_list)
		plain_list = []
		for i in range(len(msg_list)):
			_t = super().decrypt(msg_list[i])
			if i == 0:
				plain_list.append(xor(self._IV, bytes.fromhex(_t)))
			else :
				plain_list.append(xor(bytes.fromhex(_t), msg_list[i-1]))
		return plain_list
	
	def msg_to_blocks(self, msg:bytes=None, debug=True):
		# zero pad message to a multiple of 64 bits/8 bytes
		while len(msg) % 8 != 0:
			msg += b'0'
		msg_block = [msg[8*i:8*(i+1)] for i in range(0, len(msg)//8)]
		if debug:
			# print(f':. blocked msg> {msg_block}')
			pass
		return msg_block	


if __name__ == '__main__':
	#key = bytes.fromhex('0123456789abcdef0123') 
	#plain = bytes.fromhex('0123456789abcdefabcd')
	key = b'abcdefghij'
	IV = b'12341234'
	cipher = PRESENT_CBC(IV, key)
	q = cipher.encrypt(b'Bananas 12345678Bananas ')
	for i in q:
		print(i.hex(), end=' ')
