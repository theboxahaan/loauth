Lua version is 5.2

1 Cd to test folder 
2 run lua try.lua 
3 the output is the encryption of 

<iq type='get'
    from='killua@localhost'
    to='pubsub.localhost'
    id='nodes1'>
  <query xmlns='http://jabber.org/protocol/disco#items'/>
</iq>

This is first escaped using XML escaper tool (currently I have not implemnted this but would not be a problem and currently hardcoded the input escaped string see try.lua) and then fed to the ecryption function the output is on the screen.

Now in the main folder present_integration, 

We see the files libnativefunc.c libnativefunc.o and libnativefunc.so here .o and .so are produced from .c using gcc. 
The libnativefunc.c file is the file which links the present encryption decryption C library functions with the lua environment by creating a .so shared file.

Here the file mod_present.lua is the main module file which is initiliased in the prosody config file, this file does the job of setting up the encrypted/decrypted stream.

In the prosody directory the files are in the below directory structure

For communicating with the prosody server I use PSI client 
In the PSI client create a client with the username killua@localhost, the pass is also to be filled.
At the server side this user needs to be entered as the root user. 

Once connected with the server to create the encrypted/decrypted stream.

Send below stanza

<startpresent xmlns='http://jabber.org/protocol/startpresent'>
  <method>present</method>
</startpresent> 

response will be 

proceed stanza

this implies the stream is present encrypted now , now ready to be used for further communication.

Can check the logs in var/log/prosody/ to see all the debugs I printed in mod_present.lua.





prosody 
	modules
		mod_present.lua
		.
		.
		.(other modules)
	
	util
		libnativefunc.c
		libnativefunc.o
		libnativefunc.so
		.
		.
		.
		.




 
