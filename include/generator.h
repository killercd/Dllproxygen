#ifndef DLLPROXYGEN_GENERATOR_H
#define DLLPROXYGEN_GENERATOR_H
#include "model.h"
bool generate_proxy(const char *out_dir, const char *dll_path, const DpgImports *imports,
                    const DpgExports *exports, bool all_exports, char *err, size_t errcap);
#endif
