#include<stdio.h>
#include"miracl.h"
#include"encrypt.h"
#include"decrypt.h"
#include<stdlib.h>

int main()
{
	//初始化
    	miracl *mip = mirsys(512*3 ,10);
	mip->IOBASE = 16;
	big p = mirvar(0);
	cinstr(p, "FFFFFFFE" "FFFFFFFF" "FFFFFFFF" "FFFFFFFF"
		  "FFFFFFFF" "00000000" "FFFFFFFF" "FFFFFFFF");
	big a = mirvar(0);
	cinstr(a, "FFFFFFFE" "FFFFFFFF" "FFFFFFFF" "FFFFFFFF"
		  "FFFFFFFF" "00000000" "FFFFFFFF" "FFFFFFFC");
	big b = mirvar(0);
	cinstr(b, "28E9FA9E" "9D9F5E34" "4D5A9E4B" "CF6509A7" 
		  "F39789F5" "15AB8F92" "DDBCBD41" "4D940E93");
	big n = mirvar(0);
	cinstr(n, "FFFFFFFE" "FFFFFFFF" "FFFFFFFF" "FFFFFFFF"
		  "7203DF6B" "21C6052B" "53BBF409" "39D54123");
	big Gx = mirvar(0);
	cinstr(Gx, "32C4AE2C" "1F198119" "5F990446" "6A39C994"
		   "8FE30BBF" "F2660BE1" "715A4589" "334C74C7");
	big Gy = mirvar(0);
	cinstr(Gy, "BC3736A2" "F4F6779C" "59BDCEE3" "6B692153"
		   "D0A9877C" "C62A4740" "02DF32E5" "2139F0A0");
	big h = mirvar(1);
	ecurve_init(a, b, p, MR_PROJECTIVE);
	epoint *G = epoint_init();
	epoint_set(Gx, Gy, 0, G);
	big dB = mirvar(2);
	//
	encrypt(G,dB);
	decrypt(G,dB,h);
	//释放空间
	mirkill(a);mirkill(b);
	mirkill(n);
	mirkill(Gx);mirkill(Gy);
	mirkill(h);
	epoint_free(G);
	mirkill(dB);
	mirexit();
	return 0;
}
