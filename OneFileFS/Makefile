obj-m := onefilefs.o
simplefs-objs := onefilefs.o

all:
	gcc onefilemakefs.c -o onefilemakefs
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm onefilemakefs