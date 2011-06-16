/*
  gunzip.c by Pasi Ojala, a1bert@iki.fi
  http://www.iki.fi/a1bert/

  A hopefully easier to understand guide to GZip
  (deflate) decompression routine than the GZip
  source code.

*/

/* ========================================================================
 * Table of CRC-32's of all single-byte values (made by makecrc.c)
 */
unsigned long crc_32_tab[] = {
    0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
    0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
    0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL, 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
    0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
    0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
    0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
    0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
    0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L, 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
    0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
    0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
    0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
    0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
    0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L, 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
    0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
    0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
    0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
    0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
    0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L, 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
    0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
    0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
    0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
    0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
    0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L, 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
    0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
    0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
    0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
    0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
    0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL, 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
    0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
    0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
    0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
    0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
};


#define updcrc(cp, crc) (crc_32_tab[((int)crc ^ cp) & 255] ^ (crc >> 8))


/*----------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>


#define F_ERROR     1
#define F_VERBOSE   2


static const unsigned short bitReverse[] = {
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};



/****************************************************************
    Tables for deflate from PKZIP's appnote.txt
****************************************************************/

/* Order of the bit length code lengths */
static const unsigned border[] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

/* Copy lengths for literal codes 257..285 */
static const unsigned short cplens[] = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
    35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};

/* Extra bits for literal codes 257..285 */
static const unsigned short cplext[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99}; /* 99==invalid */

/* Copy offsets for distance codes 0..29 */
static const unsigned short cpdist[] = {
    0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0007, 0x0009, 0x000d,
    0x0011, 0x0019, 0x0021, 0x0031, 0x0041, 0x0061, 0x0081, 0x00c1,
    0x0101, 0x0181, 0x0201, 0x0301, 0x0401, 0x0601, 0x0801, 0x0c01,
    0x1001, 0x1801, 0x2001, 0x3001, 0x4001, 0x6001};

/* Extra bits for distance codes */
static const unsigned short cpdext[] = {
    0,  0,  0,  0,  1,  1,  2,  2,
    3,  3,  4,  4,  5,  5,  6,  6,
    7,  7,  8,  8,  9,  9, 10, 10,
    11, 11, 12, 12, 13, 13};


/****************************************************************
    Bit-I/O variables and routines/macros

    These routines work in the bit level because the target
    environment does not have a barrel shifter. Trying to
    handle several bits at once would've only made the code
    slower.

    If the environment supports multi-bit shifts, you should
    write these routines again (see e.g. the GZIP sources).

****************************************************************/

static unsigned char bb = 1;
static unsigned char inBuf[256];
static FILE *inFp = NULL;
static unsigned char bs = 0, be = 0;
static unsigned int inOffset = 0;

#define INITBYTE(fp) inFp=fp

static int READBYTE(void) {
    int c;

    if(be == bs) {
        int i;

        /* Read a (disk) data block */
        for(i=0;i<254;i++) {
            c = fgetc(inFp);
            inOffset++;
            if(c == -1)
                break;
            inBuf[be++] = c;
        }
    }
    if(be != bs) {
        c = inBuf[bs++];
        return c;
    }
    return -1; /* EOF */
}


static void BYTEALIGN(void) {
    bb = 1;
}


static int READBIT(void) {
    int carry = (bb & 1);

    bb >>= 1;
    if(bb==0) {
        bb = READBYTE();
        carry = (bb & 1);
        bb = (bb>>1) | 0x80;
    }
    return carry;
}


static int READBITS(int a) {
    int res = 0, i = a;

    if (a > 8) {
        fprintf(stderr, "READBITS(%d)!!\n", a);
    }
    while(i--) {
        res = (res<<1) | READBIT();
    }
    if(a) {
        res = bitReverse[res]>>(8-a);
    }
    return res;
}


/****************************************************************
    Output/LZ77 history buffer and related routines/macros
****************************************************************/

static unsigned char buf32k[32768];
static int bIdx = 0;

static long CRC, SIZE;

#define FLUSHBUFFER(o) do{fwrite(buf32k,bIdx,1,(o));bIdx=0;}while(0)
#define ADDBUFFER(a,o) do{SIZE++;CRC=updcrc(a,CRC);buf32k[bIdx++]=(a);if(bIdx==0x8000){fwrite(buf32k,bIdx,1,(o));bIdx=0;}}while(0)



/****************************************************************
    Huffman tree structures, variables and related routines

    These routines are one-bit-at-a-time decode routines. They
    are not as fast as multi-bit routines, but maybe a bit easier
    to understand and use a lot less memory.

    The tree is folded into a table.

****************************************************************/

struct HufNode {
    unsigned short b0;      /* 0-branch value + leaf node flag */
    unsigned short b1;      /* 1-branch value + leaf node flag */
    struct HufNode *jump;   /* 1-branch jump address */
};

#define LITERALS 288

static struct HufNode literalTree[LITERALS];
static struct HufNode distanceTree[32];
static struct HufNode *Places = NULL;

