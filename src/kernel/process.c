#include <process.h>
#include <sched.h>
#include <stdint.h>
#include <interrupt.h>
#include <initrd.h>
#include <string.h>
#include <uart.h>
#include <syscall.h>
#define STACK_PAGES 2

#define process_flag_running 0b1
#define process_flag_wait 0b10
#define process_flag_zombie 0b100
#define process_readflag(t,f) ( ((Thread*)t)->status & f )
#define process_setflag(t,f) ( ((Thread*)t)->status |= f )
#define process_unsetflag(t,f) ( ((Thread*)t)->status &= ~f )

// uint64_t Process_ID;
Process *Process_list;

void process_init()
{
    Process_list = 0;
}

Process *get_process(uint64_t pid)
{
    Process *cur_proc = Process_list;
    //Process *pre_proc = 0;
    while(cur_proc){
        if(cur_proc->pid == pid){
            return cur_proc;
        }
        cur_proc = cur_proc->next;
    }
    return 0;
}

void unlink_process(Process *process)
{
    Process *cur_proc = Process_list;
    Process *pre_proc = 0;
    while(cur_proc){
        if(cur_proc == process){
            if(pre_proc){
                pre_proc->next = cur_proc->next;
            }
            else{
                Process_list = cur_proc->next;
            }
            return ;
        }
        pre_proc = cur_proc;
        cur_proc = cur_proc->next;
    }
}

ProcMem *alloc_procmem(uint64_t size)
{
    ProcMem *proc_mem = (ProcMem*)kmalloc(sizeof(ProcMem));
    proc_mem->size = size;
    proc_mem->next = 0;
    proc_mem->addr = buddy_alloc(size);
    if(proc_mem->addr==0){
        kfree(proc_mem);
        return 0;
    }
    return proc_mem;
}

Process *process_create(const char *filename)
{
    uart_puts("process_create()");
    Process* newprocess = (Process*)kmalloc(sizeof(Process));
    newprocess->tid = thread_get_current();
    size_t filenamelen = strlen(filename);
    newprocess->filename = (char *)kmalloc(filenamelen+1);
    memcpy(newprocess->filename, filename, filenamelen);
    newprocess->filename[filenamelen] = 0;
    newprocess->status = ProcessStatus_init;
    newprocess->exit_code = 0;
    newprocess->pid = newprocess->tid;
    uart_print("process_create(): newprocess->filename = ");
    uart_puts(newprocess->filename);
    // interrupt_disable();
    // newprocess->pid = ++Process_ID;
    // interrupt_enable();
    INITRD_FILE* execfile = initrd_get(newprocess->filename);
    if(execfile == 0){
        goto err;
    }
    uint64_t exec_pagenum = 0;
    exec_pagenum = (uint64_t)execfile->filesize / (uint64_t)(1<<PAGE_SIZE) + 1;
    uint64_t stack_pagenum = STACK_PAGES;
    ProcMem *exec_mem = alloc_procmem(exec_pagenum);
    uart_print("process_create(): exec_mem = 0x");
    uart_putshex(exec_mem->addr);
    if(exec_mem==0){
        goto err;
    }
    newprocess->process_memory = exec_mem;
    ProcMem *stack_mem = alloc_procmem(stack_pagenum);
    uart_print("process_create(): stack_mem = 0x");
    uart_putshex(stack_mem->addr);
    if(stack_mem==0){
        goto err;
    }
    stack_mem->next = newprocess->process_memory;
    newprocess->process_memory = stack_mem;
    newprocess->stack = stack_mem->addr;
    newprocess->sp = (void*)((uint64_t)stack_mem->addr + ((uint64_t)stack_mem->size << PAGE_SIZE) - 16);
    newprocess->exec = exec_mem->addr;
    newprocess->entry = exec_mem->addr;
    uart_print("process_create(): execfile->filecontent = 0x");
    uart_putshex(execfile->filecontent);
    uart_print("process_create(): execfile->filesize = 0x");
    uart_putshex(execfile->filesize);
    memcpy(exec_mem->addr, execfile->filecontent, execfile->filesize);
    uart_puts("process_create(): exec file has been copied to exec memory.");

    newprocess->signal_tid = 0;
    newprocess->signal_handling = 0;
    newprocess->signal_handlers = 0;
    for(int i=0;i<=SIGNAL_NUM;i++)newprocess->signal[i] = 0;

    newprocess->next = Process_list;
    Process_list = newprocess;

    return newprocess;

    err:
    return 0;
}

