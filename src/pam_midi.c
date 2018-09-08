#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

#define PM_FILE_DEFAULT "/var/lib/pam-midi/%s"
#define PM_DEVICE_DEFAULT "24:0"


#define PAM_SM_AUTH
#define PAM_SM_PASSWORD
#define PAM_SM_ACCOUNT
#define PAM_SM_SESSION

#include <security/pam_modules.h>
#include <security/pam_modutil.h>
#include <security/_pam_macros.h>
#include <security/pam_ext.h>



static char* file = PM_FILE_DEFAULT;
static char* midi_dev = PM_DEVICE_DEFAULT;

static int parse_args(int argc, const char **argv){
	for (; argc-- > 0; ++argv){
		if (!strncmp (*argv, "keyfile=", 8))
			file = (8 + *argv);
		else if (!strncmp (*argv, "midi_dev=", 9))
			midi_dev = (9 + *argv);
	}

}



int
pam_sm_authenticate (pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	char* username;
	r = pam_get_user(pamh, &username, NULL);
	if (r != PAM_SUCCESS)
		return PAM_AUTHINFO_UNAVAIL;

	
	
	return pam_echo (pamh, flags, argc, argv);
}

int
pam_sm_setcred (pam_handle_t *pamh UNUSED, int flags UNUSED, int argc UNUSED, const char **argv UNUSED)
{
	return PAM_IGNORE;
}

int
pam_sm_acct_mgmt (pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	return PAM_IGNORE;
}


int
pam_sm_chauthtok (pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	if (flags & PAM_PRELIM_CHECK)
		return PAM_AUTHTOK_DISABLE_AGING;
	//ignore if PAM_CHANGE_EXPIRED_AUTHTOK is set
	if (flags & PAM_PRELIM_CHECK)
		return PAM_SUCCESS;

	//if (flags & PAM_UPDATE_AUTHTOK)
		return PAM_IGNORE;
	//get current password
	//confirm password

	//get new password
	//get new password again

	//?
	
}


//not neccessary for this type of module
int
pam_sm_open_session (pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	return PAM_IGNORE;
}

int
pam_sm_close_session (pam_handle_t *pamh UNUSED, int flags UNUSED, int argc UNUSED, const char **argv UNUSED)
{
	return PAM_IGNORE;
}

