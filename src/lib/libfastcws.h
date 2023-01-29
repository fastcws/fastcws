// SPDX-License-Identifier: BSD-2-Clause

#ifndef _LIBFASTCWS_H_
#define _LIBFASTCWS_H_

#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32
	#ifdef FASTCWS_EXPORTING
		#define FASTCWS_API __declspec(dllexport)
	#else
		#define FASTCWS_API __declspec(dllimport)
	#endif
#else
	#ifdef FASTCWS_EXPORTING
		#define FASTCWS_API __attribute__((visibility("default")))
	#else
		#define FASTCWS_API
	#endif
#endif

#define FASTCWS_OK 0
#define FASTCWS_E_INTERNAL -1
#define FASTCWS_E_IO -2
#define FASTCWS_E_BAD_ENCODING -3
#define FASTCWS_E_OVERFLOW -4

typedef struct fastcws_ctx_s fastcws_ctx;
typedef struct fastcws_result_s fastcws_result;

FASTCWS_API void fastcws_init();

FASTCWS_API fastcws_ctx* fastcws_alloc_ctx();
FASTCWS_API fastcws_result* fastcws_alloc_result();
FASTCWS_API void fastcws_ctx_free(fastcws_ctx*);
FASTCWS_API void fastcws_result_free(fastcws_result*);

FASTCWS_API int fastcws_load_freq_dict(const char *filename, fastcws_ctx* ctx);
FASTCWS_API int fastcws_load_hmm_model(const char *filename, fastcws_ctx* ctx);

FASTCWS_API int fastcws_word_break(const char *cstr, fastcws_result* result);
FASTCWS_API int fastcws_word_break2(const char *cstr, fastcws_result* result, const fastcws_ctx* ctx);

FASTCWS_API int fastcws_result_next(fastcws_result*, const char** word_begin, size_t* word_len);

FASTCWS_API const char* fastcws_strerr(int);

#endif

