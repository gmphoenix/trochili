/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include <string.h>

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.cpu.h"
#include "tcl.ipc.h"
#include "tcl.debug.h"
#include "tcl.kernel.h"
#include "tcl.timer.h"
#include "tcl.thread.h"

/* �ں˽��������ж���,���ھ��������е��̶߳�������������� */
static TThreadQueue ThreadReadyQueue;

/* �ں��̸߳������ж��壬������ʱ���������ߵ��̶߳�������������� */
static TThreadQueue ThreadAuxiliaryQueue;


#if (TCLC_THREAD_STACK_CHECK_ENABLE)
/*************************************************************************************************
 *  ���ܣ��澯�ͼ���߳�ջ�������                                                               *
 *  ������(1) pThread  �̵߳�ַ                                                                  *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static void CheckThreadStack(TThread* pThread)
{
    if ((pThread->StackTop < pThread->StackBarrier) || (*(TBase32*)(pThread->StackBarrier) !=
            TCLC_THREAD_STACK_BARRIER_VALUE))
    {
        uKernelVariable.Diagnosis |= KERNEL_DIAG_THREAD_ERROR;
        pThread->Diagnosis |= THREAD_DIAG_STACK_OVERFLOW;
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    if (pThread->StackTop < pThread->StackAlarm)
    {
        pThread->Diagnosis |= THREAD_DIAG_STACK_ALARM;
    }
}

#endif


/*************************************************************************************************
 *  ���ܣ��߳����м��������̵߳����ж�����Ϊ����                                               *
 *  ������(1) pThread  �̵߳�ַ                                                                  *
 *  ���أ���                                                                                     *
 *  ˵�������ﴦ���̵߳ķǷ��˳�����                                                             *
 *  ˵������������ǰ׺'x'(eXtreme)��ʾ��������Ҫ�����ٽ�������                                   *
 *************************************************************************************************/
static void xSuperviseThread(TThread* pThread)
{
    TState state;
    TError error;
    TReg32 imask;

    /* �����û�ASR�߳������������ຯ�����ص��ǽ�Һ�һ����ִ�У�Ȼ�����̹��𣬵ȴ��´ε��á�
    ��ȻIrq��Timer�ػ��߳�Ҳ���жϷ����̣߳���������ΪASR�ڴ˴���
    �����������Լ������������ */
    if ((pThread->Property &THREAD_PROP_RUNASR))
    {
        while (eTrue)
        {
            /* ִ���̺߳��� */
            pThread->Entry(pThread->Argument);

            /* �߳����н�����ֱ�ӹ���,�ȴ��ٴα�ִ�� */
            CpuEnterCritical(&imask);
            state = uThreadSetUnready(pThread, eThreadSuspended, 0U, &error);
            if (state == eFailure)
            {
                uKernelVariable.Diagnosis |= KERNEL_DIAG_THREAD_ERROR;
                pThread->Diagnosis |= THREAD_DIAG_INVALID_STATE;
                uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
            }
            CpuLeaveCritical(imask);
        }
    }
    /* ����RUN TO COMPLETION �߳��˳�����, ��ֹ�˳�������ִ�е��Ƿ�(δ֪)ָ�� */
    else if (pThread->Property &THREAD_PROP_RUN2COMPLETION)
    {
        /* ִ���̺߳��� */
        pThread->Entry(pThread->Argument);

        /* �߳����н�����ֱ�ӹ���,�ȴ���̴��� */
        CpuEnterCritical(&imask);
        state = uThreadSetUnready(pThread, eThreadDormant, 0U, &error);
        if (state == eFailure)
        {
            uKernelVariable.Diagnosis |= KERNEL_DIAG_THREAD_ERROR;
            pThread->Diagnosis |= THREAD_DIAG_INVALID_STATE;
            uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
        }
        CpuLeaveCritical(imask);
    }
    else
    {
        /* ִ���û��̺߳��� */
        pThread->Entry(pThread->Argument);

        /* ��ֹRUNFOREVER�̲߳�С���˳����·Ƿ�ָ������������� */
        uKernelVariable.Diagnosis |= KERNEL_DIAG_THREAD_ERROR;
        pThread->Diagnosis |= THREAD_DIAG_INVALID_EXIT;
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }
}


/*************************************************************************************************
 *  ���ܣ���������̶߳����е�������ȼ�����                                                     *
 *  ��������                                                                                     *
 *  ���أ�HiRP (Highest Ready Priority)                                                          *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void uThreadCalcHiRP(TPriority* priority)
{
    /* ����������ȼ���������˵���ں˷����������� */
    if (ThreadReadyQueue.PriorityMask == (TBitMask)0)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }
    *priority = CpuCalcHiPRIO(ThreadReadyQueue.PriorityMask);
}

/*************************************************************************************************
 *  ���ܣ��������ᷢ���߳���ռ                                                                   *
 *  ������(1) HiRP �Ƿ��Ѿ��õ��ȵ�ǰ�߳����ȼ��ߵ��߳�                                          *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void uThreadPreempt(TBool HiRP)
{
    /* ���̻߳����£������ǰ�̵߳����ȼ��Ѿ��������߳̾������е�������ȼ���
    �����ں˴�ʱ��û�йر��̵߳��ȣ���ô����Ҫ����һ���߳���ռ */
    if ((uKernelVariable.State == eThreadState) &&
            (uKernelVariable.Schedulable == eTrue) &&
            (HiRP == eTrue))
    {
        uThreadSchedule();
    }
}


