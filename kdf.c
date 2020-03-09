#include"sm3.h"
#include"kdf.h"
#include<stdlib.h>
#include<string.h>
#include<stdio.h>

char * KDF(unsigned char Z[64], int klen)
{
	int ct = 1;
	unsigned char t[68];
	memcpy(t, Z, 64);
	int v = 256;
	int times = klen/v;
	if (klen%v != 0) times++;
	char H[32];
	char *K = (char*)malloc(times*32);
	for (int i=0;i<times;i++)
	{
		for (int j=0;j<4;j++)
			t[64+j] = (ct>>(8*(3-j)))&0xff;
		sm3(t, 68, H);
		ct++;
		memcpy(K+32*i, H, 32);
	}
	return K;
}
