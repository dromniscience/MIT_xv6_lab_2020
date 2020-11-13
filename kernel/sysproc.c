#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

void cpalarm2trap(struct trapframe *, struct alarmframe *);

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  
  backtrace();
  
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_sigalarm(void)
{
	int ticks;
	uint64 temp;
	void (*ptr)();
	struct proc *p = myproc();

	
	if(argint(0, &ticks) < 0 || argaddr(1, &temp) < 0)
		return -1;
	ptr = (void (*)())temp;
	if(ticks < 0) ticks = -ticks;
	
	if(!ticks){
		p->ticks = 0;
		p->handler = (void (*)())0;
		p->ticksleft = 0; // Must be after p->ticks = 0;
		return 0;
	}
	
	if(p->killed) return -1;
	p->ticksleft = ticks; // Must be before p->ticks = ticks;
	p->ticks = ticks;
	p->handler = ptr;
	return 0;
}

uint64
sys_sigreturn(void)
{
	struct proc *p = myproc();
	if(!(p->ticks > 0 && p->ticksleft == p->ticks && p->in_handler))
		return -1;
	
	if(p->killed) return -1;
	cpalarm2trap(p->trapframe, &(p->alarmframe));
	p->in_handler = 0;
	
	return 0;
}


void cpalarm2trap(struct trapframe *p, struct alarmframe *q)
{
	p->epc = q->epc;
	p->ra = q->ra;
	p->sp = q->sp;
	p->gp = q->gp;
	p->tp = q->tp;
	p->t0 = q->t0;
	p->t1 = q->t1;
	p->t2 = q->t2;
	p->s0 = q->s0;
	p->s1 = q->s1;
	p->a0 = q->a0;
	p->a1 = q->a1;
	p->a2 = q->a2;
	p->a3 = q->a3;
	p->a4 = q->a4;
	p->a5 = q->a5;
	p->a6 = q->a6;
	p->a7 = q->a7;
	p->s2 = q->s2;
	p->s3 = q->s3;
	p->s4 = q->s4;
	p->s5 = q->s5;
	p->s6 = q->s6;
	p->s7 = q->s7;
	p->s8 = q->s8;
	p->s9 = q->s9;
	p->s10 = q->s10;
	p->s11 = q->s11;
	p->t3 = q->t3;
	p->t4 = q->t4;
	p->t5 = q->t5;
	p->t6 = q->t6;
}
