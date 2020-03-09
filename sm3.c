#include"miracl.h"
#include<string.h>
#include"sm3.h"
#define AND 0
#define OR 3
#define XOR 2
#define PLUS 1
#define MAXN 300

static int fill(big m, int l)
//消息填充
{
	//填充1到末尾
	sftbit(m, 1, m);
        add(m, mirvar(1), m);
	//填充k个0
	int k = ((447-l)%512+512)%512;
	sftbit(m, k, m);
	//填充64位比特串
	sftbit(m, 64 ,m);
	add(m, mirvar(l), m);
	return (l+k+65)/512;
}

static void leftshift(big a, int len, big ans)
//32位的a 循环左移len位
{
	len %= 32;
	big x = mirvar(0);
	big b = mirvar(0);
	expb2(32-len, x);
	copy(a, ans);
	divide(ans, x, b);
	sftbit(ans, len, ans);
	add(ans, b, ans);
	mirkill(x);mirkill(b);
}

static void inverse(big x, big ans)
//对32位的x进行取反位操作
{
	char xb[4];
	big_to_bytes(4, x, xb, TRUE);
	for (int i=0;i<4;i++)
		xb[i] = ~xb[i];
	bytes_to_big(4, xb, ans);
}

static void operation(big a, big b, int op, big ans)
//对32位的a和b进行操作
{
	if (op == PLUS)
	{
		big x = mirvar(0);expb2(32, x);
		add(a, b, ans);
		divide(ans, x, x);
		mirkill(x);
		return;
	}
	char ab[4], bb[4];
        big_to_bytes(4, a, ab, TRUE);
        big_to_bytes(4, b, bb, TRUE);
	switch(op){
		case XOR:for (int i=0;i<4;i++) ab[i] = ab[i] ^ bb[i];break;
		case AND:for (int i=0;i<4;i++) ab[i] = ab[i] & bb[i];break;
		case OR:for (int i=0;i<4;i++) ab[i] = ab[i] | bb[i];break;
	}
        bytes_to_big(4, ab, ans);
}

static void P(int k, big x, big ans)
/*置换函数
P0(X) = X xor (X <<< 9) xor (X<<<17)
P1(X) = X xor (X <<< 15) xor (X<<<23)*/
{
	big a = mirvar(0);
	big b = mirvar(0);
	copy(x, a);
	copy(x, b);
	if (k == 1)
	{
		leftshift(a, 15, a);
		leftshift(b, 23, b);
	}
	else{
		leftshift(a, 9, a);
		leftshift(b, 17, b);
	}
	operation(x, a, XOR, ans);
	operation(ans, b, XOR, ans);
}

static void T(int j, big x)
{
	if (0<=j && j<=15)
		cinstr(x, "79cc4519");
	else if (16<=j && j<=63)
		cinstr(x, "7a879d8a");
}

static void expand(big B, big W[], big Wap[])
//填充
{
	big t[2];
	t[0] = mirvar(0);copy(B, t[0]);
	t[1] = mirvar(0);
	big x = mirvar(0);expb2(32, x);
	int l = 0;

	for (int i=15;i>=0;i--)
	{
		divide(t[l], x, t[1-l]);
		W[i] = mirvar(0);
		copy(t[l],W[i]);
		l=1-l;
	}
	//Wj = P1(W[j−16] xor W[j−9] xor (W[j−3]<<<15)) xor (W[j−13]<<<7) xor W[j−6]
	for (int i=16;i<=67;i++)
	{
		W[i] = mirvar(0);
		operation(W[i-16],W[i-9], XOR, W[i]);
		big t = mirvar(0);
		leftshift(W[i-3], 15, t);
		operation(W[i], t, XOR, W[i]);
		P(1, W[i], W[i]);
		leftshift(W[i-13], 7, t);
		operation(W[i], t, XOR, W[i]);
		operation(W[i], W[i-6], XOR, W[i]);
	}
	for (int i=0;i<=63;i++)
	{
		Wap[i] = mirvar(0);
		operation(W[i], W[i+4], XOR, Wap[i]);
	}
	
	mirkill(t[0]);mirkill(t[1]);mirkill(x);
}

static void FF(int j, big X, big Y, big Z, big ans)
{
	if (0<=j && j<=15)
	{
		operation(X, Y, XOR, ans);
		operation(ans, Z, XOR, ans);
	}
	else if (16<=j && j<=63)
	{
		//(X and Y ) or (X and Z) or (Y and Z )
		big t = mirvar(0);
		operation(X, Y, AND, ans);
		operation(X, Z, AND, t);
		operation(ans, t, OR, ans);
		operation(Y, Z, AND, t);
		operation(ans, t, OR, ans);
		mirkill(t);
	}
}

static void GG(int j, big X, big Y, big Z, big ans)
{
	if (0<=j && j<=15)
        {
                operation(X, Y, XOR, ans);
                operation(ans, Z, XOR, ans);
        }
        else if (16<=j && j<=63)
        {
		//(X and Y) or (not X and Z)
                big t = mirvar(0);
                operation(X, Y, AND, ans);
		inverse(X, t);
		operation(t, Z, AND, t);
		operation(ans, t, OR, ans);
		mirkill(t);
        }
}

