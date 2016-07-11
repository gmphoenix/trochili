/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_KERNEL_H
#define _TCL_KERNEL_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.thread.h"
#include "tcl.irq.h"
#include "tcl.debug.h"

/* �û���ں������Ͷ���                         */
typedef void (*TUserEntry)(void);

/* �弶��ʼ���������Ͷ���                       */
typedef void (*TBoardSetupEntry)(void);

/* ��������ʼ���������Ͷ���                     */
typedef void (*TCpuSetupEntry)(void);

/* �弶�ַ�����ӡ�������Ͷ���                   */
typedef void (*TTraceEntry)(const char* pStr);

/* ϵͳIDLE�������Ͷ���                         */
typedef void (*TSysIdleEntry)(void);

/* ϵͳFault���������Ͷ���                         */
typedef void (*TSysFaultEntry)(void);


/* �������л������Ͷ��壬ϵͳ�������������л��� */
typedef enum
{
    eOriginState = 0,                                 /* �����������ں˳�̬                    */
    eIntrState   = 1,                                 /* �����������ж�̬                      */
    eThreadState = 2,                                 /* �����������߳�̬                      */
} TKernelState;

#define KERNEL_DIAG_ERROR_NONE      (0U)  /* �߳�ջ���                                   */
#define KERNEL_DIAG_THREAD_ERROR    (0x1<<0U)  /* �߳�ջ���                                   */
#define KERNEL_DIAG_TIMER_ERROR    (0x1<<1U)  /* �߳�ջ���                                   */

/* �ں˱����ṹ���壬��¼���ں�����ʱ�ĸ������� */
struct KernelVariableDef
{
    TBool            Schedulable;                     /* �����Ƿ������̵߳���                    */
    TThread*         NomineeThread;                   /* �ں˺�ѡ�߳�ָ��                        */
    TThread*         CurrentThread;                   /* �ں˵�ǰ�߳�ָ��                        */
    TKernelState     State;                           /* ��¼����ִ��ʱ����������״̬            */
    TBase32          IntrNestTimes;                   /* ��¼�ں˱��жϵ�Ƕ�״���                */
    TTimeTick        Jiffies;                         /* ϵͳ�����ܵĽ�����                      */
    TBase32          ObjID;                           /* �ں˶��������ɼ���                    */
    TBitMask         Diagnosis;                       /* �ں�����״����¼                        */
    TDBGLog          DBGLog;
    TBoardSetupEntry BoardSetupEntry;                 /* �弶��ʼ���������                      */
    TCpuSetupEntry   CpuSetupEntry;                   /* ��������ʼ���������                    */
    TUserEntry       UserEntry;                       /* ��¼�û��������                        */
    TTraceEntry      TraceEntry;                      /* �ں˴�ӡ����                            */
    TSysIdleEntry    SysIdleEntry;                    /* �ں�IDLE����                            */
	TSysFaultEntry   SysFaultEntry;                   /* �ں�FAULT����                           */
    TThread*         IdleDaemon;                      /* ��ʼ�Լ�IDLE�߳�ָ��                    */

#if ((TCLC_TIMER_ENABLE)&&(TCLC_TIMER_DAEMON_ENABLE))
    TThread*         TimerDaemon;                     /* �û���ʱ���߳�ָ��                      */
#endif

#if ((TCLC_IRQ_ENABLE)&&(TCLC_IRQ_DAEMON_ENABLE))
    TThread*         IrqDaemon;                       /* IRQ�߳�ָ��                             */
#endif

#if (TCLC_TIMER_ENABLE)
    TTimerList*      TimerList;
#endif

#if (TCLC_IRQ_ENABLE)
    TAddr32*         IrqMapTable;
    TIrqVector*      IrqVectorTable;
#endif

    TThreadQueue*    ThreadAuxiliaryQueue;            /* �ں��̸߳�������ָ��                    */
    TThreadQueue*    ThreadReadyQueue;                /* �ں˽��������н�ָ��                    */
};
typedef struct KernelVariableDef TKernelVariable;


extern TKernelVariable uKernelVariable;

extern void uKernelTrace(const char* pNote);
extern void uKernelEnterIntrState(void);
extern void uKernelLeaveIntrState(void);

extern void xKernelTrace(const char* pNote);
extern void xKernelTickISR(void);
extern TState xKernelUnlockSched(void);
extern TState xKernelLockSched(void);
extern void xKernelSetIdleEntry(TSysIdleEntry pEntry);
extern void xKernelGetCurrentThread(TThread** pThread2);
extern void xKernelGetJiffies(TTimeTick* pJiffies);
extern void xKernelStart(TUserEntry       pUserEntry,
                         TCpuSetupEntry   pCpuEntry,
                         TBoardSetupEntry pBoardEntry,
                         TTraceEntry      pTraceEntry);


#endif /* _TCL_KERNEL_H */

