/* Copyright (c) 2023 James Tirta Halim <tirtajames45 at gmail dot com>
   This file is part of the jstring library.

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

#include "./jstring/src/jstr-io.h"
#include "./jstring/src/jstr.h"

size_t c_freq[256];
jstr_ty file_str = JSTR_INIT;

int
callback(const char *file,
         size_t file_len,
         const struct stat *st)
{
	if (st->st_size >= 100 * JSTRIO_MB)
		goto ret;
	if (jstr_chk(jstrio_readfile_len(JSTR_STRUCT(&file_str), file, st->st_size))) {
		jstr_errdie("Failed at jstrio_readfile_len().");
		return JSTR_RET_ERR;
	}
	jstrio_ext_ty ext = jstrio_exttype(file, file_len);
	if (ext == JSTRIO_FT_BINARY)
		goto ret;
	if (ext == JSTRIO_FT_UNKNOWN)
		if (jstrio_isbinary_maybe(file_str.data, file_str.size))
			goto ret;
	jstr_foreach(&file_str, p)
	{
		if (jstr_likely(c_freq[(unsigned char)*p] < (size_t)-1))
			++c_freq[(unsigned char)*p];
	}
ret:
	return JSTR_RET_SUCC;
	(void)file_len;
}

int
main(int argc,
     char **argv)
{
	if (jstr_unlikely(argc <= 1)) {
		fprintf(stderr, "Usage: %s <directory> ...\nMultiple directories may be used as arguments.", argv[0]);
	}
	for (size_t i = 1; argv[i]; ++i) {
		if (jstr_chk(jstrio_ftw_len(argv[i], strlen(argv[i]), callback, JSTRIO_FTW_REG | JSTRIO_FTW_STATREG, "*.[ch]", 0)))
			jstr_errdie("Failed at jstrio_ftw_len().");
		jstr_free_j(&file_str);
	}
	/*
	  Format:
	  ASCII bytes
	*/
	for (size_t i = 0; i < JSTR_ARRAY_SIZE(c_freq); ++i)
		printf("%zu %zu\n", i, c_freq[i]);
	return 0;
}
