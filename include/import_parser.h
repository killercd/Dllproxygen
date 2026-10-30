#ifndef DLLPROXYGEN_IMPORT_PARSER_H
#define DLLPROXYGEN_IMPORT_PARSER_H
#include "model.h"
#include "pe_reader.h"
bool parse_imports(const PeImage *exe, const char *dll_basename, DpgImports *out,
                   char *err, size_t errcap);
#endif
