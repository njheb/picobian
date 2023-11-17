#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define UF2_MAGIC_START0 0x0A324655u
#define UF2_MAGIC_START1 0x9E5D5157u
#define UF2_MAGIC_END    0x0AB16F30u
#define UF2_FLAG_FAMILY_ID_PRESENT  0x00002000u

#define RP2040_FAMILY_ID 0xe48bff56

struct uf2_block {
	uint32_t magic_start0;
	uint32_t magic_start1;
	uint32_t flags;
	uint32_t target_addr;
	uint32_t payload_size;
	uint32_t block_num;
	uint32_t num_blocks;
	uint32_t family_id;
	uint8_t data[476];
	uint32_t magic_end;
};

#define FLASH_BASE 0x10000000
#define PAGE_SIZE 256
#define FLASH_SECTOR_ERASE_SIZE 4096
#define FLASH_SECTOR_ERASE_PAGES (FLASH_SECTOR_ERASE_SIZE / PAGE_SIZE)

int bin2uf2(FILE *in, FILE *out) {
	if (fseek(in, 0, SEEK_END)) return 1;
	const long in_len = ftell(in);
	if (in_len < 0) return 1;
	if (fseek(in, 0, SEEK_SET)) return 1;

	unsigned num_pages = (in_len + (PAGE_SIZE - 1)) / PAGE_SIZE;

	// The Pico bootloader requires us to pad the image to a multiple of the flash
	// sector size.
	num_pages = (num_pages + (FLASH_SECTOR_ERASE_PAGES - 1));
	num_pages = num_pages - (num_pages % FLASH_SECTOR_ERASE_PAGES);

	struct uf2_block block;
	block.magic_start0 = UF2_MAGIC_START0;
	block.magic_start1 = UF2_MAGIC_START1;
	block.flags = UF2_FLAG_FAMILY_ID_PRESENT;
	block.payload_size = PAGE_SIZE;
	block.num_blocks = num_pages;
	block.family_id = RP2040_FAMILY_ID;
	block.magic_end = UF2_MAGIC_END;

	block.target_addr = FLASH_BASE;
	memset(block.data, 0, sizeof block.data); // Padding must be zeroed

	// When this is true, we have finished the input file, and are writing padding
	// pages. The block data is already zeroed.
	bool in_padding = false;

	for (unsigned i = 0; i < num_pages; ++i, block.target_addr += PAGE_SIZE) {
		block.block_num = i;

		unsigned zero_for_padding = 0;
		if (!in_padding) {
			size_t bytes = fread(block.data, 1, 256, in);
			if (bytes < 256) {
				memset(block.data + bytes, 0, 256 - bytes);
				in_padding = true;
				zero_for_padding = bytes;
			}
		}

		if (fwrite(&block, sizeof block, 1, out) != 1) {
			return 2;
		}

		if (zero_for_padding > 0) {
			memset(block.data, 0, zero_for_padding);
		}
	}

	return 0;
}

int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "usage: %s <in.bin> <out.uf2>\n", argv[0]);
		return 1;
	}

	FILE *in = fopen(argv[1], "rb");
	if (!in) return 2;

	FILE *out = fopen(argv[2], "wb");
	if (!out) {
		fclose(in);
		return 2;
	}

	int ret = bin2uf2(in, out);
	fclose(in);
	fclose(out);
	return ret == 0 ? 0 : ret + 2;
}
