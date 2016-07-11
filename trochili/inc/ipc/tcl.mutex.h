/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_MUTEX_H
#define _TCL_MUTEX_H

#include "tcl.types.h"
#include "tcl.object.h"
#include "tcl.thread.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MUTEX_ENABLE))

/* �����ź����ṹ���� */
struct MutexDef
{
    TProperty Property;      /* �������̵߳ĵ��Ȳ��Ե��������� */
    TThread*  Owner;         /* ռ�л����ź������߳�ָ��       */
    TBase32   Nest;          /* �����ź���Ƕ�׼������         */
    TPriority Priority;      /* ceiling value                  */
    TIpcQueue Queue;         /* �����ź������߳���������       */
    TObjNode  LockNode;      /* ������ɻ���������             */   
};
typedef struct MutexDef TMutex;

extern TState xMutexCreate(TMutex* pMutex, TPriority priority, TProperty property, TError* pError);
extern TState xMutexDelete(TMutex* pMutex, TError* pError);
extern TState xMutexLock(TMutex* pMutex, TOption option, TTimeTick timeo, TError* pError);
extern TState xMutexFree(TMutex* pMutex, TError* pError);
extern TState xMutexReset(TMutex* pMutex, TError* pError);
extern TState xMutexFlush(TMutex* pMutex, TError* pError);

#endif

#endif /*_TCL_MUTEX_H*/

