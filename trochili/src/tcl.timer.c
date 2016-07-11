/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include <string.h>

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.debug.h"
#include "tcl.cpu.h"
#include "tcl.ipc.h"
#include "tcl.kernel.h"
#include "tcl.thread.h"
#include "tcl.timer.h"

#if (TCLC_TIMER_ENABLE)

/* �ں˶�ʱ����Ϊ3�֣��ֱ������߳���ʱ��ʱ�޷�ʽ������Դ���û���ʱ�� */
static TTimerList TimerList;


/*************************************************************************************************
 *  ���ܣ��ں˶�ʱ����ʼ������                                                                   *
 *  ������(1) pTimer   ��ʱ���ṹ��ַ                                                            *
 *        (2) property ��ʱ������                                                                *
 *        (3) type     ��ʱ������                                                                *
 *        (4) ticks    ��ʱ��ʱ�ӽ�����Ŀ                                                        *
 *        (3) pRoutine ��ʱ���ص�����                                                            *
 *        (4) data     ��ʱ���ص���������                                                        *
 *        (5) pOwner   ��ʱ�������߳�                                                            *
 *  ���أ���                                                                                     *
 *  ˵��                                                                                         *
 *************************************************************************************************/
void uTimerCreate(TTimer* pTimer, TProperty property, TTimerType type, TTimeTick ticks,
                  TTimerRoutine pRoutine, TArgument data, void* pOwner)
{
    /* ��ʼ����ʱ�������ö�ʱ����Ϣ */
    pTimer->Status         = eTimerDormant;
    pTimer->Property       = (property | TIMER_PROP_READY);
    pTimer->ID             = uKernelVariable.ObjID;
    uKernelVariable.ObjID++;

    /* uTimerConfig�����ᶯ̬����Type\PeriodTicks��2������ */
    pTimer->Type           = type;
    pTimer->PeriodTicks    = ticks;
    pTimer->MatchTicks     = TCLM_MAX_VALUE64;
    pTimer->Routine        = pRoutine;
    pTimer->Argument       = data;
    pTimer->Owner          = pOwner;

    /* ���ö�ʱ������ڵ���Ϣ, ������ʱ���������߶����� */
    pTimer->ObjNode.Next   = (TObjNode*)0;
    pTimer->ObjNode.Prev   = (TObjNode*)0;
    pTimer->ObjNode.Handle = (TObjNode**)0;
    pTimer->ObjNode.Data   = (TBase32*)(&(pTimer->MatchTicks));
    pTimer->ObjNode.Owner  = (void*)pTimer;
    uObjListAddNode(&(TimerList.DormantHandle), &(pTimer->ObjNode), eQuePosHead);
}


/*************************************************************************************************
 *  ���ܣ��ں˶�ʱ��ȡ����ʼ��                                                                   *
 *  ������(1) pTimer  ��ʱ���ṹ��ַ                                                             *
 *  ���أ���                                                                                     *
 *  ˵��                                                                                         *
 *************************************************************************************************/
void uTimerDelete(TTimer* pTimer)
{
    /* �����ʱ���������������Ƴ� */
    uObjListRemoveNode(pTimer->ObjNode.Handle, &(pTimer->ObjNode));

    /* ��ն�ʱ������ */
    memset(pTimer, 0U, sizeof(TTimer));
}


/*************************************************************************************************
 *  ���ܣ����ö�ʱ�����ͺͶ�ʱʱ��                                                               *
 *  ������(1) pTimer ��ʱ���ṹ��ַ                                                              *
 *        (2) type   ��ʱ������                                                                  *
 *        (2) ticks  ��ʱ��ʱ�ӽ�����Ŀ                                                          *
 *  ���أ���                                                                                     *
 *  ˵��                                                                                         *
 *************************************************************************************************/
void uTimerConfig(TTimer* pTimer, TTimerType type, TTimeTick ticks)
{
    if (pTimer->Status == eTimerDormant)
    {
        pTimer->Type        = type;
        pTimer->PeriodTicks = ticks;
    }
}


