--package.loadlib("./libnativefunc.so", "luaopen_libnativefunc")()
local help_lib = require "libnativefunc"
--require "string"

--print(string);

--print(libnativefunc);

--local a = help_lib.encrypt_bytes("<iq type='get'\r\n    from='killua@localhost'\r\n    to='pubsub.localhost'\r\n    id='nodes1'>\r\n  <query xmlns='http:\/\/jabber.org\/protocol\/disco#items'\/>\r\n<\/iq>");

local a = help_lib.encrypt_bytes("&lt;iq type=&apos;get&apos;\
    from=&apos;killua@localhost&apos;\
    to=&apos;pubsub.localhost&apos;\
    id=&apos;nodes1&apos;&gt;\
  &lt;query xmlns=&apos;http://jabber.org/protocol/disco#items&apos;/&gt;\
&lt;/iq&gt;", "abcdefghij");
print(a);
local b = help_lib.decrypt_bytes(a,"abcdefghij");

print(b);

print(help_lib.print_fn("this is mic check"));





-- Currently I am sinply using the online XML escape to escape the XML stanza and then i feed this string to the encryption library,(** see the \ in above escaped string this helps to keep the same structure of the escaped string as the original XML stanza)
-- Next I feed this output which are enc bytes to the prosody server directly using the PSI client after I set up the ecrypt decrypt stream using the present stanza and once i get the proceed i quikly send these enc bytes, quickly is imp since see the error in zoinks.
