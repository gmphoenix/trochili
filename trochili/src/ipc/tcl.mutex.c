/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include <string.h>

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.debug.h"
#include "tcl.kernel.h"
#include "tcl.ipc.h"
#include "tcl.mutex.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MUTEX_ENABLE))

static void AddLock(TThread* pThread, TMutex* pMutex, TBool* pHiRP);
static void RemoveLock(TThread* pThread, TMutex* pMutex, TBool* pHiRP);
static TState FreeMutex(TMutex* pMutex, TError* pError);
static TState LockMutex(TMutex* pMutex, TOption option, TTimeTick timeo,
                        TReg32* pIMask, TError* pError);

/*************************************************************************************************
 *  ����: ����ʹ���̻߳�û��⻥����                                                             *
 *  ����: (1) pThread  �߳̽ṹ��ַ                                                              *
 *        (2) pMutex   �������ṹ��ַ                                                            *
 *        (3) pHiRP    �Ƿ��и������ȼ�����                                                      *
 *  ����: (1) ��                                                                                 *
 *  ˵����������һ���ǵ�ǰ�̵߳��ã����ߵ�ǰ�̻߳�û����������߰ѻ�������������߳�             *
 *        ������ĳ�������ȼ�������ߣ����Ը���ǰ�߳�ֱ�ӱȽ����ȼ�                               *
 *************************************************************************************************/
/* 1 ���̻߳����£��������ض�����ǰ�̵߳���
     1.1 ��ǰ�߳̿��ܻ���ñ�����(lock)��ռ�õĻ�������
     1.2 ��ǰ�߳̿��ܻ���ñ�����(free)�����������������߳�(���������ľ���״̬)
   2 ��isr�����²����ܵ��ñ����� */
static void AddLock(TThread* pThread, TMutex* pMutex, TBool* pHiRP)
{
    TState state;
    TError error;

    /* �������������߳������У������ȼ����� */
    uObjListAddPriorityNode(&(pThread->LockList), &(pMutex->LockNode));
    pMutex->Nest = 1U;
    pMutex->Owner = pThread;

    /* ����߳����ȼ�û�б��̶� */
    if (!(pThread->Property & THREAD_PROP_PRIORITY_FIXED))
    {
        /* �߳����ȼ���mutex��������API�����޸� */
        pThread->Property &= ~(THREAD_PROP_PRIORITY_SAFE);

        /* PCP �õ�������֮�󣬵�ǰ�߳�ʵʩ�컨���㷨,��Ϊ���߳̿��ܻ�ö����������
        ���̵߳ĵ�ǰ���ȼ����ܱ��»�õĻ��������컨�廹�ߡ� �����������Ƚ�һ�����ȼ���
        ����ֱ�����ó��»��������컨�����ȼ� */
        if (pThread->Priority > pMutex->Priority)
        {
            state = uThreadSetPriority(pThread, pMutex->Priority, eFalse, &error);
            state = state;

            /* �ڻ������ͷŵ������У���ǰ�߳��ͷŻ�����֮�󣬿��ܵ�������������ȼ���
            ��ʱ��û��������߳����ȼ�Ҳ���ܱ���ߡ�
            �����û��������߳����ȼ��ȵ�ǰ�̵߳����ȼ���Ҫ��(����͸߻�����Ϊ��û��������õ����)��
            ��ô����Ҫ�����̵߳��ȡ�ע�� AddLock()���ᵼ��pThread�����ȼ��½� */

            /* ����ͨ���ж�pThread��CurrentThread�����ȼ���ϵ�������Ƿ���ȵ�ԭ����:
               1 ���pThread�ǵ�ǰ�̣߳�ռ�û����������ȼ�δ�صõ�������
                 ����һ��������߾������ȼ���
               2 ���pThread���ǵ�ǰ�̣߳���pThread(�ȱ��������)ռ�û�������:
                 2.1 pThread�����ȼ���һ������
                 2.2 ���ǵ�ǰ�̵߳����ȼ�Ҳ��һ��������� */

            /* �϶������̻߳����µ���:
               1 ����ǰ�߳�add������ʱ������Ҫ�������ȼ��仯������̵߳������⣻
               2 ���������ǵ�ǰ�߳�add������ʱ����Ҫ�������ȼ��仯��
                 ��ǰ�̸߳��õ����������߳���ֱ�ӱȽ����ȼ�
               3 �ж�����(1)�϶����㣬�����������������Ϊ����ʾ�߼���ϵ��  */
            if (uKernelVariable.State == eThreadState) /* (1) */
            {
                if (pThread != uKernelVariable.CurrentThread) /* (2) */
                {
                    if (pThread->Priority < uKernelVariable.CurrentThread->Priority)
                    {
                        /* ֻ�����̻߳����µ�ǰ�߳��ͷ��Լ�ռ�õ�mutex���ֽ��û�����
                           ���������߳�ʱ��pHiRP�������� */
                        *pHiRP = eTrue;
                    }
                }
            }
        }
    }
}


