/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.debug.h"
#include "tcl.kernel.h"
#include "tcl.timer.h"
#include "tcl.thread.h"
#include "tcl.ipc.h"

#if (TCLC_IPC_ENABLE)

/*************************************************************************************************
 *  ���ܣ����̼߳��뵽ָ����IPC�߳�����������                                                    *
 *  ������(1) pQueue   IPC���е�ַ                                                               *
 *        (2) pThread  �߳̽ṹ��ַ                                                              *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static void EnterBlockedQueue(TIpcQueue* pQueue, TIpcContext* pContext)
{
    TProperty property;

    property = *(pQueue->Property);
    if ((pContext->Option) & IPC_OPT_USE_AUXIQ)
    {
        if (property &IPC_PROP_PREEMP_AUXIQ)
        {
            uObjQueueAddPriorityNode(&(pQueue->AuxiliaryHandle), &(pContext->ObjNode));
        }
        else
        {
            uObjQueueAddFifoNode(&(pQueue->AuxiliaryHandle), &(pContext->ObjNode), eQuePosTail);
        }
        property |= IPC_PROP_AUXIQ_AVAIL;
    }
    else
    {
        if (property &IPC_PROP_PREEMP_PRIMIQ)
        {
            uObjQueueAddPriorityNode(&(pQueue->PrimaryHandle), &(pContext->ObjNode));
        }
        else
        {
            uObjQueueAddFifoNode(&(pQueue->PrimaryHandle), &(pContext->ObjNode), eQuePosTail);
        }
        property |= IPC_PROP_PRIMQ_AVAIL;
    }

    *(pQueue->Property) = property;

    /* �����߳��������� */
    pContext->Queue = pQueue;
}


/*************************************************************************************************
 *  ���ܣ����̴߳�ָ�����̶߳������Ƴ�                                                           *
 *  ������(1) pQueue   IPC���е�ַ                                                               *
 *        (2) pThread  �߳̽ṹ��ַ                                                              *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static void LeaveBlockedQueue(TIpcQueue* pQueue, TIpcContext* pContext)
{
    TProperty property;

    property = *(pQueue->Property);

    /* ���̴߳�ָ���ķֶ�����ȡ�� */
    if ((pContext->Option) & IPC_OPT_USE_AUXIQ)
    {
        uObjQueueRemoveNode(&(pQueue->AuxiliaryHandle), &(pContext->ObjNode));
        if (pQueue->AuxiliaryHandle == (TObjNode*)0)
        {
            property &= ~IPC_PROP_AUXIQ_AVAIL;
        }
    }
    else
    {
        uObjQueueRemoveNode(&(pQueue->PrimaryHandle), &(pContext->ObjNode));
        if (pQueue->PrimaryHandle == (TObjNode*)0)
        {
            property &= ~IPC_PROP_PRIMQ_AVAIL;
        }
    }

    *(pQueue->Property) = property;

    /* �����߳��������� */
    pContext->Queue = (TIpcQueue*)0;
}


/*************************************************************************************************
 *  ���ܣ����̷߳�����Դ��������                                                                 *
 *  ������(1) pQueue  �̶߳��нṹ��ַ                                                           *
 *        (2) pThread �߳̽ṹ��ַ                                                               *
 *        (3) ticks   ��Դ�ȴ�ʱ��                                                               *
 *  ���أ���                                                                                     *
 *  ˵���������߳̽�����ض��еĲ��Ը��ݶ��в�������������                                       *
 *************************************************************************************************/
void uIpcBlockThread(TIpcContext* pContext, TIpcQueue* pQueue, TTimeTick ticks)
{
    TThread* pThread;

    KNL_ASSERT((uKernelVariable.State != eIntrState), "");

    /* ���̷߳����ں��̸߳������� */
    pThread = (TThread*)(pContext->Owner);

    /* ֻ�д��ھ���״̬���̲߳ſ��Ա����� */
    if (pThread->Status != eThreadRunning)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    uThreadLeaveQueue(uKernelVariable.ThreadReadyQueue, pThread);
    uThreadEnterQueue(uKernelVariable.ThreadAuxiliaryQueue, pThread, eQuePosTail);
    pThread->Status = eThreadBlocked;

    /* ���̷߳����������� */
    EnterBlockedQueue(pQueue, pContext);

    /* �����Ҫ�ͳ�ʼ�����Ҵ��߳����ڷ�����Դ��ʱ�޶�ʱ�� */
#if (TCLC_TIMER_ENABLE && TCLC_IPC_TIMER_ENABLE)
    if ((pContext->Option & IPC_OPT_TIMED) && (ticks > 0U))
    {
        /* �������ò������̶߳�ʱ�� */
        uTimerConfig(&(pThread->Timer), eIpcTimer, ticks);
        uTimerStart(&(pThread->Timer), 0U);
    }
#else
    ticks = ticks;
#endif
}


