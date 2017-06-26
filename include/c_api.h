#ifndef C_API_H_
#define C_API_H_

#ifdef __cplusplus
#define TLDA_EXTERN_C extern "C"
#endif

#include <stdint.h>

typedef void *TLDAHandle;

TLDA_EXTERN_C int TLDACreateFromFile(const char *corpora_filename,
                       const char *vocab_filename,
                       int num_topics,
                       double alpha,
                       double eta,
                       TLDAHandle *out);

TLDA_EXTERN_C int TLDAInference(TLDAHandle handle, int maxiter);

TLDA_EXTERN_C const char *TLDAGetAuthorName();

TLDA_EXTERN_C int TLDAFree(TLDAHandle handle);

TLDA_EXTERN_C int TLDAGetResult(TLDAHandle handle,
                                uint64_t *n_rows,
                                uint64_t *n_cols,
                                const double **out_result);

TLDA_EXTERN_C int TLDASave(TLDAHandle handle, const char *resdir); 


#endif