/*************************************************************************************************
 *  ����: ���߳���������ɾ��������                                                               *
 *  ����: (1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) pMutex  �������ṹ��ַ                                                             *
 *        (3) pHiRP   �Ƿ��и������ȼ�����                                                       *
 *  ����: ��                                                                                     *
 *  ˵������ǰ�߳����ȼ����ͣ�ֻ�ܸ������̱߳Ƚ����ȼ�                                           *
 *************************************************************************************************/
static void RemoveLock(TThread* pThread, TMutex* pMutex, TBool* pHiRP)
{
    TState    state;
    TPriority priority = TCLC_LOWEST_PRIORITY;
    TObjNode* pHead = (TObjNode*)0;
    TBool     nflag = eFalse;
    TError    error;

    /* �����������߳����������Ƴ� */
    pHead = pThread->LockList;
    uObjListRemoveNode(&(pThread->LockList), &(pMutex->LockNode));
    pMutex->Owner = (TThread*)0;
    pMutex->Nest = 0U;

    /* ����߳����ȼ�û�б��̶� */
    if (!(pThread->Property & THREAD_PROP_PRIORITY_FIXED))
    {
        /* ����߳�������Ϊ�գ����߳����ȼ��ָ����������ȼ�,
           ��mutex��߳����ȼ�һ���������̻߳������ȼ� */
        if (pThread->LockList == (TObjNode*)0)
        {
            /* ����߳�û��ռ�б�Ļ�������,�������߳����ȼ����Ա�API�޸� */
            pThread->Property |= (THREAD_PROP_PRIORITY_SAFE);

            /* ׼���ָ��߳����ȼ� */
            priority = pThread->BasePriority;
            nflag = eTrue;
        }
        else
        {
            /* ��Ϊ�������ǰ������ȼ��½����������̵߳���һ�����ȼ�һ������Ȼ��ߵ͵�,
               ע��ɾ�����������ڶ�������κ�λ�ã���������ڶ���ͷ������Ҫ�����߳����ȼ� */
            if (pHead == &(pMutex->LockNode))
            {
                /* ׼���ָ��߳����ȼ� */
                priority = *((TPriority*)(pThread->LockList->Data));
                nflag = eTrue;
            }
        }

        /* ����߳����ȼ��б仯(nflag = eTrue)������Ҫ����(priority > pThread->Priority) */
        if (nflag && (priority > pThread->Priority))
        {
            /* �޸��߳����ȼ����˴�����Ҫ�����̵߳��� */
            state = uThreadSetPriority(pThread, priority, eFalse, &error);
            state = state;

            /* 1 ���̻߳����£��������ض�����ǰ�̵߳���
                 1.1 ��ǰ�߳̿��ܻ���ñ�����(free\deinit\reset)���ͷ��Լ�ռ�õĻ�������
                 1.2 ��ǰ�߳̿��ܻ���ñ�����(deinit\reset)���ͷ������߳�(״̬����)ռ�õĻ�������
               2 ��isr�����£�����ʱ����Ҫ�����߳����ȼ���ռ */

            /* ����ͨ������Ĵ����������Ƿ���ȵ�ԭ����:
               1 pThread����ǵ�ǰ�̣߳��ͷ��Լ�ռ�õĻ�������������������ȼ���
                 ����ܲ�������߾������ȼ��Ҳ���ܻ�����߾������ȼ�
               2 pThread������ǵ�ǰ�̣߳���ǰ�̰߳���\�ͷ�pThreadռ�õĻ�������
                 pThread�����ȼ�һ�����������������Ե�ǰ�̵߳����ȼ�һ�����������
               */

            /* ���̻߳����µ���ʱ:
               1 ���Ǿ����߳�remove������ʱ������Ҫ�������ȼ��仯�Ե��ȵ�Ҫ��;
               2 �������߳�(�ǵ�ǰ)remove������ʱ������Ҫ�������ȼ��仯�Ե��ȵ�Ҫ��;
               3 ��ǰ�߳�remove������ʱ����Ҫ�������ȼ��仯����ȫ�������߳����Ƚ����ȼ� */
            if (uKernelVariable.State == eThreadState)
            {
                if (pThread == uKernelVariable.CurrentThread)
                {
                    uThreadCalcHiRP(&priority);
                    if (priority < uKernelVariable.CurrentThread->Priority)
                    {
                        /* ֻ�����̻߳����µ�ǰ�߳��ͷ��Լ�ռ�õ�mutexʱ��pHiRP�������� */
                        *pHiRP = eTrue;
                    }
                }
            }
        }
    }
}


