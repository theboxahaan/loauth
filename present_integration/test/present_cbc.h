#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>

#define BLOCK_SIZE 16
#define TEXT_SIZE 31 

static const char hexdigits[] = "0123456789abcdef";

typedef struct __attribute__((__packed__)) byte{
    uint8_t nibble1 : 4;
    uint8_t nibble2 : 4;
} byte;

uint8_t S[] = {0xC, 0x5, 0x6, 0xB, 0x9, 0x0, 0xA, 0xD, 0x3, 0xE, 0xF, 0x8, 0x4, 0x7, 0x1, 0x2};

uint8_t invS[] = {0x5, 0xe, 0xf, 0x8, 0xC, 0x1, 0x2, 0xD, 0xB, 0x4, 0x6, 0x3, 0x0, 0x7, 0x9, 0xA};

uint8_t P[] = {0, 16, 32, 48, 1, 17, 33, 49, 2, 18, 34, 50, 3, 19, 35, 51,
                    4, 20, 36, 52, 5, 21, 37, 53, 6, 22, 38, 54, 7, 23, 39, 55,
                    8, 24, 40, 56, 9, 25, 41, 57, 10, 26, 42, 58, 11, 27, 43, 59,
                    12, 28, 44, 60, 13, 29, 45, 61, 14, 30, 46, 62, 15, 31, 47, 63};

byte* fromHexStringToBytes (char *block)
{
    byte* bytes = malloc(8 * sizeof(byte));
    int i;
    for (i=0; i<8; i++)
    {
        bytes[i].nibble1 = (block[2*i]>='0' && block[2*i]<='9')? (block[2*i] - '0') : (block[2*i] - 'a' + 10);
        bytes[i].nibble2 = (block[2*i+1]>='0' && block[2*i+1]<='9')? (block[2*i+1] - '0') : (block[2*i+1] - 'a' + 10);
    }
    return bytes;
}

uint64_t fromBytesToLong (byte* bytes)
{
    uint64_t result = 0;
    int i;
    for (i=0; i<8; i++)
    {
        result = (result << 4) | (bytes[i].nibble1 & 0xFUL);
        result = (result << 4) | (bytes[i].nibble2 & 0xFUL);
    }
    return result;
}

uint64_t fromHexStringToLong (char* block)
{
    uint64_t result;
    int i;
    for (i=0; i<16; i++)
        result = (result << 4) | ((block[i]>='0' && block[i]<='9')? (block[i] - '0') : (block[i] - 'a' + 10));
    return result;
}

byte* fromLongToBytes (uint64_t block)
{
    byte* bytes = malloc (8 * sizeof(byte));
    int i;
    for (i=7; i>=0; i--)
    {
        bytes[i].nibble2 = (block >> 2 * (7 - i) * 4) & 0xFLL;
        bytes[i].nibble1 = (block >> (2 * (7 - i) + 1) * 4) & 0xFLL;
    }
    return bytes;
}

char* fromLongToHexString (uint64_t block)
{
    char* hexString = malloc (17 * sizeof(char));
    sprintf(hexString, "%016llx", block);
    return hexString;
}

uint8_t Sbox(uint8_t input)
{
    return S[input];
}

uint8_t inverseSbox(uint8_t input)
{
    return invS[input];
}

uint64_t permute(uint64_t source)
{
    uint64_t permutation = 0;
    int i;
    for (i=0; i<64; i++)
    {
        int distance = 63 - i;
        permutation = permutation | ((source >> distance & 0x1) << 63 - P[i]);
    }
    return permutation;
}

uint64_t inversepermute(uint64_t source)
{
    uint64_t permutation = 0;
    int i;
    for (i=0; i<64; i++)
    {
        int distance = 63 - P[i];
        permutation = (permutation << 1) | ((source >> distance) & 0x1);
    }
    return permutation;
}

uint16_t getKeyLow(char* key)
{
    int i;
    uint16_t keyLow = 0;
    for (i=16; i<20; i++)
        keyLow = (keyLow << 4) | (((key[i]>='0' && key[i]<='9')? (key[i] - '0') : (key[i] - 'a' + 10)) & 0xF);
    return keyLow;
}

