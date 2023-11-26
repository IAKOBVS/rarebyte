/* Copyright (c) 2023 James Tirta Halim <tirtajames45 at gmail dot com>
   This file is part of rarebyte.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.

   MIT License (Expat) */

#define JSTR_USE_LGPL 0
#define JSTR_USE_UNLOCKED_IO 1

#include "./jstring/jstr/jstr-io.h"
#include "./jstring/jstr/jstr.h"

#ifndef MAX_FILE_SIZE
#	define MAX_FILE_SIZE 100 * JSTRIO_MB
#endif

typedef struct ftw_args_ty {
	jstr_ty *file_str;
	unsigned long long *array;
} ftw_args_ty;

static JSTRIO_FTW_FUNC(callback, ftw, args)
{
	if (jstr_unlikely(ftw->ftw_state == JSTRIO_FTW_STATE_NS))
		goto ret;
	ftw_args_ty *arg = (ftw_args_ty *)args;
	/* Ignore large files. */
	if (ftw->st->st_size >= MAX_FILE_SIZE)
		goto ret;
	if (jstr_chk(jstrio_readfile_len_j(arg->file_str, ftw->dirpath, ftw->st->st_size))) {
		jstr_errdie("Failed at jstrio_readfile_len().");
		return JSTR_RET_ERR;
	}
	jstrio_ext_ty ext = jstrio_exttype(ftw->dirpath, ftw->dirpath_len);
	if (ext == JSTRIO_FT_BINARY)
		goto ret;
	if (ext == JSTRIO_FT_UNKNOWN)
		if (jstrio_isbinary_maybe(arg->file_str->data, arg->file_str->size))
			goto ret;
	jstr_foreach(arg->file_str, p)
	{
		if (jstr_likely(arg->array[(unsigned char)*p] < (unsigned long long)-1))
			++arg->array[(unsigned char)*p];
	}
ret:
	return JSTR_RET_SUCC;
}

int
main(int argc,
     char **argv)
{
	if (jstr_unlikely(argc <= 1)) {
		fprintf(stderr, "Usage: %s <directory> ...\nMultiple directories may be used as arguments.", argv[0]);
		exit(EXIT_FAILURE);
	}
	jstr_ty file_str = JSTR_INIT;
	if (jstr_chk(jstr_reserve_j(&file_str, JSTR_PAGE_SIZE)))
		jstr_errdie("Failed at jstr_reserve_j().");
	ftw_args_ty arg;
	arg.file_str = &file_str;
	unsigned long long c_freq[256];
	arg.array = c_freq;
	for (int i = 1; argv[i]; ++i) {
		if (jstr_chk(jstrio_ftw(argv[i], callback, &arg, JSTRIO_FTW_REG | JSTRIO_FTW_STATREG, "*.[ch]", 0)))
			jstr_errdie("Failed at jstrio_ftw().");
	}
	file_str.size = 0;
	/*
	  Format:
	  ASCII_CODE N
	*/
	for (size_t i = 0; i < JSTR_ARRAY_SIZE(c_freq); ++i) {
		if (jstr_chk(jstr_utoa(JSTR_STRUCT(&file_str), i, 10)))
			jstr_errdie("Failed at jstr_ulltoa().");
		if (jstr_pushback_j(&file_str, ' '))
			jstr_errdie("Failed at jstr_pushback_j().");
		if (jstr_chk(jstr_ulltoa(JSTR_STRUCT(&file_str), c_freq[i], 10)))
			jstr_errdie("Failed at jstr_ulltoa().");
		if (jstr_pushback_j(&file_str, '\n'))
			jstr_errdie("Failed at jstr_pushback_j().");
	}
	if (jstr_chk(jstrio_fwrite(file_str.data, 1, file_str.size, stdout)))
		jstr_errdie("Failed at jstr_print().");
	jstr_free_j(&file_str);
	return EXIT_SUCCESS;
}
