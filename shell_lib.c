#include <stddef.h>
void* memset(void *dest, int val, size_t len)
  {
    int i;
    for(i=0; i<len; i++) ((char*)dest)[i] = val;
    return dest;
  }

int strcmp(const char *s1, const char *s2)
  {
    while (*s1 && (*s1 == *s2))
      {
        s1++; s2++;
      }
    return (*(unsigned char *)s1 - *(unsigned char *)s2);
  }