static struct HufNode impDistanceTree[64];
static struct HufNode impLengthTree[64];

static unsigned char len = 0;
static short fpos[17] = {0,};
static int *flens;
static short fmaxn;


int IsPat(void);
int IsPat() {
    while (1) {
        if (fpos[len] >= fmaxn)
            return -1;
        if (flens[fpos[len]] == len)
            return fpos[len]++;
        fpos[len]++;
    }
}


/*
  A recursive routine which creates the Huffman decode tables

  No presorting of code lengths are needed, because a counting
  sort is perfomed on the fly.
*/

/* Maximum recursion depth is equal to the maximum
   Huffman code length, which is 15 in the deflate algorithm.
   (16 in Inflate!) */
int Rec(void);
int Rec() {
    struct HufNode *curplace = Places;
    int tmp;

    if(len==17) {
        return -1;
    }
    Places++;
    len++;

    tmp = IsPat();
    if(tmp >= 0) {
        curplace->b0 = tmp; /* leaf cell for 0-bit */
    } else {
        /* Not a Leaf cell */
        curplace->b0 = 0x8000;
        if(Rec())
            return -1;
    }
    tmp = IsPat();
    if(tmp >= 0) {
        curplace->b1 = tmp; /* leaf cell for 1-bit */
        curplace->jump = NULL;  /* Just for the display routine */
    } else {
        /* Not a Leaf cell */
        curplace->b1 = 0x8000;
        curplace->jump = Places;
        if(Rec())
            return -1;
    }
    len--;
    return 0;
}


/* In C64 return the most significant bit in Carry */
int DecodeValue(struct HufNode *currentTree);
int DecodeValue(struct HufNode *currentTree) {
    struct HufNode *X = currentTree;

    /* decode one symbol of the data */
    while(1) {
        if(READBIT()) {
            if(!(X->b1 & 0x8000))
                return X->b1;   /* If leaf node, return data */
            X = X->jump;
        } else {
            if(!(X->b0 & 0x8000))
                return X->b0;   /* If leaf node, return data */
            X++;
        }
    }
    return -1;
}

/* The same as DecodeValue(), except that 0/1 is reversed */
int DecodeSFValue(struct HufNode *currentTree);
int DecodeSFValue(struct HufNode *currentTree) {
    struct HufNode *X = currentTree;

    /* decode one symbol of the data */
    while(1) {
        if(!READBIT()) {    /* Only the decision is reversed! */
            if(!(X->b1 & 0x8000))
                return X->b1;   /* If leaf node, return data */
            X = X->jump;
        } else {
            if(!(X->b0 & 0x8000))
                return X->b0;   /* If leaf node, return data */
            X++;
        }
    }
    return -1;
}


/*
  Note:
  The tree create and distance code trees <= 32 entries
  and could be represented with the shorter tree algorithm.
  I.e. use a X/Y-indexed table for each struct member.
*/
int CreateTree(struct HufNode *currentTree,
               int numval, int *lengths, int show);
int CreateTree(struct HufNode *currentTree,
               int numval, int *lengths, int show) {
    int i;
    /* Create the Huffman decode tree/table */
    Places = currentTree;
    flens = lengths;
    fmaxn  = numval;
    for (i=0;i<17;i++)
        fpos[i] = 0;
    len = 0;
    if(Rec()) {
        fprintf(stderr, "invalid huffman tree\n");
        return -1;
    }

#if 0
/*    fprintf(stderr, "%d table entries used (max code length %d)\n",
      Places-currentTree, maxlen);*/
    if(show) {
        struct HufNode *tmp;

        for(tmp=currentTree;tmp<Places;tmp++) {
            fprintf(stdout, "0x%03x  0x%03x (0x%04x)",
                    tmp-currentTree, tmp->jump?tmp->jump-currentTree:0,
                    (tmp->jump?tmp->jump-currentTree:0)*6+0xcf0);
            if(!(tmp->b0 & 0x8000)) {
                fprintf(stdout, "  0x%03x (%c)", tmp->b0,
                        (tmp->b0<256 && isprint(tmp->b0))?tmp->b0:'x');
            }
            if(!(tmp->b1 & 0x8000)) {
                if((tmp->b0 & 0x8000))
                    fprintf(stdout, "           ");
                fprintf(stdout, "  0x%03x (%c)", tmp->b1,
                        (tmp->b1<256 && isprint(tmp->b1))?tmp->b1:'x');
            }
            fprintf(stdout, "\n");
        }
    }
#endif
    return 0;
}


static unsigned long crc;

int DecodeSF(int *table) {
    int i, a, n = READBYTE() + 1, v = 0;

    for (i=0; i<n; i++) {
        int nv, bl;
        a = READBYTE();
        nv = ((a >> 4) & 15) + 1;
        bl = (a & 15) + 1;
        while (nv--) {
            table[v++] = bl;
        }
    }
/*for(i=0;i<v;i++) {
  printf("%02x ", table[i]);
  if((i&7)==7)
  printf("\n");
  }
  printf(" %d\n", v);*/
    return v; /* entries used */
}

