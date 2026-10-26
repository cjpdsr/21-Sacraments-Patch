"""Verify the imported public 0.5.2 snapshot without loading game/mod code."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parent.parent
p=json.loads((R/'SOURCE-PROVENANCE.json').read_text())
for name,digest in p['source_sha256'].items():
    assert hashlib.sha256((R/name).read_bytes()).hexdigest()==digest, f'Source changed: {name}'
assert (R/'LICENSE').read_bytes()==(R/'src/LICENSE').read_bytes()
worker=(R/'src/rt_budget.h').read_text()
assert 'VK_F7' not in worker and 'LANDMARK F7' not in worker
assert 'VK_F8' in worker and 'static LONG limit=50000;' in worker
body=(R/'src/silenttraverse.cpp').read_text()
assert '0.5.2-nexus-beta' in body
assert 'c.suballocate=0; c.toggleKey[0]=0; c.captureKey[0]=0;' in body
assert 'return false; // SH2 residency/aliasing/lifetime integration gate' in body
assert 'Lookahead::enabled=false;' in body
assert body.index('if (!SupportedHost())') < body.index('    LoadConfig();',body.index('static DWORD WINAPI InitThread'))
assert not any((R/'src').rglob('*.asi'))
for f in R.rglob('*'):
    if not f.is_file() or 'build' in f.relative_to(R).parts:continue
    assert f.suffix.lower() not in {'.asi','.dll','.exe','.lib','.pdb','.pak','.mp4','.etl','.ngfx-capture'}, f'Unexpected binary/private artifact: {f.name}'
    if f.suffix in {'.md','.txt','.json','.csv','.cpp','.c','.h','.ps1','.py'}:
        text=f.read_text(encoding='utf-8',errors='replace')
        assert 'C:'+'\\Users\\' not in text and 'C:'+'/Users/' not in text, f'Personal path: {f.name}'
print(f'PASS: {len(p["source_sha256"])} public-source hashes; F8-only worker; disabled allocator/prefetch; licenses; publication exclusions')
