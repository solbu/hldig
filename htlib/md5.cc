#include <stdio.h>
extern "C" {
#include "mhash_md5.h"
}

#define MD5_LENGTH 16

void md5(char *rhash, char *buf, int len, time_t *date, bool debug)
{

  int i;
  MD5_CTX *td;    
  char *hash;  

  td = (MD5_CTX *)malloc(sizeof(MD5_CTX));
  MD5Init( td);
  //  td = mhash_init(MHASH_MD5);

  MD5Update(td,(unsigned char *) buf, len);
  //  mhash(td, buf, len);

  if (date) {
    MD5Update(td,(unsigned char *)date, sizeof(*date));
  }

  hash = (char *)MD5Final(td);
  //  hash = (char *)mhash_end(td);

  memcpy(rhash,hash,MD5_LENGTH);
             
  if (debug) {
    printf(" ");
    for (i = 0; i < MD5_LENGTH; i++) {
      printf("%.2x", hash[i]);
    }
    printf(" ");
  }
  delete td;
}  
