// SPDX-License-Identifier: BSD-2-Clause

#include <stdio.h>
#include <stdlib.h>

#include "libfastcws.h"

int main(void) {
#ifdef _WIN32
	// we are going to output utf-8
	system("chcp 65001");
#endif
	fastcws_init();
	fastcws_result* result = fastcws_alloc_result();
	
	int err = fastcws_word_break("在春风吹拂的季节翩翩起舞", result);
	if (err) {
		fprintf(stderr, "fastcws_word_break(): %s\n", fastcws_strerr(err));
		fastcws_result_free(result);
		return 1;
	}
	const char *word_begin;
	size_t word_len;
	size_t i = 0;
	while(fastcws_result_next(result, &word_begin, &word_len) == 0) {
		if (i != 0) {
			printf("\n");
		}
		printf("%.*s", (int)word_len, word_begin);
		i++;
	}
	fflush(stdout);
	fastcws_result_free(result);
	return 0;
}

