param([Parameter(Mandatory=$true)][string]$Zig)
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath $Zig).Path
$zigVersion=(& $compiler version).Trim()
if($LASTEXITCODE -ne 0 -or $zigVersion -ne '0.13.0'){throw 'Use Zig 0.13.0 for the public release source.'}
$source=Join-Path $PSScriptRoot 'src'
$output=Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Path $output -Force | Out-Null
$oldGlobal=$env:ZIG_GLOBAL_CACHE_DIR
$oldLocal=$env:ZIG_LOCAL_CACHE_DIR
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $output '.zig-global'
$env:ZIG_LOCAL_CACHE_DIR=Join-Path $output '.zig-cache'
Push-Location $output
try {
    $inputs=@('silenttraverse.cpp','vtindex.c','third_party/minhook/src/hook.c','third_party/minhook/src/buffer.c','third_party/minhook/src/trampoline.c','third_party/minhook/src/hde/hde64.c') | ForEach-Object {Join-Path $source $_}
    & $compiler c++ -target x86_64-windows-gnu -O2 -s -shared -o silenttraverse.asi @inputs -luser32 -lversion -lwinmm -ladvapi32 -ldxguid
    if($LASTEXITCODE -ne 0){throw 'Native build failed'}
    & $compiler cc -target x86_64-windows-gnu -O2 -o host_guard_test.exe (Join-Path $source 'host_guard_test.c')
    if($LASTEXITCODE -ne 0){throw 'Host fixture build failed'}
    & $compiler c++ -target x86_64-windows-gnu -O2 -o budget_test.exe (Join-Path $source 'budget_test.cpp')
    if($LASTEXITCODE -ne 0){throw 'Budget fixture build failed'}
    $results=@()
    $results+= & .\host_guard_test.exe
    if($LASTEXITCODE -ne 0){throw 'Host guard fixture failed'}
    $results+= & .\budget_test.exe
    if($LASTEXITCODE -ne 0){throw 'Budget ownership fixture failed'}
    $results | Set-Content -LiteralPath (Join-Path $output 'test-results.txt')
    $results | Write-Output
    Write-Output 'Build and fixtures complete. No game files installed or changed.'
} finally {
    Pop-Location
    $env:ZIG_GLOBAL_CACHE_DIR=$oldGlobal
    $env:ZIG_LOCAL_CACHE_DIR=$oldLocal
}