/*************************************************************************************************
 *  ���ܣ�����IPC����������ָ�����߳�                                                            *
 *  ������(1) pThread �̵߳�ַ                                                                   *
 *        (2) state   �߳���Դ���ʷ��ؽ��                                                       *
 *        (3) error   ��ϸ���ý��                                                               *
 *        (4) pHiRP   �Ƿ����Ѹ������ȼ���������Ҫ�����̵߳��ȵı��                           *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void uIpcUnblockThread(TIpcContext* pContext, TState state, TError error, TBool* pHiRP)
{
    TThread* pThread;

    /* ���̴߳�IPC��Դ�������������Ƴ������뵽�ں��߳̾�������,
    �����ǰ�̸߳ոձ����������������У�����δ�����߳��л���
    ���ڴ�ʱ��ISR��ϲ���ISR�ֽ���ǰ�̻߳��ѣ���ǰ�߳�Ҳ���ط��ؾ�������ͷ */
    pThread = (TThread*)(pContext->Owner);

    /* ֻ�д�������״̬���̲߳ſ��Ա�������� */
    if (pThread->Status != eThreadBlocked)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /* �����̣߳�����̶߳��к�״̬ת��,ע��ֻ���жϴ���ʱ��
    ��ǰ�̲߳Żᴦ��ThreadAuxiliaryQueue������(��Ϊ��û���ü��߳��л�) */
    if (pThread == uKernelVariable.CurrentThread)
    {
        if (uKernelVariable.State != eIntrState)
        {
            uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
        }
    }

    uThreadLeaveQueue(uKernelVariable.ThreadAuxiliaryQueue, pThread);
    uThreadEnterQueue(uKernelVariable.ThreadReadyQueue, pThread, eQuePosTail);

    /* ���߳��뿪��������ʱ���Ѿ��������ı���ִ�У�����ʱ��Ƭ��δ�ľ���
       ���߳��ٴν����������ʱ����Ҫ�ָ��̵߳�ʱ�ӽ����������¼������������ʱ������� */
    pThread->Ticks  = pThread->BaseTicks;
    pThread->Status = eThreadReady;

    /* ���̴߳����������Ƴ� */
    LeaveBlockedQueue(pContext->Queue, pContext);

    /* �����̷߳�����Դ�Ľ���ʹ������ */
    *(pContext->State) = state;
    *(pContext->Error) = error;

    /* ����߳�����ʱ�޷�ʽ������Դ��ȡ�����̵߳�ʱ�޶�ʱ�� */
#if ((TCLC_IPC_TIMER_ENABLE) && (TCLC_TIMER_ENABLE))
    if ((pContext->Option & IPC_OPT_TIMED) && (error != IPC_ERR_TIMEO))
    {
        KNL_ASSERT((pThread->Timer.Type == eIpcTimer), "");
        uTimerStop(&(pThread->Timer));
    }
#endif

    /* �����̵߳���������,�˱��ֻ���̻߳�������Ч��
       ��ISR���ǰ�߳̿������κζ����
       ����ǰ�߳���Ƚ����ȼ�Ҳ��������� */
    if (pThread->Priority < uKernelVariable.CurrentThread->Priority)
    {
        *pHiRP = eTrue;
    }
}


/*************************************************************************************************
 *  ���ܣ�ѡ�������������е�ȫ���߳�                                                           *
 *  ������(1) pQueue  �̶߳��нṹ��ַ                                                           *
 *        (2) state   �߳���Դ���ʷ��ؽ��                                                       *
 *        (3) error   ��ϸ���ý��                                                               *
 *        (4) pData   �̷߳���IPC�õ�������                                                      *
 *        (5) pHiRP  �߳��Ƿ���Ҫ���ȵı��                                                      *
 *  ���أ�                                                                                       *
 *  ˵����ֻ���������Ϣ���й㲥ʱ�Żᴫ��pData2����                                             *
 *************************************************************************************************/
void uIpcUnblockAll(TIpcQueue* pQueue, TState state, TError error, void** pData2, TBool* pHiRP)
{
    TIpcContext* pContext;

    /* ���������е��߳����ȱ�������� */
    while (pQueue->AuxiliaryHandle != (TObjNode*)0)
    {
        pContext = (TIpcContext*)(pQueue->AuxiliaryHandle->Owner);
        uIpcUnblockThread(pContext, state, error, pHiRP);

        if ((pData2 != (void**)0) && (pContext->Data.Addr2 != (void**)0))
        {
            *(pContext->Data.Addr2) = *pData2;
        }
    }

    /* ���������е��߳���󱻽������ */
    while (pQueue->PrimaryHandle != (TObjNode*)0)
    {
        pContext = (TIpcContext*)(pQueue->PrimaryHandle->Owner);
        uIpcUnblockThread(pContext, state, error, pHiRP);

        if ((pData2 != (void**)0) && (pContext->Data.Addr2 != (void**)0))
        {
            *(pContext->Data.Addr2) = *pData2;
        }
    }
}


