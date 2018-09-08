#include <sodium.h>
#include <stdbool.h>

//define easily interchangable hash functions
//(watch out(!!), already saved passwords can be invalid when changing this!)
#define PM_HASH_SIZE crypto_pwhash_argon2i_STRBYTES
#define PM_GENERATE_HASH(hash, passwd, passwd_len) \
	crypto_pwhash_argon2i_str(hash,passwd,passwd_len,4,32*1024*1024);
#define PM_VERIFY_HASH(hash, passwd, passwd_len) \
	crypto_pwhash_str_verify(hash,passwd,passwd_len);

//The default file for the saved password hash (%s will be replaced with user)
#define PM_PASSWD_DEFAULT "/etc/pam-midi/%s"













static char * pm_passwd_file = PM_PASSWD_DEFAULT

int pam_midi_store_password(const char* password, username, int pw_len, bool overwrite, bool free_pw){
	//generate hash
	char hash[HASH_SIZE];
	int v=PM_GENERATE_HASH(hash,password,pw_len)
	//erase and free password
	if(free_pw){
		for(int i=0;i<pw_len;++i)
			password[i]=0
		free(password)
	}
	if(v!=0)
		return -1
	
	//check if file already exists and if it is writable
		//check if it should overwrite the file if it exists and is writable
	
	//write hash to file
	
	//close file
}

int pam_midi_check_password(const char* password, username, int pw_len, bool free_pw){
	char hash[PM_HASH_SIZE];
	//check if file exists and is readable
	
	//open file
	
	//read hash
	
	//verify password against the hash
	if(PM_VERIFY_HASH(hash,password,pw_len)!=0)
		return -1;
	//erase and free password
	if(free_pw){
		for(int i=0;i<pw_len;++i)
			password[i]=0
		free(password)
	}

	//close file

}

int pam_midi_set_passwd_file(const char* filename){
	pm_passwd_file = filename;
}