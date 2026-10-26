// Silent Traverse: fresh committed-resource lookahead. No used-resource recycling.
// Requires original CCR function pointers, MemoryInfo, Now/Ms, Log and config path.
#include <atomic>
static bool PrefetchHasLiveQueues(ID3D12Device* d);
namespace Lookahead {
constexpr int KEYS=128, DEPTH=2;
#ifndef PREFETCH_CAP_MB
#define PREFETCH_CAP_MB 512
#endif
constexpr UINT64 CAP=(UINT64)PREFETCH_CAP_MB<<20, MAX_RESOURCE=16ull<<20;
struct Spec {
 ID3D12Device* device=nullptr; D3D12_HEAP_PROPERTIES hp={};
 D3D12_HEAP_FLAGS flags=D3D12_HEAP_FLAG_NONE; D3D12_RESOURCE_DESC desc={};
 D3D12_RESOURCE_STATES state=D3D12_RESOURCE_STATE_COMMON; GUID iid={}; int api=0;
};
inline bool Same(const Spec&a,const Spec&b){
 return a.device==b.device && a.api==b.api && a.flags==b.flags && a.state==b.state && a.iid==b.iid &&
 a.hp.Type==b.hp.Type && a.hp.CPUPageProperty==b.hp.CPUPageProperty && a.hp.MemoryPoolPreference==b.hp.MemoryPoolPreference &&
 a.hp.CreationNodeMask==b.hp.CreationNodeMask && a.hp.VisibleNodeMask==b.hp.VisibleNodeMask &&
 a.desc.Dimension==b.desc.Dimension && a.desc.Alignment==b.desc.Alignment && a.desc.Width==b.desc.Width &&
 a.desc.Height==b.desc.Height && a.desc.DepthOrArraySize==b.desc.DepthOrArraySize && a.desc.MipLevels==b.desc.MipLevels &&
 a.desc.Format==b.desc.Format && a.desc.SampleDesc.Count==b.desc.SampleDesc.Count && a.desc.SampleDesc.Quality==b.desc.SampleDesc.Quality &&
 a.desc.Layout==b.desc.Layout && a.desc.Flags==b.desc.Flags;
}
struct Fresh { IUnknown* object=nullptr; HRESULT hr=S_OK; LONGLONG born=0; };
struct Entry {
 bool used=false, flight=false; UINT64 epoch=0, bytes=0;
 Spec spec; ID3D12Device* pending=nullptr; LONGLONG seen=0;
 Fresh ready[DEPTH]; int count=0;
};
struct Counters { LONG64 hits=0, misses=0, made=0, discarded=0, failed=0, rejected=0, budgetHeld=0; };
static SRWLOCK lock=SRWLOCK_INIT;
static Entry entries[KEYS]; static UINT64 readyBytes=0,flightBytes=0,epoch=1;
static LONGLONG cooldownUntil=0;static int cursor=0;
static std::atomic<LONGLONG> lastRequest{0};
static std::atomic<bool> enabled{true},stopping{false};
static std::atomic<LONG64> hits{0},misses{0},made{0},discarded{0},failed{0},rejected{0},budgetHeld{0};
static ID3D12Device* banned[16]={}; static int bannedCount=0;
static bool Banned(ID3D12Device*d){for(int i=0;i<bannedCount;i++)if(banned[i]==d)return true;return false;}
// All private objects are virgin: these are their only references. Release them
// only outside the cache lock; they were never exposed to a GPU command list.
static void Drain(ID3D12Device* device=nullptr,bool ban=false){
 IUnknown* release[KEYS*(DEPTH+1)]={};int n=0;UINT64 retiring=0;
 AcquireSRWLockExclusive(&lock);
 if(ban&&device&&bannedCount<16&&!Banned(device))banned[bannedCount++]=device;
 for(auto&e:entries)if(e.used&&(!device||e.spec.device==device)){
  for(int i=0;i<e.count;i++){release[n++]=e.ready[i].object;retiring+=e.bytes;discarded++;}
  e.count=0;if(e.pending){release[n++]=e.pending;e.pending=nullptr;}
  // An in-flight worker owns a copied spec/device reference and returns to this
  // slot only if its generation still matches. It retains its reserved bytes.
  e.used=false;e.epoch=++epoch;
 }
 ReleaseSRWLockExclusive(&lock);
 for(int i=0;i<n;i++)release[i]->Release();
 AcquireSRWLockExclusive(&lock);readyBytes-=retiring;ReleaseSRWLockExclusive(&lock);
}
static bool Allowed(const Spec&s,UINT_PTR caller){
 HMODULE exe=GetModuleHandleW(nullptr);auto dos=(IMAGE_DOS_HEADER*)exe;
 auto nt=(IMAGE_NT_HEADERS*)((BYTE*)exe+dos->e_lfanew);
 if(caller<(UINT_PTR)exe||caller>=(UINT_PTR)exe+nt->OptionalHeader.SizeOfImage)return false;
 if(!g_adapterKnown)return false;
 LUID luid=s.device->GetAdapterLuid();
 if(luid.HighPart!=g_adapterLuid.HighPart||luid.LowPart!=g_adapterLuid.LowPart)return false;
 if(s.api<0||s.api>1||s.hp.Type!=D3D12_HEAP_TYPE_DEFAULT||s.hp.CreationNodeMask>1||s.hp.VisibleNodeMask>1)return false;
 if(s.flags!=D3D12_HEAP_FLAG_NONE && s.flags!=D3D12_HEAP_FLAG_CREATE_NOT_ZEROED)return false;
 if(s.desc.Flags!=D3D12_RESOURCE_FLAG_NONE||s.desc.SampleDesc.Count!=1||s.desc.SampleDesc.Quality)return false;
 if(s.desc.Dimension<D3D12_RESOURCE_DIMENSION_BUFFER||s.desc.Dimension>D3D12_RESOURCE_DIMENSION_TEXTURE3D)return false;
 return s.iid==__uuidof(ID3D12Resource); // Exact base interface only in this first candidate.
}
static Spec Make(ID3D12Device*d,const D3D12_HEAP_PROPERTIES*hp,D3D12_HEAP_FLAGS flags,const D3D12_RESOURCE_DESC*desc,D3D12_RESOURCE_STATES st,REFIID iid,int api){
 Spec s;s.device=d;s.hp=*hp;s.flags=flags;s.desc=*desc;s.state=st;s.iid=iid;s.api=api;return s;
}
static bool Take(const Spec&s,void**pp,HRESULT&hr){
 if(!enabled.load()||stopping.load()||!pp)return false;
 if(!PrefetchHasLiveQueues(s.device))return false;
 if(s.device->GetDeviceRemovedReason()!=S_OK){Drain(s.device,true);return false;}
 IUnknown* object=nullptr;bool expired=false;
 AcquireSRWLockExclusive(&lock);
 if(enabled.load()&&!Banned(s.device))for(auto&e:entries)if(e.used&&Same(e.spec,s)){
  if(e.count){
   Fresh f=e.ready[e.count-1];
   expired=Ms(Now()-f.born)>10000||Ms(Now()-e.seen)>10000;
   if(!expired){--e.count;object=f.object;hr=f.hr;readyBytes-=e.bytes;hits++;}
  }
  break;
 }
 ReleaseSRWLockExclusive(&lock);
 if(expired)Drain(s.device);
 if(!object){misses++;return false;}
 *pp=object; // Transfer original creation reference, with no AddRef or wrapping.
 return true;
}
static void Learn(const Spec&s,UINT64 bytes){
 if(!enabled.load()||stopping.load()||!bytes||bytes>MAX_RESOURCE)return;
 AcquireSRWLockExclusive(&lock);
 if(!enabled.load()||Banned(s.device)){ReleaseSRWLockExclusive(&lock);return;}
 Entry* slot=nullptr;
 for(auto&e:entries)if(e.used&&Same(e.spec,s)){slot=&e;break;}
 if(!slot)for(auto&e:entries)if(!e.used&&!e.flight){slot=&e;e=Entry();e.used=true;e.epoch=++epoch;e.spec=s;e.bytes=bytes;break;}
 if(slot){
  slot->seen=Now();
  if(slot->count<DEPTH&&!slot->pending&&!slot->flight){s.device->AddRef();slot->pending=s.device;}
 }
 ReleaseSRWLockExclusive(&lock);
}
static bool TryTake(ID3D12Device*d,const D3D12_HEAP_PROPERTIES*hp,D3D12_HEAP_FLAGS flags,const D3D12_RESOURCE_DESC*desc,D3D12_RESOURCE_STATES st,const D3D12_CLEAR_VALUE*cv,REFIID iid,void**pp,int api,UINT_PTR caller,HRESULT&hr){
 if(!hp||!desc||!pp||cv||!enabled.load())return false;
 Spec s=Make(d,hp,flags,desc,st,iid,api);
 if(!Allowed(s,caller)){rejected++;return false;}
 lastRequest=Now();
 return Take(s,pp,hr);
}
static void Observe(ID3D12Device*d,const D3D12_HEAP_PROPERTIES*hp,D3D12_HEAP_FLAGS flags,const D3D12_RESOURCE_DESC*desc,D3D12_RESOURCE_STATES st,const D3D12_CLEAR_VALUE*cv,REFIID iid,void**pp,int api,UINT_PTR caller,HRESULT hr){
 if(hr!=S_OK||!hp||!desc||!pp||!*pp||cv||!enabled.load())return;
 Spec s=Make(d,hp,flags,desc,st,iid,api);if(!Allowed(s,caller))return;
 lastRequest=Now();
 auto ai=d->GetResourceAllocationInfo(0,1,desc);
 if(ai.SizeInBytes==UINT64_MAX||!ai.Alignment||ai.Alignment>65536||!ai.SizeInBytes)return;
 Learn(s,ai.SizeInBytes);
}
// One background creation at a time. In-flight reservations prevent overbooking;
// resident ready bytes are already included in DXGI CurrentUsage.
static bool Pump(){
 DXGI_QUERY_VIDEO_MEMORY_INFO mem={};bool budget=MemoryInfo(true,mem)&&mem.Budget>0;
 if(!budget||mem.CurrentUsage>mem.Budget*9/10){budgetHeld++;Drain();return false;}
 Entry* chosen=nullptr;Spec s;UINT64 gen=0,bytes=0;ID3D12Device* dev=nullptr;
 IUnknown* stale[KEYS*(DEPTH+1)]={};int staleN=0;UINT64 staleBytes=0;LONGLONG now=Now();
 AcquireSRWLockExclusive(&lock);
 for(int k=0;k<KEYS;k++){auto&e=entries[(cursor+k)%KEYS];if(e.used){
  if(Ms(now-e.seen)>10000){
   for(int i=0;i<e.count;i++){stale[staleN++]=e.ready[i].object;staleBytes+=e.bytes;discarded++;}
   e.count=0;if(e.pending){stale[staleN++]=e.pending;e.pending=nullptr;}
   e.used=false;e.epoch=++epoch;continue;
  }
  while(e.count && Ms(now-e.ready[0].born)>10000){stale[staleN++]=e.ready[0].object;staleBytes+=e.bytes;discarded++;for(int i=1;i<e.count;i++)e.ready[i-1]=e.ready[i];--e.count;}
  if(!e.pending||e.flight||e.count>=DEPTH||Ms(now-e.seen)<150||Ms(now-lastRequest.load())<150)continue;
  if(readyBytes+flightBytes+e.bytes>CAP || mem.CurrentUsage+flightBytes+e.bytes>mem.Budget*85/100){budgetHeld++;continue;}
  if(!chosen&&enabled.load()&&!Banned(e.spec.device)&&now>=cooldownUntil){
   chosen=&e;s=e.spec;gen=e.epoch;bytes=e.bytes;dev=e.pending;e.pending=nullptr;e.flight=true;flightBytes+=bytes;
  }
 }}
 cursor=(cursor+1)%KEYS;
 ReleaseSRWLockExclusive(&lock);
 for(int i=0;i<staleN;i++)stale[i]->Release();
 AcquireSRWLockExclusive(&lock);readyBytes-=staleBytes;ReleaseSRWLockExclusive(&lock);
 if(!chosen)return false;
 IUnknown* fresh=nullptr;HRESULT hr=E_FAIL;
 if(PrefetchHasLiveQueues(dev)&&dev->GetDeviceRemovedReason()==S_OK){
  if(s.api==0&&o_CCR)hr=o_CCR(dev,&s.hp,s.flags,&s.desc,s.state,nullptr,s.iid,(void**)&fresh);
  if(s.api==1&&o_CCR1)hr=o_CCR1(dev,&s.hp,s.flags,&s.desc,s.state,nullptr,nullptr,s.iid,(void**)&fresh);
 }
 bool kept=false;
 AcquireSRWLockExclusive(&lock);
 chosen->flight=false;
 if(chosen->used&&chosen->epoch==gen&&enabled.load()&&!Banned(dev)&&hr==S_OK&&fresh&&chosen->count<DEPTH){
  chosen->ready[chosen->count++]={fresh,hr,Now()};readyBytes+=bytes;flightBytes-=bytes;made++;kept=true;
  if(chosen->count<DEPTH&&!chosen->pending){dev->AddRef();chosen->pending=dev;}
 }else {failed++;cooldownUntil=Now()+g_qpcFreq;}
 ReleaseSRWLockExclusive(&lock);
 if(fresh&&!kept)fresh->Release();
 if(!kept){AcquireSRWLockExclusive(&lock);flightBytes-=bytes;ReleaseSRWLockExclusive(&lock);}
 dev->Release();return kept;
}
static void Report(){
 UINT64 bytes,flight;int keys=0,ready=0;
 AcquireSRWLockShared(&lock);bytes=readyBytes;flight=flightBytes;for(auto&e:entries)if(e.used){keys++;ready+=e.count;}ReleaseSRWLockShared(&lock);
 char vram[160];VramText(vram,sizeof(vram));
 Log2("PREFETCH %s hits=%lld misses=%lld made=%lld rejected=%lld failures=%lld discarded=%lld budget_holds=%lld | keys=%d ready=%d %.1f MB in_flight=%.1f MB | %s",
 enabled.load()?"ON":"off",hits.load(),misses.load(),made.load(),rejected.load(),failed.load(),discarded.load(),budgetHeld.load(),keys,ready,bytes/1048576.0,flight/1048576.0,vram);
}
static DWORD WINAPI Worker(LPVOID){
 SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
 enabled=GetPrivateProfileIntA("NativePrefetch","Enabled",1,g_files.iniPath)!=0;
 if(!enabled.load())Drain();
 Log("PREFETCH: active fresh committed-resource lookahead; cap=512 MB, depth=2, max=16 MB; F9 toggles. Used resources are never recycled.");
 int ticks=0;bool down=false;
 while(!stopping.load()){
  DWORD pid=0;GetWindowThreadProcessId(GetForegroundWindow(),&pid);bool key=pid==GetCurrentProcessId()&&(GetAsyncKeyState(VK_F9)&0x8000);
  if(key&&!down){enabled=!enabled.load();if(!enabled.load())Drain();WritePrivateProfileStringA("NativePrefetch","Enabled",enabled.load()?"1":"0",g_files.iniPath);Log("PREFETCH toggled %s; already handed-off game resources are unchanged",enabled.load()?"ON":"off");}
  down=key;
  if(enabled.load())Pump();
  if(++ticks%500==0)Report();
  Sleep(20);
 }
 Drain();return 0;
}
} // namespace Lookahead
static void PrefetchRetire(ID3D12Device*d){Lookahead::Drain(d,true);}
