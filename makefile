CFILES1 := pam_echo4.c
CFILES2 := pam_midi.c
TARGET1 := $(CFILES1:%.c=%.so)
TARGET2 := $(CFILES2:%.c=%.so)

OBJECTS := $(CFILES1:%.c=%.o) $(CFILES2:%.c=%.o)


CC     := gcc
CFLAGS := -fPIC -Wall -pedantic -std=c99 -O2
LINKER := ld
LFLAGS := -x --sort-common --shared




all:


obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $(input) -o $(output)

target/%.so: obj/%.o
	$(LINKER) $(LFLAGS) $(input) -o $(output)

$(TARGET1): $(CFILES1:%.c=%.o)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)
$(TARGET2): $(CFILES2:%.c=%.o)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $< -c

clean:
	$(RM) $(RMFLAGS) $(TARGET2) $(TARGET1) $(CFILES1:%.c=%.o) $(CFILES2:%.c=%.o)