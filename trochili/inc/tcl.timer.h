/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_TIMER_H_
#define _TCL_TIMER_H_

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"

#if (TCLC_TIMER_ENABLE)

#define TIMER_ERR_NONE               (0x0U)
#define TIMER_ERR_FAULT              (0x1<<0)           /* һ���Դ���                          */
#define TIMER_ERR_UNREADY            (0x1<<1)           /* ��ʱ������ṹδ��ʼ��              */
#define TIMER_ERR_TYPE               (0x1<<2)           /* ��ʱ�����ʹ���                      */


/* ��ʱ������ö�ٶ��� */
enum TimerTypeDef
{
    eThreadTimer = 0,                                    /* �����߳���ͨ��ʱ��ʱ��               */
    eIpcTimer,                                           /* ����IPC��ʱ��������ʱ��              */
    eUserTimer,                                          /* �û���ʱ��                           */
};
typedef enum TimerTypeDef TTimerType;

/* ��ʱ��״̬ö�ٶ��� */
enum TimerStatusDef
{
    eTimerDormant = 0,                                   /* ��ʱ���ĳ�ʼ״̬                     */
    eTimerActive,                                        /* ��ʱ������״̬                       */
#if (TCLC_TIMER_DAEMON_ENABLE)
    eTimerExpired                                        /* �û���ʱ������״̬                   */
#endif
};
typedef enum TimerStatusDef TTimerStatus;

/* ��ʱ�����Ա�Ƕ��� */
#define TIMER_PROP_NONE           (0x0)                 /* ��ʱ�������Ա��                     */
#define TIMER_PROP_READY          (0x1<<0)              /* ��ʱ���������                       */
#define TIMER_PROP_PERIODIC       (0x1<<1)              /* �û����ڻص���ʱ��                   */
#define TIMER_PROP_URGENT         (0x1<<2)              /* �û������ص���ʱ��                   */

/* �û���ʱ���ص��������Ͷ��� */
typedef void(*TTimerRoutine)(TArgument data);

/* ��ʱ���ṹ���� */
struct TimerDef
{
    TProperty     Property;                              /* ��ʱ������                           */
    TBase32       ID;                                    /* ��ʱ�����                           */
    void*         Owner;                                 /* ��ʱ�������߳�                       */
    TTimerStatus  Status;                                /* ��ʱ��״̬                           */
    TTimerType    Type;                                  /* ��ʱ������                           */
    TTimeTick     MatchTicks;                            /* ��ʱ����ʱʱ��                       */
    TTimeTick     PeriodTicks;                           /* ��ʱ����ʱ����                       */
    TTimerRoutine Routine;                               /* �û���ʱ���ص�����                   */
    TArgument     Argument;                              /* ��ʱ����ʱ�ص�����                   */
    TObjNode      ObjNode;                               /* ��ʱ�����ڶ��е�����ָ��             */
};
typedef struct TimerDef TTimer;

/* ��ʱ�����нṹ���� */
struct TimerListDef
{
    TObjNode*    DormantHandle;
    TObjNode*    ActiveHandle[TCLC_TIMER_WHEEL_SIZE];
#if (TCLC_TIMER_DAEMON_ENABLE)
    TObjNode*    ExpiredHandle;
#endif
};
typedef struct TimerListDef TTimerList;

//#define TCLM_NODE2TIMER(NODE) ((TThread*)((TByte*)(NODE)-OFF_SET_OF(TTimer, ObjNode)))

extern void uTimerModuleInit(void);
extern void uTimerCreate(TTimer* pTimer, TProperty property, TTimerType type, TTimeTick ticks,
                         TTimerRoutine pRoutine, TArgument data, void* pOwner);
extern void uTimerDelete(TTimer* pTimer);
extern void uTimerConfig(TTimer* pTimer, TTimerType type, TTimeTick ticks);
extern void uTimerStart(TTimer* pTimer, TTimeTick lagticks);
extern void uTimerStop(TTimer* pTimer);
extern void uTimerTickISR(void);

extern TState xTimerCreate(TTimer* pTimer, TProperty property, TTimeTick ticks,
                           TTimerRoutine pRoutine, TArgument data, TError* pError);
extern TState xTimerDelete(TTimer * pTimer, TError* pError);
extern TState xTimerStart(TTimer* pTimer, TTimeTick lagticks, TError* pError);
extern TState xTimerStop(TTimer* pTimer, TError* pError);
extern TState xTimerConfig(TTimer* pTimer, TTimeTick ticks, TError* pError);

#if (TCLC_TIMER_DAEMON_ENABLE)
extern void uTimerCreateDaemon(void);
#endif
#endif


#endif /*_TCL_TIMER_H_*/

