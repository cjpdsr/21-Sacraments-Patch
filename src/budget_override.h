#pragma once
// Build-specific experiment: the engine directly reads this registered int-ref.
// Does not modify CVar priority metadata, executable instructions, or GPU fences.
struct BudgetOverride {
 volatile LONG* field=nullptr;LONG previous=0,applied=0;bool owns=false;
 LONG Read() const {return field?InterlockedCompareExchange(field,0,0):0;}
 bool Enable(volatile LONG* p,LONG limit){
  if(!p||limit<10000||limit>2000000||owns)return false;
  LONG before=InterlockedCompareExchange(p,0,0);
  if(InterlockedCompareExchange(p,limit,before)!=before)return false;
  field=p;previous=before;applied=limit;owns=true;return true;
 }
 bool Disable(){
  if(!owns)return false;
  bool restored=InterlockedCompareExchange(field,previous,applied)==applied;
  owns=false;return restored;
 }
 bool ObserveOwnership(){if(owns&&Read()!=applied){owns=false;return false;}return owns;}
};
