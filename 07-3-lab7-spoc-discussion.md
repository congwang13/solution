# 同步互斥(lec 19 lab7) spoc 思考题

## 小组思考题


2. （扩展练习）请在lab7-answer中分析
  -  cvp->count含义是什么？cvp->count是否可能<0, 是否可能>1？请举例或说明原因。  
  cvp->count含义是等在条件变量cvp上的睡眠进程的个数。cvp->count不能小于0，但可能大于1。  
  因为cvp->count总是先增后减的所以不能小于0；可能出现多个进程等待的情况，故可以大于1。

  -  cvp->owner->next_count含义是什么？cvp->owner->next_count是否可能<0, 是否可能>1？请举例或说明原因。  
  含义是发出条件变量cpv的singal的睡眠进程个数。不可能小于0，不可能大于1。  
  因为总是先增后减，不能小于0；某一个进程在执行期间会有互斥锁。

  -  目前的lab7-answer中管程的实现是Hansen管程类型还是Hoare管程类型？  
  Hoare管程类型。