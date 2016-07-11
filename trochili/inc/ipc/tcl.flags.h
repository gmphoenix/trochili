/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_FLAGS_H
#define _TCL_FLAGS_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.ipc.h"
#include "tcl.thread.h"

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_FLAGS_ENABLE))

/* �¼���ǽṹ���� */
struct FlagsDef
{
    TProperty Property;  /* �̵߳ĵ��Ȳ��Ե���������   */
    TBitMask  Value;     /* �¼���ǵĵ�ǰ�¼���       */
    TIpcQueue Queue;     /* �¼���ǵ��߳���������     */
};
typedef struct FlagsDef TFlags;

extern TState xFlagsCreate(TFlags* pFlags, TProperty property, TError* pError);
extern TState xFlagsDelete(TFlags* pFlags, TError* pError);
extern TState xFlagsReset(TFlags* pFlags, TError* pError);
extern TState xFlagsFlush(TFlags* pFlags, TError* pError);
extern TState xFlagsSend(TFlags* pFlags, TBitMask pattern, TError* pError);
extern TState xFlagsReceive(TFlags* pFlags, TBitMask* pPattern,
                            TOption option, TTimeTick timeo, TError* pError);

#endif

#endif /* _TCL_FLAGS_H */

