

int pam_midi_store_password(const char* password, username, int pw_len, bool overwrite, bool free_pw);

int pam_midi_check_password(const char* password, username, int pw_len, bool free_pw);

int pam_midi_set_passwd_file(const char* filename);