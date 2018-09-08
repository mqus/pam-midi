#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/poll.h>
#include <stdbool.h>
#include <string.h>
#include <sodium.h>

#include "get_chord_seq.h"

#define PM_HASH_SIZE crypto_pwhash_argon2i_STRBYTES
#define PM_GENERATE_HASH(hash, passwd, passwd_len) crypto_pwhash_argon2i_str(hash,passwd,passwd_len,4,32*1024*1024);
#define PM_VERIFY_HASH(hash, passwd, passwd_len) crypto_pwhash_argon2i_str_verify(hash,passwd,passwd_len);
int main(int argc, char **argv){
	if (sodium_init() == -1) {
		return 1;
	}
	char * s; //=  "MeinPasswort\0das ist egal";
	//const char * s2 = "MeinPasswort\0das ist mega";
	unsigned long len;

	int err = get_chord_sequence("24:0",&s, &len);
	char hash[PM_HASH_SIZE];
	//int c = crypto_pwhash_argon2i_str(hash,s,strlen(s),4,32*1024*1024);
	int c = PM_GENERATE_HASH(hash,s,len);
	
	int err2 = get_chord_sequence("24:0",&s, &len);
	int d = PM_VERIFY_HASH(hash,s,len);
	printf("%d %d %d %d %lu %lu:%s\n",err,c,err2,d,len,strlen(hash),hash);
}