PubSub Architecture is very well suited for devices like IOT sensors, RFID Readers etc . A lot of these devices can be subscribed to a group and receive notifications on the event when any item is published. Prosody server supports the PubSub protocol. These devices are generally low powered and heavy computations like those of TLS encryption (currently only TLS supported by Prosody) takes a toll on them in terms of energy consumption. We seek to resolve this problem by giving PRESENT encryption support to prosody which is a lightweight cipher but highly secure at the same time. We also develop a Jabber client that is very light and supports PRESENT encryption.


### Plugin - Pls

This plugin is analogous to tls in terms of functionality. It implements PRESENT encryption scheme in the server. This is a very lightweight cipher when compared to TLS. It is especially suited for Low Powered clients like RFID Readers or IOT nodes. Like any other Prosody module we can include this simply by adding the corresponding lua file in the modules directory and also enabling the module in the Prosody Configuration file. 

This encryption is top level and will not affect any application developer developing or using other modules for Prosody.

The file `mod_pls.lua` is the plugin file to be located in `/usr/lib/prosody/modules/`
It requires the shared library` libnativefunc.so `in the `/usr/lib/prosody/util `folder.

The `libnativefunc.so` file defines the present encryption/decryption helper functions which can be conveniently called in the pls module. 

To enable the pls module : In the Configuration file, add the "pls" module in modules_enabled = {}.



##
