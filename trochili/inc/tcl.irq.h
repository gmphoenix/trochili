/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_IRQ_H
#define _TCL_IRQ_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.debug.h"
#include "tcl.thread.h"

#if (TCLC_IRQ_ENABLE)

/* ISR����ֵ */
#define IRQ_ISR_DONE           (TBitMask)(0x0)       /* �жϴ���������              */
#define IRQ_CALL_ASR           (TBitMask)(0x1<<0)    /* ������ø߼��첽�жϴ����߳�  */

#define IRQ_ERR_NONE           (TError)(0x0)
#define IRQ_ERR_FAULT          (TError)(0x1<<0)      /* һ���Դ���                    */
#define IRQ_ERR_UNREADY        (TError)(0x1<<1)      /* �ж��������δ��ʼ��          */
#define IRQ_ERR_LOCKED         (TError)(0x1<<2)      /* �ж�����������              */

#define IRQ_PROP_NONE          (TProperty)(0x0)      /* IRQ�������                   */
#define IRQ_PROP_READY         (TProperty)(0x1<<0)   /* IRQ�������                   */

/* ISR�������Ͷ��� */
typedef TBitMask (*TISR)(TArgument data);

/* �ж������ṹ���� */
typedef struct
{
    TProperty  Property;
    TIndex     IRQn;                                 /* �����жϺ�                    */
    TISR       ISR;                                  /* ͬ���жϴ�����              */
    TThread*   ASR;                                  /* �첽�жϴ����߳�              */
    TArgument  Argument;                             /* �ж���������                  */
} TIrqVector;

/* ISR�������Ͷ��� */
typedef TBitMask (*TISR)(TArgument data);

#if (TCLC_IRQ_DAEMON_ENABLE)
/* IRQ�ص��������Ͷ��� */
typedef void(*TIrqEntry)(TArgument data);            

/* IRQ����ṹ���� */
typedef struct IrqDef
{
    TProperty Property;
    TPriority Priority;                              /* IRQ���ȼ�                     */
    TIrqEntry Entry;                                 /* IRQ�ص�����                   */
    TArgument Argument;                              /* IRQ�ص�����                   */
    TObjNode  ObjNode;                               /* IRQ���ڶ��е�����ָ��         */
} TIrq;
#endif

extern void uIrqModuleInit(void);
extern void xIrqEnterISR(TIndex irqn);
extern TState xIrqSetVector(TIndex irqn, TISR pISR, TThread* pASR, TArgument data, TError* pError);
extern TState xIrqCleanVector(TIndex irqn, TError* pError);

#if (TCLC_IRQ_DAEMON_ENABLE)
extern TState xIrqPostRequest(TIrq* pIRQ, TPriority priority, 
                              TIrqEntry pEntry, TArgument data, TError* pError);
extern TState xIrqCancelRequest(TIrq* pIRQ, TError* pError);
extern void uIrqCreateDaemon(void);
#endif

#endif

#endif /* _TCL_IRQ_H */

