#ifndef PRIVSIMPOKE_NM_H
#define PRIVSIMPOKE_NM_H

#include <stdint.h>
#include <sys/types.h>

struct nm_symbol_entry {
	char *name;
	unsigned char type;
	intptr_t address;
};

struct nm_symbol_list {
	uint32_t entry_count;
	struct nm_symbol_entry *entries;
};

struct nm_symbol_list *nm_read_symbols_from_file(const char *fname);
void nm_free_symbol_list(struct nm_symbol_list *symlist);

#endif // PRIVSIMPOKE_NM_H

