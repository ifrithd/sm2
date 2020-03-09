obj = main.o sm3.o kdf.o encrypt.o decrypt.o
cc = gcc
sm2 : $(obj)
	cc -o sm2 $(obj) -L. miracl.a
main.o : 
kdf.o : sm3.h
sm3.o :  
encrypt.o : sm3.h kdf.h
decrypt.o : sm3.h kdf.h
.PHONY: clean
clean:
	-rm sm2 $(obj)
