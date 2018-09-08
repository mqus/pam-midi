#include <alsa/asoundlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/poll.h>
#include <stdbool.h>


// create a new client
snd_seq_t* open_client()
{
	snd_seq_t *handle;
	int err;
	err = snd_seq_open(&handle, "default", SND_SEQ_OPEN_INPUT, 0);
	if (err < 0)
		return NULL;
	snd_seq_set_client_name(handle, "pam-midi");
	return handle;
}
// create a new port; return the port id
// port will be writable and accept the write-subscription.
int my_new_port(snd_seq_t *handle)
{
	// if (snd_seq_open(&handle, "default", SND_SEQ_OPEN_INPUT, 0) < 0) {
	//	 fprintf(stderr, "Error opening ALSA sequencer.\n");
	//	 exit(1);
	// }

	return snd_seq_create_simple_port(handle, "pam-midi Port 0",
			SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
			SND_SEQ_PORT_TYPE_APPLICATION|SND_SEQ_PORT_TYPE_APPLICATION);
}

void capture_keyboard(snd_seq_t *seq, int cid, int pid)
{
	snd_seq_addr_t sender, dest;
	snd_seq_port_subscribe_t *subs;
	sender.client = 64;
	sender.port = 0;
	dest.client = cid;
	dest.port = pid;
	snd_seq_port_subscribe_alloca(&subs);
	snd_seq_port_subscribe_set_sender(subs, &sender);
	snd_seq_port_subscribe_set_dest(subs, &dest);
	snd_seq_port_subscribe_set_queue(subs, 1);
	snd_seq_port_subscribe_set_time_update(subs, 1);
	snd_seq_port_subscribe_set_time_real(subs, 1);
	snd_seq_subscribe_port(seq, subs);
}

static bool _is_eof(const u_int8_t* chord){ //C3 (60=7*8+4) is pressed only
	printf("%d,%d %d\n",chord[7],(chord[7]==16),!(chord[0]||chord[1]||chord[2]||chord[3]||chord[4]||chord[6]||chord[7]));
	return (chord[7]==16) && !(chord[0]||chord[1]||chord[2]||chord[3]||chord[4]||chord[5]||chord[6]
						||chord[8]||chord[9]||chord[10]||chord[11]||chord[12]||chord[13]||chord[14]||chord[15]);
}

static void _print_chord(const u_int8_t* chord){
	for(int i=0;i<16;++i){
		for(int i2=0;i2<8;++i2)
			if(1&(chord[i]>>i2))
				printf("%d %d | ",i,i2);
	}
	printf("\n");
	for(int i=0;i<16;++i){
		for(int i2=0;i2<8;++i2)
			if(1&(chord[i]>>i2))
				printf("O");
			else
				printf("-");
	}
	printf("\n");
}

static bool _was_saved=false;
static u_int8_t _last_chord[16]={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
static void _save_chord(){
	_print_chord(_last_chord);
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



bool midi_action(snd_seq_t *seq_handle) {
	bool eof=false;
	snd_seq_event_t *ev;
	do {
		snd_seq_event_input(seq_handle, &ev);
		switch (ev->type) {
		case SND_SEQ_EVENT_NOTEON:
			if (ev->data.note.velocity){
				// fprintf(stderr, "Note On event on Channel %2d: %5d, %d       \n",
				// 	ev->data.control.channel, ev->data.note.note, ev->data.note.velocity);
				pam_midi_add_note(ev->data.note.note);
				break;
			}//else fallthrough because no velocity =^= noteoff event
		case SND_SEQ_EVENT_NOTEOFF:
			// fprintf(stderr, "Note Off event on Channel %2d: %5d      \n",	 
			// 	ev->data.control.channel, ev->data.note.note);	   
			eof=pam_midi_del_note(ev->data.note.note))

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
static void check_snd(const char *operation, int err)
{
	if (err < 0)
		printf("Cannot %s - %s\n", operation, snd_strerror(err));
}

int main(int argc,char** argv)
{
	snd_seq_t * seq = open_client();
	int npfd;
	struct pollfd *pfd;

	int cid=snd_seq_client_id(seq);
	int pid=my_new_port(seq);
	//connect to 24:0
	int e1=snd_seq_connect_from(seq, pid, 24, 0);
	check_snd("connect", e1);
	int e2=snd_seq_nonblock(seq, 1);
	//int i=snd_seq_alloc_named_queue(seq, "my queue");
	//capture_keyboard(seq,cid,pid);
	printf("%d %d\n",e1,e2);
	printf("%d:%d\n",cid,pid);
	npfd = snd_seq_poll_descriptors_count(seq, POLLIN);
	pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
	snd_seq_poll_descriptors(seq, pfd, npfd, POLLIN);

	while (true) {
		if (poll(pfd, npfd, 100000) > 0) {
			bool eof=midi_action(seq);
			if(eof){
				return 0;
			}
		}
	}
}

