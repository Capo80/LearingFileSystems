obj-m := basic_fs_src.o
simplefs-objs := basic_fs.o

all:
	gcc basicmakefs.c -o basicmakefs
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm basicmakefs