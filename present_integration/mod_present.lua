
local st = require "util.stanza";
local pcall = pcall;
local tostring = tostring;
local add_filter = require "util.filters".add_filter;
local presentlib = require "util.libnativefunc";

--require the function decryptf which takes in bytes of data and decrypts them and returns decrypted bytes

--basically pcall is just a function caller it is of the syntax pcall(f,arg1,arg2.....)



--Technically anything other than 'message', 'presence' or 'iq' is not a stanza. However these other elements fire an event prefixed with 'stanza/' for consistency:

  --stanza/XMLNS:TAG
--Where XMLNS is the namespace and TAG is the name of the element. These elements are typically stream feature negotiations such as TLS, SASL or compression.

-- from xmpp doocumentation on hooks hence below will trigger the hook defined
--<startpresent xmlns='http://jabber.org/protocol/startpresent'>
  --<method>present</method>
--</startpresent> 


function unescape(str)
  str = string.gsub( str, '&lt;', '<' )
  str = string.gsub( str, '&gt;', '>' )
  str = string.gsub( str, '&quot;', '"' )
  str = string.gsub( str, '&apos;', "'" )
  str = string.gsub( str, '&#(%d+);', function(n) return string.char(n) end )
  str = string.gsub( str, '&#x(%d+);', function(n) return string.char(tonumber(n,16)) end )
  str = string.gsub( str, '&amp;', '&' ) -- Be sure to do this after all others
  return str
end




local function pseudo_decrypt(data,session)
	session.log("debug","perform decryption");
	session.log("debug","see data here");
	--local decrypted_text = "example decrpted text";
	--session.log("debug", "inside pseudo_decrypt %s", data);
	session.log("debug", "inside pseudo_decrypt %s", tostring(data));
		
	return data;
end

--below function was just for testng
local function main_decrypt(data,session)
	if data == nil then
		return data;
	end
	
	session.log("debug", "inside the main_decrypt incoming data : %s", tostring(data));
	local decrypted_text = presentlib.decrypt_bytes(data,"abcdefghij");
	session.log("debug", "inside the main_decrypt decrypted :%s", tostring(decrypted_text));
	decrypted_text = unescape(decrypted_text)
	session.log("debug", "after unescape :%s", tostring(decrypted_text));
	
	return decrypted_text;


end
	
	
local function return_decryptf(session)
	session.log("debug","inside decryptf");
	return pseudo_decrypt;
end




local function get_inflate_stream(session)
	local status, func = pcall(return_decryptf,session); --this should return the function that decrypts the incoming stream
	return func;
end


--bytes/in must mean incoming bytes which are encrypted

local function setup_decompression(session, func)
	add_filter(session, "bytes/in", function(data)
		session.log("debug", "this is the encrypted text %s", tostring(data));
		local status,decrypted_data = pcall(func, data,session);
		session.log("debug", "final layer %s", tostring(decrypted_data));
		return decrypted_data;
	end);
end






module:hook("stanza/http://jabber.org/protocol/startpresent:startpresent", function(event)


	
	local session, stanza = event.origin, event.stanza;
	local method = stanza:get_child_text("method");
	session.log("debug", "hello there %s", tostring(method));
	
	if session.type == "c2s"  then
		
		-- checking if the compression method is supported
		local method = stanza:get_child_text("method");
		if method == "present" then
			session.log("debug", "encryption enabled");
			
			-- create deflate and inflate streams
			--local inflate_stream = get_inflate_stream(session);
			
			(session.send)(st.stanza("proceed", {xmlns="urn:ietf:params:xml:ns:xmpp-present"}));
			-- I do not restart the stream because that is done to usually get a clean strat but it is leading to problems hence continuing the original stream it is working with that 
			--session:reset_stream();
			
			-- setup decompression for session.data
			--setup_decompression(session, pseudo_decrypt);
			setup_decompression(session, main_decrypt);
			
			session.encryption = true;
			
		end
		return true;
	end
end);





-- input incoming bytes 
-- input key 
-- output decrypted bytes