/*************************************************************************************************
 *  ����: �ͷŻ��⻥����                                                                         *
 *  ����: (1) pMutex   �������ṹ��ַ                                                            *
 *        (2) pHiRP    �Ƿ��и������ȼ�����                                                      *
 *        (3) pErrno   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵����ֻ�е�ǰ�̲߳��ܹ��ͷ�ĳ��������������ǰ�߳�һ����������״̬��                         *
 *        Ҳ�Ͳ�������ʽ���ȼ�����������                                                         *
 *************************************************************************************************/
static TState TryFreeMutex(TMutex* pMutex, TBool* pHiRP, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_FORBIDDEN;
    TIpcContext* pContext;
    TThread* pThread;

    /* ֻ��ռ�л��������̲߳����ͷŸû����� */
    if (pMutex->Owner == uKernelVariable.CurrentThread)
    {
        /* ���߳�Ƕ��ռ�л�����������£���Ҫ����������Ƕ�״��� */
        pMutex->Nest--;

        /* ���������Ƕ����ֵΪ0��˵��Ӧ�ó����ͷŻ�����,
           �����ǰ�߳������������ȼ��컨��Э�飬���ǵ����߳����ȼ� */
        if (pMutex->Nest == 0U)
        {
            /* �����������߳����������Ƴ�,���û�����������Ϊ��. */
            RemoveLock(uKernelVariable.CurrentThread, pMutex, pHiRP);

            /* ���Դӻ���������������ѡ����ʵ��̣߳�ʹ�ø��̵߳õ������� */
            if (pMutex->Property & IPC_PROP_PRIMQ_AVAIL)
            {
                pContext = (TIpcContext*)(pMutex->Queue.PrimaryHandle->Owner);
                uIpcUnblockThread(pContext, eSuccess, IPC_ERR_NONE, pHiRP);

                pThread = (TThread*)(pContext->Owner);
                AddLock(pThread, pMutex, pHiRP);
            }
        }

        error = IPC_ERR_NONE;
        state = eSuccess;
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: �ͷŻ��⻥����                                                                         *
 *  ����: (1) pMutex   �������ṹ��ַ                                                            *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵����ֻ�е�ǰ�̲߳��ܹ��ͷ�ĳ��������                                                       *
 *************************************************************************************************/
static TState FreeMutex(TMutex* pMutex, TError* pError)
{
    TState state = eFailure;
    TBool HiRP = eFalse;

    if (uKernelVariable.State == eThreadState)
    {
        state = TryFreeMutex(pMutex, &HiRP, pError);
        if (uKernelVariable.Schedulable == eTrue)
        {
            if (state == eSuccess)
            {
                if (HiRP == eTrue)
                {
                    uThreadSchedule();
                }
            }
        }
    }

    return state;
}


/*************************************************************************************************
 *  ����: �̻߳�û��⻥����                                                                     *
 *  ����: (1) pMutex   �������ṹ��ַ                                                            *
 *        (2) pHiRP    �Ƿ��и������ȼ�����                                                      *
 *        (3) pErrno   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *        (3) eError   ��������                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static TState TryLockMutex(TMutex* pMutex, TBool* pHiRP, TError* pError)
{
    TState state = eSuccess;
    TError error = IPC_ERR_NONE;

    /* �̻߳�û���������
    Priority Ceilings Protocol
    ������ɹ�, PCP�����µ�ǰ�߳����ȼ����ή��,ֱ�ӷ���
    �����ʧ�ܲ����Ƿ�������ʽ���ʻ�������ֱ�ӷ���
    �����ʧ�ܲ�����������ʽ���ʻ����������߳������ڻ����������������У�Ȼ����ȡ�
    */
    if (pMutex->Owner == (TThread*)0)
    {
        /* ��ǰ�̻߳�û����������ȼ���ʹ�б䶯Ҳ���ɱ������, ����Ҫ�߳����ȼ���ռ��
        HiRP��ֵ��ʱ���ô� */
        AddLock(uKernelVariable.CurrentThread, pMutex, pHiRP);
    }
    else if (pMutex->Owner == uKernelVariable.CurrentThread)
    {
        pMutex->Nest++;
    }
    else
    {
        error = IPC_ERR_FORBIDDEN;
        state = eFailure;
    }

    *pError  = error;
    return state;
}


/*************************************************************************************************
 *  ����: �̻߳�û��⻥����                                                                     *
 *  ����: (1) pMutex �������ṹ��ַ                                                              *
 *        (2) option   ���������ģʽ                                                            *
 *        (3) timeo    ʱ������ģʽ�·��ʻ�������ʱ�޳���                                        *
 *        (4) pIMask   �ж����μĴ���ֵ                                                          *
 *        (5) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *        (3) eError   ��������                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static TState LockMutex(TMutex* pMutex, TOption option, TTimeTick timeo,
                        TReg32* pIMask, TError* pError)
{
    TState state = eSuccess;
    TBool HiRP = eFalse;
    TIpcContext* pContext;

    /* �̻߳�û���������
    Priority Ceilings Protocol
    ������ɹ�, PCP�����µ�ǰ�߳����ȼ����ή��,ֱ�ӷ���
    �����ʧ�ܲ����Ƿ�������ʽ���ʻ�������ֱ�ӷ���
    �����ʧ�ܲ�����������ʽ���ʻ����������߳������ڻ����������������У�Ȼ����ȡ�
    */
    if (uKernelVariable.State == eThreadState)
    {
        state = TryLockMutex(pMutex, &HiRP, pError);

        if (uKernelVariable.Schedulable == eTrue)
        {
            if (state == eFailure)
            {
                if (option & IPC_OPT_WAIT)
                {
                    /* �õ���ǰ�̵߳�IPC�����Ľṹ��ַ */
                    pContext = &(uKernelVariable.CurrentThread->IpcContext);

                    /* �趨�߳����ڵȴ�����Դ����Ϣ */
                    uIpcSaveContext(pContext, (void*)pMutex, 0U, 0U, (option | IPC_OPT_MUTEX), &state, pError);

                    /* ��ǰ�߳������ڸû����������������� */
                    uIpcBlockThread(pContext, &(pMutex->Queue), timeo);

                    /* ��ǰ�̷߳����������ȣ������̵߳���ִ�� */
                    uThreadSchedule();

                    CpuLeaveCritical(*pIMask);
                    /* ��ʱ�˴�����һ�ε��ȣ���ǰ�߳��Ѿ�������IPC������������С�
                       ��������ʼִ�б���̣߳����������ٴδ����߳�ʱ���ӱ����������С�*/
                    CpuEnterCritical(pIMask);

                    /* ����̹߳�����Ϣ */
                    uIpcCleanContext(pContext);
                }
            }
        }
    }

    return state;
}