/*************************************************************************************************
 *  ���ܣ���ʼ���ں��̹߳���ģ��                                                                 *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵�����ں��е��̶߳�����Ҫ��һ�¼��֣�                                                       *
 *        (1) �߳̾�������,���ڴ洢���о����߳�(�������е��߳�)���ں���ֻ��һ����������          *
 *        (2) �̸߳�������, ���г�ʼ��״̬����ʱ״̬������״̬���̶߳��洢����������С�         *
 *            ͬ���ں���ֻ��һ�����߶���                                                         *
 *        (3) IPC������߳���������                                                              *
 *************************************************************************************************/
void uThreadModuleInit(void)
{
    /* ����ں��Ƿ��ڳ�ʼ״̬ */
    if (uKernelVariable.State != eOriginState)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    memset(&ThreadReadyQueue, 0, sizeof(ThreadReadyQueue));
    memset(&ThreadAuxiliaryQueue, 0, sizeof(ThreadAuxiliaryQueue));

    uKernelVariable.ThreadReadyQueue = &ThreadReadyQueue;
    uKernelVariable.ThreadAuxiliaryQueue = &ThreadAuxiliaryQueue;
}

/*
1 ��ǰ�߳��뿪�������к��ٴμ����������ʱ��һ��������Ӧ�Ķ���β���������¼���ʱ��Ƭ��
2 ��ǰ�߳��ھ��������ڲ��������ȼ�ʱ�����µĶ�����Ҳһ��Ҫ�ڶ���ͷ��
 */

/*************************************************************************************************
 *  ���ܣ����̼߳��뵽ָ�����̶߳�����                                                           *
 *  ������(1) pQueue  �̶߳��е�ַ��ַ                                                           *
 *        (2) pThread �߳̽ṹ��ַ                                                               *
 *        (3) pos     �߳����̶߳����е�λ��                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void uThreadEnterQueue(TThreadQueue* pQueue, TThread* pThread, TQueuePos pos)
{
    TPriority priority;
    TObjNode** pHandle;

    /* ����̺߳��̶߳��� */
    KNL_ASSERT((pThread != (TThread*)0), "");
    KNL_ASSERT((pThread->Queue == (TThreadQueue*)0), "");

    /* �����߳����ȼ��ó��߳�ʵ�������ֶ��� */
    priority = pThread->Priority;
    pHandle = &(pQueue->Handle[priority]);

    /* ���̼߳���ָ���ķֶ��� */
    uObjQueueAddFifoNode(pHandle, &(pThread->ObjNode), pos);

    /* �����߳��������� */
    pThread->Queue = pQueue;

    /* �趨���߳����ȼ�Ϊ�������ȼ� */
    pQueue->PriorityMask |= (0x1 << priority);
}


/*************************************************************************************************
 *  ���ܣ����̴߳�ָ�����̶߳������Ƴ�                                                           *
 *  ������(1) pQueue  �̶߳��е�ַ��ַ                                                           *
 *        (2) pThread �߳̽ṹ��ַ                                                               *
 *  ���أ���                                                                                     *
 *  ˵����FIFO PRIO���ַ�����Դ�ķ�ʽ                                                            *
 *************************************************************************************************/
void uThreadLeaveQueue(TThreadQueue* pQueue, TThread* pThread)
{
    TPriority priority;
    TObjNode** pHandle;

    /* ����߳��Ƿ����ڱ�����,������������ں˷����������� */
    KNL_ASSERT((pThread != (TThread*)0), "");
    KNL_ASSERT((pQueue == pThread->Queue), "");

    /* �����߳����ȼ��ó��߳�ʵ�������ֶ��� */
    priority = pThread->Priority;
    pHandle = &(pQueue->Handle[priority]);

    /* ���̴߳�ָ���ķֶ�����ȡ�� */
    uObjQueueRemoveNode(pHandle, &(pThread->ObjNode));

    /* �����߳��������� */
    pThread->Queue = (TThreadQueue*)0;

    /* �����߳��뿪���к�Զ������ȼ�������ǵ�Ӱ�� */
    if (pQueue->Handle[priority] == (TObjNode*)0)
    {
        /* �趨���߳����ȼ�δ���� */
        pQueue->PriorityMask &= (~(0x1 << priority));
    }
}


/*************************************************************************************************
 *  ���ܣ��߳�ʱ��Ƭ����������ʱ��Ƭ�жϴ���ISR�л���ñ�����                                  *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵��������������˵�ǰ�̵߳�ʱ��Ƭ��������û��ѡ����Ҫ���ȵĺ���̺߳ͽ����߳��л�         *
 *************************************************************************************************/