/*************************************************************************************************
 *  ���ܣ��ں˶�ʱ����������                                                                     *
 *  ������(1) pTimer     ��ʱ���ṹ��ַ                                                          *
 *        (2) lagTicks   ��ʱ���ӻ���ʼ����ʱ��                                                  *
 *  ���أ���                                                                                     *
 *  ˵��  ֻ�д���eTimerDormant״̬�Ķ�ʱ�����ܹ�������                                          *
 *        ֻ���û��̲߳��ж�ʱ���ӻ���ʼ��ʱ������                                               *
 *************************************************************************************************/
void uTimerStart(TTimer* pTimer, TTimeTick lagticks)
{
    TIndex spoke;

    if (pTimer->Status == eTimerDormant)
    {
        /* ����ʱ�������߶������Ƴ� */
        uObjListRemoveNode(pTimer->ObjNode.Handle, &(pTimer->ObjNode));

        /* ����ʱ������������ */
        pTimer->MatchTicks  = uKernelVariable.Jiffies + pTimer->PeriodTicks + lagticks;
        spoke = (TBase32)(pTimer->MatchTicks % TCLC_TIMER_WHEEL_SIZE);
        uObjListAddPriorityNode(&(TimerList.ActiveHandle[spoke]), &(pTimer->ObjNode));
        pTimer->Status = eTimerActive;
    }
}


/*************************************************************************************************
 *  ���ܣ��ں˶�ʱ��ֹͣ����                                                                     *
 *  ������(1) pTimer ��ʱ����ַ                                                                  *
 *  ���أ���                                                                                     *
 *  ˵��  ֻ�д���eTimerActive��eTimerExpired ״̬�Ķ�ʱ�����ܹ�ֹͣ                             *
 *        ��ʱ����ֹͣ��������eTimerDormant״̬���Կɱ��ٴ�����                                  *
 *************************************************************************************************/
void uTimerStop(TTimer* pTimer)
{
    /* ����ʱ���ӻ����/�����������Ƴ����ŵ����߶����� */
    if (pTimer->Status != eTimerDormant)
    {
        uObjListRemoveNode(pTimer->ObjNode.Handle, &(pTimer->ObjNode));
        uObjListAddNode(&(TimerList.DormantHandle), &(pTimer->ObjNode), eQuePosHead);
        pTimer->Status = eTimerDormant;
    }
}


/*************************************************************************************************
 *  ���ܣ��û���ʱ�����ú���                                                                     *
 *  ������(1) pTimer  ��ʱ���ṹ��ַ                                                             *
 *  ���أ���                                                                                     *
 *  ˵��                                                                                         *
 *************************************************************************************************/
static void ResetTimer(TTimer* pTimer)
{
    TIndex spoke;
    KNL_ASSERT((pTimer->Type == eUserTimer), "");

    /* ����ʱ���������������Ƴ� */
    uObjListRemoveNode(pTimer->ObjNode.Handle, &(pTimer->ObjNode));

    /* ���������͵��û���ʱ�����·Żػ��ʱ�������� */
    if (pTimer->Property & TIMER_PROP_PERIODIC)
    {
        /* ��Ҫ���»ָ���ʱ������ֵ */
        pTimer->MatchTicks = uKernelVariable.Jiffies + pTimer->PeriodTicks;
        spoke = (TBase32)(pTimer->MatchTicks % TCLC_TIMER_WHEEL_SIZE);
        uObjListAddPriorityNode(&(TimerList.ActiveHandle[spoke]), &(pTimer->ObjNode));
        pTimer->Status = eTimerActive;
    }
    else
    {
        /* �����λص���ʱ���ŵ����߶����� */
        uObjListAddNode(&(TimerList.DormantHandle), &(pTimer->ObjNode), eQuePosHead);
        pTimer->Status = eTimerDormant;
    }
}


/*************************************************************************************************
 *  ���ܣ��ں˶�ʱ��ִ�д�����                                                                 *
 *  ������(1) pTimer ��ʱ��                                                                      *
 *  ���أ���                                                                                     *
 *  ˵��: (1)�߳���ʱ��ʱ���̱߳������ں��߳���ʱ������                                        *
 *        (2)�߳���ʱ�޷�ʽ������Դ��ʱ������ò�����Դ�ᱻ������Դ���߳���������              *
 *           ������Ȼ���̶߳��в������ǲ����е��ȣ�����Ϊ������������ж��е��õģ�              *
 *           �����һ���жϷ��غ󣬻᳢�Խ���һ���߳��л����������������л��Ļ��ǰװ��˷�ʱ��    *
 *************************************************************************************************/
