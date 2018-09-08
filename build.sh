gcc -fPIC -O2 -fno-stack-protector -c src/pam_midi.c 

sudo ld -x --sort-common --shared -o /lib/security/pam_midi.so pam_midi.o