/* Note: Imploding could use the lighter huffman tree routines, as the
   max number of entries is 256. But too much code would need to
   be duplicated.
*/
int ImplodeLoop(FILE *errfp, FILE *outfp, int flags, long size) {
    int c, i, minMatchLen = 3, len, dist;
    static int ll[256];

    /* initialize the history buffer to 0 */
    memset(buf32k, 0, 32768);

    BYTEALIGN();
    if ((flags & 4)) {
        /* 3 trees: literals, lenths, distance top 6 */
        minMatchLen = 3;
        if (CreateTree(literalTree, DecodeSF(ll), ll, 0))
            return 1;
    } else {
        /* 2 trees: lenths, distance top 6 */
        minMatchLen = 2;
    }
    if (CreateTree(impLengthTree, DecodeSF(ll), ll, 0))
        return 1;
    if (CreateTree(impDistanceTree, DecodeSF(ll), ll, 0))
        return 1;

    while (SIZE < size) {
        c = READBITS(1);
        if (c) {
            /* literal data */
            if ((flags & 4)) {
                c = DecodeSFValue(literalTree);
            } else {
                c = READBITS(8);
            }
/*printf("lit 0x%02x\n", c);*/
            ADDBUFFER(c, outfp);
        } else {
            if ((flags & 2)) {
                /* 8k dictionary */
                dist = READBITS(7);
                /*printf("%02x ",  dist);*/
                c = DecodeSFValue(impDistanceTree);
                /*printf("%02x ",  c);*/
                dist |= (c<<7);
            } else {
                /* 4k dictionary */
                dist = READBITS(6);
                /*printf("%02x ",  dist);*/
                c = DecodeSFValue(impDistanceTree);
                /*printf("%02x ",  c);*/
                dist |= (c<<6);
            }
            len = DecodeSFValue(impLengthTree);
            if (len == 63) {
                len += READBITS(8);
            }
            len += minMatchLen;
            dist++;
            /*
            if (dist > SIZE) {
                printf("before SOF ");
            }
            printf("P: %5d  lz %d %d\n", SIZE, dist, len);
            */
            /*
            fprintf(errfp, "LZ77 len %d dist %d\n", len, dist);
            */
            for(i=0;i<len;i++) {
                int c = buf32k[(bIdx - dist) & 0x7fff];
                ADDBUFFER(c, outfp);
            }
        }
    }
    FLUSHBUFFER(outfp);

    BYTEALIGN();
    return 0;
}



