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
#include "tcl.thread.h"
#include "tcl.kernel.h"
#include "tcl.ipc.h"
#include "tcl.flags.h"

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_FLAGS_ENABLE))

static TState TryReceiveFlags(TFlags* pFlags, TBitMask* pPattern, TOption option, TError* pError);
static TState ReceiveFlags(TFlags* pFlags, TBitMask* pPattern, TOption option, TTimeTick timeo,
                           TReg32* pIMask, TError* pError);
static TState TrySendFlags(TFlags* pFlags, TBitMask pattern, TBool* pHiRP,  TError* pError);
static TState SendFlags(TFlags* pFlags, TBitMask pattern, TError* pError);

/*************************************************************************************************
 *  ���ܣ����Խ����¼����                                                                       *
 *  ������(1) pFlags   �¼���ǵĵ�ַ                                                            *
 *        (2) pPattern ��Ҫ���յı�ǵ����                                                      *
 *        (3) option   �����¼���ǵĲ���                                                        *
 *        (4) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static TState TryReceiveFlags(TFlags* pFlags, TBitMask* pPattern, TOption option, TError* pError)
{
    TState state;
    TBitMask match;
    TBitMask pattern;

    pattern = *pPattern;
    match = (pFlags->Value) & pattern;

    if (((option & IPC_OPT_AND) && (match == pattern)) ||
            ((option & IPC_OPT_OR) && (match != 0U)))
    {
        if (option & IPC_OPT_CONSUME)
        {
            pFlags->Value &= (~match);
        }

        *pPattern = match;

        *pError = IPC_ERR_NONE;
        state = eSuccess;
    }
    else
    {
        *pError = IPC_ERR_FLAGS;
        state = eFailure;
    }
    return state;
}


/*************************************************************************************************
 *  ���ܣ����Է����¼����                                                                       *
 *  ������(1) pFlags   �¼���ǵĵ�ַ                                                            *
 *        (2) pPattern ��Ҫ���͵ı�ǵ����                                                      *
 *        (3) pHiRP    �Ƿ��ں����л��ѹ������߳�                                                *
 *        (4) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static TState TrySendFlags(TFlags* pFlags, TBitMask pattern, TBool* pHiRP, TError* pError)
{
    TObjNode* pHead = (TObjNode*)0;
    TObjNode* pTail = (TObjNode*)0;
    TObjNode* pCurrent = (TObjNode*)0;
    TBitMask mask = 0U;
    TBool match = eFalse;
    TOption option;
    TBitMask* pTemp;
    TState state;
    TIpcContext* pContext;

    /* ����¼��Ƿ���Ҫ���� */
    mask = pFlags->Value | pattern;
    if (mask != pFlags->Value)
    {
        *pError = IPC_ERR_NONE;
        state = eSuccess;

        /* ���¼����͵��¼������ */
        pFlags->Value |= pattern;

        /* �¼�����Ƿ����߳��ڵȴ��¼��ķ��� */
        if (pFlags->Queue.PrimaryHandle != (TObjNode*)0)
        {
            /* ��ʼ�����¼����������� */
            pHead = pFlags->Queue.PrimaryHandle;
            pTail = pFlags->Queue.PrimaryHandle->Prev;
            do
            {
                pCurrent = pHead;
                pHead = pHead->Next;
                match = eFalse;

                /* ��õȴ��¼���ǵ��̺߳���ص��¼��ڵ� */
                pContext =  (TIpcContext*)(pCurrent->Owner);
                option = pContext->Option;
                pTemp = (TBitMask*)(pContext->Data.Addr1);

                /*  �õ�����Ҫ����¼���� */
                mask = pFlags->Value & (*pTemp);
                if (((option & IPC_OPT_AND) && (mask == *pTemp)) ||
                        ((option & IPC_OPT_OR) && (mask != 0U)))
                {
                    match = eTrue;
                    *pTemp = mask;
                    uIpcUnblockThread(pContext, eSuccess, IPC_ERR_NONE, pHiRP);
                }

                /* ����¼���Ǽ��ɹ� */
                if ((match == eTrue) && (option & IPC_OPT_CONSUME))
                {
                    /* ����ĳЩ�¼�������¼�ȫ�������Ĵ��������˳� */
                    pFlags->Value &= (~mask);
                    if (pFlags->Value == 0U)
                    {
                        break;
                    }
                }
            }
            while(pCurrent != pTail);
        }
    }
    else
    {
        *pError = IPC_ERR_FLAGS ;
        state = eError;
    }

    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳̽����¼����                                                                       *
 *  ������(1) pFlags   �¼���ǵĵ�ַ                                                            *
 *        (2) pPattern ��Ҫ���յı�ǵ����                                                      *
 *        (3) timeo    ʱ������ģʽ�·����¼���ǵ�ʱ�޳���                                      *
 *        (4) option   �����¼���ǵĲ���                                                        *
 *        (5) pIMask   �ж����μĴ���ֵ                                                          *
 *        (6) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static TState ReceiveFlags(TFlags* pFlags, TBitMask* pPattern, TOption option, TTimeTick timeo,
                           TReg32* pIMask, TError* pError)
{
    TState state = eFailure;
    TIpcContext* pContext;

    /* ���Դ��¼�����л���¼� */
    state = TryReceiveFlags(pFlags, pPattern, option, pError);

    /* ��Ϊ�¼�����̶߳����в�������¼����Ͷ��У����Բ���Ҫ�ж��Ƿ������߳�Ҫ���ȣ�
    ����Ҫ�����Ƿ���Ҫ���¼����ĵ����� */
    if ((uKernelVariable.State == eThreadState) &&
            (uKernelVariable.Schedulable == eTrue))
    {
        /* �����ǰ�̲߳��ܵõ��¼������Ҳ��õ��ǵȴ���ʽ��
        ��ô��ǰ�̱߳����������¼���ǵĵȴ������У�����ǿ���̵߳��� */
        if (state == eFailure)
        {
            if (option & IPC_OPT_WAIT)
            {
                /* �õ���ǰ�̵߳�IPC�����Ľṹ��ַ */
                pContext = &(uKernelVariable.CurrentThread->IpcContext);

                /* �����̹߳�����Ϣ */
                uIpcSaveContext(pContext, (void*)pFlags, (TBase32)pPattern, sizeof(TBase32), 
                                option | IPC_OPT_FLAGS, &state, pError);

                /* ��ǰ�߳������ڸ��������������,��ȡ���������߳̽����̻߳����������У�
                ע��IPC�̹߳������������߳�״̬ */
                uIpcBlockThread(pContext, &(pFlags->Queue), timeo);

                /* ��ǰ�߳�������ȣ������̼߳�������ִ�� */
                uThreadSchedule();

                CpuLeaveCritical(*pIMask);
                /* ��ʱ�˴�����һ�ε��ȣ���ǰ�߳��Ѿ�������IPC������������С���������ʼִ�б���̣߳�
                ���������ٴδ����߳�ʱ���ӱ����������С�*/
                CpuEnterCritical(pIMask);

                /* ����߳�IPC������Ϣ */
                uIpcCleanContext(pContext);
            }
        }
    }

    return state;
}


