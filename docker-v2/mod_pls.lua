-- Prosody IM
-- Copyright (C) 2008-2010 Matthew Wild
-- Copyright (C) 2008-2010 Waqas Hussain
--
-- This project is MIT/X11 licensed. Please see the
-- COPYING file in the source package for more information.
--

local create_context = require "core.certmanager".create_context;
local rawgetopt = require"core.configmanager".rawget;
local st = require "util.stanza";
local add_filter = require "util.filters".add_filter;
local presentlib = require "util.libnativefunc";

local c2s_require_encryption = module:get_option("c2s_require_encryption", module:get_option("require_encryption"));
local s2s_require_encryption = module:get_option("s2s_require_encryption");
local allow_s2s_tls = module:get_option("s2s_allow_encryption") ~= false;
local s2s_secure_auth = module:get_option("s2s_secure_auth");

if s2s_secure_auth and s2s_require_encryption == false then
	module:log("warn", "s2s_secure_auth implies s2s_require_encryption, but s2s_require_encryption is set to false");
	s2s_require_encryption = true;
end

local xmlns_startpls = 'urn:ietf:params:xml:ns:xmpp-pls';
local startpls_attr = { xmlns = xmlns_startpls };
local startpls_initiate= st.stanza("startpls", startpls_attr);
local startpls_proceed = st.stanza("proceed", startpls_attr);
local startpls_failure = st.stanza("failure", startpls_attr);
local c2s_feature = st.stanza("startpls", startpls_attr);
local s2s_feature = st.stanza("startpls", startpls_attr);
if c2s_require_encryption then c2s_feature:tag("required"):up(); end
if s2s_require_encryption then s2s_feature:tag("required"):up(); end

local hosts = prosody.hosts;
local host = hosts[module.host];

local ssl_ctx_c2s, ssl_ctx_s2sout, ssl_ctx_s2sin;
local ssl_cfg_c2s, ssl_cfg_s2sout, ssl_cfg_s2sin;



local function main_decrypt(data,session)
	if data == nil then
		return data;
	end
	
	session.log("debug", "inside the main_decrypt incoming data : %s", tostring(data));
	local decrypted_text = presentlib.decrypt_bytes(data,"abcdefghij");
	session.log("debug", "inside the main_decrypt decrypted :%s", tostring(decrypted_text));
	--decrypted_text = unescape(decrypted_text)
	
	return decrypted_text;


end


local function main_encrypt(data,session)
	if data == nil then
		return data;
	end
	
	session.log("debug", "inside the main_encrypt outgoing data : %s", tostring(data));
	local encrypted_text = presentlib.encrypt_bytes(data,"abcdefghij");
	session.log("debug", "inside the main_encrypt encrypted :%s", tostring(encrypted_text));
	--decrypted_text = unescape(decrypted_text)
	
	return encrypted_text;


end




local function setup_decompression(session, func)
	add_filter(session, "bytes/in", function(data)
		session.log("debug", "this is the encrypted text %s", tostring(data));
		local status,decrypted_data = pcall(func, data,session);
		session.log("debug", "final layer %s", tostring(decrypted_data));
		return decrypted_data;
	end);
end


local function setup_compression(session, func)

	add_filter(session, "bytes/out", function(data)
		session.log("debug", "this was the text going out %s", tostring(data));

		local status, encrypted_data = pcall(func, data, session);
		session.log("debug", "encrypted_data going out %s", tostring(encrypted_data));
		return encrypted_data;
	end);
end


	



function module.load()
	local NULL, err = {};
	local modhost = module.host;
	local parent = modhost:match("%.(.*)$");

	local parent_ssl = rawgetopt(parent,  "ssl") or NULL;
	local host_ssl   = rawgetopt(modhost, "ssl") or parent_ssl;

	local global_c2s = rawgetopt("*",     "c2s_ssl") or NULL;
	local parent_c2s = rawgetopt(parent,  "c2s_ssl") or NULL;
	local host_c2s   = rawgetopt(modhost, "c2s_ssl") or parent_c2s;

	local global_s2s = rawgetopt("*",     "s2s_ssl") or NULL;
	local parent_s2s = rawgetopt(parent,  "s2s_ssl") or NULL;
	local host_s2s   = rawgetopt(modhost, "s2s_ssl") or parent_s2s;

	module:log("debug", "Creating context for c2s");
	ssl_ctx_c2s, err, ssl_cfg_c2s = create_context(host.host, "server", host_c2s, host_ssl, global_c2s); -- for incoming client connections
	if not ssl_ctx_c2s then module:log("error", "Error creating context for c2s: %s", err); end

	module:log("debug", "Creating context for s2sout");
	ssl_ctx_s2sout, err, ssl_cfg_s2sout = create_context(host.host, "client", host_s2s, host_ssl, global_s2s); -- for outgoing server connections
	if not ssl_ctx_s2sout then module:log("error", "Error creating contexts for s2sout: %s", err); end

	module:log("debug", "Creating context for s2sin");
	ssl_ctx_s2sin, err, ssl_cfg_s2sin = create_context(host.host, "server", host_s2s, host_ssl, global_s2s); -- for incoming server connections
	if not ssl_ctx_s2sin then module:log("error", "Error creating contexts for s2sin: %s", err); end
