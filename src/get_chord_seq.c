#include <alsa/asoundlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/poll.h>
#include <stdbool.h>
#include "get_chord_seq.h"


#define CHECK_ERROR(msg, errorcode, returncode) \
	if(errorcode<0){\
		_output_err(msg, errorcode);\
		return returncode;\
	}

#define CHECK_ERROR_1(msg, errorcode) CHECK_ERROR(msg, errorcode, 1)


// create a new client
static int _open_client(snd_seq_t** handle_p)
{
	int err=0;
	err = snd_seq_open(handle_p, "default", SND_SEQ_OPEN_INPUT, 0);
	if (err < 0)
		return err;
	err = snd_seq_set_client_name(*handle_p, "pam-midi");
	return err;
}
// create a new port; return the port id
// port will be writable and accept the write-subscription.
static int _new_port(snd_seq_t *handle)
{
	return snd_seq_create_simple_port(handle, "pam-midi Port 0",
			SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
			SND_SEQ_PORT_TYPE_APPLICATION);
}


static bool _is_eof(const u_int8_t* chord){ //C3 (60=7*8+4) is pressed only
//	printf("%d,%d %d\n",chord[7],(chord[7]==16),!(chord[0]||chord[1]||chord[2]||chord[3]||chord[4]||chord[6]||chord[7]));
	return (chord[7]==16) && !(chord[0]||chord[1]||chord[2]||chord[3]||chord[4]||chord[5]||chord[6]
						||chord[8]||chord[9]||chord[10]||chord[11]||chord[12]||chord[13]||chord[14]||chord[15]);
}

static void _print_chord(const u_int8_t* chord, bool details){
	if(details){
		for(int i=0;i<16;++i){
			for(int i2=0;i2<8;++i2)
				if(1&(chord[i]>>i2))
					printf("%d %d | ",i,i2);
		}
		printf("\n");
	}
	for(int i=0;i<16;++i){
		for(int i2=0;i2<8;++i2)
			if(1&(chord[i]>>i2))
				printf("O");
			else
				printf("-");
	}
	printf("\n");
}

static void _print_chordbuf(const char* chord, unsigned long len){
	for(unsigned long i=0;i<len;i+=16){
		_print_chord((unsigned char*)chord+sizeof(char)*i,false);
	}
	printf("\n");
}

static bool _was_saved=false;
static u_int8_t _last_chord[16]={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};

static char* _chord_buffer=NULL;
static unsigned long _buf_cap=0;
static unsigned long _buf_size=0;
//WATCH OUT TODO CATCH ERROR
static void _save_chord(){
	//realloc buffer if neccessary
	if(_buf_cap==_buf_size){
		//double capacity or set to 4
		_buf_cap = (_buf_cap==0) ? 4*16 : (2*_buf_cap);
		_chord_buffer = realloc(_chord_buffer,_buf_cap*sizeof(char));
	}
	//append the current chord to the buffer
#define PM_CP(i) _chord_buffer[_buf_size+i] = (char) _last_chord[i];
	PM_CP(0)
	PM_CP(1)
	PM_CP(2)
	PM_CP(3)
	PM_CP(4)
	PM_CP(5)
	PM_CP(6)
	PM_CP(7)
	PM_CP(8)
	PM_CP(9)
	PM_CP(10)
	PM_CP(11)
	PM_CP(12)
	PM_CP(13)
	PM_CP(14)
	PM_CP(15)

	_buf_size +=16;
	_print_chord(_last_chord, false);
	//_print_chordbuf(_chord_buffer, _buf_size);
}


static void _add_note(u_int8_t n){
	int i1=n%8;
	int i2=n/8;
	_last_chord[i2] = _last_chord[i2] | (u_int8_t)(1<<i1);
	_was_saved=false;
}

static bool _del_note(u_int8_t n){
	bool eof=false;
	if(!_was_saved){
		if(_is_eof(_last_chord))
			eof=true;
		else
			_save_chord();
	}
	_was_saved=1;
	u_int8_t i1=n%8;
	u_int8_t i2=n/8;
	_last_chord[i2] = _last_chord[i2] & (u_int8_t)(255 - (1<<i1));
	return eof;
}



static bool _process_midi_action(snd_seq_t *seq_handle) {
	bool eof=false;
	snd_seq_event_t *ev;
	do {
		snd_seq_event_input(seq_handle, &ev);
		switch (ev->type) {
		case SND_SEQ_EVENT_NOTEON:
			if (ev->data.note.velocity){
				// fprintf(stderr, "Note On event on Channel %2d: %5d, %d       \n",
				// 	ev->data.control.channel, ev->data.note.note, ev->data.note.velocity);
				_add_note(ev->data.note.note);
				break;	
			}//else fallthrough because no velocity =^= noteoff event
		case SND_SEQ_EVENT_NOTEOFF:
			
			// fprintf(stderr, "Note Off event on Channel %2d: %5d      \n",	 
			// 	ev->data.control.channel, ev->data.note.note);	   
			eof = _del_note(ev->data.note.note);
		break;
		// default:
		// 	//fprintf(stderr, "Nix da      \n");	   
		// break;
		}
		snd_seq_free_event(ev);
	} while (!eof && snd_seq_event_input_pending(seq_handle, 0) > 0);
	return eof;
}

/* error handling for ALSA functions */
static void _output_err(const char *operation, int err)
{
	printf("Cannot %s - %s\n", operation, snd_strerror(err));
}

int get_chord_sequence(const char* midi_dev,char** buf, unsigned long* bufsize)
{
	int err;
	_buf_size=0;
	*bufsize = _buf_size;
	snd_seq_t * seq ;
	err=_open_client(&seq);
	CHECK_ERROR_1("create client", err);
	int npfd;
	struct pollfd *pfd;

	//int alsa_device,alsa_port
	snd_seq_addr_t addr;
	err =snd_seq_parse_address(seq, &addr, midi_dev);
	CHECK_ERROR_1("parse address", err);


	//int cid=snd_seq_client_id(seq);
	int pid=_new_port(seq);
	CHECK_ERROR_1("create port",pid);
	
	err=snd_seq_connect_from(seq, pid, addr.client, addr.port);
	CHECK_ERROR_1("connect port(couldn't find device)", err);
	err=snd_seq_nonblock(seq, 1);
	CHECK_ERROR_1("set seq handle to nonblocking", err);
	

	npfd = snd_seq_poll_descriptors_count(seq, POLLIN);
	pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
	err=snd_seq_poll_descriptors(seq, pfd, npfd, POLLIN);
	CHECK_ERROR_1("set poll descriptors", err);
	
	while (true) {
		if (poll(pfd, npfd, 100000) > 0) {
			bool eof=_process_midi_action(seq);
			if(eof){
				*buf = _chord_buffer;
				*bufsize = _buf_size;
				return 0;
			}
		}
	}
}

