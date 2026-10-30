#ifndef DLLPROXYGEN_MODEL_H
#define DLLPROXYGEN_MODEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DPG_MAX_NAME 512

typedef struct {
    char dll_name[DPG_MAX_NAME];
    char name[DPG_MAX_NAME];
    bool by_ordinal;
    uint16_t ordinal;
    uint16_t hint;
    uint32_t iat_rva;
    uint32_t ilt_rva;
    bool matched;
    size_t export_index;
} DpgImport;

typedef struct {
    char name[DPG_MAX_NAME];
    uint16_t ordinal;
    uint32_t rva;
    bool has_name;
    bool noname;
    bool is_forwarder;
    char forwarder[DPG_MAX_NAME];
} DpgExport;

typedef struct { DpgImport *items; size_t count; size_t capacity; } DpgImports;
typedef struct { DpgExport *items; size_t count; size_t capacity; } DpgExports;

void dpg_imports_free(DpgImports *v);
void dpg_exports_free(DpgExports *v);
bool dpg_imports_push(DpgImports *v, const DpgImport *item);
bool dpg_exports_push(DpgExports *v, const DpgExport *item);

#endif
