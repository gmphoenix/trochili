#include "tcl.gd32f190.h"
#include "colibri190bsp.h"

/* ����ʹ���������ϵ��豸 */
void EvbSetupEntry(void)
{
   EvbUart2Config();
   EvbLedConfig();
   EvbKeyConfig();
}


void EvbTraceEntry(const char* str)
{
    EvbUart2WriteStr(str);
}


