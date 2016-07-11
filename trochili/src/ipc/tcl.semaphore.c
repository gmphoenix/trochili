/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include <string.h>

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.thread.h"
#include "tcl.debug.h"
#include "tcl.kernel.h"
#include "tcl.ipc.h"
#include "tcl.semaphore.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_SEMAPHORE_ENABLE))

static TState TryObtainSemaphore(TSemaphore* pSemaphore, TBool* pHiRP, TError* pErrno);
static TState ObtainSemaphore(TSemaphore* pSemaphore, TOption option, TTimeTick timeo, TReg32*
                              pIMask, TError* pErrno);
static TState TryReleaseSemaphore(TSemaphore* pSemaphore, TBool* pHiRP, TError* pErrno);
static TState ReleaseSemaphore(TSemaphore* pSemaphore, TOption option, TTimeTick timeo, TReg32*
                               pIMask, TError* pErrno);

/*************************************************************************************************
 *  ����: ���Ի�ü����ź���                                                                     *
 *  ����: (1) pSemaphore �����ź����ṹ��ַ                                                      *
 *        (2) pHiRP     �Ƿ����Ѹ������ȼ���������Ҫ�����̵߳��ȵı��                        *
 *        (3) pErrno     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵�������ź������������ʱ��������ź��������������д����̣߳���ô˵���ź���������������   *
 *        Release����,��Ҫ���ź����������������ҵ�һ�����ʵ��̣߳�����ֱ��ʹ�����ͷ��ź����ɹ�,  *
 *        ͬʱ�����ź����ļ�������                                                               *
 *************************************************************************************************/
static TState TryObtainSemaphore(TSemaphore* pSemaphore, TBool* pHiRP, TError* pErrno)
{
    TState state;
    TIpcContext* pContext = (TIpcContext*)0;

    if (pSemaphore->Value == 0U)
    {
        *pErrno = IPC_ERR_INVALID_VALUE;
        state = eFailure;
    }
    else if (pSemaphore->Value == pSemaphore->LimitedValue)
    {
        /* ���Դ��ź����������������ҵ�һ�����ʵ��̲߳�����,�����ź����������� */
        /* ��������ѵ��̵߳����ȼ����ڵ�ǰ�߳����ȼ��������̵߳��������� */
        if (pSemaphore->Property & IPC_PROP_PRIMQ_AVAIL)
        {
            pContext = (TIpcContext*)(pSemaphore->Queue.PrimaryHandle->Owner);
            uIpcUnblockThread(pContext, eSuccess, IPC_ERR_NONE, pHiRP);
        }
        else
        {
            /* ���û���ҵ����ʵ��̣߳����ź���������1 */
            pSemaphore->Value--;
        }

        *pErrno = IPC_ERR_NONE;
        state = eSuccess;
    }
    else
    {
        /* �ź�������ֱ�Ӽ�1 */
        pSemaphore->Value--;

        *pErrno = IPC_ERR_NONE;
        state = eSuccess;
    }

    return state;
}


/*************************************************************************************************
 *  ����: ��ü����ź���                                                                         *
 *  ����: (1) pSemaphore �����ź����ṹ��ַ                                                      *
 *        (2) option     �����ź����ĵ�ģʽ                                                      *
 *        (3) timeo      ʱ������ģʽ�·����ź�����ʱ�޳���                                      *
 *        (4) pIMask     �ж����μĴ���ֵ                                                        *
 *        (5) pErrno     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵�������ź������������ʱ��������ź��������������д����̣߳���ô˵���ź���������������   *
 *        Release����,��Ҫ���ź����������������ҵ�һ�����ʵ��̣߳�����ֱ��ʹ�����ͷ��ź����ɹ�,  *
 *        ͬʱ�����ź����ļ�������                                                               *
 *************************************************************************************************/
