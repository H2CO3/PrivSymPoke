#include "PrivSymPoke.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>

PrivSymFile *privsym_OpenFile(const char *fname)
{
	// specify RTLD_NOW so that all symbol addresses are
	// valid when we start messing around with them
	void *hndl = dlopen(fname, RTLD_NOW | RTLD_GLOBAL);
	if (hndl == NULL) {
		return NULL;
	}

	// malloc does not tend to fail under modern OSes
	// (and if it does, Houston, we've got a problem!)
	PrivSymFile *file = malloc(sizeof(*file));

	file->dlhandle = hndl;
	file->symtab = nm_read_symbols_from_file(fname);

	return file;
}

// Looks up the symbol using 'symname'. If the lookup fails, and
// the symbol name starts with an underscore, then it removes
// the leading underscore from the symbol name and tries again.
static void *eager_dlsym(void *dlhandle, const char *symname)
{
	void *ptr = dlsym(dlhandle, symname);
	if (ptr)
		return ptr;

	if (symname[0] == '_')
		return dlsym(dlhandle, symname + 1);

	return NULL;
}

static void *symbol_lookup_helper(void *dlhndl, struct nm_symbol_entry *symtab, int32_t prv_idx, int32_t pub_idx)
{
	unsigned char prv_symtype = symtab[prv_idx].type;
	unsigned char pub_symtype = symtab[pub_idx].type;

	assert(islower(prv_symtype));

	// IFF the pub_idx-th symbol is extern, and it's in the
	// same section as our private symbol, then we've got a match
	if (!isupper(pub_symtype) || tolower(pub_symtype) != prv_symtype) {
		return NULL;
	}

	// Compute difference between addresses of symbols
	ptrdiff_t delta_addr = symtab[prv_idx].address - symtab[pub_idx].address;
	void *pub_symptr = eager_dlsym(dlhndl, symtab[pub_idx].name);

	// If the allegedly public symbol couldn't be retrieved,
	// then continue searching and look for the next candidate
	if (pub_symptr == NULL) {
		return NULL;
	}

	return (void *)((intptr_t)pub_symptr + delta_addr);
}

void *privsym_GetSymbol(PrivSymFile *file, const char *symname)
{
	// First, try getting the symbol using dlsym(), in case
	// it is an external-linkage (public) symbol.
	// (So, we don't hack unnecessarily...)
	void *addr = eager_dlsym(file->dlhandle, symname);
	if (addr != NULL) {
		printf("found public symbol!\n");
		return addr;
	}

	// If, however, it was not found using dlsym, it either does not
	// exist or it is private. Search for its name to see if it exists.
	struct nm_symbol_entry *symtab = file->symtab->entries;
	int32_t i, n = file->symtab->entry_count;
	for (i = 0; i < n; i++) {
		if (strcmp(symtab[i].name, symname) != 0) {
			continue;
		}

		// Got a match!
		// Check our assumption that the symbol is static/private,
		// i. e. that its type is a lowercase letter.
		if (!islower(symtab[i].type)) {
			return NULL;
		}

		// Search for a public/external symbol within the
		// same section as our private symbol.
		// First, start searching backwards, i. e. towards
		// lower addresses, since in this case there's a better
		// chance that the first public symbol we find resides
		// within the same 'region' (memory page? subsection?
		// whatever?) as our wanted private symbol. We need this
		// because - apparently - it's only within the same individual
		// region that the difference of offsets read from the Mach-O
		// file equals the difference in virtual memory addresses
		// (dlsym()'d pointers).
		int32_t j;
		for (j = i - 1; j >= 0; j--) {
			void *symptr = symbol_lookup_helper(file->dlhandle, symtab, i, j);

			if (symptr != NULL) {
				printf("Found private symbol (before)!\n");
				return symptr;
			}
		}

		// If no public symbol has been found _before_ the
		// searched-for private symbol, retry by stepping towards
		// higher addresses. This, however, decreases our chance to
		// find a public symbol within the same VM region, so we
		// now may get nonsense with a higher probability.
		// But this is a hack, so deal with it...
		for (j = i + 1; j < n; j++) {
			void *symptr = symbol_lookup_helper(file->dlhandle, symtab, i, j);

			if (symptr != NULL) {
				printf("Found private symbol (after)!\n");
				return symptr;
			}
		}

		break;
	}

	// If we got here, the symbol was not found
	return NULL;
}

void privsym_CloseFile(PrivSymFile *file)
{
	nm_free_symbol_list(file->symtab);
	dlclose(file->dlhandle);
	free(file);
}

