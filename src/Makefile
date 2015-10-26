# Répertoire des sources du noyau
SOURCES := /usr/src/linux-headers-3.16.0-4-common O=/usr/src/linux-headers-3.16.0-4-amd64

# Objets à compiler
obj-m := faircrave.o
faircrave-objs := tools.o hooks.o scheduler.o control.o main.o

# Flags communs à chaque objet
CFLAGS_commons := -s -O2 -Wall

# Flags propres à chaque objet
CFLAGS_tools.o := $(CFLAGS_commons) -D __FILENAME__=\"tools.c\"
CFLAGS_hooks.o := $(CFLAGS_commons) -D __FILENAME__=\"hooks.c\"
CFLAGS_scheduler.o := $(CFLAGS_commons) -D __FILENAME__=\"scheduler.c\"
CFLAGS_control.o := $(CFLAGS_commons) -D __FILENAME__=\"control.c\"
CFLAGS_main.o := $(CFLAGS_commons) -D __FILENAME__=\"main.c\"

module:
	make -C $(SOURCES) M=$(PWD) modules
	mv faircrave.ko ../bin/

clean:
	make -C $(SOURCES) M=$(PWD) clean