/*************************************************************************************************
 *  ���ܣ��ı䴦��IPC���������е��̵߳����ȼ�                                                    *
 *  ������(1) pThread  �߳̽ṹ��ַ                                                              *
 *        (2) priority ��Դ�ȴ�ʱ��                                                              *
 *  ���أ���                                                                                     *
 *  ˵��������߳������������в������ȼ����ԣ����̴߳������������������Ƴ���Ȼ���޸��������ȼ�,*
 *        ����ٷŻ�ԭ���С�����������ȳ������򲻱ش���                                       *
 *************************************************************************************************/
void uIpcSetPriority(TIpcContext* pContext, TPriority priority)
{
    TProperty property;
    TIpcQueue* pQueue;
    TThread* pThread;

    pQueue = pContext->Queue;
    pThread = (TThread*)(pContext->Owner);

    /* ��Ϊ�߳�����Ϊ����״̬�����Կ���ֱ�����ں��̸߳����������޸��̵߳����ȼ� */
    pThread->Priority = priority;

    /* ����ʵ����������°����߳���IPC�����������λ�� */
    property = *(pContext->Queue->Property);
    if (pContext->Option & IPC_OPT_USE_AUXIQ)
    {
        if (property & IPC_PROP_PREEMP_AUXIQ)
        {
            uObjQueueRemoveNode(&(pQueue->AuxiliaryHandle), &(pContext->ObjNode));
            uObjQueueAddPriorityNode(&(pQueue->AuxiliaryHandle), &(pContext->ObjNode));
        }
    }
    else
    {
        if (property & IPC_PROP_PREEMP_PRIMIQ)
        {
            uObjQueueRemoveNode(&(pQueue->PrimaryHandle), &(pContext->ObjNode));
            uObjQueueAddPriorityNode(&(pQueue->PrimaryHandle), &(pContext->ObjNode));
        }
    }
}


/*************************************************************************************************
 *  ���ܣ��趨�����̵߳�IPC�������Ϣ                                                            *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) pIpc    ���ڲ�����IPC����ĵ�ַ                                                    *
 *        (3) data    ָ������Ŀ�����ָ���ָ��                                                 *
 *        (4) len     ���ݵĳ���                                                                 *
 *        (5) option  ����IPC����ʱ�ĸ��ֲ���                                                    *
 *        (6) state   IPC������ʽ��                                                            *
 *        (7) pError  ��ϸ���ý��                                                               *
 *  ���أ���                                                                                     *
 *  ˵����dataָ���ָ�룬������Ҫͨ��IPC���������ݵ��������߳̿ռ��ָ��                        *
 *************************************************************************************************/
void uIpcSaveContext(TIpcContext* pContext, void* pIpc, TBase32 data, TBase32 len,
                     TOption option, TState* pState, TError* pError)
{
    pContext->Object     = pIpc;
    pContext->Queue      = (TIpcQueue*)0;
    pContext->Data.Value = data;
    pContext->Length     = len;
    pContext->Option     = option;
    pContext->State      = pState;
    pContext->Error      = pError;
    *pState              = eError;
    *pError              = IPC_ERR_FAULT;
}


/*************************************************************************************************
 *  ���ܣ���������̵߳�IPC�������Ϣ                                                            *
 *  ������(1) pThread  �߳̽ṹ��ַ                                                              *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void uIpcCleanContext(TIpcContext* pContext)
{
    pContext->Object     = (void*)0;
    pContext->Queue      = (TIpcQueue*)0;
    pContext->Data.Value = 0U;
    pContext->Length     = 0U;
    pContext->Option     = IPC_OPTION;
    pContext->State      = (TState*)0;
    pContext->Error      = (TError*)0;
}


/*************************************************************************************************
 *  ���ܣ���ʼ��IPC����                                                                          *
 *  ������(1) pQueue   IPC���нṹ��ַ                                                           *
 *        (2) property IPC���е���Ϊ����                                                         *
 *  ���أ���                                                                                     *
 *  ˵����pContext->Owner ��Ա�ڴ˳�ʼ��֮�����Զ��Ҫ�����                                     *
 *************************************************************************************************/
void uIpcInitContext(TIpcContext* pContext, void* pOwner)
{
    TThread* pThread;

    pThread = (TThread*)pOwner;
    pContext->Owner      = pOwner;
    pContext->Object     = (void*)0;
    pContext->Queue      = (TIpcQueue*)0;
    pContext->Data.Value = 0U;
    pContext->Length     = 0U;
    pContext->Option     = IPC_OPTION;
    pContext->State      = (TState*)0;
    pContext->Error      = (TError*)0;

    pContext->ObjNode.Next   = (TObjNode*)0;
    pContext->ObjNode.Prev   = (TObjNode*)0;
    pContext->ObjNode.Handle = (TObjNode**)0;
    pContext->ObjNode.Data   = (TBase32*)(&(pThread->Priority));
    pContext->ObjNode.Owner  = (void*)pContext;
}

#endif

