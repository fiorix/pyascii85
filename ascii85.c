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
*/

#include <Python.h>

extern void encode85(FILE *fin, FILE *fout, int begin_tag, int end_tag);
extern PyObject *decode85(FILE *fin, FILE *fout, int preserve);


static PyObject *
ascii85_file(PyObject *self, PyObject *args, PyObject *keywds, int is_enc)
{
    FILE *fin, *fout;
    PyObject *err, *pin, *pout, *begin = Py_True, *end = Py_True;

    static char *kwlist[] = {"input", "output", "begin_tag", "end_tag", NULL};

    if(!PyArg_ParseTupleAndKeywords(args, keywds, "OO|OO", 
                kwlist, &pin, &pout, &begin, &end))
        return NULL;

    if(!PyFile_Check(pin) || !PyFile_Check(pout))
        return 
        PyErr_Format(PyExc_TypeError, "input and output must be file objects\n");
    else if(!PyBool_Check(begin) || !PyBool_Check(end))
        return
        PyErr_Format(PyExc_TypeError, "begin_tag and end_tag must be boolean\n");
    else {
        fin = PyFile_AsFile(pin);
        fout = PyFile_AsFile(pout);
    }

    if(is_enc)
        encode85(fin, fout, 
            PyObject_IsTrue(begin), PyObject_IsTrue(end));
    else {
        if((err = decode85(fin, fout, 0)) != NULL)
            return err;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
ascii85_string(PyObject *self, PyObject *args, PyObject *keywds, int is_enc)
{
    int str_size;
    struct stat st;
    char *str, *buf, *p;
    FILE *fin = tmpfile();
    FILE *fout = tmpfile();
    PyObject *res, *begin = Py_True, *end = Py_True, *err = NULL;

    static char *kwlist[] = {"buffer", "begin_tag", "end_tag", NULL};

    if(!fin || !fout)
        return PyErr_SetFromErrno(PyExc_IOError);
    else if(!PyArg_ParseTupleAndKeywords(args, keywds, "t#|OO", 
                kwlist, &str, &str_size, &begin, &end))
        return NULL;

    if(!PyBool_Check(begin) || !PyBool_Check(end))
        return
        PyErr_Format(PyExc_TypeError, "begin_tag and end_tag must be boolean\n");

    fwrite(str, str_size, 1, fin);
    fflush(fin);
    fseek(fin, 0, SEEK_SET);

    if(is_enc)
        encode85(fin, fout, PyObject_IsTrue(begin), PyObject_IsTrue(end));
    else {
        if((err = decode85(fin, fout, 0)) != NULL)
            return err;
    }

    fflush(fout);
    fseek(fout, 0, SEEK_SET);
    memset(&st, 0, sizeof(st));
    fstat(fileno(fout), &st);

    p = buf = (char *) PyMem_Malloc(st.st_size+1);
    if(!buf)
        return PyErr_NoMemory();

    memset(buf, 0, st.st_size+1);
    while(!feof(fout))
        *p++ = getc(fout);

    fclose(fin);
    fclose(fout);

    res = PyString_FromStringAndSize(buf, st.st_size);
    PyMem_Free(buf);

    return res;
}

static char encode_doc[] = \
    "Encode a file\n" \
    "encode(input, output, begin_tag=True, end_tag=True)";

static PyObject *
encode(PyObject *self, PyObject *args, PyObject *keywds)
{
    return ascii85_file(self, args, keywds, 1);
}

static char b85encode_doc[] = \
    "Encode string buffer using Ascii85\n" \
    "b85encode(buffer, begin_tag=True, end_tag=True)";

static PyObject *
b85encode(PyObject *self, PyObject *args, PyObject *keywds)
{
    return ascii85_string(self, args, keywds, 1);
}


static char decode_doc[] = \
    "Decode a file\ndecode(input, output)";

static PyObject *
decode(PyObject *self, PyObject *args, PyObject *keywds)
{
    return ascii85_file(self, args, keywds, 0);
}

static char b85decode_doc[] = \
    "Decode buffer, an Ascii85 encoded string\n" \
    "b85decode(buffer)";

static PyObject *
b85decode(PyObject *self, PyObject *args, PyObject *keywds)
{
    return ascii85_string(self, args, keywds, 0);
}

static char ascii85_doc[] = "Ascii85 Data Encodings (version 0.1)";
static PyMethodDef ascii85_methods[] = {
        {"encode",    (PyCFunction)encode,    METH_VARARGS|METH_KEYWORDS, encode_doc},
        {"b85encode", (PyCFunction)b85encode, METH_VARARGS|METH_KEYWORDS, b85encode_doc},
        {"decode",    (PyCFunction)decode,    METH_VARARGS|METH_KEYWORDS, decode_doc},
        {"b85decode", (PyCFunction)b85decode, METH_VARARGS|METH_KEYWORDS, b85decode_doc},
        {NULL, NULL}
};

PyMODINIT_FUNC initascii85(void)
{
    Py_InitModule3("ascii85", ascii85_methods, ascii85_doc);
}
