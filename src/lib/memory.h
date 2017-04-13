#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdlib.h>

void *mem_alloc(size_t size);

void mem_free(void *ptr);

#endif /* _MEMORY_H_ */
