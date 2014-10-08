PrivSymPoke, a library and tool to mess around with private symbols in Mach-O files.

Created by Arpad Goretity "H2CO3" on 08/10/2014.

The code heavily relies on a modified version of the Mac OS X 'nm' tool,
from Apple's cctools. See APPLE_LICENSE for annoying legal stuff I didn't
want to pollute this readme with.

The API
=======

High-level private symbol searching
-----------------------------------

Declared in `PrivSymPoke.h`.

    typedef struct PrivSymFile PrivSymFile;

A structure that represents a Mach-O file being raped.

    PrivSymFile *privsym_OpenFile(const char *fname);

Open a Mach-O file, read private symbols. Returns NULL pointer on failure.

    void privsym_CloseFile(PrivSymFile *file);

Close and free an open file structure.

    void *privsym_GetSymbol(PrivSymFile *file, const char *symname);

Retrieve a pointer to a symbol, private or public, with the name 'symname'.
The symbol name must be specified **with** the leading underscore (if any).
Returns NULL pointer on failure.

Lower-level Mach-O symbol table API
-----------------------------------

Declared in `nm.h`.

    struct nm_symbol_entry {
        char *name;
        unsigned char type;
        intptr_t address;
    };

Describes one individual symbol in a Mach-O file. 'type' is the type as
reported by 'nm': 't' for "TEXT", 'd' for DATA segments, etc. A lowercase
type means a private (`static`) symbol, whereas caps denote an exported
symbol.

    struct nm_symbol_list {
        uint32_t entry_count;
        struct nm_symbol_entry *entries;
    };
				
A simple dynamic array of symbol descriptors.


    struct nm_symbol_list *nm_read_symbols_from_file(const char *fname);

Returns an array of symbol descriptors from 'file'.
	
    void nm_free_symbol_list(struct nm_symbol_list *symlist);

Frees resources associated with `symlist`.


The tool
========

Usage, or this is how you pretty-print a hard-wired 1024-bit RSA-key:

    dumpsym /path/to/file.dylib '_MySecretRSAKeyThatYouWontEverFindOhWait' u8 128

`u8` denotes unsigned integer, 8 bits long (`uint8_t`). Other valid type specifiers:

    u16
	u32
	u64
	i8  // i* specifiers mean 'signed'
	i16
	i32
	i64
	float
	double

The last number is an integer specified using either decimal, hexadecimal or octal
notation (as required by `strtoul()`). It is the number of blocks to print.
Each block will be interpreted according to the type specified above, and will
also be formatted relevantly.

Enjoy!
-- H2CO3
