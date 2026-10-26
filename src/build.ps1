param([Parameter(Mandatory=$true)][string]$Zig)
$ErrorActionPreference='Stop'
Push-Location $PSScriptRoot
try {
  $env:ZIG_GLOBAL_CACHE_DIR=Join-Path $PSScriptRoot '.zig-global'
  $env:ZIG_LOCAL_CACHE_DIR=Join-Path $PSScriptRoot '.zig-cache'
  & $Zig c++ -target x86_64-windows-gnu -O2 -s -shared -o silenttraverse.asi silenttraverse.cpp vtindex.c third_party/minhook/src/hook.c third_party/minhook/src/buffer.c third_party/minhook/src/trampoline.c third_party/minhook/src/hde/hde64.c -luser32 -lversion -lwinmm -ladvapi32 -ldxguid
  if($LASTEXITCODE -ne 0){throw 'Native build failed'}
  & $Zig cc -target x86_64-windows-gnu -O2 -o host_guard_test.exe host_guard_test.c
  if($LASTEXITCODE -ne 0){throw 'Guard fixture build failed'}
  & .\host_guard_test.exe
  if($LASTEXITCODE -ne 0){throw 'Guard fixture failed'}
} finally {Pop-Location}

