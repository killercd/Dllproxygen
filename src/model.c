#include "model.h"
#include <stdlib.h>

static bool grow(void **p, size_t *capacity, size_t item_size) {
    size_t next = *capacity ? *capacity * 2U : 16U;
    if (next < *capacity || next > SIZE_MAX / item_size) return false;
    void *q = realloc(*p, next * item_size);
    if (!q) return false;
    *p = q; *capacity = next; return true;
}
bool dpg_imports_push(DpgImports *v, const DpgImport *x) {
    if (v->count == v->capacity && !grow((void **)&v->items, &v->capacity, sizeof *v->items)) return false;
    v->items[v->count++] = *x; return true;
}
bool dpg_exports_push(DpgExports *v, const DpgExport *x) {
    if (v->count == v->capacity && !grow((void **)&v->items, &v->capacity, sizeof *v->items)) return false;
    v->items[v->count++] = *x; return true;
}
void dpg_imports_free(DpgImports *v) { free(v->items); *v = (DpgImports){0}; }
void dpg_exports_free(DpgExports *v) { free(v->items); *v = (DpgExports){0}; }
