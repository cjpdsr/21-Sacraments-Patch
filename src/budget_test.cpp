#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "budget_override.h"
int checks=0;
void Check(bool c,const char*name){if(!c){printf("FAIL %s\n",name);ExitProcess(1);}printf("PASS %s\n",name);checks++;}
int main(){volatile LONG engine=-1;BudgetOverride a;
 Check(!a.Enable(nullptr,50000),"null field refused");
 Check(!a.Enable(&engine,9999)&&engine==-1,"out-of-range budget leaves engine unchanged");
 Check(a.Enable(&engine,50000)&&engine==50000&&a.previous==-1,"enable captures unlimited original and applies budget");
 Check(!a.Enable(&engine,100000)&&a.previous==-1,"re-enable cannot replace captured baseline");
 Check(a.Disable()&&engine==-1,"disable restores original value");
 Check(!a.Disable()&&engine==-1,"duplicate disable does not write");
 engine=250000;Check(a.Enable(&engine,50000)&&a.previous==250000,"nondefault engine budget captured");
 Check(a.Disable()&&engine==250000,"nondefault baseline restored exactly");
 Check(a.Enable(&engine,50000),"enable for concurrent-writer test");InterlockedExchange(&engine,123456);
 Check(!a.Disable()&&engine==123456,"disable never overwrites a changed engine value");
 Check(a.Enable(&engine,50000),"explicit re-enable captures latest engine value");InterlockedExchange(&engine,654321);
 Check(!a.ObserveOwnership()&&!a.owns&&engine==654321,"external write invalidates ownership without fighting it");
 printf("ALL %d OVERRIDE TESTS PASSED\n",checks);return 0;}