int DeflateLoop(FILE *errfp, FILE *outfp) {
    int last, c, type, i;

    do {
        if((last = READBIT())) {
            fprintf(errfp, "Last Block: ");
        } else {
            fprintf(errfp, "Not Last Block: ");
        }

        type = READBITS(2);
        switch(type) {
        case 0:
            fprintf(errfp, "Stored\n");
            break;
        case 1:
            fprintf(errfp, "Fixed Huffman codes\n");
            break;
        case 2:
            fprintf(errfp, "Dynamic Huffman codes\n");
            break;
        case 3:
            fprintf(errfp, "Reserved block type!!\n");
            break;
        default:
            fprintf(errfp, "Unexpected value %d!\n", type);
            break;
        }

        if(type==0) {
            int blockLen, cSum;

            /* Stored */
            BYTEALIGN();
            blockLen = READBYTE();
            blockLen |= (READBYTE()<<8);

            cSum = READBYTE();
            cSum |= (READBYTE()<<8);

            if(((blockLen ^ ~cSum) & 0xffff)) {
                fprintf(errfp, "BlockLen checksum mismatch\n");
            }
            while(blockLen--) {
                c = READBYTE();
                ADDBUFFER(c, outfp);
            }
        } else if(type==1) {
            int j;

            /* Fixed Huffman tables -- fixed decode routine */
            while(1) {
                /*
                  256 0000000     0
                  :   :     :
                  279 0010111     23
                  0   00110000    48
                  :   :      :
                  143 10111111    191
                  280 11000000    192
                  :   :      :
                  287 11000111    199
                  144 110010000   400
                  :   :       :
                  255 111111111   511

                  Note the bit order!
                */

                j = (bitReverse[READBITS(7)]>>1);
                if(j > 23) {
                    j = (j<<1) | READBIT(); /* 48..255 */

                    if(j > 199) {   /* 200..255 */
                        j -= 128;   /*  72..127 */
                        j = (j<<1) | READBIT();     /* 144..255 << */
                    } else {        /*  48..199 */
                        j -= 48;    /*   0..151 */
                        if(j > 143) {
                            j = j+136;  /* 280..287 << */
                            /*   0..143 << */
                        }
                    }
                } else {    /*   0..23 */
                    j += 256;   /* 256..279 << */
                }
                if(j < 256) {
                    ADDBUFFER(j, outfp);
                    /*fprintf(errfp, "@%d %02x\n", SIZE, j);*/
                } else if(j == 256) {
                    /* EOF */
                    break;
                } else {
                    int len, dist;

                    j -= 256 + 1;   /* bytes + EOF */
                    len = READBITS(cplext[j]) + cplens[j];

                    j = bitReverse[READBITS(5)]>>3;
                    if(cpdext[j] > 8) {
                        dist = READBITS(8);
                        dist |= (READBITS(cpdext[j]-8)<<8);
                    } else {
                        dist = READBITS(cpdext[j]);
                    }
                    dist += cpdist[j];

                    /*fprintf(errfp, "@%d (l%02x,d%04x)\n", SIZE, len, dist);*/
                    for(j=0;j<len;j++) {
                        int c = buf32k[(bIdx - dist) & 0x7fff];
                        ADDBUFFER(c, outfp);
                    }
                }
            }
        } else if(type==2) {
            int j, n, literalCodes, distCodes, lenCodes;
            static int ll[288+32];  /* "static" just to preserve stack */

            /* Dynamic Huffman tables */

            literalCodes = 257 + READBITS(5);
            distCodes = 1 + READBITS(5);
            lenCodes = 4 + READBITS(4);
#if 0
            fprintf(stderr, "l: %d d: %d l: %d\n", literalCodes, distCodes, lenCodes);
#endif
            for(j=0; j<19; j++) {
                ll[j] = 0;
            }

            /* Get the decode tree code lengths */

            for(j=0; j<lenCodes; j++) {
                ll[border[j]] = READBITS(3);
                fprintf(errfp, "%d ", ll[border[j]]);
            }
            fprintf(errfp, "\n");
#if 0
            for(j=0; j<19; j++) {
                fprintf(errfp, "0x%02x ", ll[j]);
            }
            fprintf(errfp, "\n");
#endif
            if(CreateTree(distanceTree, 19, ll, 0)) {
                FLUSHBUFFER(outfp);
                return 1;
            }
#if 0
            fprintf(stderr, "Len tree ok\n");
#endif
            /* read in literal and distance code lengths */
            n = literalCodes + distCodes;
            i = 0;
            while(i < n) {
                j = DecodeValue(distanceTree);
                if(j<16) {  /* length of code in bits (0..15) */
#if 0
                    fprintf(stderr, "%d ", j);
#endif
                    ll[i++] = j;
                } else if(j==16) {  /* repeat last length 3 to 6 times */
                    int l;

                    j = 3 + READBITS(2);
#if 0
                    fprintf(stderr, "*%d ", j);
#endif
                    if(i+j > n) {
                        FLUSHBUFFER(outfp);
                        return 1;
                    }

                    l = i ? ll[i-1] : 0;
                    while(j--) {
                        ll[i++] = l;
                    }
                } else {
                    if(j==17) {     /* 3 to 10 zero length codes */
                        j = 3 + READBITS(3);
                    } else {        /* j == 18: 11 to 138 zero length codes */
                        j = 11 + READBITS(7);
                    }
#if 0
                    fprintf(stderr, "%d*z ", j);
#endif
                    if(i+j > n) {
                        FLUSHBUFFER(outfp);
                        return 1;
                    }

                    while(j--) {
                        ll[i++] = 0;
                    }
                }
            }

#if 0
            for(j=0; j<literalCodes+distCodes; j++) {
                fprintf(errfp, "%d ", ll[j]);
                if ((j&7)==7)
                    fprintf(errfp, "\n");
            }
            fprintf(errfp, "\n");
#endif
            /* Can overwrite tree decode tree as it is not used anymore */
            if(CreateTree(literalTree, literalCodes, &ll[0], 0)) {
                FLUSHBUFFER(outfp);
                return 1;
            }
#if 0
            fprintf(stderr, "Literal tree ok\n");
#endif
            if(CreateTree(distanceTree, distCodes, &ll[literalCodes], 0)) {
                FLUSHBUFFER(outfp);
                return 1;
            }
#if 0
            fprintf(stderr, "Distance tree ok\n");
#endif
            while(1) {
                j = DecodeValue(literalTree);
                if(j >= 256) {      /* In C64: if carry set */
                    int len, dist;

                    j -= 256;
                    if(j == 0) {
                        /* EOF */
                        break;
                    }
                    /*printf("%04x ", j);*/
                    j--;
                    len = READBITS(cplext[j]) + cplens[j];
                    /*printf("%04x ", len);*/

                    j = DecodeValue(distanceTree);
                    /*printf("%02x ", j);*/
                    if(cpdext[j] > 8) {
                        dist = READBITS(8);
                        dist |= (READBITS(cpdext[j]-8)<<8);
                    } else {
                        dist = READBITS(cpdext[j]);
                    }
                    dist += cpdist[j];
                    /*printf("%04x ", dist);*/

                    /*printf("LZ77 len %d dist %d @%04x\n", len, dist, bIdx);*/
                    while(len--) {
                        int c = buf32k[(bIdx - dist) & 0x7fff];
                        /*printf("%02x ", c);*/
                        ADDBUFFER(c, outfp);
                    }
                    /*printf("\n");*/
                } else {
                    /*printf("%02x\n", j);*/
                    ADDBUFFER(j, outfp);
                }
            }
        }
    } while(!last);
    FLUSHBUFFER(outfp);

    BYTEALIGN();
    return 0;
}