static TState ObtainSemaphore(TSemaphore* pSemaphore, TOption option, TTimeTick timeo, TReg32*
                              pIMask, TError* pErrno)
{
    TBool HiRP = eFalse;
    TState state;
    TIpcContext* pContext;

    /* ���ȳ��Ի���ź���
       ���¼�Ƿ������ȼ����ߵ��߳̾�����������¼�������ĸ��߳� */
    state = TryObtainSemaphore(pSemaphore, &HiRP, pErrno);

    /* �����ISR��������ֱ�ӷ��ء�
    ֻ�����̻߳����²��������̵߳��Ȳſɼ���������
    ����ʹ֮ǰ�����˸������ȼ����߳�Ҳ������е��ȡ�
    ���ߵ���ǰ�̻߳���ź���ʧ�ܣ�Ҳ����������ǰ�߳� */
    if ((uKernelVariable.State == eThreadState) &&
        (uKernelVariable.Schedulable == eTrue))
    {
        /* �����ǰ�߳��ܹ��õ��ź���,ֱ�ӴӺ�������;
        �����ǰ�̲߳��ܵõ��ź��������ǲ��õ��Ǽ򵥷��ط���������Ҳֱ�ӷ���
        �����ǰ�߳��ڵõ��ź�����ͬʱ�������ź������������е��̣߳�����Ҫ���Ե����߳� */
        if (state == eSuccess)
        {
            if (HiRP == eTrue)
            {
                uThreadSchedule();
            }
        }
        else
        {
            /* ��ȷ֪����ǰ�̱߳�����
               �����ǰ�̲߳��ܵõ��ź��������Ҳ��õ��ǵȴ���ʽ��
               ��ô��ǰ�̱߳����������ź��������� */
            if (option & IPC_OPT_WAIT)
            {
                /* �õ���ǰ�̵߳�IPC�����Ľṹ��ַ */
                pContext = &(uKernelVariable.CurrentThread->IpcContext);

                /* �趨�߳����ڵȴ�����Դ����Ϣ */
                uIpcSaveContext(pContext, (void*)pSemaphore, 0U, 0U, option | IPC_OPT_SEMAPHORE,
                                &state, pErrno);

                /* ��ǰ�߳������ڸ��ź������������� */
                uIpcBlockThread(pContext, &(pSemaphore->Queue), timeo);

                /* ��ǰ�߳��޷�����ź����������̵߳��� */
                uThreadSchedule();

                CpuLeaveCritical(*pIMask);
                /* ��Ϊ��ǰ�߳��Ѿ�������IPC������߳��������У����Դ�������Ҫִ�б���̡߳�
                ���������ٴδ����߳�ʱ���ӱ����������С�*/
                CpuEnterCritical(pIMask);

                /* ����̹߳�����Ϣ */
                uIpcCleanContext(pContext);
            }
        }
    }

    return state;
}


/*************************************************************************************************
 *  ����: �����ͷż����ź���                                                                     *
 *  ����: (1) pSemaphore �����ź����ṹ��ַ                                                      *
 *        (2) pHiRP      �Ƿ����Ѹ������ȼ���������Ҫ�����̵߳��ȵı��                        *
 *        (3) pErrno     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵�������ź��������ͷŵ�ʱ��������ź��������������д����̣߳���ô˵���ź���������������   *
 *        Obtain����,��Ҫ���ź����������������ҵ�һ�����ʵ��̣߳�����ֱ��ʹ�����ɹ�����ź���,   *
 *        ͬʱ�����ź����ļ�������                                                               *
 *************************************************************************************************/
static TState TryReleaseSemaphore(TSemaphore* pSemaphore, TBool* pHiRP, TError* pErrno)
{
    TState state;
    TIpcContext* pContext = (TIpcContext*)0;

    if (pSemaphore->Value == pSemaphore->LimitedValue)
    {
        *pErrno = IPC_ERR_INVALID_VALUE;
        state = eFailure;
    }
    else if (pSemaphore->Value == 0U)
    {
        /* ���ź��������ͷŵ�ʱ��������̴߳����������У�˵�����ź���������� */
        /* ��������ѵ��̵߳����ȼ����ڵ�ǰ�߳����ȼ��������̵߳��������� */
        if (pSemaphore->Property & IPC_PROP_PRIMQ_AVAIL)
        {
            pContext = (TIpcContext*)(pSemaphore->Queue.PrimaryHandle->Owner);
            uIpcUnblockThread(pContext, eSuccess, IPC_ERR_NONE, pHiRP);
        }
        else
        {
            pSemaphore->Value++;
        }

        *pErrno = IPC_ERR_NONE;
        state = eSuccess;
    }
    else
    {
        pSemaphore->Value++;

        *pErrno = IPC_ERR_NONE;
        state = eSuccess;
    }

    return state;
}