static void DispatchTimer(TTimer* pTimer)
{
    TThread* pThread;
#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_TIMER_ENABLE))
    TBool HiRP = eFalse;
#endif

    /* �����ʱ�����߳���ʱ���͵Ķ�ʱ�� */
    if (pTimer->Type == eThreadTimer)
    {
        /* ����ʱ���ӻ�������Ƴ�,����ʱ���ŵ����߶����� */
        uObjListRemoveNode(pTimer->ObjNode.Handle, &(pTimer->ObjNode));
        uObjListAddNode(&(TimerList.DormantHandle), &(pTimer->ObjNode), eQuePosHead);
        pTimer->Status = eTimerDormant;

        /* �����ʱ�����ڵ��̴߳�����ʱ״̬����̷߳����ں��̻߳���� */
        pThread = (TThread*)(pTimer->Owner);
        KNL_ASSERT((pThread->Status == eThreadDelayed), "");
        uThreadLeaveQueue(uKernelVariable.ThreadAuxiliaryQueue, pThread);
        uThreadEnterQueue(uKernelVariable.ThreadReadyQueue, pThread, eQuePosTail);

        /* ���߳��뿪��������ʱ���Ѿ��������ı���ִ�У�����ʱ��Ƭ��δ�ľ���
           ���߳��ٴν����������ʱ����Ҫ�ָ��̵߳�ʱ�ӽ�������
           ���¼������������ʱ������� */
        pThread->Ticks = pThread->BaseTicks;
        pThread->Status = eThreadReady;
    }

    /* �����ʱ�����û���ʱ������ */
    else if (pTimer->Type == eUserTimer)
    {
#if (TCLC_TIMER_DAEMON_ENABLE)
        /* �����Ķ�ʱ��ֱ����ISR�ﴦ��ص�����;
        ���򽫶�ʱ�������ں˶�ʱ�������б�����ɶ�ʱ���ػ��̴߳��� */
        if (pTimer->Property & TIMER_PROP_URGENT)
        {
            pTimer->Routine(pTimer->Argument);
            ResetTimer(pTimer);
        }
        else
        {
            uObjListRemoveNode(pTimer->ObjNode.Handle, &(pTimer->ObjNode));
            uObjListAddNode(&(TimerList.ExpiredHandle), &(pTimer->ObjNode), eQuePosTail);
            pTimer->Status = eTimerExpired;
        }
#else
        /* ��ISR��ֱ�Ӵ���ʱ���ص����� */
        pTimer->Routine(pTimer->Argument);
        ResetTimer(pTimer);
#endif
    }

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_TIMER_ENABLE))
    /* �����ʱ�����߳��������͵Ķ�ʱ����ʱ�޷�ʽ������Դ�����̴߳����������л��� */
    else if (pTimer->Type == eIpcTimer)
    {
        /* ����ʱ���ӻ�������Ƴ�,����ʱ���ŵ����߶����� */
        uObjListRemoveNode(pTimer->ObjNode.Handle, &(pTimer->ObjNode));
        uObjListAddNode(&(TimerList.DormantHandle), &(pTimer->ObjNode), eQuePosHead);
        pTimer->Status = eTimerDormant;

        /* IPC����ȷ�������ٴ�ֹͣtimer */
        pThread = (TThread*)(pTimer->Owner);
        KNL_ASSERT((pThread->Status == eThreadBlocked), "");
        uIpcUnblockThread(&(pThread->IpcContext), eFailure, IPC_ERR_TIMEO, &HiRP);
    }
#endif

    else
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }
}


/*************************************************************************************************
 *  ���ܣ��ں˶�ʱ��ISR������                                                                  *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵��:                                                                                        *
 *************************************************************************************************/