/*************************************************************************************************
 *  ���ܣ��̻߳���ISR���¼���Ƿ����¼�                                                          *
 *  ������(1) pFlags   �¼���ǵĵ�ַ                                                            *
 *        (2) pPattern ��Ҫ���յı�ǵ����                                                      *
 *        (3) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure   ����ʧ��                                                                *
 *        (2) eSuccess   �����ɹ�                                                                *
 *  ˵������������������ǰ�߳�����                                                             *
 *************************************************************************************************/
static TState SendFlags(TFlags* pFlags, TBitMask pattern, TError* pError)
{
    TState state = eFailure;
    TBool HiRP = eFalse;

    state = TrySendFlags(pFlags, pattern, &HiRP, pError);

    /*  �����ISR��������ֱ�ӷ��ء�
        ֻ�����̻߳����²��������̵߳��Ȳſɼ������� */
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
    }

    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳�/ISR�����¼����                                                                   *
 *  ������(1) pFlags   �¼���ǵĵ�ַ                                                            *
 *        (2) pPattern ��Ҫ���յı�ǵ����                                                      *
 *        (3) timeo    ʱ������ģʽ�·����¼���ǵ�ʱ�޳���                                      *
 *        (4) option   �����¼���ǵĲ���                                                        *
 *        (5) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xFlagsReceive(TFlags* pFlags, TBitMask* pPattern, TOption option, TTimeTick timeo,
                     TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;

    CpuEnterCritical(&imask);

    if (pFlags->Property & IPC_PROP_READY)
    {
        /* ���ǿ��Ҫ����ISR�¶�ȡ�¼� */
        if (option & IPC_OPT_ISR)
        {
            /* �жϳ���ֻ���Է�������ʽ���¼�����н����¼������Ҳ������̵߳�������  */
            /* ���ж�isr�У���ǰ�߳�δ������߾������ȼ��̣߳�Ҳδ�ش����ں˾����̶߳��С�
            ������isr�е���TryReceiveFlags()��õ���HiRP������κ����塣*/
            KNL_ASSERT((uKernelVariable.State == eIntrState),"");
            state = TryReceiveFlags(pFlags, pPattern, option, &error);
        }
        else
        {
            /* �Զ��ж���ν����¼� */
            state = ReceiveFlags(pFlags, pPattern, option, timeo, &imask, &error);
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳�/ISR���¼���Ƿ����¼�                                                             *
 *  ������(1) pFlags   �¼���ǵĵ�ַ                                                            *
 *        (2) pPattern ��Ҫ���յı�ǵ����                                                      *
 *        (3) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure   ����ʧ��                                                                *
 *        (2) eSuccess   �����ɹ�                                                                *
 *  ˵������������������ǰ�߳�����,���Բ��������̻߳���ISR������                               *
 *************************************************************************************************/
TState xFlagsSend(TFlags* pFlags, TBitMask pattern, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;

    CpuEnterCritical(&imask);

    if (pFlags->Property & IPC_PROP_READY)
    {
        state = SendFlags(pFlags, pattern, &error);
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: ����¼������������                                                                   *
 *  ������(1) pFlags   �¼���ǵĵ�ַ                                                            *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure   ����ʧ��                                                                *
 *        (2) eSuccess   �����ɹ�                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xFlagsReset(TFlags* pFlags, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pFlags->Property & IPC_PROP_READY)
    {
        /* �����������ϵ����еȴ��̶߳��ͷţ������̵߳ĵȴ��������IPC_ERR_RESET */
        uIpcUnblockAll(&(pFlags->Queue), eFailure, IPC_ERR_RESET, (void**)0, &HiRP);

        pFlags->Property &= IPC_RESET_FLAG_PROP;
        pFlags->Value = 0U;

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
 *  ���ܣ��¼������ֹ����,��ָ�����̴߳��¼���ǵ��߳�������������ֹ����������                  *
 *  ������(1) pFlags   �¼���ǽṹ��ַ                                                          *
 *        (2) option   ����ѡ��                                                                  *
 *        (3) pThread  �̵߳�ַ                                                                  *
 *        (4) pError   ��ϸ���ý��                                                              *
 *  ���أ�(1) eSuccess                                                                           *
 *        (2) eFailure                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xFlagsFlush(TFlags* pFlags, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pFlags->Property & IPC_PROP_READY)
    {
        /* ���¼�������������ϵ����еȴ��̶߳��ͷţ������̵߳ĵȴ��������TCLE_IPC_FLUSH  */
        uIpcUnblockAll(&(pFlags->Queue), eFailure, IPC_ERR_FLUSH, (void**)0, &HiRP);

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
 *  ���ܣ���ʼ���¼����                                                                         *
 *  ������(1) pFlags     �¼���ǵĵ�ַ                                                          *
 *        (2) property   �¼���ǵĳ�ʼ����                                                      *
 *        (3) pError     ����������ϸ����ֵ                                                      *
 *  ����: (1) eFailure   ����ʧ��                                                                *
 *        (2) eSuccess   �����ɹ�                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xFlagsCreate(TFlags* pFlags, TProperty property, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    if (!(pFlags->Property & IPC_PROP_READY))
    {
        property |= IPC_PROP_READY;
        pFlags->Property = property;
        pFlags->Value = 0U;

        pFlags->Queue.PrimaryHandle   = (TObjNode*)0;
        pFlags->Queue.AuxiliaryHandle = (TObjNode*)0;
        pFlags->Queue.Property        = &(pFlags->Property);

        state = eSuccess;
        error = IPC_ERR_NONE;
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ�ȡ���¼���ǳ�ʼ��                                                                     *
 *  ������(1) pFlags   �¼���ǵĵ�ַ                                                            *
 *        (2) pError   ����������ϸ����ֵ                                                        *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xFlagsDelete(TFlags* pFlags, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pFlags->Property & IPC_PROP_READY)
    {
        /* �����������ϵ����еȴ��̶߳��ͷţ������̵߳ĵȴ��������IPC_ERR_DELETE  */
        uIpcUnblockAll(&(pFlags->Queue), eFailure, IPC_ERR_DELETE, (void**)0, &HiRP);

        /* ����¼���Ƕ����ȫ������ */
        memset(pFlags, 0U, sizeof(TFlags));

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
 *  ���ܣ��¼���ǲ�ѯ                                                                           *
 *  ������(1) pFlags �¼���ǽṹ��ַ                                                            *
 *        (2) pData  ���Ʊ����¼�������ݵ�ַ                                                    *
 *        (3) pError ����������ϸ����ֵ                                                          *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void xFlagsQuery(TFlags* pFlags, TFlags* pData, TError* pError)
{
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;

    CpuEnterCritical(&imask);

    if (pFlags->Property & IPC_PROP_READY)
    {
        memcpy(pData, pFlags, sizeof(TFlags));
        error = IPC_ERR_NONE;
    }

    CpuLeaveCritical(imask);

    *pError = error;
}

#endif