/* ��ǰ�߳̿��ܴ���3��λ��
1 �������е�ͷλ��(�κ����ȼ�)
2 �������е�����λ��(�κ����ȼ�)
3 ����������
ֻ�����1����Ҫ����ʱ��Ƭ��ת�Ĵ�������ʱ���漰�߳��л�,��Ϊ������ֻ��ISR�е��á�*/

/* ������Ҫ����Ӧ�ô�������ж����ȼ���� */
void uThreadTickISR(void)
{
    TThread* pThread;
    TObjNode* pHandle;
    TPriority priority;

    /* ����ǰ�߳�ʱ��Ƭ��ȥ1��������,�߳������ܽ�������1 */
    pThread = uKernelVariable.CurrentThread;
    pThread->Ticks--;
    pThread->Jiffies++;

    /* �������ʱ��Ƭ������� */
    if (pThread->Ticks == 0U)
    {
        /* �ָ��̵߳�ʱ�ӽ����� */
        pThread->Ticks = pThread->BaseTicks;

        /* ����ں˴�ʱ�����̵߳��� */
        if (uKernelVariable.Schedulable == eTrue)
        {
            /* �ж��߳��ǲ��Ǵ����ں˾����̶߳��е�ĳ�����ȼ��Ķ���ͷ */
            pHandle = ThreadReadyQueue.Handle[pThread->Priority];
            if ((TThread*)(pHandle->Owner) == pThread)
            {
                priority = pThread->Priority;
                /* ����ʱ��Ƭ���ȣ�֮��pThread�����̶߳���β��,
                ��ǰ�߳������̶߳���Ҳ����ֻ�е�ǰ�߳�Ψһ1���߳� */
                ThreadReadyQueue.Handle[priority] = (ThreadReadyQueue.Handle[priority])->Next;

                /* ���߳�״̬��Ϊ����,׼���߳��л� */
                pThread->Status = eThreadReady;
            }
        }
    }
}


/*************************************************************************************************
 *  ���ܣ����������̵߳���                                                                       *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵�����̵߳ĵ���������ܱ�ISR����ȡ��                                                        *
 *************************************************************************************************/
/*
1 ��ǰ�߳��뿪���м������������������У��ٴν������ʱ,ʱ��Ƭ��Ҫ���¼���,
  �ڶ����е�λ��Ҳ�涨һ�����ڶ�β
2 ���µ�ǰ�̲߳�����߾������ȼ���ԭ����
  1 ������ȼ����ߵ��߳̽����������
  2 ��ǰ�߳��Լ��뿪����
  3 ����̵߳����ȼ������
  4 ��ǰ�̵߳����ȼ�������
  5 ��ǰ�߳�Yiled
  6 ʱ��Ƭ�ж��У���ǰ�̱߳���ת

3 ��cortex��������, ������һ�ֿ���:
��ǰ�߳��ͷ��˴�����������PendSV�жϵõ���Ӧ֮ǰ���������������ȼ��жϷ�����
�ڸ߼�isr���ְѵ�ǰ�߳���Ϊ������
1 ���ҵ�ǰ�߳���Ȼ����߾������ȼ���
2 ���ҵ�ǰ�߳���Ȼ����߾����̶߳��еĶ���ͷ��
��ʱ��Ҫ����ȡ��PENDSV�Ĳ��������⵱ǰ�̺߳��Լ��л� */
void uThreadSchedule(void)
{
    TPriority priority;

    /* ����������ȼ���������˵���ں˷����������� */
    if (ThreadReadyQueue.PriorityMask == (TBitMask)0)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /* ������߾������ȼ�����ú���̣߳��������߳�ָ��Ϊ����˵���ں˷����������� */
    uThreadCalcHiRP(&priority);
    uKernelVariable.NomineeThread = (TThread*)((ThreadReadyQueue.Handle[priority])->Owner);
    if (uKernelVariable.NomineeThread == (TThread*)0)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /* ����̵߳����ȼ���ռ����ʱ��Ƭ��ת;
    ��������̵߳���(��������������"��ռ"��"����"�ĺ���) */
    if (uKernelVariable.NomineeThread != uKernelVariable.CurrentThread)
    {
#if (TCLC_THREAD_STACK_CHECK_ENABLE)
        CheckThreadStack(uKernelVariable.NomineeThread);
#endif
        uKernelVariable.NomineeThread->Status = eThreadRunning;
        if (uKernelVariable.CurrentThread->Status == eThreadRunning)
        {
            uKernelVariable.CurrentThread->Status = eThreadReady;
        }
        CpuConfirmThreadSwitch();
    }
    else
    {
        CpuCancelThreadSwitch();
        uKernelVariable.CurrentThread->Status = eThreadRunning;
    }
}


