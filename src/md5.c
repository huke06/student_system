/*
 * md5.c - MD5消息摘要算法实现
 * 符合RFC 1321标准，用于密码加密存储
 *
 * 参考：RSA Data Security, Inc. MD5 Message-Digest Algorithm
 */

#include <stdio.h>
#include <string.h>
#include "utils.h"

/* MD5上下文结构体，保存中间状态 */
struct md5_ctx {
    unsigned int state[4];      /* A,B,C,D四个状态寄存器 */
    unsigned int count[2];      /* 已处理数据的比特数(64位计数器) */
    unsigned char buffer[64];   /* 待处理的512位数据块 */
};

/* ===== 常量定义 ===== */

/* 每轮移位量 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

/* ===== 基本位操作宏 ===== */
#define F(x,y,z) (((x)&(y))|((~x)&(z)))
#define G(x,y,z) (((x)&(z))|((y)&(~z)))
#define H(x,y,z) ((x)^(y)^(z))
#define I(x,y,z) ((y)^((x)|(~z)))

/* 循环左移 */
#define ROTATE_LEFT(x,n) (((x)<<(n))|((x)>>(32-(n))))

/*
 * MD5单步变换宏
 * 对a执行: a=b+((a+F(b,c,d)+x+ac)<<<s)
 */
#define FF(a,b,c,d,x,s,ac) { \
    (a)+=F((b),(c),(d))+(x)+(unsigned int)(ac); \
    (a)=ROTATE_LEFT((a),(s)); \
    (a)+=(b); \
}
#define GG(a,b,c,d,x,s,ac) { \
    (a)+=G((b),(c),(d))+(x)+(unsigned int)(ac); \
    (a)=ROTATE_LEFT((a),(s)); \
    (a)+=(b); \
}
#define HH(a,b,c,d,x,s,ac) { \
    (a)+=H((b),(c),(d))+(x)+(unsigned int)(ac); \
    (a)=ROTATE_LEFT((a),(s)); \
    (a)+=(b); \
}
#define II(a,b,c,d,x,s,ac) { \
    (a)+=I((b),(c),(d))+(x)+(unsigned int)(ac); \
    (a)=ROTATE_LEFT((a),(s)); \
    (a)+=(b); \
}

/* ===== 内部函数 ===== */

/*
 * 将unsigned char数组按小端序解码为unsigned int数组
 * input: 输入字节数组(64字节)
 * output: 输出整数数组(16个unsigned int)
 */
static void decode(unsigned int* output, const unsigned char* input)
{
    int i, j;
    for(i=0, j=0; j<64; i++, j+=4) {
        output[i]=((unsigned int)input[j])
                | (((unsigned int)input[j+1])<<8)
                | (((unsigned int)input[j+2])<<16)
                | (((unsigned int)input[j+3])<<24);
    }
}

/*
 * 将unsigned int数组按小端序编码为unsigned char数组
 * input: 输入整数数组
 * output: 输出字节数组
 */
static void encode(unsigned char* output, const unsigned int* input, int len)
{
    int i, j;
    for(i=0, j=0; j<len; i++, j+=4) {
        output[j]=(unsigned char)(input[i]&0xff);
        output[j+1]=(unsigned char)((input[i]>>8)&0xff);
        output[j+2]=(unsigned char)((input[i]>>16)&0xff);
        output[j+3]=(unsigned char)((input[i]>>24)&0xff);
    }
}

/*
 * MD5核心变换：对64字节的数据块进行处理
 * state: 当前A,B,C,D状态值
 * block: 64字节数据块
 */
