# Version du noyau et architecture cible
VERSION := Linux-3.14.33

# Objets à compiler
obj-m := faircrave.o
faircrave-objs := tools.o hooks.o scheduler.o control.o main.o

# Flags communs à chaque object
CFLAGS_commons := -s -O0 -Wall

# Flags pour chaque objet (ajout du nom)
CFLAGS_tools.o := $(CFLAGS_commons) -D __FILENAME__=\"tools.c\"
CFLAGS_hooks.o := $(CFLAGS_commons) -D __FILENAME__=\"hooks.c\"
CFLAGS_scheduler.o := $(CFLAGS_commons) -D __FILENAME__=\"scheduler.c\"
CFLAGS_control.o := $(CFLAGS_commons) -D __FILENAME__=\"control.c\"
CFLAGS_main.o := $(CFLAGS_commons) -D __FILENAME__=\"main.c\"

module:
	make -C /usr/src/$(VERSION) M=$(PWD) modules
	mv faircrave.ko ../bin/

clean:
	make -C /usr/src/$(VERSION) M=$(PWD) clean