/*************************************************************************************************
 *  ���ܣ��߳̽ṹ��ʼ������                                                                     *
 *  ������(1)  pThread  �߳̽ṹ��ַ                                                             *
 *        (2)  status   �̵߳ĳ�ʼ״̬                                                           *
 *        (3)  property �߳�����                                                                 *
 *        (4)  acapi    ���̹߳���API����ɿ���                                                  *
 *        (5)  pEntry   �̺߳�����ַ                                                             *
 *        (6)  TArgument�̺߳�������                                                             *
 *        (7)  pStack   �߳�ջ��ַ                                                               *
 *        (8)  bytes    �߳�ջ��С������Ϊ��λ                                                   *
 *        (9)  priority �߳����ȼ�                                                               *
 *        (10) ticks    �߳�ʱ��Ƭ����                                                           *
 *  ���أ�(1)  eFailure                                                                          *
 *        (2)  eSuccess                                                                          *
 *  ˵����ע��ջ��ʼ��ַ��ջ��С��ջ�澯��ַ���ֽڶ�������                                       *
 *  ˵������������ǰ׺'u'(Universal)��ʾ������Ϊģ���ͨ�ú���                                   *
 *************************************************************************************************/
void uThreadCreate(TThread* pThread, TThreadStatus status, TProperty property, TBitMask acapi,
                   TThreadEntry pEntry, TArgument argument, void* pStack, TBase32 bytes,
                   TPriority priority, TTimeTick ticks)
{
    TThreadQueue* pQueue;

    /* �����߳�ջ������ݺ͹����̳߳�ʼջջ֡ */
    KNL_ASSERT((bytes >= TCLC_CPU_MINIMAL_STACK), "");

    /* ջ��С����4byte���� */
    bytes &= (~((TBase32)0x3));
    pThread->StackBase = (TBase32)pStack + bytes;

    /* ����߳�ջ�ռ� */
    if (property &THREAD_PROP_CLEAN_STACK)
    {
        memset(pStack, 0U, bytes);
    }

    /* ����(α��)�̳߳�ʼջ֡,���ｫ�߳̽ṹ��ַ��Ϊ���������̼߳�ܺ��� */
    CpuBuildThreadStack(&(pThread->StackTop), pStack, bytes, (void*)(&xSuperviseThread),
                        (TArgument)pThread);

    /* �����߳�ջ�澯��ַ */
#if (TCLC_THREAD_STACK_CHECK_ENABLE)
    pThread->StackAlarm = (TBase32)pStack + bytes - (bytes* TCLC_THREAD_STACK_ALARM_RATIO) / 100;
    pThread->StackBarrier = (TBase32)pStack;
    (*(TAddr32*)pStack) = TCLC_THREAD_STACK_BARRIER_VALUE;
#endif

    /* �����߳�ʱ��Ƭ��ز��� */
    pThread->Ticks = ticks;
    pThread->BaseTicks = ticks;
    pThread->Jiffies = 0U;

    /* �����߳����ȼ� */
    pThread->Priority = priority;
    pThread->BasePriority = priority;

    /* �����߳�ΨһID��ֵ */
    pThread->ThreadID = uKernelVariable.ObjID;
    uKernelVariable.ObjID++;

    /* �����߳���ں������̲߳��� */
    pThread->Entry = pEntry;
    pThread->Argument = argument;

    /* �����߳�����������Ϣ */
    pThread->Queue = (TThreadQueue*)0;

    /* �����̶߳�ʱ����Ϣ */
#if (TCLC_TIMER_ENABLE)
    uTimerCreate(&(pThread->Timer), (TProperty)0, eThreadTimer, TCLM_MAX_VALUE64,
                 (TTimerRoutine)0, (TArgument)0, (void*)pThread);
#endif

    /* ����߳�IPC���������� */
#if (TCLC_IPC_ENABLE)
    uIpcInitContext(&(pThread->IpcContext), (void*)pThread);
#endif

    /* ����߳�ռ�е���(MUTEX)���� */
#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MUTEX_ENABLE))
    pThread->LockList = (TObjNode*)0;
#endif

    /* ��ʼ�߳����������Ϣ */
    pThread->Diagnosis = THREAD_DIAG_NORMAL;

    /* �����߳��ܹ�֧�ֵ��̹߳���API */
    pThread->ACAPI = acapi;

    /* �����߳�����ڵ���Ϣ���̴߳�ʱ�������κ��̶߳��� */
    pThread->ObjNode.Owner = (void*)pThread;
    pThread->ObjNode.Data = (TBase32*)(&(pThread->Priority));
    pThread->ObjNode.Prev = (TObjNode*)0;
    pThread->ObjNode.Next = (TObjNode*)0;
    pThread->ObjNode.Handle = (TObjNode**)0;

    /* ���̼߳����ں��̶߳��У������߳�״̬ */
    pQueue = (status == eThreadReady) ? (&ThreadReadyQueue): (&ThreadAuxiliaryQueue);
    uThreadEnterQueue(pQueue, pThread, eQuePosTail);
    pThread->Status = status;

    /* ����߳��Ѿ���ɳ�ʼ�� */
    property |= THREAD_PROP_READY;
    pThread->Property = property;
}


/*************************************************************************************************
 *  ���ܣ��߳�ע��                                                                               *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵������ʼ���̺߳Ͷ�ʱ���̲߳��ܱ�ע��                                                       *
 *************************************************************************************************/