/* Partially tested */

int ReduceLoop(FILE *errfp, FILE *outfp, int level, long size) {
    static unsigned char S[256][32], N[256];
    static unsigned char B[64] = {
        0,
        1,1,2,2,3,3,3,3, 4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5, 5,5,5,5,5,5,5,5,
        6 };
    int j, i, lastC, state, c, len = 0, dist = 0, cnt;

    /* Note:
       In my opinion it takes 0 bits to select among 1 follower.
       For a reason not apparrent to me, B[1] is set to 1.
    */

    /* initialize the history buffer to 0 */
    memset(buf32k, 0, 32768);

    BYTEALIGN();
    cnt = 0;
    for (j=255; j>=0; j--) {
        N[j] = READBITS(6);
        if (N[j] > 32) {
            fprintf(errfp, "Follower set %d too large: %d\n", j, N[j]);
            N[j] = 32;
        }
        for (i=0; i<N[j]; i++) {
            S[j][i] = READBITS(8);
            cnt++;
        }
    }
    fprintf(errfp, "%d followers\n", cnt);
    lastC = 0;
    state = 0;
    while (SIZE < size) {
        if (!N[lastC]) {
            c = READBITS(8);
            /*fprintf(errfp, "%3d:%d %d 8\n", SIZE, state, c);*/
        } else {
            if (READBITS(1)) {
                c = READBITS(8);
                /*fprintf(errfp, "%3d:%d %d 8 2\n", SIZE, state, c);*/
            } else {
                c = 0;
                if (N[lastC] != 0)
                    c = READBITS(B[N[lastC]]);
                c = S[lastC][c];
                /*fprintf(errfp, "%3d:%d %d S[lastC][READBITS(B[N[lastC]])]\n", SIZE, state, c);*/
            }
        }
        lastC = c;
        switch (state) {
        case 0:
            if (c != 144) {
                ADDBUFFER(c, outfp);
            } else {
                state = 1;
            }
            break;
        case 1:
            if (c) {
                switch(level) {
                case 2:
                    dist = (c >> 7)*256;
                    len = 0x7f & c;
                    state = (len == 0x7f)?2:3;
                    break;
                case 3:
                    dist = (c >> 6)*256;
                    len = 0x3f & c;
                    state = (len == 0x3f)?2:3;
                    break;
                case 4:
                    dist = (c >> 5)*256;
                    len = 0x1f & c;
                    state = (len == 0x1f)?2:3;
                    break;
                case 5:
                    dist = (c >> 4)*256;
                    len = 0x0f & c;
                    state = (len == 0x0f)?2:3;
                    break;
                }
            } else {
                ADDBUFFER(144, outfp);
                state = 0;
            }
            break;
        case 2:
            len += c;
            state = 3;
            break;
        case 3:
            dist += c + 1;
            len += 3;
            while(len--) {
                int c = buf32k[(bIdx - dist) & 0x7fff];
                ADDBUFFER(c, outfp);
            }
            state = 0;
            break;
        }
    }
    FLUSHBUFFER(outfp);

    BYTEALIGN();
    return 0;
}



/* HSIZE is defined as 2^13 (8192) in unzip.h */
#define HSIZE      8192
#define BOGUSCODE  256
#define CODE_MASK  (HSIZE - 1)   /* 0x1fff (lower bits are parent's index) */
#define FREE_CODE  HSIZE         /* 0x2000 (code is unused or was cleared) */
#define HAS_CHILD  (HSIZE << 1)  /* 0x4000 (code has a child--do not clear) */