void process_free(Process *process)
{
    unlink_process(process);
    kfree(process->filename);
    ProcMem *cur_mem = process->process_memory;
    ProcMem *pre_mem = 0;
    while(cur_mem){
        // uart_print("process_free(): free process memory: 0x");
        // uart_putshex((uint64_t)cur_mem->addr);
        buddy_free(cur_mem->addr);
        pre_mem = cur_mem;
        cur_mem = cur_mem->next;
        kfree(pre_mem);
    }
    kfree(process);
}

int32_t process_exec(const char* name)
{
    int pid = process_getpid();
    if(pid){
        Process *oldprocess = get_process(pid);
        if(oldprocess){
            process_free(oldprocess);
        }
    }
    
    Process *newprocess = process_create(name);
    if(newprocess==0){
        uart_puts("Create new process error.");
        return -1;
    }
    uart_print("process_exec(): newprocess=0x");
    uart_putshex(newprocess);
    interrupt_disable();
    newprocess->status = ProcessStatus_running;
    Thread *thread = get_thread(thread_get_current());
    thread->process = newprocess;
    asm("msr spsr_el1, %0"::"r"((uint64_t)0x0)); // 0x0 enable all interrupt
    asm("msr elr_el1, %0"::"r"(newprocess->entry));
    asm("msr sp_el0, %0"::"r"(newprocess->sp));
    asm(
        "mov x0, #0\t\n"
        "mov x1, #0\t\n"
        "mov x2, #0\t\n"
        "mov x3, #0\t\n"
        "mov x4, #0\t\n"
        "mov x5, #0\t\n"
        "mov x6, #0\t\n"
        "mov x7, #0\t\n"
        "mov x8, #0\t\n"
        "mov x9, #0\t\n"
        "mov x10, #0\t\n"
        "mov x11, #0\t\n"
        "mov x12, #0\t\n"
        "mov x13, #0\t\n"
        "mov x14, #0\t\n"
        "mov x15, #0\t\n"
        "mov x16, #0\t\n"
        "mov x17, #0\t\n"
        "mov x18, #0\t\n"
        "mov x19, #0\t\n"
        "mov x20, #0\t\n"
        "mov x21, #0\t\n"
        "mov x22, #0\t\n"
        "mov x23, #0\t\n"
        "mov x24, #0\t\n"
        "mov x25, #0\t\n"
        "mov x26, #0\t\n"
        "mov x27, #0\t\n"
        "mov x28, #0\t\n"
        "mov x29, #0\t\n"
        "mov x30, #0\t\n"
    );
    asm("eret");
}

int process_getpid()
{
    Thread *thread = get_thread(thread_get_current());
    return thread->process->pid;
}

void _process_exit(Thread *thread, int status)
{
    Process *process = thread->process;
    process->exit_code = status;
    process->status = ProcessStatus_exit;
    thread->status = ThreadStatus_zombie;
}

void process_exit(int status)
{
    Thread *cur_thread = get_thread(thread_get_current());
    Thread *proc_thread = get_thread(cur_thread->process->tid);
    _process_exit(proc_thread, status);
    schedule();
}

void process_kill(int pid)
{
    Process *process = get_process(pid);
    if(process == 0) return ;
    _process_exit(get_thread(process->tid), 0);
}

typedef struct ForkProcess_{
    Process *process;
    TrapFrame *trapframe;
} ForkProcess;

