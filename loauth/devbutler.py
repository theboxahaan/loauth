import socketserver
import http.server
from urllib.parse import urlparse
from urllib.parse import parse_qs

def Factory(handler_method):
	class FactoryGeneratedHandlerClass(http.server.BaseHTTPRequestHandler):
		_session = {}
		def __init__(self, *args, **kwargs):
			self.__handler_method = handler_method
			super().__init__(*args, **kwargs)
		
		def session_add(self, dic):
			for x in dic:
				self._session[x] = dic[x]

		def do_GET(self):
			self.send_response(200)
			self.send_header('Content-Type', 'text')
			self.end_headers()
			query_args = parse_qs(urlparse(self.path).query)
			print('Args Received: ', query_args)
			response = self.__handler_method(query_args, self)
			self.wfile.write(bytes(response, 'utf-8'))
			return
	return FactoryGeneratedHandlerClass



def deploy_butler(fn, PORT):
	handler = Factory(fn)
	server = socketserver.TCPServer(('localhost', PORT), handler)
	server.serve_forever()

