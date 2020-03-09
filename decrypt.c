#include"miracl.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"sm3.h"
#include"kdf.h"

static epoint *C1;
static unsigned char C2[100];
static int klen;
static unsigned char C3[33];

static int trans(char ch)
{
	if (ch>='0' && ch<='9')
		return ch-'0';
	if (ch>='A' && ch<='F')
		return ch-'A'+10;
	if (ch>='a' && ch<='f')
		return ch-'a'+10;
}

static void init(big h)
{
 	char str[100];
        printf("C1_x: ");
        scanf("%s", str);
        big x1 = mirvar(0);
        cinstr(x1, str);
	printf("C1_y: ");
	scanf("%s", str);
	big y1 = mirvar(0);
	cinstr(y1, str);
	C1 = epoint_init();
	if (epoint_set(x1,y1,0,C1)==FALSE)
	{
		mirkill(x1);mirkill(y1);epoint_free(C1);
		fprintf(stderr,"C1不满足曲线\n");
		exit(1);
	}	
        epoint *O = epoint_init();
	epoint *S = epoint_init();
	ecurve_mult(h, C1, S);		
	if (epoint_comp(S,O)==TRUE)
	{
		mirkill(x1);mirkill(y1);epoint_free(C1);
		fprintf(stderr,"S=O");
		exit(1);
	}
	mirkill(x1);mirkill(y1);
	
	
	printf("C2: ");
	scanf("%s", str);
	int len = strlen(str);
	for (int i=0;i<len;i+=2)
		C2[i/2] = trans(str[i])*16+trans(str[i+1]);
	klen = len*4;
	
	printf("C3: ");
	scanf("%s", str);
	for (int i=0;i<64;i+=2)
		C3[i/2] = trans(str[i])*16+trans(str[i+1]);
	C3[32] = 0;
}

void decrypt(epoint *G, big dB, big h)
{
	printf("------decryption------\n");
	init(h);
	epoint *tmp = epoint_init();
	ecurve_mult(dB, C1, tmp);
	big x2 = mirvar(0); big y2 = mirvar(0);
	epoint_get(tmp,x2,y2);
	unsigned char x2b[32],y2b[32];
        big_to_bytes(32, x2, x2b, TRUE);
        big_to_bytes(32, y2, y2b, TRUE);
	unsigned char *t = (char*)malloc(64);
        for (int i=0;i<32;i++)
                t[i] = x2b[i];
        for (int i=32;i<64;i++)
                t[i] = y2b[i-32];
        unsigned char *tt = KDF(t, klen);
	unsigned char M[100];
	for (int i=0;i<klen/8;i++)
		M[i] = C2[i]^tt[i];

      	//u = Hash(x2||M||y2)
        t =(char*)realloc(t, 64+klen/8);
        for (int i=0;i<32;i++)
                t[i] = x2b[i];
        for (int i=32;i<32+klen/8;i++)
                t[i] = M[i-32];
        for (int i=32+klen/8;i<64+klen/8;i++)
                t[i] = y2b[i-32-klen/8];
	unsigned char u[33];
	sm3(t, 64+klen/8, u);
	u[32]=0;
	if (strcmp(u, C3))
	{
		printf("u: ");
		for (int i=0;i<32;i++)
			printf("%02x",u[i]);
		putchar('\n');
		printf("u<>C3\n");
	}
	else
	{
		printf("M: ");
		for (int i=0;i<klen/8;i++)
			printf("%02x",M[i]);
		putchar('\n');
		M[klen/8] = 0;
		printf("%s\n", M);
		putchar('\n');
	}
	mirkill(x2);mirkill(y2);
	epoint_free(tmp);
	free(t);free(tt);
}
