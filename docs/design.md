# Design

The implementation is split into byte-level PE ingestion, import parsing, export
parsing/matching, reporting, and generation modules. `pe_rva_to_offset` accepts
only header bytes or raw bytes actually present in a section; mapped-only virtual
tails are rejected. All arithmetic crossing the file boundary is checked before
access.

The import parser selects descriptors by case-insensitive DLL basename and uses
`OriginalFirstThunk` when present. If it is absent, `FirstThunk` is accepted only
when its values decode as valid ordinal or import-by-name entries; bound addresses
are rejected. Static import presence is not interpreted as evidence of a call.

The export parser uses the ordinal base and address/name/ordinal tables separately.
Zero EAT entries are holes. Multiple names targeting one EAT entry become alias
records. An address inside the export-directory range is parsed as a forwarder.

Generation maps selected names/ordinals to a separately loadable `*_real` module.
The generated C file contains only a loader-lock-safe `DllMain`; per-call user code
cannot run through an export forwarder. Future verified wrappers belong in a
separate hook-generation stage, not in signature inference.
