$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$work = Join-Path $PSScriptRoot 'work'
if (Test-Path -LiteralPath $work) { Remove-Item -LiteralPath $work -Recurse -Force }
New-Item -ItemType Directory -Path $work | Out-Null

& gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -I (Join-Path $root 'include') `
  (Join-Path $PSScriptRoot 'test_pe.c') (Join-Path $root 'src/model.c') `
  (Join-Path $root 'src/pe_reader.c') (Join-Path $root 'src/import_parser.c') `
  (Join-Path $root 'src/export_parser.c') -o (Join-Path $work 'test_pe.exe')
if ($LASTEXITCODE) { throw 'unit test compilation failed' }
& (Join-Path $work 'test_pe.exe')
if ($LASTEXITCODE) { throw 'unit tests failed' }

$implibOption = '-Wl,--out-implib,' + (Join-Path $work 'libsample.dll.a')
& gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -shared `
  (Join-Path $PSScriptRoot 'fixtures/sample.c') (Join-Path $PSScriptRoot 'fixtures/sample.def') `
  $implibOption -o (Join-Path $work 'sample.dll')
if ($LASTEXITCODE) { throw 'fixture DLL compilation failed' }
& gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion `
  (Join-Path $PSScriptRoot 'fixtures/sample_exe.c') (Join-Path $work 'libsample.dll.a') `
  -o (Join-Path $work 'sample.exe')
if ($LASTEXITCODE) { throw 'fixture EXE compilation failed' }

Push-Location $work
try {
  $baseline = & .\sample.exe
  if ($LASTEXITCODE -or $baseline -ne '42 15') { throw "baseline failed: $baseline" }
  & (Join-Path $root 'dllproxygen.exe') --exe sample.exe --dll sample.dll --out proxy --strict
  if ($LASTEXITCODE) { throw 'dllproxygen failed' }
  Copy-Item sample.dll proxy\sample_real.dll
  Push-Location proxy
  try {
    & gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -O2 -shared -o sample.dll proxy.c proxy.def
    if ($LASTEXITCODE) { throw 'generated proxy compilation failed' }
    $exports = & objdump -p sample.dll
    if (($exports -join "`n") -notmatch 'sample_real\.Named') { throw 'forwarder not found in generated DLL' }
    $proxied = & ..\sample.exe
    if ($LASTEXITCODE -or $proxied -ne '42 15') { throw "proxy execution failed: $proxied" }
  } finally { Pop-Location }
} finally { Pop-Location }
Write-Host 'functional proxy: all checks passed'