TState uThreadDelete(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_STATUS;

    if (pThread->Status == eThreadDormant)
    {
#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MUTEX_ENABLE))
        if (pThread->LockList)
        {
            error = THREAD_ERR_FAULT;
            state = eFailure;
        }
        else
#endif
        {
            uThreadLeaveQueue(pThread->Queue, pThread);
#if (TCLC_TIMER_ENABLE)
            uTimerDelete(&(pThread->Timer));
#endif
            /* �����ǰ�߳���ISR���ȱ�deactivate,Ȼ��deinit����ô���˳�isrʱ���ᷢ���߳��л���
            ������һ����Ե�ǰ�̵߳�ջ����������������ò�Ҫ�Ա�deinit���߳̽ṹ���ڴ����������
            �����ڻ�������Դ�������д�����deinit��ĵ�ǰ�̲߳�����������ջ���� */
            memset(pThread, 0, sizeof(pThread));
            error = THREAD_ERR_NONE;
            state = eSuccess;
        }
    }
    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ������߳����ȼ�                                                                         *
 *  ������(1) pThread  �߳̽ṹ��ַ                                                              *
 *        (2) priority �߳����ȼ�                                                                *
 *        (3) flag     �Ƿ�SetPriority API����                                                 *
 *        (4) pError   ����������                                                              *
 *  ���أ�(1) eFailure �����߳����ȼ�ʧ��                                                        *
 *        (2) eSuccess �����߳����ȼ��ɹ�                                                        *
 *  ˵�����������ʱ�޸����ȼ������޸��߳̽ṹ�Ļ������ȼ�                                     *
 *************************************************************************************************/
TState uThreadSetPriority(TThread* pThread, TPriority priority, TBool flag, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_PRIORITY;
    TPriority temp;

    /* ������ֻ�ᱻ�̵߳��� */
    KNL_ASSERT((uKernelVariable.State == eThreadState), "");

    if (pThread->Priority != priority)
    {
        if (pThread->Status == eThreadBlocked)
        {
            uIpcSetPriority(&(pThread->IpcContext), priority);
            state = eSuccess;
            error = THREAD_ERR_NONE;
        }
        /* �����̵߳������ȼ�ʱ������ֱ�ӵ������ھ����̶߳����еķֶ���
        ���ڴ��ھ����̶߳����еĵ�ǰ�̣߳�����޸��������ȼ���
        ��Ϊ��������Ƴ��߳̾������У����Լ�ʹ�ں˲��������Ҳû���� */
        else if (pThread->Status == eThreadReady)
        {
            uThreadLeaveQueue(&ThreadReadyQueue, pThread);
            pThread->Priority = priority;
            uThreadEnterQueue(&ThreadReadyQueue, pThread, eQuePosTail);

            if ((flag == eTrue) && (uKernelVariable.Schedulable == eTrue))
            {
                /* �õ���ǰ�������е���߾������ȼ�����Ϊ�����߳�(������ǰ�߳�)
                ���߳̾��������ڵ����ڻᵼ�µ�ǰ�߳̿��ܲ���������ȼ��� */
                if (priority < uKernelVariable.CurrentThread->Priority)
                {
                    uThreadSchedule();
                }
            }

            state = eSuccess;
            error = THREAD_ERR_NONE;
        }
        else if (pThread->Status == eThreadRunning)
        {
            /* ���赱ǰ�߳����ȼ������Ψһ����������������ȼ�֮����Ȼ����ߣ�
            �������µ����ȼ����ж�������̣߳���ô��ðѵ�ǰ�̷߳����µľ�������
            ��ͷ������������������ʽ��ʱ��Ƭ��ת����ǰ�߳��Ⱥ󱻶�ε������ȼ�ʱ��ֻ��
            ÿ�ζ��������ڶ���ͷ���ܱ�֤�����һ�ε������ȼ��󻹴��ڶ���ͷ�� */
            uThreadLeaveQueue(&ThreadReadyQueue, pThread);
            pThread->Priority = priority;
            uThreadEnterQueue(&ThreadReadyQueue, pThread, eQuePosHead);

            if ((flag == eTrue) && (uKernelVariable.Schedulable == eTrue))
            {
                /* ��Ϊ��ǰ�߳����߳̾��������ڵ����ڻᵼ�µ�ǰ�߳̿��ܲ���������ȼ���
                ������Ҫ���¼��㵱ǰ�������е���߾������ȼ���*/
                uThreadCalcHiRP(&temp);
                if (temp < uKernelVariable.CurrentThread->Priority)
                {
                    pThread->Status = eThreadReady;
                    uThreadSchedule();
                }
            }

            state = eSuccess;
            error = THREAD_ERR_NONE;
        }
        else
        {
            /*����״̬���̶߳��ڸ������������ֱ���޸����ȼ�*/
            pThread->Priority = priority;
            state = eSuccess;
            error = THREAD_ERR_NONE;
        }

        /* �����Ҫ���޸��̶̹߳����ȼ� */
        if (flag == eTrue)
        {
            pThread->BasePriority = priority;
        }
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ����̴߳�ָ����״̬ת��������̬��ʹ���߳��ܹ������ں˵���                               *
 *  ������(1) pThread   �߳̽ṹ��ַ                                                             *
 *        (2) status    �̵߳�ǰ״̬�����ڼ��                                                   *
 *        (3) pError    ����������                                                             *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵������������ǰ׺'u'(communal)��ʾ��������ȫ�ֺ���                                          *
 *************************************************************************************************/
/* �жϻ����̶߳��п��ܵ��ñ����� */
TState uThreadSetReady(TThread* pThread, TThreadStatus status, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_STATUS;
    TBool HiRP = eFalse;

    /* �߳�״̬У��,ֻ��״̬���ϵ��̲߳��ܱ����� */
    if (pThread->Status == status)
    {
        /* �����̣߳�����̶߳��к�״̬ת��,ע��ֻ���жϴ���ʱ��
        ��ǰ�̲߳Żᴦ���ں��̸߳���������(��Ϊ��û���ü��߳��л�) */
        uThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
        uThreadEnterQueue(&ThreadReadyQueue, pThread, eQuePosTail);

        /* ���߳��뿪��������ʱ���Ѿ��������ı���ִ�У�����ʱ��Ƭ��δ�ľ���
        ���߳��ٴν����������ʱ����Ҫ�ָ��̵߳�ʱ�ӽ����������¼������������ʱ������� */
        pThread->Ticks = pThread->BaseTicks;
        pThread->Status = eThreadReady;
        state = eSuccess;
        error = THREAD_ERR_NONE;

        /* ��������̻߳����£���ô��ʱpThreadһ�����ǵ�ǰ�̣߳�
        ��������жϻ����£���ô
        ��һ����ǰ�̲߳�һ���ھ��������������Ϊ������ȵıȽϱ�׼
        �ڶ����ж������һ������ʱͳһ���̵߳��ȼ�飬����õ���HiPR������ */
        if (pThread->Priority < uKernelVariable.CurrentThread->Priority)
        {
            HiRP = eTrue;
        }

        /* ���Է����߳���ռ */
        uThreadPreempt(HiRP);

#if (TCLC_TIMER_ENABLE)
        /* �����ȡ����ʱ��������Ҫֹͣ�̶߳�ʱ�� */
        if ((state == eSuccess) && (status == eThreadDelayed))
        {
            uTimerStop(&(pThread->Timer));
        }
#endif
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��̹߳�����                                                                           *
 *  ������(1) pThread   �߳̽ṹ��ַ                                                             *
 *        (2) status    �̵߳�ǰ״̬�����ڼ��                                                   *
 *        (3) ticks     �߳���ʱʱ��                                                             *
 *        (4) pError    ����������                                                             *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState uThreadSetUnready(TThread* pThread, TThreadStatus status, TTimeTick ticks, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_STATUS;

    /* ISR������ñ����� */
    KNL_ASSERT((uKernelVariable.State != eIntrState), "");

    /* ����������ǵ�ǰ�̣߳�����Ҫ���ȼ���ں��Ƿ�������� */
    if (pThread->Status == eThreadRunning)
    {
        /* ����ں˴�ʱ��ֹ�̵߳��ȣ���ô��ǰ�̲߳��ܱ����� */
        if (uKernelVariable.Schedulable == eTrue)
        {
            uThreadLeaveQueue(&ThreadReadyQueue, pThread);
            uThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, eQuePosTail);
            pThread->Status = status;

            /* �̻߳������Ͽ�ʼ�̵߳��� */
            uThreadSchedule();

            error = THREAD_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = THREAD_ERR_FAULT;
        }
    }
    else if (pThread->Status == eThreadReady)
    {
        /* ������������̲߳��ǵ�ǰ�̣߳��򲻻������̵߳��ȣ�����ֱ�Ӵ����̺߳Ͷ��� */
        uThreadLeaveQueue(&ThreadReadyQueue, pThread);
        uThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, eQuePosTail);
        pThread->Status = status;

        error = THREAD_ERR_NONE;
        state = eSuccess;
    }
    else
    {
        error = error;
    }

#if (TCLC_TIMER_ENABLE)
    if ((state == eSuccess) && (status == eThreadDelayed))
    {
        /* ���ò������̶߳�ʱ�� */
        uTimerConfig(&(pThread->Timer), eThreadTimer, ticks);
        uTimerStart(&(pThread->Timer), 0U);
    }
#endif

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳̽ṹ��ʼ������                                                                     *
 *  ������(1)  pThread  �߳̽ṹ��ַ                                                             *
 *        (2)  status   �̵߳ĳ�ʼ״̬                                                           *
 *        (3)  property �߳�����                                                                 *
 *        (4)  acapi    ���̹߳���API����ɿ���                                                  *
 *        (5)  pEntry   �̺߳�����ַ                                                             *
 *        (6)  pArg     �̺߳�������                                                             *
 *        (7)  pStack   �߳�ջ��ַ                                                               *
 *        (8)  bytes    �߳�ջ��С������Ϊ��λ                                                   *
 *        (9)  priority �߳����ȼ�                                                               *
 *        (10) ticks    �߳�ʱ��Ƭ����                                                           *
 *        (11) pError   ��ϸ���ý��                                                             *
 *  ���أ�(1)  eFailure                                                                          *
 *        (2)  eSuccess                                                                          *
 *  ˵������������ǰ׺'x'(eXtreme)��ʾ��������Ҫ�����ٽ�������                                   *
 *************************************************************************************************/
TState xThreadCreate(TThread* pThread, TThreadStatus status, TProperty property, TBitMask acapi,
                     TThreadEntry pEntry, TArgument argument, void* pStack, TBase32 bytes,
                     TPriority priority, TTimeTick ticks, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* ֻ�����ڷ��жϴ�����ñ����� */
    if (uKernelVariable.State != eIntrState)
    {
        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (!(pThread->Property &THREAD_PROP_READY))
        {
            uThreadCreate(pThread, status, property, acapi, pEntry, argument, pStack, bytes,
                          priority, ticks);
            error = THREAD_ERR_NONE;
            state = eSuccess;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳�ע��                                                                               *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����IDLE�̡߳��жϴ����̺߳Ͷ�ʱ���̲߳��ܱ�ע��                                           *
 *************************************************************************************************/
TState xThreadDelete(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* ֻ�����ڷ��жϴ�����ñ����� */
    if (uKernelVariable.State != eIntrState)
    {
        /* ���û�и������������̵߳�ַ����ǿ��ʹ�õ�ǰ�߳� */
        if (pThread == (TThread*)0)
        {
            pThread = uKernelVariable.CurrentThread;
        }

        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI &THREAD_ACAPI_DEINIT)
            {
                state = uThreadDelete(pThread, &error);
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ������߳����ȼ�                                                                         *
 *  ������(1) pThread  �߳̽ṹ��ַ                                                              *
 *        (2) priority �߳����ȼ�                                                                *
 *        (3) pError   ��ϸ���ý��                                                              *
 *  ���أ�(1) eFailure �����߳����ȼ�ʧ��                                                        *
 *        (2) eSuccess �����߳����ȼ��ɹ�                                                        *
 *  ˵����(1) �������ʱ�޸����ȼ������޸��߳̽ṹ�Ļ������ȼ�����                             *
 *        (2) ������ʵʩ���ȼ��̳�Э���ʱ����AUTHORITY����                                    *
 *************************************************************************************************/
TState xThreadSetPriority(TThread* pThread, TPriority priority, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* ֻ�����ڷ��жϴ�����ñ����� */
    if (uKernelVariable.State != eIntrState)
    {
        /* ���û�и������������̵߳�ַ����ǿ��ʹ�õ�ǰ�߳� */
        if (pThread == (TThread*)0)
        {
            pThread = uKernelVariable.CurrentThread;
        }

        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI &THREAD_ACAPI_SET_PRIORITY)
            {
                if ((!(pThread->Property & THREAD_PROP_PRIORITY_FIXED)) &&
                        (pThread->Property & THREAD_PROP_PRIORITY_SAFE))
                {
                    state = uThreadSetPriority(pThread, priority, eTrue, &error);
                }
                else
                {
                    error = THREAD_ERR_FAULT;
                    state = eFailure;
                }
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}



/*************************************************************************************************
 *  ���ܣ��޸��߳�ʱ��Ƭ����                                                                     *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) slice   �߳�ʱ��Ƭ����                                                             *
 *        (3) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eSuccess                                                                           *
 *        (2) eFailure                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xThreadSetTimeSlice(TThread* pThread, TTimeTick ticks, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* ֻ�����ڷ��жϴ�����ñ����� */
    if (uKernelVariable.State != eIntrState)
    {
        /* ���û�и������������̵߳�ַ����ǿ��ʹ�õ�ǰ�߳� */
        if (pThread == (TThread*)0)
        {
            pThread = uKernelVariable.CurrentThread;
        }

        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI &THREAD_ACAPI_SET_SLICE)
            {
                /* �����߳�ʱ��Ƭ���� */
                if (pThread->BaseTicks > ticks)
                {
                    pThread->Ticks = (pThread->Ticks < ticks) ? (pThread->Ticks): ticks;
                }
                else
                {
                    pThread->Ticks += (ticks - pThread->BaseTicks);
                }
                pThread->BaseTicks = ticks;

                error = THREAD_ERR_NONE;
                state = eSuccess;
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}



/*************************************************************************************************
 *  ���ܣ��̼߳��̵߳��Ⱥ�������ǰ�߳������ó�������(���־���״̬)                               *
 *  ������(1) pError    ��ϸ���ý��                                                             *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵������Ϊ�����ƻ���߾������ȼ�ռ�ô�������ԭ��                                           *
 *        ����Yield����ֻ����ӵ����߾������ȼ����߳�֮�����                                    *
 *************************************************************************************************/
TState xThreadYield(TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TReg32 imask;
    TPriority priority;
    TThread* pThread;

    CpuEnterCritical(&imask);

    /* ֻ�����ڷ��жϴ�����ñ����� */
    /* ֻ�����̻߳�����ͬʱ�ں������̵߳��ȵ������²��ܵ��ñ����� */
    if ((uKernelVariable.State == eThreadState) && (uKernelVariable.Schedulable == eTrue))
    {
        /* ����Ŀ���ǵ�ǰ�߳� */
        pThread = uKernelVariable.CurrentThread;
        priority = pThread->Priority;

        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI &THREAD_ACAPI_YIELD)
            {
                /* ������ǰ�߳����ڶ��е�ͷָ��
                ��ǰ�߳������̶߳���Ҳ����ֻ�е�ǰ�߳�Ψһ1���߳� */
                ThreadReadyQueue.Handle[priority] = (ThreadReadyQueue.Handle[priority])->Next;
                pThread->Status = eThreadReady;

                uThreadSchedule();
                error = THREAD_ERR_NONE;
                state = eSuccess;
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ������̣߳�ʹ���߳��ܹ������ں˵���                                                     *
 *  ������(1) pThread  �߳̽ṹ��ַ                                                              *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xThreadActivate(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* ֻ�����ڷ��жϴ�����ñ����� */
    if (uKernelVariable.State != eIntrState)
    {
        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI &THREAD_ACAPI_ACTIVATE)
            {
                state = uThreadSetReady(pThread, eThreadDormant, &error);
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳���ֹ��ʹ���̲߳��ٲ����ں˵���                                                     *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����(1) ��ʼ���̺߳Ͷ�ʱ���̲߳��ܱ�����                                                   *
 *************************************************************************************************/
TState xThreadDeactivate(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* ֻ�����ڷ��жϴ�����ñ����� */
    if (uKernelVariable.State != eIntrState)
    {
        /* ���û�и������������̵߳�ַ����ǿ��ʹ�õ�ǰ�߳� */
        if (pThread == (TThread*)0)
        {
            pThread = uKernelVariable.CurrentThread;
        }

        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI &THREAD_ACAPI_DEACTIVATE)
            {
                state = uThreadSetUnready(pThread, eThreadDormant, 0U, &error);
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��̹߳�����                                                                           *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����(1) �ں˳�ʼ���̲߳��ܱ�����                                                           *
 *************************************************************************************************/
TState xThreadSuspend(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* ֻ�����ڷ��жϴ�����ñ����� */
    if (uKernelVariable.State != eIntrState)
    {
        /* ���û�и������������̵߳�ַ����ǿ��ʹ�õ�ǰ�߳� */
        if (pThread == (TThread*)0)
        {
            pThread = uKernelVariable.CurrentThread;
        }

        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI &THREAD_ACAPI_SUSPEND)
            {
                state = uThreadSetUnready(pThread, eThreadSuspended, 0U, &error);
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳̽�Һ���                                                                           *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xThreadResume(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* ֻ�����ڷ��жϴ�����ñ����� */
    if (uKernelVariable.State != eIntrState)
    {
        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI &THREAD_ACAPI_RESUME)
            {
                state = uThreadSetReady(pThread, eThreadSuspended, &error);
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}

/*************************************************************************************************
 *  ���ܣ��߳̽�Һ���                                                                           *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xThreadUnblock(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_UNREADY;
    TBool HiRP = eFalse;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* ����߳��Ƿ��Ѿ�����ʼ�� */
    if (pThread->Property &THREAD_PROP_READY)
    {
        /* ����߳��Ƿ�������API���� */
        if (pThread->ACAPI &THREAD_ACAPI_UNBLOCK)
        {
            if (pThread->Status == eThreadBlocked)
            {
                /* �����������ϵ�ָ�������߳��ͷ� */
                uIpcUnblockThread(&(pThread->IpcContext), eFailure, IPC_ERR_ABORT, &HiRP);

                /* ���Է����߳���ռ */
                uThreadPreempt(HiRP);

                error = THREAD_ERR_NONE;
                state = eSuccess;
            }
            else
            {
                error = THREAD_ERR_STATUS;
            }
        }
        else
        {
            error = THREAD_ERR_ACAPI;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}

#if (TCLC_TIMER_ENABLE)
/*************************************************************************************************
 *  ���ܣ��߳���ʱģ��ӿں���                                                                   *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) ticks   ��Ҫ��ʱ�ĵδ���Ŀ                                                         *
 *        (3) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eSuccess                                                                           *
 *        (2) eFailure                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xThreadDelay(TThread* pThread, TTimeTick ticks, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* ֻ�����ڷ��жϴ�����ñ����� */
    if (uKernelVariable.State != eIntrState)
    {
        /* ���û�и������������̵߳�ַ����ǿ��ʹ�õ�ǰ�߳� */
        if (pThread == (TThread*)0)
        {
            pThread = uKernelVariable.CurrentThread;
        }

        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI &THREAD_ACAPI_DELAY)
            {
                state = uThreadSetUnready(pThread, eThreadDelayed, ticks, &error);
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳���ʱȡ������                                                                       *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eSuccess                                                                           *
 *        (2) eFailure                                                                           *
 *  ˵����(1) �����������ʱ�޵ȴ���ʽ������IPC�߳����������ϵ��߳���Ч                          *
 *************************************************************************************************/
TState xThreadUndelay(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* ֻ�����ڷ��жϴ�����ñ����� */
    if (uKernelVariable.State != eIntrState)
    {
        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI &THREAD_ACAPI_UNDELAY)
            {
                state = uThreadSetReady(pThread, eThreadDelayed, &error);
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}

#endif
