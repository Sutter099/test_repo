#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

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
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    backtrace();
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
  struct proc *p = myproc();
  void (*fn)() = 0;
  int interval;

  if(argint(0, &interval) < 0 || argaddr(1, (uint64*)&fn) < 0)
    return -1;

  acquire(&p->lock);

  p->interval = interval;
  p->alarm_handler = fn;
  p->sig_flag = SIG_COUNTING;

  release(&p->lock);

  printf("sig alarm!\n");

  return 0;
}

uint64
sys_sigreturn(void)
{
  struct proc *p = myproc();

  // user registers saved before syscall sig_alarm
  p->trapframe->epc = p->sig_ctx.pc;
  p->trapframe->ra = p->sig_ctx.ra;
  p->trapframe->sp = p->sig_ctx.sp;
  p->trapframe->gp = p->sig_ctx.gp;
  p->trapframe->tp = p->sig_ctx.tp;
  p->trapframe->t0 = p->sig_ctx.t0;
  p->trapframe->t1 = p->sig_ctx.t1;
  p->trapframe->t2 = p->sig_ctx.t2;
  p->trapframe->s0 = p->sig_ctx.s0;
  p->trapframe->s1 = p->sig_ctx.s1;
  p->trapframe->a0 = p->sig_ctx.a0;
  p->trapframe->a1 = p->sig_ctx.a1;
  p->trapframe->a2 = p->sig_ctx.a2;
  p->trapframe->a3 = p->sig_ctx.a3;
  p->trapframe->a4 = p->sig_ctx.a4;
  p->trapframe->a5 = p->sig_ctx.a5;
  p->trapframe->a6 = p->sig_ctx.a6;
  p->trapframe->a7 = p->sig_ctx.a7;
  p->trapframe->s2 = p->sig_ctx.s2;
  p->trapframe->s3 = p->sig_ctx.s3;
  p->trapframe->s4 = p->sig_ctx.s4;
  p->trapframe->s5 = p->sig_ctx.s5;
  p->trapframe->s6 = p->sig_ctx.s6;
  p->trapframe->s7 = p->sig_ctx.s7;
  p->trapframe->s8 = p->sig_ctx.s8;
  p->trapframe->s9 = p->sig_ctx.s9;
  p->trapframe->s10 = p->sig_ctx.s10;
  p->trapframe->s11 = p->sig_ctx.s11;
  p->trapframe->t3 = p->sig_ctx.t3;
  p->trapframe->t4 = p->sig_ctx.t4;
  p->trapframe->t5 = p->sig_ctx.t5;
  p->trapframe->t6 = p->sig_ctx.t6;

  p->sig_flag = SIG_COUNTING;

  return 0;
}
