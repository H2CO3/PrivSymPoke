#ifndef PRIVSYMPOKE_H
#define PRIVSYMPOKE_H

#include <dlfcn.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include "nm.h"

typedef struct PrivSymFile {
	struct nm_symbol_list *symtab;
	void *dlhandle;
} PrivSymFile;

PrivSymFile *privsym_OpenFile(const char *fname);
void *privsym_GetSymbol(PrivSymFile *file, const char *symname);
void privsym_CloseFile(PrivSymFile *file);


#endif // PRIVSYMPOKE_H

