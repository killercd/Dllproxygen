#ifndef DLLPROXYGEN_EXPORT_PARSER_H
#define DLLPROXYGEN_EXPORT_PARSER_H
#include "model.h"
#include "pe_reader.h"
bool parse_exports(const PeImage *dll, DpgExports *out, char *dll_internal_name,
                   size_t namecap, char *err, size_t errcap);
void match_imports(DpgImports *imports, const DpgExports *exports);
#endif
