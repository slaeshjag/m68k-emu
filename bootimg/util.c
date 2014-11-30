#include <stdint.h>
/* Non-portable way of confusing GCC's optimizer */
void *memset(void *s, int c, unsigned int n) {
	unsigned char *b = s;
	unsigned int *ib = s;
	unsigned int i, z;

	if (!(((unsigned int) s) & 3) && !(n & 3)) {
		n >>= 2;
		z = (c & 0xFF);
		z = (z | (z << 8) | (z << 16) | (z << 24));
		for (i = 0; i < n; i += 2)
			ib[i] = z;
		for (i = 1; i < n; i += 2)
			ib[i] = z;
		return s;
	}

	for (i = 0; i < n; i += 2)
		b[i] = c;
	for (i = 1; i < n; i += 2)
		b[i] = c;
	return s;
}


/* Retarded, slow memcpy to fool GCC from "optimizing" it to a memcpy call */
void *memcpy(void *dest, void *src, unsigned int n) {
	/*char *cdest = dest, *csrc = src;
	int i;

	for (i = 0; i < n; i += 2)
		cdest[i] = csrc[i];
	for (i = 1; i < n; i += 2)
		cdest[i] = csrc[i];*/
	
	register uint8_t *to = dest, *from = src;
	register unsigned int count = (n + 7)/8;
	switch(n % 8) {
		case 0:
			do {
				*to++ = *from++;
		case 7:
				*to++ = *from++;
		case 6:
				*to++ = *from++;
		case 5:
				*to++ = *from++;
		case 4:
				*to++ = *from++;
		case 3:
				*to++ = *from++;
		case 2:
				*to++ = *from++;
		case 1:
				*to++ = *from++;
			} while(count-- > 0);
	 }
	
	return dest;
}


int streq(const char *s1, const char *s2, int limit) {
	int i;

	for (i = 0; i < limit && s1[i] && s2[i] && s1[i] == s2[i]; i++);
	if (i == limit)
		return 1;
	return s1[i] == s2[i];
}


int strlen(const char *s) {
	int i;

	for (i = 0; s[i]; i++);
	return i;
}
