# HTTP-Client-Server-Implementation
Implemented HTTP client and server GET-POST functionality using TCP stream sockets belonging to AF_INET family from scratch.Also implemented a http server side parser to parse the incoming http get/post request and return a parsed req.

For running http client, cd in the root directory  and run the command "python3 main.py" from the command line it will ask for URL this client also supports HTTPS requests.After this it will ask for port For http it is port 80(for normal web servers on the internet for my own http server on localhost the port is 8080 mentioned in http/server/httpserver.py) and for https it is 443.


Currently the post request needs to be hardcoded with respect to the post url and the request body.For now the default is posting to a form hosted on my http server on my local host "127.0.0.1:8888" which is run by default after a get request but make sure the server is running on port 8888
For running http server, cd in the server folder run the command "python3 httpserver.py" it will create a http server listening on port 8888 with hostname as "127.0.0.1"


At the root path in url "/" it provides a form when request using GET it handles the submition of the form at the url "/dump.txt" and you can also see the dump by requesting a GET at path "/dump.txt" 


Since this is a proper HTTP web server you can also test it by first running http server on localhost:8888 and then using a browser and navigate to http://127.0.0.1:8888/ fill the form and submit and then access the dump by http://127.0.0.1/dump.txt the server also has a image for now http://127.0.0.1/pic.jpg