int ShrinkLoop(FILE *errfp, FILE *outfp, long size) {
    static unsigned short Parent[HSIZE];
    static unsigned char Value[HSIZE], Stack[HSIZE];
    unsigned char *newstr;
    int len;
    char KwKwK, codesize = 1;   /* start at 9 bits/code */
    short code, oldcode, freecode, curcode;

    freecode = BOGUSCODE;
    for (code = 0; code < BOGUSCODE; code++) {
        Value[code] = code;
        Parent[code] = BOGUSCODE;
    }
    for (code = BOGUSCODE+1; code < HSIZE; code++)
        Parent[code] = FREE_CODE;


    oldcode = READBITS(8);
    oldcode |= (READBITS(codesize)<<8);
    if (SIZE < size) {
        ADDBUFFER(oldcode, outfp);
        /*printf("0x%02x\n", oldcode);*/
    }

    while (SIZE < size) {
        code = READBITS(8);
        code |= (READBITS(codesize)<<8);
        if (code == BOGUSCODE) {   /* possible to have consecutive escapes? */
            code = READBITS(8);
            code |= (READBITS(codesize)<<8);
            if (code == 1) {
                /*printf("codesize++ at %ld\n", SIZE);*/
                codesize++;
            } else if (code == 2) {
                /* clear leafs (nodes with no children) */
                /*printf("clearcode at %ld\n", SIZE);*/

                /* first loop:  mark each parent as such */
                for (code = BOGUSCODE+1;  code < HSIZE;  ++code) {
                    curcode = (Parent[code] & CODE_MASK);

                    if (curcode > BOGUSCODE)
                        Parent[curcode] |= HAS_CHILD;   /* set parent's child-bit */
                }

                /* second loop:  clear all nodes *not* marked as parents; reset flag bits */
                for (code = BOGUSCODE+1;  code < HSIZE;  ++code) {
                    if (Parent[code] & HAS_CHILD) {   /* just clear child-bit */
                        Parent[code] &= ~HAS_CHILD;
                    } else {                              /* leaf:  lose it */
                        Parent[code] = FREE_CODE;
                        /*printf("free %d\n", code);*/
                    }
                }
                freecode = BOGUSCODE;
            }
            continue;
        }

        newstr = &Stack[HSIZE-1];
        curcode = code;

        if (Parent[curcode] == FREE_CODE) {
            KwKwK = 1;
            newstr--;   /* last character will be same as first character */
            curcode = oldcode;
            len = 1;
        } else {
            KwKwK = 0;
            len = 0;
        }

        do {
            *newstr-- = Value[curcode];
            len++;
            curcode = (Parent[curcode] & CODE_MASK);
        } while (curcode != BOGUSCODE);

        newstr++;
        if (KwKwK) {
            Stack[HSIZE-1] = *newstr;
        }

        do {
            freecode++;
        } while (Parent[freecode] != FREE_CODE);

        Parent[freecode] = oldcode;
        Value[freecode] = *newstr;
        oldcode = code;

        /*printf("--%s\n", KwKwK?"KwKwK":"");*/
        while (len--) {
            ADDBUFFER(*newstr, outfp);
            /*printf("0x%02x\n", *newstr);*/
            newstr++;
        }
    }
    FLUSHBUFFER(outfp);

    BYTEALIGN();
    return 0;
}

#define MODE_GZIP 1
#define MODE_ZIP  2
#define NAME_MAX 256