end

module:hook_global("config-reloaded", module.load);

local function can_do_pls(session)
	if not session.conn.starttls then
		if not session.secure then
			session.log("debug", "Underlying connection does not support STARTTLS");
		end
		return false;
	elseif session.ssl_ctx ~= nil then
		return session.ssl_ctx;
	end
	if session.type == "c2s_unauthed" then
		session.ssl_ctx = ssl_ctx_c2s;
		session.ssl_cfg = ssl_cfg_c2s;
	elseif session.type == "s2sin_unauthed" and allow_s2s_tls then
		session.ssl_ctx = ssl_ctx_s2sin;
		session.ssl_cfg = ssl_cfg_s2sin;
	elseif session.direction == "outgoing" and allow_s2s_tls then
		session.ssl_ctx = ssl_ctx_s2sout;
		session.ssl_cfg = ssl_cfg_s2sout;
	else
		session.log("debug", "Unknown session type, don't know which TLS context to use");
		return false;
	end
	if not session.ssl_ctx then
		session.log("debug", "Should be able to do TLS but no context available");
		return false;
	end
	return session.ssl_ctx;
end




-- Hook <startpls/>
module:hook("stanza/urn:ietf:params:xml:ns:xmpp-pls:startpls", function(event)
	local origin = event.origin;
	if can_do_pls(origin) then
		(origin.sends2s or origin.send)(startpls_proceed);
		origin:reset_stream();
		setup_decompression(origin, main_decrypt);
		setup_compression(origin,main_encrypt);
		
		origin.log("debug", "PLS negotiation started for %s...", origin.type);
		origin.secure = false;
	else
		origin.log("warn", "Attempt to start PLS, but PLS is not available on this %s connection", origin.type);
		(origin.sends2s or origin.send)(startpls_failure);
		origin:close();
	end
	return true;
end);



-- Hook <starttls/>
--module:hook("stanza/urn:ietf:params:xml:ns:xmpp-tls:starttls", function(event)
	--local origin = event.origin;
	--if can_do_tls(origin) then
		--(origin.sends2s or origin.send)(starttls_proceed);
		--origin:reset_stream();
		--origin.conn:starttls(origin.ssl_ctx);
		--origin.log("debug", "TLS negotiation started for %s...", origin.type);
		--origin.secure = false;
	--else
		--origin.log("warn", "Attempt to start TLS, but TLS is not available on this %s connection", origin.type);
		--(origin.sends2s or origin.send)(starttls_failure);
		--origin:close();
	--end
	--return true;
--end);

-- Advertize stream feature
module:hook("stream-features", function(event)
	local origin, features = event.origin, event.features;
	if can_do_pls(origin) then
		features:add_child(c2s_feature);
	end
end);
module:hook("s2s-stream-features", function(event)
	local origin, features = event.origin, event.features;
	if can_do_tls(origin) then
		features:add_child(s2s_feature);
	end
end);

-- For s2sout connections, start TLS if we can
module:hook_tag("http://etherx.jabber.org/streams", "features", function (session, stanza)
	module:log("debug", "Received features element");
	if can_do_tls(session) and stanza:get_child("starttls", xmlns_starttls) then
		module:log("debug", "%s is offering TLS, taking up the offer...", session.to_host);
		session.sends2s(starttls_initiate);
		return true;
	end
end, 500);

module:hook_tag(xmlns_startls, "proceed", function (session, stanza) -- luacheck: ignore 212/stanza
	if session.type == "s2sout_unauthed" and can_do_tls(session) then
		module:log("debug", "Proceeding with TLS on s2sout...");
		session:reset_stream();
		session.conn:starttls(session.ssl_ctx);
		session.secure = false;
		return true;
	end
end);
