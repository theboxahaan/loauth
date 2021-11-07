#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//#include "/home/manas/Desktop/projects/deliverables/Present-8bit/present_try.h"

#define ROUNDS               32
#define ROUND_KEY_SIZE_BYTES  8
#define PRESENT_80_KEY_SIZE_BYTES 10
#define PRESENT_128_KEY_SIZE_BYTES 16
#define PRESENT_BLOCK_SIZE_BYTES 8

void present80_encryptBlock( unsigned char *block, const unsigned char *key );
void present80_decryptBlock( unsigned char *block, const unsigned char *key );

void present128_encryptBlock( unsigned char *block, const unsigned char *key );
void present128_decryptBlock( unsigned char *block, const unsigned char *key );

void present80CBC_encrypt( unsigned char *message, const unsigned int messageLength, const unsigned char *key, const unsigned char *iv );
void present80CBC_decrypt( unsigned char *message, const unsigned int messageLength, const unsigned char *key, const unsigned char *iv );
void present128CBC_encrypt( unsigned char *message, const unsigned int messageLength, const unsigned char *key, const unsigned char *iv );
void present128CBC_decrypt( unsigned char *message, const unsigned int messageLength, const unsigned char *key, const unsigned char *iv );





unsigned char key80[10] = {0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a};
unsigned char iv[8] = {0x31,0x32,0x33,0x34,0x31,0x32,0x33,0x34};


unsigned char sBox[16] = {
    0xC, 0x5, 0x6, 0xB, 0x9, 0x0, 0xA, 0xD, 0x3, 0xE, 0xF, 0x8, 0x4, 0x7, 0x1, 0x2 };

unsigned char sBoxInverse[16] = {
    0x5, 0xE, 0xF, 0x8, 0xC, 0x1, 0x2, 0xD, 0xB, 0x4, 0x6, 0x3, 0x0, 0x7, 0x9, 0xA };




void printMessage( const unsigned char *message ){
    printf( "Got: " );
    for( int i = 0; i < strlen(message); i+=2 ){
        printf( "%02x%02x", message[i], message[i+1] );
    }
    printf( "\n" );
}

void copyKey( const unsigned char *from, unsigned char *to, const unsigned char keyLen ){
    int i;
    for( i = 0; i < keyLen; i++ ){
        to[i] = from[i];
    }
}

void generateRoundKeys80( const unsigned char *suppliedKey, unsigned char keys[32][ROUND_KEY_SIZE_BYTES]){
    // trashable key copies
    unsigned char key[PRESENT_80_KEY_SIZE_BYTES];
    unsigned char newKey[PRESENT_80_KEY_SIZE_BYTES];
    unsigned char i, j;
    copyKey( suppliedKey, key, PRESENT_80_KEY_SIZE_BYTES );
    copyKey( key, keys[0], ROUND_KEY_SIZE_BYTES );
    for( i = 1; i < ROUNDS; i++ ){
        // rotate left 61 bits
        for( j = 0; j < PRESENT_80_KEY_SIZE_BYTES; j++ ){
            newKey[j] = (key[(j+7) % PRESENT_80_KEY_SIZE_BYTES] << 5) |
                        (key[(j+8) % PRESENT_80_KEY_SIZE_BYTES] >> 3);
        }
        copyKey( newKey, key, PRESENT_80_KEY_SIZE_BYTES );

        // pass leftmost 4-bits through sBox
        key[0] = (sBox[key[0] >> 4] << 4) | (key[0] & 0xF);

        // xor roundCounter into bits 15 through 19
        key[8] ^= i << 7; // bit 15
        key[7] ^= i >> 1; // bits 19-16

        copyKey( key, keys[i], ROUND_KEY_SIZE_BYTES );
    }
}

void generateRoundKeys128( const unsigned char *suppliedKey, unsigned char keys[32][ROUND_KEY_SIZE_BYTES]){
    // trashable key copies
    unsigned char key[PRESENT_128_KEY_SIZE_BYTES];
    unsigned char newKey[PRESENT_128_KEY_SIZE_BYTES];
    unsigned char i, j;
    copyKey( suppliedKey, key, PRESENT_128_KEY_SIZE_BYTES );
    copyKey( key, keys[0], ROUND_KEY_SIZE_BYTES );
    for( i = 1; i < ROUNDS; i++ ){
        // rotate left 61 bits
        for( j = 0; j < PRESENT_128_KEY_SIZE_BYTES; j++ ){
            newKey[j] = (key[(j+7) % PRESENT_128_KEY_SIZE_BYTES] << 5) | (key[(j+8) % PRESENT_128_KEY_SIZE_BYTES] >> 3);
        }
        copyKey( newKey, key, PRESENT_128_KEY_SIZE_BYTES );

        // pass leftmost 8-bits through sBoxes
        key[0] = (sBox[key[0] >> 4] << 4) | (sBox[key[0] & 0xF]);

        // xor roundCounter into bits 62 through 66
        key[8] ^= i << 6; // bits 63-62
        key[7] ^= i >> 2; // bits 66-64

        copyKey( key, keys[i], ROUND_KEY_SIZE_BYTES );
    }
}