static void md5_transform(unsigned int state[4], const unsigned char block[64])
{
    unsigned int a, b, c, d;
    unsigned int x[16];

    decode(x, block);

    a=state[0];
    b=state[1];
    c=state[2];
    d=state[3];

    /* 第1轮 */
    FF(a,b,c,d,x[ 0],S11,0xd76aa478);
    FF(d,a,b,c,x[ 1],S12,0xe8c7b756);
    FF(c,d,a,b,x[ 2],S13,0x242070db);
    FF(b,c,d,a,x[ 3],S14,0xc1bdceee);
    FF(a,b,c,d,x[ 4],S11,0xf57c0faf);
    FF(d,a,b,c,x[ 5],S12,0x4787c62a);
    FF(c,d,a,b,x[ 6],S13,0xa8304613);
    FF(b,c,d,a,x[ 7],S14,0xfd469501);
    FF(a,b,c,d,x[ 8],S11,0x698098d8);
    FF(d,a,b,c,x[ 9],S12,0x8b44f7af);
    FF(c,d,a,b,x[10],S13,0xffff5bb1);
    FF(b,c,d,a,x[11],S14,0x895cd7be);
    FF(a,b,c,d,x[12],S11,0x6b901122);
    FF(d,a,b,c,x[13],S12,0xfd987193);
    FF(c,d,a,b,x[14],S13,0xa679438e);
    FF(b,c,d,a,x[15],S14,0x49b40821);

    /* 第2轮 */
    GG(a,b,c,d,x[ 1],S21,0xf61e2562);
    GG(d,a,b,c,x[ 6],S22,0xc040b340);
    GG(c,d,a,b,x[11],S23,0x265e5a51);
    GG(b,c,d,a,x[ 0],S24,0xe9b6c7aa);
    GG(a,b,c,d,x[ 5],S21,0xd62f105d);
    GG(d,a,b,c,x[10],S22,0x02441453);
    GG(c,d,a,b,x[15],S23,0xd8a1e681);
    GG(b,c,d,a,x[ 4],S24,0xe7d3fbc8);
    GG(a,b,c,d,x[ 9],S21,0x21e1cde6);
    GG(d,a,b,c,x[14],S22,0xc33707d6);
    GG(c,d,a,b,x[ 3],S23,0xf4d50d87);
    GG(b,c,d,a,x[ 8],S24,0x455a14ed);
    GG(a,b,c,d,x[13],S21,0xa9e3e905);
    GG(d,a,b,c,x[ 2],S22,0xfcefa3f8);
    GG(c,d,a,b,x[ 7],S23,0x676f02d9);
    GG(b,c,d,a,x[12],S24,0x8d2a4c8a);

    /* 第3轮 */
    HH(a,b,c,d,x[ 5],S31,0xfffa3942);
    HH(d,a,b,c,x[ 8],S32,0x8771f681);
    HH(c,d,a,b,x[11],S33,0x6d9d6122);
    HH(b,c,d,a,x[14],S34,0xfde5380c);
    HH(a,b,c,d,x[ 1],S31,0xa4beea44);
    HH(d,a,b,c,x[ 4],S32,0x4bdecfa9);
    HH(c,d,a,b,x[ 7],S33,0xf6bb4b60);
    HH(b,c,d,a,x[10],S34,0xbebfbc70);
    HH(a,b,c,d,x[13],S31,0x289b7ec6);
    HH(d,a,b,c,x[ 0],S32,0xeaa127fa);
    HH(c,d,a,b,x[ 3],S33,0xd4ef3085);
    HH(b,c,d,a,x[ 6],S34,0x04881d05);
    HH(a,b,c,d,x[ 9],S31,0xd9d4d039);
    HH(d,a,b,c,x[12],S32,0xe6db99e5);
    HH(c,d,a,b,x[15],S33,0x1fa27cf8);
    HH(b,c,d,a,x[ 2],S34,0xc4ac5665);

    /* 第4轮 */
    II(a,b,c,d,x[ 0],S41,0xf4292244);
    II(d,a,b,c,x[ 7],S42,0x432aff97);
    II(c,d,a,b,x[14],S43,0xab9423a7);
    II(b,c,d,a,x[ 5],S44,0xfc93a039);
    II(a,b,c,d,x[12],S41,0x655b59c3);
    II(d,a,b,c,x[ 3],S42,0x8f0ccc92);
    II(c,d,a,b,x[10],S43,0xffeff47d);
    II(b,c,d,a,x[ 1],S44,0x85845dd1);
    II(a,b,c,d,x[ 8],S41,0x6fa87e4f);
    II(d,a,b,c,x[15],S42,0xfe2ce6e0);
    II(c,d,a,b,x[ 6],S43,0xa3014314);
    II(b,c,d,a,x[13],S44,0x4e0811a1);
    II(a,b,c,d,x[ 4],S41,0xf7537e82);
    II(d,a,b,c,x[11],S42,0xbd3af235);
    II(c,d,a,b,x[ 2],S43,0x2ad7d2bb);
    II(b,c,d,a,x[ 9],S44,0xeb86d391);

    state[0]+=a;
    state[1]+=b;
    state[2]+=c;
    state[3]+=d;
}

