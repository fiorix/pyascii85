/*
    pyascii85 - Ascii85 Data Encodings extension for Python
    Copyright (C) 2008  Alexandre Fiori

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    
    This is a modification of an existing program written by Paul Haahr
    http://www.stillhq.com/cgi-bin/cvsweb/ascii85/
    
    encode85 -- convert to ascii85 format
*/

#include <Python.h>

static unsigned long width = 72;

typedef struct {
        unsigned long int tuple;
        unsigned long int pos;
        int count;
        FILE *fp;
} enc85_data;

static void enc85(enc85_data *d) {
	int i;
	char buf[5], *s = buf;
	i = 5;
	do {
		*s++ = d->tuple % 85;
		d->tuple /= 85;
	} while (--i > 0);

	i = d->count;
	do {
                --s;
		putc(*s + '!', d->fp);
		if (d->pos++ >= width) {
			d->pos = 0;
			putc('\n', d->fp);
		}
	} while (i-- > 0);
}

static void put85(unsigned c, enc85_data *d) {
	switch (d->count++) {
	case 0:	d->tuple |= (c << 24); break;
	case 1: d->tuple |= (c << 16); break;
	case 2:	d->tuple |= (c <<  8); break;
	case 3:
		d->tuple |= c;
		if (d->tuple == 0) {
			putc('z', d->fp);
			if (d->pos++ >= width) {
				d->pos = 0;
				putc('\n', d->fp);
			}
		} else
			enc85(d);
		d->tuple = 0;
		d->count = 0;
		break;
	}
}

void encode85(FILE *fin, FILE *fout, int begin_tag, int end_tag) {
        unsigned c;
        enc85_data d;

        memset(&d, 0, sizeof(d));
        d.fp = fout;

        /* init 85 */
        if(begin_tag) {
            d.pos = 2;
            fprintf(fout, "<~");
        }

        /* encode */
        while((c = getc(fin)) != EOF)
	    put85(c, &d);

        /* cleanup */
	if(d.count > 0)
	    enc85(&d);
	if(d.pos + 2 > width)
            putc('\n', fout);

        if(end_tag)
            fprintf(fout, "~>");

        putc('\n', fout);
}
