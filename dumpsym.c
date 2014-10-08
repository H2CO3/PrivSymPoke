#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>

#include "PrivSymPoke.h"

enum {
	SIGNED_TYPE = 0x80,
	BASETYPE_MASK = 0x0f,
	TYPEFLAG_MASK = 0xf0
};

enum {
	DUMP_AS_INT8 = 1,
	DUMP_AS_INT16,
	DUMP_AS_INT32,
	DUMP_AS_INT64,
	DUMP_AS_FLOAT32,
	DUMP_AS_FLOAT64
};

enum DUMP_TYPE {
	DUMP_INVALID   = -1,
	DUMP_AS_UINT8  = DUMP_AS_INT8,
	DUMP_AS_UINT16 = DUMP_AS_INT16,
	DUMP_AS_UINT32 = DUMP_AS_INT32,
	DUMP_AS_UINT64 = DUMP_AS_INT64,
	DUMP_AS_SINT8  = DUMP_AS_INT8  | SIGNED_TYPE,
	DUMP_AS_SINT16 = DUMP_AS_INT16 | SIGNED_TYPE,
	DUMP_AS_SINT32 = DUMP_AS_INT32 | SIGNED_TYPE,
	DUMP_AS_SINT64 = DUMP_AS_INT64 | SIGNED_TYPE,
	DUMP_AS_FLOAT  = DUMP_AS_FLOAT32,
	DUMP_AS_DOUBLE = DUMP_AS_FLOAT64
};

#define BASETYPE(type) ((type) & BASETYPE_MASK)
#define IS_SIGNED(type) (((type) & SIGNED_TYPE) != 0)

static const struct {
	enum DUMP_TYPE type;
	size_t size;
	const char *flag;
} dump_desc[] = {
	{ DUMP_AS_UINT8,  sizeof(uint8_t),  "u8"     },
	{ DUMP_AS_UINT16, sizeof(uint16_t), "u16"    },
	{ DUMP_AS_UINT32, sizeof(uint32_t), "u32"    },
	{ DUMP_AS_UINT64, sizeof(uint64_t), "u64"    },
	{ DUMP_AS_SINT8,  sizeof(int8_t),   "i8"     },
	{ DUMP_AS_SINT16, sizeof(int16_t),  "i16"    },
	{ DUMP_AS_SINT32, sizeof(int32_t),  "i32"    },
	{ DUMP_AS_SINT64, sizeof(int64_t),  "i64"    },
	{ DUMP_AS_FLOAT,  sizeof(float),    "float"  },
	{ DUMP_AS_DOUBLE, sizeof(double),   "double" }
};

static enum DUMP_TYPE get_dump_type(const char *arg)
{
	const size_t N = sizeof dump_desc / sizeof dump_desc[0];
	for (size_t i = 0; i < N; i++) {
		if (strcmp(arg, dump_desc[i].flag) == 0) {
			return dump_desc[i].type;
		}
	}

	return DUMP_INVALID;
}

