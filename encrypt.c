#include"miracl.h"
#include"sm3.h"
#include"encrypt.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"kdf.h"

static unsigned char M[100];
static int klen;

static void init()
{
	printf("明文: ");
	fgets(M,100,stdin);
	*strchr(M,'\n') = '\0';
	klen = 8*strlen(M);
}

void encrypt(epoint *G, big dB)
{
	printf("------encryption------\n");
	init();
	big k = mirvar(3);
	//C1=KG=(x1,y1)
	epoint *C1 = epoint_init();
	ecurve_mult(k, G, C1);
	big x1 = mirvar(0);
	big y1 = mirvar(0);
	epoint_get(C1, x1, y1);
	printf("C1_x: ");cotnum(x1, stdout);
	printf("C1_y: ");cotnum(y1, stdout);
	//PB=dBG;kPB=(x2,y2)
	epoint *PB = epoint_init();
	ecurve_mult(dB, G, PB);
	ecurve_mult(k, PB, PB);
	big x2 = mirvar(0);
	big y2 = mirvar(0);
	epoint_get(PB, x2, y2);
	unsigned char x2b[32],y2b[32];
	big_to_bytes(32, x2, x2b, TRUE);
	big_to_bytes(32, y2, y2b, TRUE);
	//C2 = M xor t
	unsigned char *t = (char*)malloc(64);
	for (int i=0;i<32;i++)
		t[i] = x2b[i];
	for (int i=32;i<64;i++)
		t[i] = y2b[i-32];
	unsigned char *tt = KDF(t, klen);
	unsigned char *C2 = (unsigned char *)malloc(klen/8);
	for (int i=0;i<klen/8;i++)
		C2[i] = M[i]^tt[i];
	printf("C2: ");
	for (int i=0;i<klen/8;i++)
		printf("%02x",C2[i]);
	putchar('\n');
	//C3 = Hash(x2 || M || y2)
	t =(char*)realloc(t, 64+klen/8);
	for (int i=0;i<32;i++)
		t[i] = x2b[i];
	for (int i=32;i<32+klen/8;i++)
		t[i] = M[i-32];
	for (int i=32+klen/8;i<64+klen/8;i++)
		t[i] = y2b[i-32-klen/8];

	unsigned char C3[32];
	sm3(t, 64+klen/8, C3);
	printf("C3: ");
	for (int i=0;i<32;i++)
		printf("%02x", C3[i]);
	putchar('\n');
	//释放空间
	mirkill(k);
	epoint_free(C1);
	epoint_free(PB);
	mirkill(x1);mirkill(y1);
	mirkill(x2);mirkill(y2);
	free(t);free(tt);free(C2);
}
