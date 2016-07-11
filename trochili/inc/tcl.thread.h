/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_THREAD_H
#define _TCL_THREAD_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.ipc.h"
#include "tcl.timer.h"

/* �߳����д����붨��                 */
#define THREAD_DIAG_NORMAL            (TBitMask)(0x0)     /* �߳�����                                */
#define THREAD_DIAG_STACK_OVERFLOW    (TBitMask)(0x1<<0)  /* �߳�ջ���                              */
#define THREAD_DIAG_STACK_ALARM       (TBitMask)(0x1<<1)  /* �߳�ջ�澯                              */
#define THREAD_DIAG_INVALID_EXIT      (TBitMask)(0x1<<2)  /* �̷߳Ƿ��˳�                            */
#define THREAD_DIAG_INVALID_STATE     (TBitMask)(0x1<<3)  /* �̲߳���ʧ��                            */

/* �̵߳��ô����붨��                 */
#define THREAD_ERR_NONE               (TError)(0x0)
#define THREAD_ERR_UNREADY            (TError)(0x1<<0)    /* �߳̽ṹδ��ʼ��                        */
#define THREAD_ERR_ACAPI              (TError)(0x1<<1)    /* �̲߳����ܲ���                          */
#define THREAD_ERR_FAULT              (TError)(0x1<<2)    /* һ���Դ��󣬲�������������              */
#define THREAD_ERR_STATUS             (TError)(0x1<<3)    /* �߳�״̬����                            */
#define THREAD_ERR_PRIORITY           (TError)(0x1<<4)    /* �߳����ȼ�����                          */


/* �߳����Զ���                       */
#define THREAD_PROP_NONE              (TProperty)(0x0)    /*                              */
#define THREAD_PROP_READY             (TProperty)(0x1<<0) /* �̳߳�ʼ����ϱ��λ,
                                                             ����Ա�ڽṹ���е�λ�ø����������    */
#define THREAD_PROP_PRIORITY_FIXED    (TProperty)(0x1<<1) /* �߳����ȼ���ȫ���                               */
#define THREAD_PROP_PRIORITY_SAFE     (TProperty)(0x1<<2) /* �߳����ȼ���ȫ���                               */
#define THREAD_PROP_CLEAN_STACK       (TProperty)(0x1<<3) /* ��������߳�ջ�ռ�                               */
#define THREAD_PROP_RUNASR            (TProperty)(0x1<<4) /* �첽�жϴ����̱߳��λ */
#define THREAD_PROP_DAEMON            (TProperty)(0x1<<5) /* �ػ��̱߳��λ */
#define THREAD_PROP_RUN2COMPLETION    (TProperty)(0x1<<6) /* �߳�run-to-completion                            */

/* �߳�Ȩ�޿��ƣ������߳�API����ʱ�����λ */
#define THREAD_ACAPI_NONE             (TBitMask)(0x0)
#define THREAD_ACAPI_DEINIT           (TBitMask)(0x1<<0)
#define THREAD_ACAPI_ACTIVATE         (TBitMask)(0x1<<1)
#define THREAD_ACAPI_DEACTIVATE       (TBitMask)(0x1<<2)
#define THREAD_ACAPI_SUSPEND          (TBitMask)(0x1<<3)
#define THREAD_ACAPI_RESUME           (TBitMask)(0x1<<4)
#define THREAD_ACAPI_DELAY            (TBitMask)(0x1<<5)
#define THREAD_ACAPI_UNDELAY          (TBitMask)(0x1<<6)
#define THREAD_ACAPI_YIELD            (TBitMask)(0x1<<7)
#define THREAD_ACAPI_SET_PRIORITY     (TBitMask)(0x1<<8)
#define THREAD_ACAPI_SET_SLICE        (TBitMask)(0x1<<9)
#define THREAD_ACAPI_UNBLOCK          (TBitMask)(0x1<<10)
#define THREAD_ACAPI_ALL \
    (THREAD_ACAPI_DEINIT|\
    THREAD_ACAPI_ACTIVATE|\
    THREAD_ACAPI_DEACTIVATE|\
    THREAD_ACAPI_SUSPEND|\
    THREAD_ACAPI_RESUME|\
    THREAD_ACAPI_DELAY|\
    THREAD_ACAPI_UNDELAY|\
    THREAD_ACAPI_SET_PRIORITY|\
    THREAD_ACAPI_SET_SLICE|\
    THREAD_ACAPI_UNBLOCK|\
    THREAD_ACAPI_YIELD)
#define THREAD_ACAPI_ASR \
    (THREAD_ACAPI_DEINIT | \
    THREAD_ACAPI_DEACTIVATE |\
    THREAD_ACAPI_SUSPEND |\
    THREAD_ACAPI_RESUME)

/* �߳�״̬����  */
enum ThreadStausdef
{
    eThreadRunning   = (TBitMask)(0x1<<0),     /* ����                                             */
    eThreadReady     = (TBitMask)(0x1<<1),     /* ����                                             */
    eThreadDormant   = (TBitMask)(0x1<<2),     /* ����                                             */
    eThreadBlocked   = (TBitMask)(0x1<<3),     /* ����                                             */
    eThreadDelayed   = (TBitMask)(0x1<<4),     /* ��ʱ����                                         */
    eThreadSuspended = (TBitMask)(0x1<<5),     /* ��������                                         */
};
typedef enum ThreadStausdef TThreadStatus;

/* �̶߳��нṹ���壬�ýṹ��С���ں�֧�ֵ����ȼ���Χ���仯��
   ����ʵ�̶ֹ�ʱ����߳����ȼ������㷨                                                          */
