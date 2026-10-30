# DLLProxyGen

DLLProxyGen is a C11 command-line tool for inspecting the static imports of an
x86-64 PE executable and the exports of a selected DLL. It emits auditable text
and JSON reports plus a MinGW-w64 proxy DLL project made exclusively from PE
export forwarders. No guessed C prototype is compiled into the proxy.

## Build

On MSYS2 UCRT64/MINGW64 or native MinGW-w64:

```sh
mingw32-make
mingw32-make test
```

For a Linux cross compiler, use `make CC=x86_64-w64-mingw32-gcc`; running the
Windows functional test requires Windows or Wine. Warning flags are
`-std=c11 -Wall -Wextra -Wpedantic -Wconversion`; debug builds use
`mingw32-make debug`.

## Use

```text
dllproxygen --exe target.exe --dll example.dll --out build/proxy
dllproxygen --exe target.exe --dll example.dll --out build/proxy --scope all --strict
```

The generated README gives the exact build command. The original must be staged
as `example_real.dll`, next to the proxy named `example.dll`. Forwarders point to
`example_real`, never back to `example`. Test this only in a controlled directory;
Windows DLL search and dependency resolution still apply.

`--scope imported` is the default. `--format text|json|both` controls reports.
`--strict` refuses unresolved imports. Delay imports and hook configuration are
explicitly rejected in the current release rather than being handled unsafely.

## Safety and current status

M1 (PE/import/export reporting) and M2 (working forwarding proxy) are implemented.
Inputs are parsed as bytes and never loaded or executed by the analyzer. Bounds,
section mappings, directory tables and terminated strings are checked. Import by
name and ordinal, aliases, ordinal holes and existing forwarded exports are parsed.

M3-M5 are not implemented. In particular, the report says `unknown` for every
signature. Capstone-backed call-site/callee analysis, deterministic constraint
synthesis and user-verified typed hooks remain future work. This is intentional:
an unverified prototype can never silently turn into a wrapper.

See [design](docs/design.md), [limitations](docs/limitations.md), and the
[report schema](docs/report-schema.json).