static size_t get_dump_size(enum DUMP_TYPE type)
{
	const size_t N = sizeof dump_desc / sizeof dump_desc[0];
	for (size_t i = 0; i < N; i++) {
		if (dump_desc[i].type == type) {
			return dump_desc[i].size;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != 5) {
		printf("Usage: %s file symname blocktype count\n", argv[0]);
		return -1;
	}

	enum DUMP_TYPE dump_type = get_dump_type(argv[3]);
	if (dump_type == DUMP_INVALID) {
		printf("blocktype must be one of u[8..64], i[8..64], float, double\n");
		return -1;
	}

	size_t blocksize = get_dump_size(dump_type);
	assert(blocksize > 0);

	const char *filename = argv[1];
	const char *symname = argv[2];
	size_t count = strtoul(argv[4], NULL, 0);

	PrivSymFile *file = privsym_OpenFile(filename);
	uint8_t *symaddr = privsym_GetSymbol(file, symname);

	if (symaddr == NULL) {
		printf("Symbol '%s' could not be found\n", symname);
		privsym_CloseFile(file);
		return -1;
	}

	for (; count--; symaddr += blocksize) {
		switch (BASETYPE(dump_type)) {
		case DUMP_AS_INT8: {
			uint8_t val = symaddr[0];
			printf("0x%02" PRIx8, val);

			if (IS_SIGNED(dump_type)) {
				printf(" (%" PRIi8 " == '%c')\n", (int8_t)val, (signed char)val);
			} else {
				printf(" (%" PRIu8 " == '%c')\n", val, (unsigned char)val);
			}

			break;
		}
		case DUMP_AS_INT16: {
			uint16_t b0 = symaddr[0];
			uint16_t b1 = symaddr[1];
			uint16_t val = (b0 << 0) | (b1 << 8);
			printf("%0x04" PRIx16, val);

			if (IS_SIGNED(dump_type)) {
				printf(" (%" PRIi16 ")\n", (int16_t)val);
			} else {
				printf(" (%" PRIu16 ")\n", val);
			}

			break;
		}
		case DUMP_AS_INT32: {
			uint32_t b0 = symaddr[0];
			uint32_t b1 = symaddr[1];
			uint32_t b2 = symaddr[2];
			uint32_t b3 = symaddr[3];
			uint32_t val = (b0 << 0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
			printf("0x%08" PRIx32, val);

			if (IS_SIGNED(dump_type)) {
				printf(" (%" PRIi32 ")\n", (int32_t)val);
			} else {
				printf(" (%" PRIu32 ")\n", val);
			}

			break;
		}
		case DUMP_AS_INT64: {
			uint64_t b0 = symaddr[0];
			uint64_t b1 = symaddr[1];
			uint64_t b2 = symaddr[2];
			uint64_t b3 = symaddr[3];
			uint64_t b4 = symaddr[4];
			uint64_t b5 = symaddr[5];
			uint64_t b6 = symaddr[6];
			uint64_t b7 = symaddr[7];
			uint64_t val = (b0 <<  0) | (b1 << 8)  | (b2 << 16) | (b3 << 24)
			             | (b4 << 32) | (b5 << 40) | (b6 << 48) | (b7 << 56);
			printf("0x%016" PRIx64, val);

			if (IS_SIGNED(dump_type)) {
				printf(" (%" PRIi64 ")\n", (int64_t)val);
			} else {
				printf(" (%" PRIu64 ")\n", val);
			}

			break;
		}
		case DUMP_AS_FLOAT: {
			uint32_t b0 = symaddr[0];
			uint32_t b1 = symaddr[1];
			uint32_t b2 = symaddr[2];
			uint32_t b3 = symaddr[3];

			uint32_t val = (b0 << 0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
			union {
				uint32_t u;
				float f;
			} pun = { .u = val };

			printf(
				"0x%02x%02x%02x%02x (%.*e = %.*a hex)\n",
				(unsigned)b3, (unsigned)b2, (unsigned)b1, (unsigned)b0,
				FLT_DIG, pun.f, FLT_MANT_DIG / 4 /* log2(16) = 4 */, pun.f
			);
			break;
		}
		case DUMP_AS_DOUBLE: {
			uint64_t b0 = symaddr[0];
			uint64_t b1 = symaddr[1];
			uint64_t b2 = symaddr[2];
			uint64_t b3 = symaddr[3];
			uint64_t b4 = symaddr[4];
			uint64_t b5 = symaddr[5];
			uint64_t b6 = symaddr[6];
			uint64_t b7 = symaddr[7];

			uint64_t val = (b0 <<  0) | (b1 << 8)  | (b2 << 16) | (b3 << 24)
			             | (b4 << 32) | (b5 << 40) | (b6 << 48) | (b7 << 56);

			union {
				uint64_t u;
				double f;
			} pun = { .u = val };

			printf(
				"0x%02x%02x%02x%02x%02x%02x%02x%02x (%.*le = %.*la hex)\n",
				(unsigned)b7, (unsigned)b6, (unsigned)b5, (unsigned)b4,
				(unsigned)b3, (unsigned)b2, (unsigned)b1, (unsigned)b0,
				DBL_DIG, pun.f, DBL_MANT_DIG / 4 /* log2(16) = 4 */, pun.f
			);
			break;
		}
		default:
			assert(0 == "You should never get here");
		}
	}

	privsym_CloseFile(file);
	return 0;
}