static void compress(big W[], big Wap[], big V[])
//压缩
{
	big SS1,SS2,TT1,TT2,t;
	SS1 = mirvar(0);SS2 = mirvar(0);
	TT1 = mirvar(0);TT2 = mirvar(0);
	t = mirvar(0);
	big A,B,C,D,E,F,G,H;
	A = mirvar(0);copy(V[0], A);B = mirvar(0);copy(V[1], B);
	C = mirvar(0);copy(V[2], C);D = mirvar(0);copy(V[3], D);
	E = mirvar(0);copy(V[4], E);F = mirvar(0);copy(V[5], F);
        G = mirvar(0);copy(V[6], G);H = mirvar(0);copy(V[7], H);

	for (int i=0;i<=63;i++)
	{
		//SS1 = ((A<<<12) + E + (Tj<<<j))<<<7
		leftshift(A, 12, SS1);
		operation(SS1, E, PLUS, SS1);
		T(i, t);
		leftshift(t, i, t);
		operation(SS1, t, PLUS, SS1);
		leftshift(SS1, 7, SS1);	
		//SS2 = SS1 xor (A<<<12)
		leftshift(A, 12, SS2);
		operation(SS1, SS2, XOR, SS2);
		//TT1 = FFj(A,B,C) + D + SS2 +Wj′
		FF(i, A, B, C, TT1);
		operation(TT1, D, PLUS, TT1);
		operation(TT1, SS2, PLUS, TT1);
		operation(TT1, Wap[i], PLUS, TT1);
		//TT2 = GGj(E, F, G) + H + SS1 +Wj
		GG(i, E, F, G, TT2);
		operation(TT2, H, PLUS, TT2);
		operation(TT2, SS1, PLUS, TT2);
		operation(TT2, W[i], PLUS, TT2);
		//D=C; C=B<<<9; B=A; A=TT1; H=G; G=F<<<19; F=E; E=P0(TT2)
		copy(C,D);
		leftshift(B, 9, C);
		copy(A, B);
		copy(TT1, A);
		copy(G, H);
		leftshift(F, 19, G);
		copy(E, F);
		P(0, TT2, E);
		//printf("%2d: ", i);
		//print(A,B,C,D,E,F,G,H);
	}
	//V[i+1] = ABCDEFGH xor V[i]
	operation(A, V[0], XOR, V[0]);operation(B, V[1], XOR, V[1]);
	operation(C, V[2], XOR, V[2]);operation(D, V[3], XOR, V[3]);
	operation(E, V[4], XOR, V[4]);operation(F, V[5], XOR, V[5]);
        operation(G, V[6], XOR, V[6]);operation(H, V[7], XOR, V[7]);
	
	mirkill(A);mirkill(B);mirkill(C);mirkill(D);
	mirkill(E);mirkill(F);mirkill(G);mirkill(H);
	mirkill(SS1);mirkill(SS2);mirkill(TT1);mirkill(TT2);
	mirkill(t);
}

static void iteration(big m, int n, big V[])
//迭代
{
	big Bi[50], t[2], W[68], Wap[64];
        t[0] = mirvar(0);copy(m, t[0]);
	t[1] = mirvar(0);
        int l = 0;
	big x = mirvar(0);expb2(512, x);

        for (int i=n-1;i>=0;i--)
        {
                divide(t[l], x, t[1-l]);
                Bi[i] = mirvar(0);
                copy(t[l],Bi[i]);
                l=1-l;
        }
	
	for (int i=0;i<n;i++)
	{
		expand(Bi[i], W, Wap);
		compress(W, Wap, V);	
	}

	for (int i=0;i<n;i++)
		mirkill(Bi[i]);
	for (int i=0;i<68;i++)
		mirkill(W[i]);
	for (int i=0;i<64;i++)
		mirkill(Wap[i]);
	mirkill(t[0]);mirkill(t[1]);mirkill(x);
}

void sm3(char m[], int bytes, char ans[])
{
	big mb = mirvar(0);
	bytes_to_big(bytes, m, mb);
	int n = fill(mb, bytes*8);
	big V[8];
	for (int i=0;i<8;i++)
		V[i] = mirvar(0);
	cinstr(V[0], "7380166f"); cinstr(V[1], "4914b2b9"); cinstr(V[2], "172442d7");
	cinstr(V[3], "da8a0600"); cinstr(V[4], "a96f30bc"); cinstr(V[5], "163138aa");
	cinstr(V[6], "e38dee4d"); cinstr(V[7], "b0fb0e4e");
	iteration(mb, n, V);
	big x = mirvar(0);expb2(32, x);
	big ansb = mirvar(0);
	for (int i=0;i<8;i++)
	{
		multiply(ansb, x, ansb);
		add(ansb, V[i], ansb);
	}
	for (int i=0;i<8;i++)
		mirkill(V[i]);
	big_to_bytes(32, ansb, ans, TRUE);
	mirkill(x);
	mirkill(mb);
	mirkill(ansb);
}

