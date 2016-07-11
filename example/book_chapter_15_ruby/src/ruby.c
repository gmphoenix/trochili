#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH15_RUBY_EXAMPLE)

#include "ruby.h"

/* �û�Ӧ����ں��� */
extern void OS_SetupMemory(void);
extern void OS_SetupThreads(void);
extern void OS_SetupFlags(void);
extern void OS_SetupQueues(void);
extern void OS_SetupMemory(void);
extern void OS_SetupISR(void);
extern void OS_SetupTimers(void);

static void RubyEntry(void)
{
    OS_SetupThreads();
    OS_SetupFlags();
    OS_SetupQueues();
    OS_SetupMemory();    
	  OS_SetupTimers();
    OS_SetupISR();
}

/* ������BOOT֮������main�����������ṩ */
int main(void)
{
    TclStartKernel(&RubyEntry,
                   &CpuSetupEntry,
                   &EvbSetupEntry,
                   &EvbTraceEntry);
    return 1;
}

#endif