int gunzip_file(FILE* infp, FILE* outfp) {
    FILE *errfp = stderr;
    int c, i, gpflags, tmp[4], mode = 0, oldMode = 0;
    long size;
    static const char *flagNames[] = {
        "ASCII", "CRC16", "Extra Field", "Original Name",
        "Comment", "Reserved 5", "Reserved 6", "Reserved 7"};
    static const char *methodStr[] = {
        "Stored", "Shrunk", "Reduced1", "Reduced2",
        "Reduced3", "Reduced4", "Imploded", "Tokenized", "Deflated"};
    char nameBuf[NAME_MAX];
    unsigned char os;

    if(!infp) {
        fprintf(stderr, "Could not open file for reading\n");
        return 20;
    }

    INITBYTE(infp);
nextFile:
    oldMode = mode;

    bzero(tmp, 4);

    tmp[0] = READBYTE();
    if(tmp[0]==-1) {
        /* No more files to process */
        return 0;
    }
    mode = 0;
    tmp[1] = READBYTE();

    if(tmp[0]==0x1f && tmp[1]==0x8b)
        mode = MODE_GZIP;
    else if(tmp[0]==0x50 && tmp[1]==0x4b) {
        tmp[2] = READBYTE();
        tmp[3] = READBYTE();
        if(tmp[2]==0x03 && tmp[3]==0x04) {
            /* local header */
            mode = MODE_ZIP;
        } else if(tmp[2]==0x01 && tmp[3]==0x02) {
            fprintf(errfp, "ZIP central directory reached\n");
            return 0;
        } else if(tmp[2]=='0' && tmp[3]=='0') {
            /* "PK00" */
            goto nextFile;
        } else {
            char buf[10];
            fprintf(errfp, "Corrupted archive. Try to extract: y/n?\n");
            fgets(buf, 9, stdin);
            if (buf[0]=='y' || buf[0]=='Y') {
                while (1) {
                    if (tmp[0]==-1 && tmp[1]==-1 && tmp[2]==-1 && tmp[3]==-1)
                        break;
                    if (tmp[0]==0x50 && tmp[1]==0x4b &&
                            tmp[2]==0x03 && tmp[3]==0x04) {
                        fprintf(errfp, "Found local header\n");
                        mode = MODE_ZIP;
                        break;
                    }
                    tmp[0] = tmp[1];
                    tmp[1] = tmp[2];
                    tmp[2] = tmp[3];
                    tmp[3] = READBYTE();
                }
            }
        }
    } else {
        if (oldMode != MODE_ZIP) {
            char buf[10];
            fprintf(errfp, "Corrupted archive. Try to extract: y/n?\n");
            fgets(buf, 9, stdin);
            if (buf[0]=='y' || buf[0]=='Y')
                oldMode = MODE_ZIP;
        }
        if (oldMode == MODE_ZIP) {
            fprintf(errfp, "Skipping corrupted data\n");
            while (1) {
                if (tmp[0]==-1 && tmp[1]==-1 && tmp[2]==-1 && tmp[3]==-1)
                    break;
                if (tmp[0]==0x50 && tmp[1]==0x4b &&
                        tmp[2]==0x03 && tmp[3]==0x04) {
                    fprintf(errfp, "Found local header\n");
                    mode = MODE_ZIP;
                    break;
                }
                tmp[0] = tmp[1];
                tmp[1] = tmp[2];
                tmp[2] = tmp[3];
                tmp[3] = READBYTE();
            }
        }
    }
    if(!mode) {
        fprintf(errfp, "Magic number mismatch 0x%02x%02x %d\n",
                tmp[0], tmp[1], inOffset - ((be-bs)&255));
        return 20;
    }
    if(mode == MODE_ZIP) {
        int method, filelen, extralen;
        long compSize;

        tmp[0] = READBYTE();
        tmp[1] = READBYTE();    /* version needed to extract */

        printf("version needed: %d %d.%d\n", tmp[1], tmp[0]/10, tmp[0]%10);

        gpflags = READBYTE();
        gpflags |= (READBYTE()<<8); /* flags */

        method = READBYTE();
        method |= (READBYTE()<<8);  /* method */

        READBYTE();
        READBYTE(); /* last mod file time */
        READBYTE();
        READBYTE(); /* last mod file date */

        crc = READBYTE();
        crc |= (READBYTE()<<8);
        crc |= (READBYTE()<<16);
        crc |= (READBYTE()<<24);

        compSize = READBYTE();
        compSize |= (READBYTE()<<8);
        compSize |= (READBYTE()<<16);
        compSize |= (READBYTE()<<24);

        size = READBYTE();
        size |= (READBYTE()<<8);
        size |= (READBYTE()<<16);
        size |= (READBYTE()<<24);

        fprintf(errfp, "local CRC:  %08lx\n", crc);
        fprintf(errfp, "local Size: %08lx\n", size);
        fprintf(errfp, "local CompSize: %08lx\n", compSize);

        filelen = READBYTE();
        filelen |= (READBYTE()<<8);

        extralen = READBYTE();
        extralen |= (READBYTE()<<8);

        fprintf(errfp, "Original file name (%d): ", filelen);
        i = 0;
        while (filelen--) {
            c = READBYTE();
            fprintf(errfp, "%c", c);
            if (c == '/' || c == ':') {
                i = 0;  /* remove path */
                continue;
            }
            if (i<NAME_MAX-1)
                nameBuf[i++] = c;
        }
        nameBuf[i] = '\0';
        fprintf(errfp, " -> %s\n", nameBuf);

        /* skip extra field */
        i = 0;
        while(i<extralen) {
            c = READBYTE();
        }

        CRC = 0xffffffff;
        SIZE = 0;

        if(outfp != stdout)
            errfp = stdout;

        if (method <= 8)
            fprintf(errfp, "%s", methodStr[method]);
        else
            fprintf(errfp, "Method %d", method);
        if (method == 8) {
            fprintf(errfp, "\n");
            if (DeflateLoop(errfp, outfp))
                goto errorExit;
        } else if (method == 6) {
            fprintf(errfp, "\n");
            if (ImplodeLoop(errfp, outfp, gpflags, size))
                goto errorExit;
        } else if (method >= 2 && method <= 5) {
            fprintf(errfp, "\n");
            if (ReduceLoop(errfp, outfp, method, size))
                goto errorExit;
        } else if (method == 1) {
            fprintf(errfp, "\n");
            if (ShrinkLoop(errfp, outfp, size))
                goto errorExit;
        } else if (method == 0) {
            fprintf(errfp, "\n");
            /* stored */
            i = 0;
            while(i<compSize) {
                c = READBYTE();
                fputc(c, outfp);
                SIZE++;
                CRC=updcrc(c,CRC);
                i++;
            }
        } else {
            fprintf(errfp, ": unknown or unsupported method\n");
            if (!(gpflags & 8)) {
                fprintf(errfp, "Skipping..\n");
                i = 0;
                while(i<compSize) {
                    c = READBYTE();
                    i++;
                }
            } else {
                goto errorExit;
            }
        }

        if ((gpflags & 8)) {
            tmp[0] = READBYTE();
            tmp[1] = READBYTE();
            tmp[2] = READBYTE();
            tmp[3] = READBYTE();

            if (tmp[0] == 0x50 && tmp[1] == 0x4b && tmp[2] == 0x07 && tmp[3] == 0x08) {
                crc = READBYTE();
                crc |= (READBYTE()<<8);
                crc |= (READBYTE()<<16);
                crc |= (READBYTE()<<24);
            } else {
                /* Allow it without the header */
                crc = tmp[0] | (tmp[1]<<8) | (tmp[2]<<16) | (tmp[3]<<24);
            }

            compSize = READBYTE();
            compSize |= (READBYTE()<<8);
            compSize |= (READBYTE()<<16);
            compSize |= (READBYTE()<<24);

            size = READBYTE();
            size |= (READBYTE()<<8);
            size |= (READBYTE()<<16);
            size |= (READBYTE()<<24);
        }
#if 0
        fprintf(errfp, "CRC:  %08lx %08lx %s\n", crc, ~CRC, (crc != ~CRC)?"**error**":"");
        fprintf(errfp, "Size: %08lx %08lx %s\n", size, SIZE, (size != SIZE)?"**error**":"");
        fprintf(errfp, "CompSize: %08lx\n", compSize);
#endif
        goto nextFile;
    }

    tmp[0] = READBYTE();
    if(tmp[0] != 8) {
        fprintf(errfp, "Unknown compression method: 0x%02x\n", tmp[0]);
        if(outfp != stdout)
            fclose(outfp);
        return 20;
    }
    /* fprintf(errfp, "Compression method: Deflate\n"); */

    gpflags = READBYTE();
    /* fprintf(errfp, "Flags: 0x%02x\n", gpflags); */
    for(i=0;i<8;i++) {
        if((gpflags & (1<<i))) {
            fprintf(errfp, "\t%s\n", flagNames[i]);
        }
    }
    if ((gpflags & ~0x1f)) {
        fprintf(errfp, "Unknown flags set!\n");
    }

    /* Skip file modification time (4 bytes) */
    READBYTE();
    READBYTE();
    READBYTE();
    READBYTE();
    /* Skip extra flags and operating system fields (2 bytes) */
    READBYTE();
    os = READBYTE();

#if 0
    switch (os) {
    case 0:
        fprintf(errfp, "MS-DOS OS/2 NT/Win32\n");
        break;
    case 1:
        fprintf(errfp, "AmigaOS\n");
        break;
    case 2:
        fprintf(errfp, "VMS / OpenVMS\n");
        break;
    case 3:
        fprintf(errfp, "Unix\n");
        break;
    case 4:
        fprintf(errfp, "VM / CMS\n");
        break;
    case 5:
        fprintf(errfp, "Atari TOS\n");
        break;
    case 6:
        fprintf(errfp, "HPFS: OS/2 NT\n");
        break;
    case 7:
        fprintf(errfp, "Mac\n");
        break;
    case 8:
        fprintf(errfp, "Z-System\n");
        break;
    case 9:
        fprintf(errfp, "CP/M\n");
        break;
    case 10:
        fprintf(errfp, "TOPS-20\n");
        break;
    case 11:
        fprintf(errfp, "NTFS: NT\n");
        break;
    case 12:
        fprintf(errfp, "QDOS\n");
        break;
    case 13:
        fprintf(errfp, "Acorn RISCOS\n");
        break;
    case 255:
        fprintf(errfp, "Unknown system\n");
        break;
    default:
        break;
    }
#endif

    if((gpflags & 4)) {
        int len;

        /* Skip extra field */
        tmp[0] = READBYTE();
        tmp[1] = READBYTE();
        len = tmp[0] + 256*tmp[1];
        /* fprintf(errfp, "extra field size %ld\n", len); */
        for(i=0;i<len;i++) {
            READBYTE();
        }
    }
    if((gpflags & 8)) {
        fprintf(errfp, "Original file name: ");
        i = 0;
        while((c = READBYTE())) {
            fprintf(errfp, "%c", c);
            if (c == '/' || c == ':') {
                i = 0;  /* remove path */
                continue;
            }
            if (i<NAME_MAX-1)
                nameBuf[i++] = c;
        }
        nameBuf[i] = '\0';
        fprintf(errfp, " -> %s\n", nameBuf);
        /* if no file defined, use the old name */
    }
    if((gpflags & 16)) {
        fprintf(errfp, "File comment: ");
        while((c = READBYTE())) {
            fprintf(errfp, "%c", c);
        }
        fprintf(errfp, "\n");
    }
    if((gpflags & 2)) {
        /* Skip CRC16 */
        READBYTE();
        READBYTE();
    }

    CRC = 0xffffffff;
    SIZE = 0;

    if(!outfp) {
        fprintf(stderr, "Could not open filefor writing\n");
        if(infp != stdin)
            fclose(infp);
        return 20;
    }
    if(outfp != stdout)
        errfp = stdout;

    if (DeflateLoop(errfp, outfp))
        goto errorExit;

    crc = READBYTE();
    crc |= (READBYTE()<<8);
    crc |= (READBYTE()<<16);
    crc |= (READBYTE()<<24);

    size = READBYTE();
    size |= (READBYTE()<<8);
    size |= (READBYTE()<<16);
    size |= (READBYTE()<<24);

#if 0
    fprintf(errfp, "CRC:  %08lx %08lx %s\n", crc, ~CRC, (crc != ~CRC)?"**error**":"");
    fprintf(errfp, "Size: %08lx %08lx %s\n", size, SIZE, (size != SIZE)?"**error**":"");
#endif
    goto nextFile;

errorExit:
    if(outfp != stdout)
        fclose(outfp);
    return 0;
}