/*************************************************************************************************
 *  ����: �ͷŻ��⻥����                                                                         *
 *  ����: (1) pMutex   �������ṹ��ַ                                                            *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵����mutex֧������Ȩ�ĸ�������߳��ͷ�mutex�Ĳ����������̷��ص�,���ͷ�mutex�������ᵼ��   *
 *        �߳�������mutex���߳�����������                                                        *
 *************************************************************************************************/
TState xMutexFree(TMutex* pMutex, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;

    CpuEnterCritical(&imask);

    if (pMutex->Property & IPC_PROP_READY)
    {
        state = FreeMutex(pMutex, &error);
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: �̻߳�û��⻥����                                                                     *
 *  ����: (1) pMutex �������ṹ��ַ                                                              *
 *        (2) option   ���������ģʽ                                                            *
 *        (3) timeo    ʱ������ģʽ�·��ʻ�������ʱ�޳���                                        *
 *        (4) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *        (3) eError   ��������                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xMutexLock(TMutex* pMutex, TOption option, TTimeTick timeo, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;

    CpuEnterCritical(&imask);

    if (pMutex->Property & IPC_PROP_READY)
    {
        /* �̲߳��÷�������ʽ��������ʽ����ʱ�޵ȴ���ʽ��û�����*/
        state = LockMutex(pMutex, option, timeo, &imask, &error);
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: �����������ʼ��                                                                       *
 *  ����: (1) pMutex   �������ṹ��ַ                                                            *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState  xMutexDelete(TMutex* pMutex, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);
    if (pMutex->Property & IPC_PROP_READY)
    {
        /* ֻ�е����������߳�ռ�е�����£����п��ܴ��ڱ��������������߳� */
        if (pMutex->Owner != (TThread*)0)
        {
            /* �����������߳����������Ƴ� */
            RemoveLock(pMutex->Owner, pMutex, &HiRP);

            /* �����������ϵ����еȴ��̶߳��ͷţ������̵߳ĵȴ��������IPC_ERR_DELETE��
               ������Щ�̵߳����ȼ�һ�������ڻ����������ߵ����ȼ� */
            uIpcUnblockAll(&(pMutex->Queue), eFailure, IPC_ERR_DELETE,
                           (void**)0, &HiRP);

            /* ��������������ȫ������ */
            memset(pMutex, 0U, sizeof(TMutex));

            /* ���Է����߳���ռ */
            uThreadPreempt(HiRP);
        }

        /* ռ�и���Դ�Ľ���Ϊ�� */
        pMutex->Owner    = (TThread*)0;
        pMutex->Nest     = 0U;
        pMutex->Property = IPC_PROPERTY;
        pMutex->Priority = TCLC_LOWEST_PRIORITY;
        pMutex->LockNode.Owner = (void*)0;
        pMutex->LockNode.Data  = (TBase32*)0;

        error = IPC_ERR_NONE;
        state = eSuccess;
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: ���û�����                                                                             *
 *  ����: (1) pMutex   �������ṹ��ַ                                                            *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xMutexReset(TMutex* pMutex, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pMutex->Property & IPC_PROP_READY)
    {
        /* ֻ�е����������߳�ռ�е�����£����п��ܴ��ڱ��������������߳� */
        if (pMutex->Owner != (TThread*)0)
        {
            /* �����������߳����������Ƴ� */
            RemoveLock(pMutex->Owner, pMutex, &HiRP);

            /* �����������ϵ����еȴ��̶߳��ͷţ������̵߳ĵȴ��������IPC_ERR_RESET��
               ������Щ�̵߳����ȼ�һ�������ڻ����������ߵ����ȼ� */
            uIpcUnblockAll(&(pMutex->Queue), eFailure, IPC_ERR_RESET,
                           (void**)0, &HiRP);

            /* �ָ����������� */
            pMutex->Property &= IPC_RESET_MUTEX_PROP;

            /* ���Է����߳���ռ */
            uThreadPreempt(HiRP);
        }

        /* ռ�и���Դ�Ľ���Ϊ�� */
        pMutex->Property &= IPC_RESET_MUTEX_PROP;
        pMutex->Owner = (TThread*)0;
        pMutex->Nest = 0U;
        /* pMutex->Priority = keep recent value; */
        pMutex->LockNode.Owner = (void*)0;
        pMutex->LockNode.Data = (TBase32*)0;

        error = IPC_ERR_NONE;
        state = eSuccess;
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ�������������ֹ����,��ָ�����̴߳ӻ��������߳�������������ֹ����������                  *
 *  ������(1) pMutex   �������ṹ��ַ                                                            *
 *        (2) option   ����ѡ��                                                                  *
 *        (3) pThread  �̵߳�ַ                                                                  *
 *        (4) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xMutexFlush(TMutex* pMutex, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pMutex->Property & IPC_PROP_READY)
    {
        /* �����������������ϵ����еȴ��̶߳��ͷţ������̵߳ĵȴ��������TCLE_IPC_FLUSH  */
        uIpcUnblockAll(&(pMutex->Queue), eFailure, IPC_ERR_FLUSH, (void**)0, &HiRP);

        /* ���Է����߳���ռ */
        uThreadPreempt(HiRP);

        state = eSuccess;
        error = IPC_ERR_NONE;
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: ��ʼ��������                                                                           *
 *  ����: (1) pMute    �������ṹ��ַ                                                            *
 *        (2) priority �����������ȼ��컨��                                                      *
 *        (3) property �������ĳ�ʼ����                                                          *
 *        (4) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xMutexCreate(TMutex* pMutex, TPriority priority, TProperty property, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    if (!(pMutex->Property & IPC_PROP_READY))
    {
        property |= IPC_PROP_READY;
        pMutex->Property = property;
        pMutex->Nest = 0U;
        pMutex->Owner = (TThread*)0;
        pMutex->Priority = priority;

        pMutex->Queue.PrimaryHandle   = (TObjNode*)0;
        pMutex->Queue.AuxiliaryHandle = (TObjNode*)0;
        pMutex->Queue.Property        = &(pMutex->Property);

        pMutex->LockNode.Owner = (void*)pMutex;
        pMutex->LockNode.Data = (TBase32*)(&(pMutex->Priority));
        pMutex->LockNode.Next = 0;
        pMutex->LockNode.Prev = 0;
        pMutex->LockNode.Handle = (TObjNode**)0;

        error = IPC_ERR_NONE;
        state = eSuccess;
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


#endif