/*************************************************************************************************
 *  ����: �ͷ��ź���                                                                             *
 *  ����: (1) pSemaphore �����ź����ṹ��ַ                                                      *
 *        (2) option     �����ź����ĵ�ģʽ                                                      *
 *        (3) timeo      ʱ������ģʽ�·����ź�����ʱ�޳���                                      *
 *        (4) pIMask     �ж����μĴ���ֵ                                                        *
 *        (5) pErrno     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static TState ReleaseSemaphore(TSemaphore* pSemaphore, TOption option, TTimeTick timeo,
                               TReg32* pIMask, TError* pErrno)
{
    TBool HiRP = eFalse;
    TState state;
    TIpcContext* pContext;

    /* ���ȳ����ͷ��ź���
       ����¼�Ƿ������ȼ����ߵ��߳̾�����������¼�������ĸ��̡߳� */
    state = TryReleaseSemaphore(pSemaphore, &HiRP, pErrno);

    /* �����ISR��������ֱ�ӷ��ء�
    ֻ�����̻߳����²��������̵߳��Ȳſɼ���������
    ����ʹ֮ǰ�����˸������ȼ����߳�Ҳ������е��ȡ�
    ���ߵ���ǰ�߳��ͷ��ź���ʧ�ܣ�Ҳ����������ǰ�߳� */
    if ((uKernelVariable.State == eThreadState) &&
        (uKernelVariable.Schedulable == eTrue))
    {
        /* �����ǰ�̻߳����˸������ȼ����߳�����е��ȡ�*/
        if (state == eSuccess)
        {
            if (HiRP == eTrue)
            {
                uThreadSchedule();
            }
        }
        else
        {
            /* ����ȷ֪����ǰ�̱߳�������
               �����ǰ�̲߳����ͷ��ź��������Ҳ��õ��ǵȴ���ʽ��
               ��ô��ǰ�̱߳����������ź��������� */
            if (option & IPC_OPT_WAIT)
            {
                /* �õ���ǰ�̵߳�IPC�����Ľṹ��ַ */
                pContext = &(uKernelVariable.CurrentThread->IpcContext);

                /* �趨�߳����ڵȴ�����Դ����Ϣ */
                uIpcSaveContext(pContext, (void*)pSemaphore, 0U, 0U, option | IPC_OPT_SEMAPHORE,
                                &state, pErrno);

                /* ��ǰ�߳������ڸ��źŵ��������У�ʱ�޻������޵ȴ�����timeo�������� */
                uIpcBlockThread(pContext, &(pSemaphore->Queue), timeo);

                /* ��ǰ�߳��޷��ͷ��ź����������̵߳��� */
                uThreadSchedule();

                CpuLeaveCritical(*pIMask);
                /* ��Ϊ��ǰ�߳��Ѿ�������IPC������߳��������У����Դ�������Ҫִ�б���̡߳�
                ���������ٴδ����߳�ʱ���ӱ����������С�*/
                CpuEnterCritical(pIMask);

                /* ����̹߳�����Ϣ */
                uIpcCleanContext(pContext);
            }
        }
    }

    return state;
}


/*************************************************************************************************
 *  ����: �߳�/ISR ����ź�����û����ź���                                                      *
 *  ����: (1) pSemaphore �ź����ṹ��ַ                                                          *
 *        (2) option     �����ź����ĵ�ģʽ                                                      *
 *        (3) timeo      ʱ������ģʽ�·����ź�����ʱ�޳���                                      *
 *        (5) pErrno     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xSemaphoreObtain(TSemaphore* pSemaphore, TOption option, TTimeTick timeo, TError* pErrno)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TReg32 imask;

    CpuEnterCritical(&imask);
    if (pSemaphore->Property & IPC_PROP_READY)
    {
        /* ���ǿ��Ҫ����ISR�»���ź��� */
        if (option & IPC_OPT_ISR)
        {
            /* �жϳ���ֻ���Է�������ʽ����ź��������Ҳ������̵߳������� */
            KNL_ASSERT((uKernelVariable.State == eIntrState),"");
            state = TryObtainSemaphore(pSemaphore, &HiRP, &error);
        }
        else
        {
            /* �Զ��ж���λ���ź��� */
            state = ObtainSemaphore(pSemaphore, option, timeo, &imask, &error);
        }
    }
    CpuLeaveCritical(imask);

    * pErrno = error;
    return state;
}


/*************************************************************************************************
 *  ����: �߳�/ISR�����ͷ��ź���                                                                 *
 *  ����: (1) pSemaphore �ź����ṹ��ַ                                                          *
 *        (2) option     �߳��ͷ��ź����ķ�ʽ                                                    *
 *        (3) timeo      �߳��ͷ��ź�����ʱ��                                                    *
 *        (4) pErrno     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xSemaphoreRelease(TSemaphore* pSemaphore, TOption option, TTimeTick timeo, TError* pErrno)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TReg32 imask;

    CpuEnterCritical(&imask);
    if (pSemaphore->Property & IPC_PROP_READY)
    {
        /* ���ǿ��Ҫ����ISR���ͷ��ź��� */
        if (option & IPC_OPT_ISR)
        {
            /* �жϳ���ֻ���Է�������ʽ�ͷ��ź��������Ҳ������̵߳������� */
            KNL_ASSERT((uKernelVariable.State == eIntrState),"");
            state = TryReleaseSemaphore(pSemaphore, &HiRP, &error);
        }
        else
        {
            /* �Զ��ж�����ͷ��ź��� */
            state = ReleaseSemaphore(pSemaphore, option, timeo, &imask, &error);
        }
    }
    CpuLeaveCritical(imask);

    * pErrno = error;
    return state;
}