uint64_t* generateSubkeys(char* key)
{
    uint64_t keyHigh = fromHexStringToLong(key);
    uint16_t keyLow = getKeyLow(key);
    uint64_t* subKeys = malloc(32 * (sizeof(uint64_t)));
    int i;
    subKeys[0] = keyHigh;
    for (i=1; i<32; i++)
    {
        uint64_t temp1 = keyHigh, temp2 = keyLow;
        keyHigh = (keyHigh << 61) | (temp2 << 45) | (temp1 >> 19);
        keyLow = ((temp1 >> 3) & 0xFFFF);
        uint8_t temp = Sbox(keyHigh >> 60);
        keyHigh = keyHigh & 0x0FFFFFFFFFFFFFFFLL;
        keyHigh = keyHigh | (((uint64_t)temp) << 60);
        keyLow = keyLow ^ ((i & 0x01) << 15);
        keyHigh = keyHigh ^ (i >> 1);
        subKeys[i] = keyHigh;
    }
    return subKeys;
}

char* encrypt(char* plaintext, char* key)
{
    uint64_t* subkeys = generateSubkeys(key);
    uint64_t state = fromHexStringToLong(plaintext);
    int i, j;
    for (i=0; i<31; i++)
    {
        state = state ^ subkeys[i];
        byte* stateBytes = fromLongToBytes(state);
        for (j=0; j<8; j++)
        {
            stateBytes[j].nibble1 = Sbox(stateBytes[j].nibble1);
            stateBytes[j].nibble2 = Sbox(stateBytes[j].nibble2);
        }
        state = permute(fromBytesToLong(stateBytes));
        free(stateBytes);
    }
    state = state ^ subkeys[31];
    free(subkeys);
    return fromLongToHexString(state);
}

char* decrypt(char* ciphertext, char* key)
{
    uint64_t* subkeys = generateSubkeys(key);
    uint64_t state = fromHexStringToLong(ciphertext);
    int i, j;
    for (i=0; i<31; i++)
    {
        state = state ^ subkeys[31 - i];
        state = inversepermute(state);
        byte* stateBytes = fromLongToBytes(state);
        for (j=0; j<8; j++)
        {
            stateBytes[j].nibble1 = inverseSbox(stateBytes[j].nibble1);
            stateBytes[j].nibble2 = inverseSbox(stateBytes[j].nibble2);
        }
        state = fromBytesToLong(stateBytes);
        free(stateBytes);
    }
    state = state ^ subkeys[0];
    free(subkeys);
    return fromLongToHexString(state);
}

static inline int hexval(char c)
{
    if (isdigit(c))
        return c - '0';
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else
        return -1;
}

void xor_byte_arrays(const char *in1, const char *in2, char *out) 
{
    for (int i = 0; i < BLOCK_SIZE; i++) 
    {
        out[i] = hexdigits[hexval(in1[i]) ^ hexval(in2[i])];
    }
    out[BLOCK_SIZE] = '\0';
}

char* String_Gen(int length) 
{
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char *randomString;

    if (length) 
    {
        randomString = malloc(length +1);

        if (randomString) 
        {
            int l = (int) (sizeof(charset) -1); 
            int key;
            for (int n = 0;n < length;n++) 
            {        
                key = rand() % l;
                randomString[n] = charset[key];
            }

            randomString[length] = '\0';
        }
    }
    return randomString;
}

void string2hexString(char* input, char* output)
{
    int loop;
    int i; 
    
    i=0;
    loop=0;
    
    while(input[loop] != '\0')
    {
        sprintf((char*)(output+i),"%02X", input[loop]);
        loop+=1;
        i+=2;
    }
    output[i++] = '\0';
}

char* textFromHexString(char* hexinput)
{
    char temp[3];
    int index = 0;
    int i=0;
    int len = strlen(hexinput);
    char text[(len/2)+1];
    temp[2] = '\0';
    while (hexinput[index])
    {
        if(hexinput[index])
        strncpy(temp, &hexinput[index], 2);
        text[i] = (char)strtol(temp, NULL, 16);
        i++;
        index += 2;
    }
    text[i] = '\0';
    char* txtoutput = text;
    return(txtoutput);
}

