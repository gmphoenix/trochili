/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_SEMAPHORE_H
#define _TCL_SEMAPHORE_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.ipc.h"
#include "tcl.thread.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_SEMAPHORE_ENABLE))

/* �����ź�������ֵ�ṹ���� */
struct SemaphoreDef
{
    TProperty Property;         /* �������̵߳ĵ��Ȳ��Ե���������     */
    TBase32   Value;            /* �����ź����ĵ�ǰ��ֵ               */
    TBase32   LimitedValue;     /* �����ź����������ֵ               */
    TBase32   InitialValue;     /* �����ź����ĳ�ʼ��ֵ               */
    TIpcQueue Queue;            /* �ź������߳���������               */
};
typedef struct SemaphoreDef TSemaphore;

extern TState xSemaphoreCreate(TSemaphore* pSemaphore, TBase32 value, TBase32 mvalue, TProperty property, TError* pError);
extern TState xSemaphoreDelete(TSemaphore* pSemaphore, TError* pError);
extern TState xSemaphoreReset(TSemaphore* pSemaphore, TError* pError);
extern TState xSemaphoreRelease(TSemaphore* pSemaphore, TOption option, TTimeTick timeo, TError* pError);
extern TState xSemaphoreObtain(TSemaphore* pSemaphore, TOption option, TTimeTick timeo, TError* pError);
extern TState xSemaphoreFlush(TSemaphore* pSemaphore, TError* pError);
#endif

#endif /*_TCL_SEMAPHORE_H*/

