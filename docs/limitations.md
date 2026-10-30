# Limitations

- Only AMD64 PE32+ static imports are supported.
- Delay imports and `LoadLibrary`/`GetProcAddress` paths are not analyzed.
- Data exports cannot be distinguished reliably from functions and need manual review.
- Signatures, argument counts and semantic types are not inferred in M1/M2.
- Variadic/custom ABI functions, aggregates, C++ methods, TLS and exceptions need
  specialized handling before typed hooks can be safe.
- Forwarder module lookup follows Windows loader rules; absolute paths cannot be
  embedded as a general solution.
- Export names containing control characters or quotes are refused. Exotic `.def`
  spellings may require manual handling.
- The functional test runs on Windows. A cross-built executable needs Wine or a
  Windows host for execution.
