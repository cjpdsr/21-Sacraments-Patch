#include "budget_override.h"
static bool DeviceReady();
namespace RtBudget {
static BudgetOverride state;
static LONG limit=50000;
static bool Range(const void*p,SIZE_T n,bool write){
 MEMORY_BASIC_INFORMATION m={};
 if(!p||!VirtualQuery(p,&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;
 if((UINT_PTR)p+n<(UINT_PTR)p||(UINT_PTR)p+n>(UINT_PTR)m.BaseAddress+m.RegionSize)return false;
 DWORD protection=m.Protect&0xff;
 if(write)return protection==PAGE_READWRITE||protection==PAGE_WRITECOPY;
 return protection==PAGE_READONLY||protection==PAGE_READWRITE||protection==PAGE_WRITECOPY||protection==PAGE_EXECUTE_READ||protection==PAGE_EXECUTE_READWRITE;
}
static volatile LONG* Resolve(){
 BYTE*base=(BYTE*)GetModuleHandleW(nullptr);auto dos=(IMAGE_DOS_HEADER*)base;
 auto nt=(IMAGE_NT_HEADERS*)(base+dos->e_lfanew);UINT_PTR size=nt->OptionalHeader.SizeOfImage;
 if(size<0x82633d8)return nullptr;
 const BYTE registration[]={0x48,0x83,0xec,0x38,0x48,0x8b,0x0d,0xed,0xa6,0x5a,0x07,0x48,0x85,0xc9,0x75,0x0c};
 const BYTE scheduler[]={0x48,0x8b,0xc4,0x44,0x88,0x40,0x18,0x48,0x89,0x50,0x10,0x48,0x89,0x48,0x08,0x55};
 if(memcmp(base+0xab7a50,registration,sizeof(registration))||memcmp(base+0x3ffed20,scheduler,sizeof(scheduler)))return nullptr;
 void* object=InterlockedCompareExchangePointer((void*volatile*)(base+0x82633d0),nullptr,nullptr);
 if(!Range(object,sizeof(void*),false))return nullptr;
 UINT_PTR table=*(UINT_PTR*)object;
 if(table<(UINT_PTR)base||table>=(UINT_PTR)base+size||!Range((void*)table,sizeof(void*),false))return nullptr;
 auto p=(volatile LONG*)(base+0x7e28824);
 if(((UINT_PTR)p&3)||!Range((void*)p,sizeof(LONG),true))return nullptr;
 return p;
}
static void Tone(bool on){Beep(on?700:1200,45);Beep(on?1200:700,65);}
static bool Enable(){
 auto field=Resolve();
 if(!field){Log("RT_BUDGET: activation refused; registration/page/fingerprint checks not ready");return false;}
 if(!state.Enable(field,limit)){Log("RT_BUDGET: activation compare/exchange failed; no repeated forcing");return false;}
 Log("RT_BUDGET ON current=%ld previous=%ld primitive_budget=%ld | native int-ref override, soft budget with immediate/force/one-build exceptions",state.Read(),state.previous,limit);
 Tone(true);return true;
}
static void Disable(){
 bool ok=state.Disable();
 Log("RT_BUDGET OFF restored=%d current=%ld captured_original=%ld | restoration requires current value to match the applied value",ok?1:0,state.Read(),state.previous);
 Tone(false);
}
static DWORD WINAPI Worker(LPVOID){
 limit=GetPrivateProfileIntA("NativeRTBudget","TrianglesPerFrame",50000,g_files.iniPath);
 if(limit<10000||limit>2000000)limit=50000;
 bool autoEnable=GetPrivateProfileIntA("NativeRTBudget","Enabled",1,g_files.iniPath)!=0;
 bool attempted=false,keyDown=false;LONGLONG readyAt=0,lastReport=Now();
 Log("RT_BUDGET: mode=isolated RT build scheduling; prefetch DISABLED; F8 toggles; budget=%ld",limit);
 for(;;){
  if(DeviceReady()&&!readyAt)readyAt=Now();
  if(autoEnable&&!attempted&&readyAt&&Ms(Now()-readyAt)>2000){
   if(Resolve()){attempted=true;Enable();}
   else if(Ms(Now()-readyAt)>60000){attempted=true;Log("RT_BUDGET: startup activation unavailable; game data left unchanged");}
  }
  if(state.owns&&!state.ObserveOwnership())Log("RT_BUDGET: engine/other writer changed the value to %ld; stopped owning it, no automatic reapply",state.Read());
  DWORD pid=0;GetWindowThreadProcessId(GetForegroundWindow(),&pid);
  bool foreground=pid==GetCurrentProcessId();bool key=foreground&&(GetAsyncKeyState(VK_F8)&0x8000);
  if(key&&!keyDown){attempted=true;if(state.owns)Disable();else Enable();WritePrivateProfileStringA("NativeRTBudget","Enabled",state.owns?"1":"0",g_files.iniPath);}
  keyDown=key;
  if(Ms(Now()-lastReport)>10000){Log2("RT_BUDGET state=%s readback=%ld original=%ld | %.3f s",state.owns?"ON":"off",state.Read(),state.previous,Ms(Now()-g_qpc0)/1000.0);lastReport=Now();}
  Sleep(50);
 }
}
}