void addRoundKey( unsigned char *block, unsigned char *roundKey ){
    unsigned char i;
    for( i = 0; i < PRESENT_BLOCK_SIZE_BYTES; i++ ){
        block[i] ^= roundKey[i];
    }
}

void pLayer( unsigned char *block ){
    unsigned char i, j, indexVal, andVal;
    unsigned char initial[PRESENT_BLOCK_SIZE_BYTES];
    copyKey( block, initial, PRESENT_BLOCK_SIZE_BYTES );
    for( i = 0; i < PRESENT_BLOCK_SIZE_BYTES; i++ ){
        block[i] = 0;
        for( j = 0; j < 8; j++ ){
            indexVal = 4 * (i % 2) + (3 - (j >> 1));
            andVal = (8 >> (i >> 1)) << ((j % 2) << 2);
            block[i] |= ((initial[indexVal] & andVal) != 0) << j;
        }
    }
}

void pLayerInverse( unsigned char *block ){
    unsigned char i, j, indexVal, andVal;
    unsigned char initial[PRESENT_BLOCK_SIZE_BYTES];
    copyKey( block, initial, PRESENT_BLOCK_SIZE_BYTES );
    for( i = 0; i < PRESENT_BLOCK_SIZE_BYTES; i++ ){
        block[i] = 0;
        for( j = 0; j < 8; j++ ){
            indexVal = (7 - ((2*j)%8)) - (i < 4);
            andVal = (7-((2*i)%8)) - (j < 4);
            block[i] |= ((initial[indexVal] & (1 << andVal)) != 0) << j;
        }
    }
}

void present80_encryptBlock( unsigned char *block, const unsigned char *key ){
    unsigned char roundKeys[ROUNDS][ROUND_KEY_SIZE_BYTES];
    unsigned char i, j;
    generateRoundKeys80( key, roundKeys );
    for( i = 0; i < ROUNDS-1; i++ ){
        addRoundKey( block, roundKeys[i] );
        for( j = 0; j < PRESENT_BLOCK_SIZE_BYTES; j++ ){
            block[j] = (sBox[block[j] >> 4] << 4) | sBox[block[j] & 0xF];
        }
        pLayer( block );
    }
    addRoundKey( block, roundKeys[ROUNDS-1] );
}

void present80_decryptBlock( unsigned char *block, const unsigned char *key ){
    unsigned char roundKeys[ROUNDS][ROUND_KEY_SIZE_BYTES];
    unsigned char i, j;
    generateRoundKeys80( key, roundKeys );
    for( i = ROUNDS-1; i > 0; i-- ){
        addRoundKey( block, roundKeys[i] );
        pLayerInverse( block );
        for( j = 0; j < PRESENT_BLOCK_SIZE_BYTES; j++ ){
            block[j] = (sBoxInverse[block[j] >> 4] << 4) | sBoxInverse[block[j] & 0xF];
        }
    }
    addRoundKey( block, roundKeys[0] );
}

void present128_encryptBlock( unsigned char *block, const unsigned char *key ){
    unsigned char roundKeys[ROUNDS][ROUND_KEY_SIZE_BYTES];
    unsigned char i, j;
    generateRoundKeys128( key, roundKeys );
    for( i = 0; i < ROUNDS-1; i++ ){
        addRoundKey( block, roundKeys[i] );
        for( j = 0; j < PRESENT_BLOCK_SIZE_BYTES; j++ ){
            block[j] = (sBox[block[j] >> 4] << 4) | sBox[block[j] & 0xF];
        }
        pLayer( block );
    }
    addRoundKey( block, roundKeys[ROUNDS-1] );
}

void present128_decryptBlock( unsigned char *block, const unsigned char *key ){
    unsigned char roundKeys[ROUNDS][ROUND_KEY_SIZE_BYTES];
    unsigned char i, j;
    generateRoundKeys128( key, roundKeys );
    for( i = ROUNDS-1; i > 0; i-- ){
        addRoundKey( block, roundKeys[i] );
        pLayerInverse( block );
        for( j = 0; j < PRESENT_BLOCK_SIZE_BYTES; j++ ){
            block[j] = (sBoxInverse[block[j] >> 4] << 4) | sBoxInverse[block[j] & 0xF];
        }
    }
    addRoundKey( block, roundKeys[0] );
}