/*************************************************************************************************
 *  ����: ��ʼ�������ź���                                                                       *
 *  ����: (1) pSemaphore �����ź����ṹ��ַ                                                      *
 *        (2) value      �����ź�����ʼֵ                                                        *
 *        (3) mvalue     �����ź���������ֵ                                                    *
 *        (4) property   �ź����ĳ�ʼ����                                                        *
 *        (5) pErrno     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵�����ź���ֻʹ�û���IPC����                                                                *
 *************************************************************************************************/
TState xSemaphoreCreate(TSemaphore* pSemaphore, TBase32 value, TBase32 mvalue,
                      TProperty property, TError* pErrno)
{
    TState state = eFailure;
    TError error = IPC_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    if (!(pSemaphore->Property & IPC_PROP_READY))
    {
        property |= IPC_PROP_READY;
        pSemaphore->Property     = property;
        pSemaphore->Value        = value;
        pSemaphore->LimitedValue = mvalue;
        pSemaphore->InitialValue = value;
        pSemaphore->Queue.PrimaryHandle   = (TObjNode*)0;
        pSemaphore->Queue.AuxiliaryHandle = (TObjNode*)0;
        pSemaphore->Queue.Property        = &(pSemaphore->Property);

        error = IPC_ERR_NONE;
        state = eSuccess;
    }

    CpuLeaveCritical(imask);

    *pErrno = error;
    return state;
}


/*************************************************************************************************
 *  ����: �ź���ȡ����ʼ��                                                                       *
 *  ����: (1) pSemaphore �ź����ṹ��ַ                                                          *
 *        (2) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xSemaphoreDelete(TSemaphore* pSemaphore, TError* pErrno)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pSemaphore->Property & IPC_PROP_READY)
    {
        /* ����¼�Ƿ������ȼ����ߵ��߳̾�����������¼�������ĸ��̡߳� */
        /* ���ź������������ϵ����еȴ��̶߳��ͷţ������̵߳ĵȴ��������TCLE_IPC_DELETE  */
        uIpcUnblockAll(&(pSemaphore->Queue), eFailure, IPC_ERR_DELETE, (void**)0, &HiRP);

        /* ����ź��������ȫ������ */
        memset(pSemaphore, 0U, sizeof(TSemaphore));

		/* ���Է����߳���ռ */
		uThreadPreempt(HiRP);

        error = IPC_ERR_NONE;
        state = eSuccess;
    }

    CpuLeaveCritical(imask);

    *pErrno = error;
    return state;
}


/*************************************************************************************************
 *  ����: ���ü����ź���                                                                         *
 *  ����: (1) pSemaphore �ź����ṹ��ַ                                                          *
 *        (2) pErrno     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xSemaphoreReset(TSemaphore* pSemaphore, TError* pErrno)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pSemaphore->Property & IPC_PROP_READY)
    {
        /* ����¼�Ƿ������ȼ����ߵ��߳̾�����������¼�������ĸ��̡߳� */
        /* ���ź������������ϵ����еȴ��̶߳��ͷţ������̵߳ĵȴ��������TCLE_IPC_RESET */
        uIpcUnblockAll(&(pSemaphore->Queue), eFailure, IPC_ERR_RESET, (void**)0, &HiRP);

        /* �����ź������������� */
        pSemaphore->Property &= IPC_RESET_SEMN_PROP;
        pSemaphore->Value = pSemaphore->InitialValue;

		/* ���Է����߳���ռ */
		uThreadPreempt(HiRP);

        error = IPC_ERR_NONE;
        state = eSuccess;
    }

    CpuLeaveCritical(imask);

    *pErrno = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��ź���������ֹ����,��ָ�����̴߳��ź������߳�������������ֹ����������                  *
 *  ������(1) pSemaphore �ź����ṹ��ַ                                                          *
 *        (2) option     ����ѡ��                                                                *
 *        (3) pThread    �̵߳�ַ                                                                *
 *        (4) pErrno     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xSemaphoreFlush(TSemaphore* pSemaphore, TError* pErrno)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pSemaphore->Property & IPC_PROP_READY)
    {
        /* ����¼�Ƿ������ȼ����ߵ��߳̾�����������¼�������ĸ��̡߳� */
        /* ���ź������������ϵ����еȴ��̶߳��ͷţ������̵߳ĵȴ��������TCLE_IPC_FLUSH  */
        uIpcUnblockAll(&(pSemaphore->Queue), eFailure, IPC_ERR_FLUSH, (void**)0, &HiRP);

		/* ���Է����߳���ռ */
		uThreadPreempt(HiRP);

        state = eSuccess;
        error = IPC_ERR_NONE;
    }

    CpuLeaveCritical(imask);

    *pErrno = error;
    return state;
}

#endif