/*
 * MD5初始化
 * 设置初始IV值
 */
static void md5_init(struct md5_ctx* ctx)
{
    ctx->count[0]=0;
    ctx->count[1]=0;

    /* MD5标准初始值 */
    ctx->state[0]=0x67452301;
    ctx->state[1]=0xefcdab89;
    ctx->state[2]=0x98badcfe;
    ctx->state[3]=0x10325476;
}

/*
 * MD5更新：追加待哈希数据
 * ctx: MD5上下文
 * input: 输入数据
 * input_len: 输入数据长度(字节)
 */
static void md5_update(struct md5_ctx* ctx, const unsigned char* input,
                       unsigned int input_len)
{
    unsigned int i, idx, part_len;

    /* 计算buffer中已有数据的偏移 */
    idx=(unsigned int)((ctx->count[0]>>3)&0x3F);

    /* 更新比特计数 */
    ctx->count[0]+=((unsigned int)input_len<<3);
    if(ctx->count[0]<((unsigned int)input_len<<3)) {
        ctx->count[1]++;
    }
    ctx->count[1]+=((unsigned int)input_len>>29);

    part_len=64-idx;

    /* 如果输入数据能填满当前buffer */
    if(input_len>=part_len) {
        memcpy(&ctx->buffer[idx], input, part_len);
        md5_transform(ctx->state, ctx->buffer);

        /* 处理完整的64字节块 */
        for(i=part_len; i+63<input_len; i+=64) {
            md5_transform(ctx->state, &input[i]);
        }
        idx=0;
    } else {
        i=0;
    }

    /* 将剩余数据拷入buffer */
    memcpy(&ctx->buffer[idx], &input[i], input_len-i);
}

/*
 * MD5填充常量：第一位为0x80(二进制10000000)，其余为0
 */
static unsigned char padding[64]={
    0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/*
 * MD5结束：填充并输出最终128位摘要
 * ctx: MD5上下文
 * digest: 输出16字节摘要
 */
static void md5_final(struct md5_ctx* ctx, unsigned char digest[16])
{
    unsigned char bits[8];
    unsigned int idx, pad_len;

    /* 将64位计数器编码为字节 */
    encode(bits, ctx->count, 8);

    /* 计算填充长度 */
    idx=(unsigned int)((ctx->count[0]>>3)&0x3F);
    if(idx<56) {
        pad_len=56-idx;
    } else {
        pad_len=120-idx;
    }

    /* 填充并追加长度 */
    md5_update(ctx, padding, pad_len);
    md5_update(ctx, bits, 8);

    /* 输出摘要 */
    encode(digest, ctx->state, 16);
}

/* ===== 对外接口 ===== */

/*
 * 计算字符串的MD5哈希值
 * input: 输入字符串
 * output: 输出32位十六进制密文
 *
 * 使用示例：
 *   char result[33];
 *   md5_hash("hello", result);
 *   // result = "5d41402abc4b2a76b9719d911017c592"
 */
void md5_hash(const char* input, char* output)
{
    struct md5_ctx ctx;
    unsigned char digest[16];
    int i;

    md5_init(&ctx);
    md5_update(&ctx, (const unsigned char*)input, strlen(input));
    md5_final(&ctx, digest);

    /* 将16字节摘要转换为32位十六进制字符串 */
    for(i=0; i<16; i++) {
        sprintf(output+i*2, "%02x", digest[i]);
    }
    output[32]='\0';
}