void performBlockFunctionForward( void (*blockFunction)(unsigned char *, const unsigned char *), unsigned char *message, const unsigned int messageLength,  const unsigned char *key, const unsigned char *iv ){
    unsigned char *nonce = (unsigned char *) iv;
    unsigned int offset = 0;
    int i;
    while( offset < messageLength ){
        for( i = 0; i < PRESENT_BLOCK_SIZE_BYTES; i++ ){
            message[offset + i] ^= nonce[i];
        }
        blockFunction( &message[offset], key );
        nonce = &message[offset];
        offset += PRESENT_BLOCK_SIZE_BYTES;
    }
}

/**
 * This will perform the given decryption function backward to perform the cbc
 * functionality in-place
 */
void performBlockFunctionBackward( void (*blockFunction)(unsigned char *, const unsigned char *), unsigned char *message, const unsigned int messageLength,  const unsigned char *key, const unsigned char *iv ){
    unsigned int offset = PRESENT_BLOCK_SIZE_BYTES;
    int i;
    while( offset < messageLength ){
        blockFunction( &message[messageLength - offset], key );
        for( i = 0; i < PRESENT_BLOCK_SIZE_BYTES; i++ ){
            message[(messageLength - offset) + i] ^= message[((messageLength - offset) - PRESENT_BLOCK_SIZE_BYTES) + i];
        }
        offset += PRESENT_BLOCK_SIZE_BYTES;
    }
    // Decrypt last block using the iv
    blockFunction( &message[0], key );
    for( i = 0; i < PRESENT_BLOCK_SIZE_BYTES; i++ ){
        message[i] ^= iv[i];
    }
}

void present80CBC_encrypt( unsigned char *message, const unsigned int messageLength,  const unsigned char *key, const unsigned char *iv ){
    performBlockFunctionForward( present80_encryptBlock, message, messageLength, key, iv );
}

void present80CBC_decrypt( unsigned char *message, const unsigned int messageLength,  const unsigned char *key, const unsigned char *iv ){
    performBlockFunctionBackward( present80_decryptBlock, message, messageLength, key, iv );
}

void present128CBC_encrypt( unsigned char *message, const unsigned int messageLength,  const unsigned char *key, const unsigned char *iv ){
    performBlockFunctionForward( present128_encryptBlock, message, messageLength, key, iv );
}

void present128CBC_decrypt( unsigned char *message, const unsigned int messageLength,  const unsigned char *key, const unsigned char *iv ){
    performBlockFunctionBackward( present128_decryptBlock, message, messageLength, key, iv );
}












static int encrypt_lua(lua_State *L)
{
	//call the encrypt function 
	// (plaintext,key) as params plaintext on the left
	const char *plaintext = luaL_checkstring(L, 1);
	const char *key = luaL_checkstring(L, 2);
	
	
	unsigned char mess[2*strlen(plaintext) + 1];

    strncpy(mess, plaintext, sizeof mess);
	
	// for (int i=0;i<strlen(plaintext);i++){
	
	// mess[i] = plaintext[i];
	// }

    //printf("%s",mess);
	
	present80CBC_encrypt( mess, strlen(mess), key80, iv );
	
	const char *encrypted_text = mess;

    //printf("%s",encrypted_text);
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
	
	//printf("%d",strlen(encrypted_text));
	
	
	unsigned char mess[2*strlen(encrypted_text)+1];
	
	// for (int i=0;i<strlen(encrypted_text);i++){
	
	//   mess[i] = encrypted_text[i];
	// }

    strncpy(mess, encrypted_text, sizeof mess);
   // mess[sizeof mess - 1] = '\0';

    //printf("%s\n",mess);
	present80CBC_decrypt( mess, strlen(mess), key80, iv );
	const char *decrypted_text = mess;

    //printMessage(mess);

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


static const luaL_Reg nativeFuncLib [] =
{
    {"encrypt_bytes", encrypt_lua},
    {"decrypt_bytes", decrypt_lua},
    {"print_fn", test_print},
    {NULL, NULL}
};


LUALIB_API int luaopen_util_libnativefunc(lua_State *L)
{




    //luaL_register(L, "libnativefunc", nativeFuncLib);
    
    
    //#ifdef lua5.1
//Use luaL_register directly under 5.1
    //luaL_register (L, "libnativefunc", nativeFuncLib);
    //#else//lua5.2
    lua_newtable (L); 
    luaL_setfuncs(L, nativeFuncLib, 0);
   //First press a table into VS, and then call luaL_setfuncs will save all func to the table
   //Note that unlike luaL_register this table is an unnamed table, you can use it with only one variable to store it in this table.
   //eg local clib = require "libname". This will not pollute the global environment. Better than luaL_register.
    //luaL_setfuncs (L, nativeFuncLib, 0);
    //#endif
    //lua_pushliteral(L, "libnativefunc");
    return 1;//Return table
//    luaL_newlib(L, libnativefunc);
    //return 1;
}
