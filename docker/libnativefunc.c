#include "lua.h"
#include <lauxlib.h>
// #include <lualib.h>
#include <stdlib.h>
#include <stdio.h>
#include "present_cbc.h"



static int encrypt_lua(lua_State *L)
{
	//call the encrypt function 
	// (plaintext,key) as params plaintext on the left
	const char *plaintext = luaL_checkstring(L, 1);
	const char *key = luaL_checkstring(L, 2);
	const char *encrypted_text;
	encrypted_text = presentCBCencr(plaintext,key); //c library function call
	lua_pushstring(L,encrypted_text);

    return 1;
}

static int decrypt_lua(lua_State *L)
{
	//call the decrypt function 

	//(encrypted_text,key) as params
	const char *encrypted_text = luaL_checkstring(L, 1);
	//printf("%s", encrypted_text);
	const char *key = luaL_checkstring(L, 2);
	const char *decrypted_text;
	decrypted_text = presentCBCdecr(encrypted_text,key); //c library function call
	lua_pushstring(L,decrypted_text);

    return 1;

}


static int test_print(lua_State *L)
{
		
	const char *print_text = luaL_checkstring(L, 1);
	printf("\n%s", print_text);
	
	
	return 0;



}


/*
 static const luaL_Reg libnativefunc[] = {
     {"encrypt_bytes", encrypt_lua},
     {"decrypt_bytes", decrypt_lua},
     {NULL, NULL},
 };
*/

static const luaL_Reg nativeFuncLib[] =
{
    {"encrypt_bytes", encrypt_lua},
    {"decrypt_bytes", decrypt_lua},
    {"print_fn", test_print},
    {NULL, NULL}
};

LUALIB_API int luaopen_util_libnativefunc(lua_State *L)
{

    
//    #ifdef lua5.1
  //  luaL_register (L, "libnativefunc", nativeFuncLib);
 //   #else//lua5.2
    lua_newtable(L); 
    luaL_setfuncs(L, nativeFuncLib, 0);
    //lua_setfield(L, -2, "present");
//    #endif
   // lua_pushliteral(L, "libnativefunc");
    return 1;//Return table
//    luaL_newlib(L, libnativefunc);
    //return 1;
}
