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
    
    decode85 -- convert from ascii85 format
*/


#include <Python.h>

static unsigned long pow85[] = {
	85*85*85*85, 85*85*85, 85*85, 85, 1
};

static void wput(unsigned long tuple, int bytes, FILE *fp) {
	switch (bytes) {
	case 4:
		putc(tuple >> 24, fp);
		putc(tuple >> 16, fp);
		putc(tuple >>  8, fp);
		putc(tuple, fp);
		break;
	case 3:
		putc(tuple >> 24, fp);
		putc(tuple >> 16, fp);
		putc(tuple >>  8, fp);
		break;
	case 2:
		putc(tuple >> 24, fp);
		putc(tuple >> 16, fp);
		break;
	case 1:
		putc(tuple >> 24, fp);
		break;
	}
}

static PyObject *dec85(FILE *fin, FILE *fout) {
	unsigned long tuple = 0;
	int c, count = 0;
	for (;;)
            switch (c = getc(fin)) {
            default:
                if (c < '!' || c > 'u')
                    return 
                    PyErr_Format(PyExc_ValueError, "decode85: bad character in ascii85 region: %#o\n", c);

                tuple += (c - '!') * pow85[count++];
                if (count == 5) {
                    wput(tuple, 4, fout);
                    count = 0;
                    tuple = 0;
                }
                break;
            case 'z':
                if (count != 0)
                    return 
                    PyErr_Format(PyExc_ValueError, "decode85: z inside ascii85 5-tuple\n");

                putc(0, fout);
                putc(0, fout);
                putc(0, fout);
                putc(0, fout);
                break;
            case '~':
                if (getc(fin) == '>') {
                    if (count > 0) {
                        count--;
                        tuple += pow85[count];
                        wput(tuple, count, fout);
                    }
                    c = getc(fin);
                    return NULL;
                }
                return 
                PyErr_Format(PyExc_ValueError, "decode85: ~ without > in ascii85 section\n");

            case '\n': case '\r': case '\t': case ' ':
            case '\0': case '\f': case '\b': case 0177:
                break;
            case EOF:
                return 
                PyErr_Format(PyExc_ValueError, "decode85: EOF inside ascii85 section\n");
            }
}

PyObject *decode85(FILE *fin, FILE *fout, int preserve) {
    int c;
    PyObject *err = NULL;

    while((c = getc(fin)) != EOF)
	if(c == '<') {
            if((c = getc(fin)) == '~') {
                if((err = dec85(fin, fout)) != NULL) return err;
            } else {
                if(preserve)
                    putc('<', fout);
                if(c == EOF)
                    break;
                if(preserve)
                    putc(c, fout);
            }
        } else
            if(preserve)
                fputc(c, fout);

    return NULL;
}
