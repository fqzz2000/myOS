#include "cmctl.h"
void machbstart_t_init(machbstart_t *initp)
{
    memset(initp, 0, sizeof(machbstart_t));
    initp->mb_migc = MBS_MIGC;
    return;
}

void init_bstartparm()
{
    machbstart_t *mbsp = MBSPADR; // 1Mb 位置的内存地址
    machbstart_t_init(mbsp);
    return;
}

int chk_cpuid()
{
    // check if the cpu support cpuid instruction
    // 支持CPUID指令的CPU，可以通过CPUID指令获取CPU的信息，包括CPU的厂商、型号、支持的功能等等
    // 可以通过EFLAG寄存的第21位判断是否支持，如果支持，第21位为1，否则为0
    int rets = 0;
    __asm__ __volatile__(
        "pushfl \n\t"
        "popl %%eax \n\t"
        "movl %%eax,%%ebx \n\t"
        "xorl $0x0200000,%%eax \n\t"
        "pushl %%eax \n\t"
        "popfl \n\t"
        "pushfl \n\t"
        "popl %%eax \n\t"
        "xorl %%ebx,%%eax \n\t"
        "jz 1f \n\t"
        "movl $1,%0 \n\t"
        "jmp 2f \n\t"
        "1: movl $0,%0 \n\t"
        "2: \n\t"
        : "=c"(rets)
        :
        :);
    return rets;
}

int chk_cpu_longmode()
{
    // check if the cpu support long mode
    int rets = 0;
    __asm__ __volatile__(
        "movl $0x80000000,%%eax \n\t"
        "cpuid \n\t"                  // 把eax中放入0x80000000调用CPUID指令
        "cmpl $0x80000001,%%eax \n\t" // 看eax中返回结果
        "setnb %%al \n\t"             // 不为0x80000001,则不支持0x80000001号功能
        "jb 1f \n\t"
        "movl $0x80000001,%%eax \n\t"
        "cpuid \n\t"         // 把eax中放入0x800000001调用CPUID指令，检查edx中的返回数据
        "bt $29,%%edx  \n\t" // 长模式 支持位  是否为1
        "setcb %%al \n\t"
        "1: \n\t"
        "movzx %%al,%%eax \n\t"
        : "=a"(rets)
        :
        :);
}

// CPU是否支持64位长模式
int init_chkcput(machbstart_t *mbsp)
{
    // check if the cpu support cpuid instruction
    // check if the cpu support long mode
    if (!chk_cpuid())
    {
        kerror("Your CPU is not support CPUID sys is die!");
        CLI_HALT();
    }
    if (!chk_cpu_longmode())
    {
        kerror("Your CPU is not support 64bits mode sys is die!");
        CLI_HALT();
    }
    mbsp->mb_cpumode = 0x40; // 如果成功则设置机器信息结构的cpu模式为64位
    return;
}
