#ifndef DLLPROXYGEN_REPORT_H
#define DLLPROXYGEN_REPORT_H
#include "model.h"
bool write_reports(const char *out_dir, const char *exe_path, const char *dll_path,
                   const DpgImports *imports, const DpgExports *exports,
                   bool want_text, bool want_json, char *err, size_t errcap);
#endif