void process_fork_exec(void *argv)
{
    uart_puts("process_fork_exec()");
    ForkProcess *forkprocess = (ForkProcess*)argv;
    Process *process = forkprocess->process;
    uint64_t pad = 0;
    TrapFrame trapframe;
    process->status = ProcessStatus_running;
    Thread *thread = get_thread(thread_get_current());
    thread->process = process;
    // uart_print("process_fork_exec(): forkprocess->trapframe->spsr_el1 = 0x");
    // uart_putshex(forkprocess->trapframe->spsr_el1);
    // uart_print("process_fork_exec(): process->entry = 0x");
    // uart_putshex(process->entry);
    // uart_print("process_fork_exec(): process->sp = 0x");
    // uart_putshex(process->sp);
    interrupt_disable();
    asm("msr spsr_el1, %0"::"r"((uint64_t)forkprocess->trapframe->spsr_el1)); // 0x0 enable all interrupt
    asm("msr elr_el1, %0"::"r"(process->entry));
    asm("msr sp_el0, %0"::"r"(process->sp));
    //uint64_t sp = 0;
    //asm("mov %0, sp":"=r"(sp));
    //sp -= ((sizeof(TrapFrame)>>4)+1)<<4;
    forkprocess->trapframe->x0 = 0; // child
    // uart_print("process_fork_exec(): &trapframe = 0x");
    // uart_putshex(&trapframe);
    // uart_print("process_fork_exec(): forkprocess->trapframe = 0x");
    // uart_putshex(forkprocess->trapframe);
    memcpy(&trapframe, forkprocess->trapframe, sizeof(TrapFrame));
    // uart_puts("process_fork_exec(): trapframe has been copied to stack.");
    // uart_print("process_fork_exec(): trapframe.x0 = 0x");
    // uart_putshex(trapframe.x0);
    kfree(forkprocess->trapframe);
    kfree(forkprocess);
    uart_puts("process_fork_exec(): Ready to exec fork process.");
    asm("mov sp, %0"::"r"(&trapframe));
    asm(
        "ldp x0, x1, [sp ,16 * 0]\t\n"
        "ldp x2, x3, [sp ,16 * 1]\t\n"
        "ldp x4, x5, [sp ,16 * 2]\t\n"
        "ldp x6, x7, [sp ,16 * 3]\t\n"
        "ldp x8, x9, [sp ,16 * 4]\t\n"
        "ldp x10, x11, [sp ,16 * 5]\t\n"
        "ldp x12, x13, [sp ,16 * 6]\t\n"
        "ldp x14, x15, [sp ,16 * 7]\t\n"
        "ldp x16, x17, [sp ,16 * 8]\t\n"
        "ldp x18, x19, [sp ,16 * 9]\t\n"
        "ldp x20, x21, [sp ,16 * 10]\t\n"
        "ldp x22, x23, [sp ,16 * 11]\t\n"
        "ldp x24, x25, [sp ,16 * 12]\t\n"
        "ldp x26, x27, [sp ,16 * 13]\t\n"
        "ldp x28, x29, [sp ,16 * 14]\t\n"
        "ldr x30, [sp, 16 * 15]\t\n"
    );
    asm("eret");
}

int process_fork(TrapFrame *trapframe)
{
    Thread *thread = get_thread(thread_get_current());
    Process* origin_process = thread->process;
    if(origin_process == 0){
        goto err;
    }
    Process* newprocess = (Process*)kmalloc(sizeof(Process));
    memcpy(newprocess, origin_process, sizeof(Process));
    size_t filenamelen = strlen(origin_process->filename);
    newprocess->filename = (char *)kmalloc(filenamelen+1);
    memcpy(newprocess->filename, origin_process->filename, filenamelen);
    newprocess->filename[filenamelen] = 0;
    newprocess->process_memory = 0;

    ProcMem *cur_mem = origin_process->process_memory;
    ProcMem *stack_mem = 0;
    ProcMem *exec_mem = 0;
    while(cur_mem){
        ProcMem *newmem = alloc_procmem(cur_mem->size);
        memcpy(newmem->addr, cur_mem->addr, cur_mem->size * (uint64_t)(1<<PAGE_SIZE));
        newmem->next = newprocess->process_memory;
        newprocess->process_memory = newmem;
        if(cur_mem->addr == origin_process->stack){
            stack_mem = newmem;
        }
        if(cur_mem->addr == origin_process->exec){
            exec_mem = newmem;
        }
        cur_mem = cur_mem->next;
    }

    if(stack_mem == 0) goto err;
    if(exec_mem == 0) goto err;

    newprocess->stack = stack_mem->addr;
    newprocess->sp = (void *)((uint64_t)trapframe->sp_el0 - (uint64_t)origin_process->stack + (uint64_t)newprocess->stack);
    newprocess->exec = exec_mem->addr;
    newprocess->entry = (void *)((uint64_t)trapframe->elr_el1 - (uint64_t)origin_process->exec + (uint64_t)newprocess->exec);

    ForkProcess *forkprocess = (ForkProcess *)kmalloc(sizeof(ForkProcess));
    forkprocess->process = newprocess;
    forkprocess->trapframe = (TrapFrame *)kmalloc(sizeof(TrapFrame));
    memcpy(forkprocess->trapframe, trapframe, sizeof(TrapFrame));

    forkprocess->trapframe->x29 = (uint64_t)forkprocess->trapframe->x29 - (uint64_t)origin_process->stack + (uint64_t)newprocess->stack;
    forkprocess->trapframe->x30 = (uint64_t)forkprocess->trapframe->x30 - (uint64_t)origin_process->exec + (uint64_t)newprocess->exec;

    //newprocess->sp = ;
    newprocess->next = Process_list;
    Process_list = newprocess;

    Thread *new_thread = create_thread(process_fork_exec, forkprocess);
    if(new_thread == 0) goto err;
    newprocess->tid = new_thread->thread_id;
    newprocess->pid = newprocess->tid;
    new_thread->process = newprocess;
    thread_run(new_thread);

    return newprocess->pid;

    err:
    uart_puts("process_fork(): error");
    return -1;
}