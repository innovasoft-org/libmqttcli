#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#define ISDIGIT(V) ( V==0x30 || V==0x31 || V==0x32 || V==0x33 || V==0x34 || V==0x35 || V==0x36 || V==0x37 || V==0x38 || V==0x39 )
#define ISSPACE(V) ( V=='\f' || V=='\n' || V=='\r' || V=='\t' || V=='\v')

uint32_t atoip(char *cp)
{
	uint32_t val, base, n;
	char c;
	uint8_t parts[4], *pp = parts;

again:
	val = 0; base = 10;

	while (c = *cp) {
		if (ISDIGIT(c)) {
			val = (val * base) + (c - '0');
			cp++;
			continue;
		}
		break;
	}
	if (*cp == '.') {
		/*
		 * Internet format:
		 *	a.b.c.d
		 *	a.b.c	(with c treated as 16-bits)
		 *	a.b	(with b treated as 24 bits)
		 */
		if (pp >= parts + 4)
			return (-1);
		*pp++ = val, cp++;
		goto again;
	}

	/*
	 * Check for trailing characters.
	 */
	if (*cp && !ISSPACE(*cp))
		return (-1);
	*pp++ = val;
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts;
	switch (n) {

	case 1:				/* a -- 32 bits */
		val = parts[0];
		break;

	case 2:				/* a.b -- 8.24 bits */
		val = (parts[0] << 24) | (parts[1] & 0xffffff);
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
			(parts[2] & 0xffff);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
		      ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
		break;

	default:
		return (-1);
	}
	//val = htonl(val);
	return (val);
}

char addr01[] = "192.168.0.1";
char addr02[] = "1.2.3.4";
char addr03[] = "255.255.255.255";

int main() {
  printf( "%s %x\r\n", addr01, atoip(addr01) );
  printf( "%s %x\r\n", addr02, atoip(addr02) );
  printf( "%s %x\r\n", addr03, atoip(addr03) );
}