void uTimerTickISR(void)
{
    TState state;
    TError error;

    TTimer*   pTimer;
    TIndex    spoke;
    TObjNode* pNode;
    TObjNode* pNext;

    /* ��õ�ǰ���ʱ������ */
    spoke = (TIndex)(uKernelVariable.Jiffies % TCLC_TIMER_WHEEL_SIZE);
    pNode = TimerList.ActiveHandle[spoke];

    /* ��鵱ǰ���ʱ���������ÿ����ʱ�� */
    while (pNode != (TObjNode*)0)
    {
        pNext = pNode->Next;
        pTimer = (TTimer*)(pNode->Owner);

        /* �����ǰ��ʱ���Ķ�ʱʱ�ӽ������ʹ�ʱ���ں�ʱ�ӽ�������� */
        if (pTimer->MatchTicks == uKernelVariable.Jiffies)
        {
            /* ����ʱ�� */
            DispatchTimer(pTimer);
        }
        else
        {
            break;
        }
        pNode = pNext;
    }

    /* �����Ҫ�����ں˶�ʱ���ػ��߳� */
#if (TCLC_TIMER_DAEMON_ENABLE)
    if (TimerList.ExpiredHandle != (TObjNode*)0)
    {
        state = uThreadSetReady(uKernelVariable.TimerDaemon,
                                eThreadSuspended, &error);
        state = state;
    }
#endif
}


/*************************************************************************************************
 *  ���ܣ��û���ʱ����ʼ������                                                                   *
 *  ������(1) pTimer   ��ʱ����ַ                                                                *
 *        (2) property ��ʱ������                                                                *
 *        (3) ticks    ��ʱ���δ���Ŀ                                                            *
 *        (4) pRoutine �û���ʱ���ص�����                                                        *
 *        (5) data     �û���ʱ���ص���������                                                    *
 *        (6) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵��                                                                                         *
 *************************************************************************************************/