struct ThreadQueueDef
{
    TBitMask   PriorityMask;                 /* �����о������ȼ�����                             */
    TObjNode*  Handle[TCLC_PRIORITY_NUM];    /* �������̷ֶ߳���                                 */
	//TBase32    Number[TCLC_PRIORITY_NUM];    /* �������̷ֶ߳����е��߳���Ŀ                     */
};
typedef struct ThreadQueueDef TThreadQueue;

/* �߳����������Ͷ���                                                                            */
typedef void (*TThreadEntry)(TArgument data);

/* �ں��߳̽ṹ���壬���ڱ����̵߳Ļ�����Ϣ                                                      */
struct ThreadDef
{
    TProperty     Property;                  /* �̵߳�����,����Ա�ڽṹ���е�λ�ø����������  */
    TThreadStatus Status;                    /* �߳�״̬,����Ա�ڽṹ���е�λ�ø����������    */
    TAddr32       StackTop;                  /* �߳�ջ��ָ��,����Ա�ڽṹ���е�λ�ø����������*/
    TAddr32       StackBase;                 /* �߳�ջ��ָ��                                     */
#if (TCLC_THREAD_STACK_CHECK_ENABLE)
    TBase32       StackAlarm;                /* �߳�ջ��������                                   */
    TBase32       StackBarrier;              /* �߳�ջ��Χ��                                     */
#endif
    TBitMask      ACAPI;                     /* �߳̿ɽ��ܵ�API                                  */
    TPriority     Priority;                  /* �̵߳�ǰ���ȼ�                                   */
    TPriority     BasePriority;              /* �̻߳������ȼ�                                   */
    TTimeTick     Ticks;                     /* ʱ��Ƭ�л�ʣ�µ�ticks��Ŀ                        */
    TTimeTick     BaseTicks;                 /* ʱ��Ƭ���ȣ�ticks��Ŀ��                          */
    TTimeTick     Jiffies;                   /* �߳��ܵ�����ʱ�ӽ�����                           */
    TThreadEntry  Entry;                     /* �̵߳�������                                     */
    TArgument     Argument;                  /* �߳����������û�����,�û�����ֵ                  */
    TBitMask      Diagnosis;                 /* �߳����д�����                                   */
#if (TCLC_TIMER_ENABLE)
    TTimer        Timer;                     /* �߳��Դ���ʱ��                                   */
#endif

#if (TCLC_IPC_ENABLE)
    TIpcContext  IpcContext;                 /* �̻߳��⡢ͬ������ͨ�ŵ�������                   */
#endif

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MUTEX_ENABLE))
    TObjNode*     LockList;                  /* �߳�ռ�е����Ķ���                               */
#endif
    TThreadQueue* Queue;                     /* ָ���߳������̶߳��е�ָ��                       */
    TBase32       ThreadID;                  /* �߳�ID                                           */
    TObjNode      ObjNode;                   /* �߳����ڶ��еĽڵ�                               */
};
typedef struct ThreadDef TThread;

#define NODE2THREAD(NODE) ((TThread*)((TByte*)(NODE)-OFF_SET_OF(TThread, ObjNode)))


extern TThreadQueue uThreadAuxiliaryQueue;   /* �ں��̸߳�������                                 */
extern TThreadQueue uThreadSetReadyQueue;    /* �ں˽��������н�                                 */

extern void uThreadLeaveQueue(TThreadQueue* pQueue, TThread* pThread);
extern void uThreadEnterQueue(TThreadQueue* pQueue, TThread* pThread, TQueuePos pos);
extern void uThreadSchedule(void);
extern void uThreadTickISR(void);
extern void uThreadModuleInit(void);
extern void uThreadCalcHiRP(TPriority* priority);
extern void uThreadPreempt(TBool HiRP);
extern void uThreadCreate(TThread* pThread,
                        TThreadStatus status,
                        TProperty property,
                        TBitMask acapi,
                        TThreadEntry pEntry,
                        TArgument argument,
                        void* pStack,
                        TBase32 bytes,
                        TPriority priority,
                        TTimeTick ticks);
extern TState uThreadDelete(TThread* pThread, TError* pError);
extern TState uThreadSetReady(TThread* pThread, TThreadStatus status, TError* pError);
extern TState uThreadSetUnready(TThread* pThread, TThreadStatus status,
                             TTimeTick ticks,TError* pError);
extern TState uThreadSetPriority(TThread* pThread, TPriority priority,
                                 TBool flag, TError* pError);

extern TState xThreadCreate(TThread* pThread,
                          TThreadStatus status,
                          TProperty     property,
                          TBitMask      acapi,
                          TThreadEntry  pEntry,
                          TBase32         argument,
                          void*         pStack,
                          TBase32         bytes,
                          TPriority     priority,
                          TTimeTick     ticks,
                          TError*       pError);
extern TState xThreadDelete(TThread* pThread, TError* pError);
extern TState xThreadActivate(TThread* pThread, TError* pError);
extern TState xThreadDeactivate(TThread* pThread, TError* pError);
extern TState xThreadSuspend(TThread* pThread, TError* pError);
extern TState xThreadResume(TThread* pThread, TError* pError);
extern TState xThreadUnblock(TThread* pThread, TError* pError);
#if (TCLC_TIMER_ENABLE)
extern TState xThreadDelay(TThread* pThread, TTimeTick ticks, TError* pError);
extern TState xThreadUndelay(TThread* pThread, TError* pError);
#endif

extern TState xThreadYield(TError* pError);
extern TState xThreadSetPriority(TThread* pThread, TPriority priority, TError* pError);
extern TState xThreadSetTimeSlice(TThread* pThread, TTimeTick ticks, TError* pError);

#endif /*_TCL_THREAD_H */