char* presentCBCencr(char* text, char* key)
{
    char plaintext[strlen(text)+1];
    sprintf(plaintext, "%s", text);
    char hex_plaintext[(strlen(text)*2)+1];
    string2hexString(plaintext, hex_plaintext);
    const int hexPtextLen = (int) strlen(hex_plaintext);
    for(int k=0;k<=hexPtextLen;k++) 
    {
      if(hex_plaintext[k]>=65 && hex_plaintext[k]<=90)
        hex_plaintext[k]=hex_plaintext[k]+32;
    }

    const int blockCount = (hexPtextLen / BLOCK_SIZE)+((hexPtextLen % BLOCK_SIZE) != 0);
    char hexPtextBlocks[blockCount][BLOCK_SIZE+1];
    int bytePos = 0;
    for (int i = 0; i < blockCount; i++) 
    {
        int j;
        for (j = 0; j < BLOCK_SIZE; j++) 
        {
            hexPtextBlocks[i][j] = hex_plaintext[bytePos++];
        }
        hexPtextBlocks[i][j] = '\0';
    }

    int padding = (blockCount*BLOCK_SIZE)-(hexPtextLen);
    int padbit =48;
    if(padding != 0)
    {
        for (int i=(BLOCK_SIZE-padding); i<BLOCK_SIZE; i++) 
        {
            hexPtextBlocks[blockCount-1][i] = (char)padbit;
        }
    }

    const int Len = (int) strlen(key);
    char hexKey[(Len*2)+1];
    string2hexString(key, hexKey);
    const int keyLen = (int) strlen(hexKey);
    for(int k=0;k<=keyLen;k++)
    {
      if(hexKey[k]>=65 && hexKey[k]<=90)
        hexKey[k]=hexKey[k]+32;
    }

    const char IV[] = "12341234";
    const int Length = (int) strlen(IV);
    char hexIV[(Length*2)+1];
    string2hexString(IV, hexIV);

    char hexCtextBlocks[blockCount][BLOCK_SIZE+1];
    for (int i = 0; i < blockCount; i++) 
    {
        char tempStore[BLOCK_SIZE];
        if (i == 0) 
            xor_byte_arrays(hexIV, hexPtextBlocks[i], tempStore);
        else 
            xor_byte_arrays(hexCtextBlocks[i-1], hexPtextBlocks[i], tempStore);

        char* temp;
        temp = encrypt(tempStore, hexKey);
        sprintf(hexCtextBlocks[i], "%s", temp);
    }

    char cipher[blockCount*BLOCK_SIZE + 1];
    int count = 0;
    for (int i = 0; i < blockCount; i++) 
    {
        for (int j = 0; j < BLOCK_SIZE; j++) 
        {
            cipher[count++] = hexCtextBlocks[i][j];
        }
    }
    cipher[count] = '\0';
    char* returnCipher;
    returnCipher = cipher;
    return (returnCipher);
}

char* presentCBCdecr(char* ctext, char* key)
{
    const int hexCtextLen = (int) strlen(ctext);
    char hexciphertext[hexCtextLen+1];
    sprintf(hexciphertext, "%s", ctext);

    const int blockCount = (hexCtextLen / BLOCK_SIZE)+((hexCtextLen % BLOCK_SIZE) != 0);
    char hexCtextBlocks[blockCount][BLOCK_SIZE+1];
    char hexPtextBlocks[blockCount][BLOCK_SIZE+1];
    int bytePos = 0;
    for (int i = 0; i < blockCount; i++) 
    {
        int j;
        for (j = 0; j < BLOCK_SIZE; j++) 
        {
            hexCtextBlocks[i][j] = hexciphertext[bytePos++];
        }
        hexCtextBlocks[i][j] = '\0';
    }

    const int Len = (int) strlen(key);
    char hexKey[(Len*2)+1];
    string2hexString(key, hexKey);
    const int keyLen = (int) strlen(hexKey);
    for(int k=0;k<=keyLen;k++)
    {
      if(hexKey[k]>=65 && hexKey[k]<=90)
        hexKey[k]=hexKey[k]+32;
    }

    const char IV[] = "12341234";
    const int Length = (int) strlen(IV);
    char hexIV[(Length*2)+1];
    string2hexString(IV, hexIV);

    char cipherStore[BLOCK_SIZE+1];
    for (int i = 0; i < blockCount; ++i) 
    {
        char tempStore[BLOCK_SIZE+1];
        char plainStore[BLOCK_SIZE+1];
        char* temp;
        temp = decrypt(hexCtextBlocks[i], hexKey);
        sprintf(tempStore, "%s", temp);

        if (i == 0) 
        {
            xor_byte_arrays(hexIV, tempStore, plainStore);
        } 
        else 
        {
            xor_byte_arrays(cipherStore, tempStore, plainStore);
        }
        strcpy(cipherStore, hexCtextBlocks[i]);
        strcpy(hexPtextBlocks[i], plainStore);
    }

    char resultHexText[blockCount*BLOCK_SIZE + 1];
    int count = 0;
    int padposition;
    for (int i = 0; i < blockCount; i++) 
    {
        for (int j = 0; j < BLOCK_SIZE; j++) 
        {
            if(hexPtextBlocks[i][j] == 48 && hexPtextBlocks[i][j-1] == 48)
            {
                resultHexText[count-1] = (char)'\0';
                resultHexText[count++] = (char)'\0';
            }
            else
            {
                resultHexText[count++] = hexPtextBlocks[i][j];
            }
        }
    }

    char* returnText;
    returnText = resultHexText;
    return textFromHexString(returnText);
}