TState xTimerCreate(TTimer* pTimer, TProperty property, TTimeTick ticks,
                    TTimerRoutine pRoutine, TArgument data, TError* pError)
{
    TState state = eFailure;
    TError error = TIMER_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* ��鶨ʱ���������� */
    if (!(pTimer->Property & TIMER_PROP_READY))
    {
        if (ticks > 0U)
        {
            uTimerCreate(pTimer, property, eUserTimer, ticks, pRoutine, data, (void*)0);
            error = TIMER_ERR_NONE;
            state = eSuccess;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��ں˶�ʱ��ȡ����ʼ��                                                                   *
 *  ������(1) pTimer   ��ʱ���ṹ��ַ                                                            *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵��                                                                                         *
 *************************************************************************************************/
TState xTimerDelete(TTimer* pTimer, TError* pError)
{
    TState state = eFailure;
    TError error = TIMER_ERR_UNREADY;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* ��鶨ʱ���������� */
    if (pTimer->Property & TIMER_PROP_READY)
    {
        if (pTimer->Type == eUserTimer)
        {
            uTimerDelete(pTimer);
            error = TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = TIMER_ERR_FAULT;
        }
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ���ʱ����������                                                                         *
 *  ������(1) pTimer     ��ʱ���ṹ��ַ                                                          *
 *        (2) lagticks   ��ʱ���ӻ���ʼ����ʱ��                                                  *
 *        (3) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵������                                                                                     *
 *************************************************************************************************/
TState xTimerStart(TTimer* pTimer,TTimeTick lagticks, TError* pError)
{
    TState state = eFailure;
    TError error = TIMER_ERR_UNREADY;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* ��鶨ʱ���������� */
    if (pTimer->Property & TIMER_PROP_READY)
    {
        if (pTimer->Type == eUserTimer)
        {
            uTimerStart(pTimer, lagticks);
            error = TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = TIMER_ERR_FAULT;
        }
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ�ֹͣ�û���ʱ������                                                                     *
 *  ������(1) pTimer   ��ʱ����ַ                                                                *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *        (3) pError   ��ϸ���ý��                                                              *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xTimerStop(TTimer* pTimer, TError* pError)
{
    TState state = eFailure;
    TError error = TIMER_ERR_UNREADY;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* ��鶨ʱ���������� */
    if (pTimer->Property & TIMER_PROP_READY)
    {
        if (pTimer->Type == eUserTimer)
        {
            uTimerStop(pTimer);
            error = TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = TIMER_ERR_FAULT;
        }
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ����ö�ʱ�����ͺͶ�ʱʱ��                                                               *
 *  ������(1) pTimer ��ʱ���ṹ��ַ                                                              *
 *        (2) ticks  ��ʱ��ʱ�ӽ�����Ŀ                                                          *
 *        (3) pError ��ϸ���ý��                                                                *
 *  ���أ���                                                                                     *
 *  ˵��                                                                                         *
 *************************************************************************************************/
TState xTimerConfig(TTimer* pTimer, TTimeTick ticks, TError* pError)
{
    TState state = eFailure;
    TError error = TIMER_ERR_UNREADY;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* ��鶨ʱ���������� */
    if (pTimer->Property & TIMER_PROP_READY)
    {
        /* ��ʱ�����ͼ�飬Ȼ�󱣳ֶ�ʱ�����Ͳ��䣬���І��� */
        if (pTimer->Type == eUserTimer)
        {
            uTimerConfig(pTimer, eUserTimer, ticks);
            error = TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = TIMER_ERR_FAULT;
        }
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


#if (TCLC_TIMER_DAEMON_ENABLE)

/* �ں˶�ʱ���ػ��̶߳����ջ���� */
static TBase32 TimerDaemonStack[TCLC_TIMER_DAEMON_STACK_BYTES >> 2];
static TThread TimerDaemonThread;

/* �ں˶�ʱ���ػ��̲߳������κ��̹߳���API���� */
#define TIMER_DAEMON_ACAPI (THREAD_ACAPI_NONE)

/*************************************************************************************************
 *  ���ܣ��ں��еĶ�ʱ���ػ��̺߳���                                                             *
 *  ������(1) argument ��ʱ���̵߳��û�����                                                      *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static void TimerDaemonEntry(TArgument argument)
{
    TState        state;
    TError        error;
    TBase32       imask;
    TTimer*       pTimer;
    TTimerRoutine pRoutine;
    TArgument     data;

    while(eTrue)
    {
        CpuEnterCritical(&imask);

        /* ���������ʱ������Ϊ���򽫶�ʱ���ػ��߳����� */
        if (TimerList.ExpiredHandle == (TObjNode*)0)
        {
            state = uThreadSetUnready(&TimerDaemonThread, eThreadSuspended, 0U, &error);
            state = state;
            CpuLeaveCritical(imask);
        }
        else
        {
            /* ��������û���ʱ�� */
            pTimer = (TTimer*)(TimerList.ExpiredHandle->Owner);
            pRoutine = pTimer->Routine;
            data = pTimer->Argument;
            ResetTimer(pTimer);
            CpuLeaveCritical(imask);

            /* ���̻߳����´���ʱ���ص����� */
            pRoutine(data);
        }
    }
}


/*************************************************************************************************
 *  ���ܣ���ʼ���û���ʱ���ػ��߳�                                                               *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void uTimerCreateDaemon(void)
{
    /* ����ں��Ƿ��ڳ�ʼ״̬ */
    if(uKernelVariable.State != eOriginState)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /* ��ʼ���ں˶�ʱ�������߳� */
    uThreadCreate(&TimerDaemonThread,
                  eThreadSuspended,
                  THREAD_PROP_PRIORITY_FIXED|\
                  THREAD_PROP_CLEAN_STACK|\
                  THREAD_PROP_DAEMON,
                  TIMER_DAEMON_ACAPI,
                  TimerDaemonEntry,
                  (TArgument)(0U),
                  (void*)TimerDaemonStack,
                  (TBase32)TCLC_TIMER_DAEMON_STACK_BYTES,
                  (TPriority)TCLC_TIMER_DAEMON_PRIORITY,
                  (TTimeTick)TCLC_TIMER_DAEMON_SLICE);

    /* ��ʼ����ص��ں˱��� */
    uKernelVariable.TimerDaemon = &TimerDaemonThread;
}
#endif


/*************************************************************************************************
 *  ���ܣ���ʱ��ģ���ʼ��                                                                       *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void uTimerModuleInit(void)
{
    /* ����ں��Ƿ��ڳ�ʼ״̬ */
    if(uKernelVariable.State != eOriginState)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    memset(&TimerList, 0, sizeof(TimerList));

    /* ��ʼ����ص��ں˱��� */
    uKernelVariable.TimerList = &TimerList;
}
#endif

