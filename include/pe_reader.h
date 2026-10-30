#ifndef DLLPROXYGEN_PE_READER_H
#define DLLPROXYGEN_PE_READER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DPG_MAX_SECTIONS 96
#define DPG_MAX_DIRECTORIES 16

typedef struct {
    char name[9];
    uint32_t virtual_size;
    uint32_t virtual_address;
    uint32_t raw_size;
    uint32_t raw_offset;
    uint32_t characteristics;
} PeSection;

typedef struct {
    uint8_t *data;
    size_t size;
    char path[1024];
    uint16_t machine;
    uint16_t characteristics;
    uint32_t size_of_headers;
    uint32_t image_size;
    uint32_t entry_rva;
    uint32_t directory_rva[DPG_MAX_DIRECTORIES];
    uint32_t directory_size[DPG_MAX_DIRECTORIES];
    PeSection sections[DPG_MAX_SECTIONS];
    size_t section_count;
} PeImage;

bool pe_open(PeImage *pe, const char *path, char *err, size_t errcap);
bool pe_parse_memory(PeImage *pe, uint8_t *data, size_t size, const char *label,
                     char *err, size_t errcap);
void pe_close(PeImage *pe);
bool pe_rva_to_offset(const PeImage *pe, uint32_t rva, size_t need, size_t *offset);
bool pe_read_u16(const PeImage *pe, size_t off, uint16_t *out);
bool pe_read_u32(const PeImage *pe, size_t off, uint32_t *out);
bool pe_read_u64(const PeImage *pe, size_t off, uint64_t *out);
bool pe_read_cstring_rva(const PeImage *pe, uint32_t rva, char *out, size_t cap);

#endif
