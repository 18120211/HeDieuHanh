#cd /home/iris/ldd/phan_1/bai_1_3
#vim Makefile

KDIR = /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD)
	$(CC) test.c -o test
clean:
	make -C $(KDIR) M=$(PWD) clean
	rm test
