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
#include "tcl.mailbox.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MAILBOX_ENABLE))

static TState TryReceiveMail(TMailBox* pMailbox, void** pMail2, TBool* pHiRP, TError* pError);
static TState TrySendMail(TMailBox* pMailbox, void** pMail2, TBool* pHiRP, TError* pError);
static TState ReceiveMail(TMailBox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo,
                          TReg32* pIMask, TError* pError);
static TState SendMail(TMailBox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo,
                       TReg32* pIMask, TError* pError);

/*************************************************************************************************
 *  ����: ���Զ�ȡ�����е��ʼ�                                                                   *
 *  ����: (1) pMailbox ����ṹ��ַ                                                              *
 *        (2) pMail2   �����ʼ��ṹ��ַ��ָ�����                                                *
 *        (3) pHiRP   �Ƿ��ں����л��ѹ������߳�                                                 *
 *        (4) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵������������Զ�ȡ��ʱ��,������̴߳����������������,˵���ǵ�ǰ������߳������������ʼ�   *
 *        ���Ͷ���, ��ʱ��Ҫ������������������ҵ�һ�����ʵ��߳�,ֱ��ʹ�÷����ʼ��ɹ�������Ҫ    *
 *        ���������״̬����                                                                     *
 *************************************************************************************************/
static TState TryReceiveMail(TMailBox* pMailbox, void** pMail2, TBool* pHiRP, TError* pError)
{
    TState state;
    TIpcContext* pContext = (TIpcContext*)0;

    /* �������״̬Ϊ��,�����̷���,�������״̬Ϊ��,�����ȶ�ȡ�����е��ʼ� */
    if (pMailbox->Status == eMailBoxFull)
    {
        /* �������ж�ȡ�ʼ� */
        *pMail2 = pMailbox->Mail;

        /* �����ʱ��������߳��������������̴߳���,�����̵߳ȴ������ʼ�,
           ����л���һ�����ʵ��̡߳���ʱ�����ʼ����� */
        if (pMailbox->Property &IPC_PROP_AUXIQ_AVAIL)
        {
            pContext = (TIpcContext*)(pMailbox->Queue.AuxiliaryHandle->Owner);
        }
        else
        {
            if (pMailbox->Property &IPC_PROP_PRIMQ_AVAIL)
            {
                pContext = (TIpcContext*)(pMailbox->Queue.PrimaryHandle->Owner);
            }
        }

        /* ������̱߳�����,�򽫸��̴߳����͵��ʼ����浽������, ��������״̬���� */
        if (pContext != (TIpcContext*)0)
        {
            uIpcUnblockThread(pContext, eSuccess, IPC_ERR_NONE, pHiRP);
            pMailbox->Mail = *((TMail*)(pContext->Data.Addr2));
        }
        /* �����������,�����ʼ�Ϊ��,��������״̬Ϊ�� */
        else
        {
            pMailbox->Mail = (void*)0;
            pMailbox->Status = eMailBoxEmpty;
        }

        /* �����̳߳ɹ���ȡ������ */
        *pError = IPC_ERR_NONE;
        state = eSuccess;
    }
    else
    {
        *pError = IPC_ERR_INVALID_STATUS;
        state = eFailure;
    }

    return state;
}


/*************************************************************************************************
 *  ����: ���������䷢���ʼ�                                                                     *
 *  ����: (1) pMailbox ����ṹ��ַ                                                              *
 *        (2) pMail2   �����ʼ��ṹ��ַ��ָ�����                                                *
 *        (3) pHiRP    �Ƿ��ں����л��ѹ������߳�                                                *
 *        (4) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static TState TrySendMail(TMailBox* pMailbox, void** pMail2, TBool* pHiRP, TError* pError)
{
    TState state;
    TIpcContext* pContext = (TIpcContext*)0;

    if (pMailbox->Status == eMailBoxEmpty)
    {
        /* ������Ϊ�յ�ʱ��,������̴߳����������������,˵���ǵ�ǰ����ĵ��߳�����������
        �ʼ���ȡ����, ��ʱ��Ҫ������������������ҵ�һ�����ʵ��߳�,��ֱ��ʹ������ȡ�ʼ��ɹ���
        ͬʱ�����״̬���� */
        if (pMailbox->Property &IPC_PROP_PRIMQ_AVAIL)
        {
            pContext = (TIpcContext*)(pMailbox->Queue.PrimaryHandle->Owner);
        }

        /* ����ҵ���һ�����ʵ��߳�,�ͽ��ʼ����͸��� */
        if (pContext != (TIpcContext*)0)
        {
            uIpcUnblockThread(pContext, eSuccess, IPC_ERR_NONE, pHiRP);
            *(pContext->Data.Addr2) = * pMail2;
        }
        else
        {
            /* �����ʼ�д������,��������״̬Ϊ�� */
            pMailbox->Mail = * pMail2;
            pMailbox->Status = eMailBoxFull;
        }

        /* �����̳߳ɹ������ʼ���� */
        *pError = IPC_ERR_NONE;
        state = eSuccess;

    }
    else
    {
        /* �������Ѿ����ʼ��ˣ������ٷ��������ʼ� */
        /* The mail could not placed in this mailbox because the mailbox alreday contains a mail */
        *pError = IPC_ERR_INVALID_STATUS;
        state = eFailure;
    }
    return state;
}


/*************************************************************************************************
 *  ����: �����̶߳�ȡ�����е��ʼ�                                                               *
 *  ����: (1) pMailbox ����ṹ��ַ                                                              *
 *        (2) pMail2   �����ʼ��ṹ��ַ��ָ�����                                                *
 *        (3) option   ���������ģʽ                                                            *
 *        (4) timeo    ʱ������ģʽ�·��������ʱ�޳���                                          *
 *        (5) pIMask   �ж����μĴ���ֵ                                                          *
 *        (6) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static TState ReceiveMail(TMailBox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo,
                          TReg32* pIMask, TError* pError)
{
    TState state;
    TBool HiRP = eFalse;
    TIpcContext* pContext;

    /* ���Դ������ȡ�ʼ� */
    state = TryReceiveMail(pMailbox, (void**)pMail2, &HiRP, pError);

    /* �����ISR�����µ��ñ�������ֱ�ӷ���;
       �����ǰ�̳߳ɹ������ʼ�,����ʱ��ֹ�̵߳���,��ʹ�����˸������ȼ����߳�Ҳ�Ӻ�������;
       �����ǰ�̳߳ɹ������ʼ�,���Ҵ�ʱû�н�ֹ�̵߳���,�����Ƿ���Ҫ�����̵߳���
       �����ǰ�̲߳��ܽ����ʼ�,����ʱ��ֹ�̵߳���,��ֱ�ӴӺ�������;
       �����ǰ�̲߳��ܽ����ʼ�,��ʹ��ʱû�н�ֹ�̵߳���,���ǲ��õ��ǲ���������,��ֱ�ӷ���;
       �����ǰ�̲߳��ܽ����ʼ�,���Ҵ�ʱû�н�ֹ�̵߳���,ͬʱ���õ�����������,�������߳�;

       ���ϸ��������
       ֻ�����̻߳����²��������̵߳��Ȳſɼ�������,
       ����ʹ֮ǰ�����˸������ȼ����߳�Ҳ������е��ȡ�
       ���ߵ�ǰ�߳̽����ʼ�ʧ��,Ҳ����������ǰ�߳� */
    if ((uKernelVariable.State == eThreadState) && (uKernelVariable.Schedulable == eTrue))
    {
        /* �����ǰ�̻߳����˸������ȼ����߳�����е��ȡ�*/
        if (state == eSuccess)
        {
            if (HiRP == eTrue)
            {
                uThreadSchedule();
            }
        }
        /* �����ǰ�̲߳��ܵõ�����,ͬʱ���õ��ǵȴ���ʽ,�����ں�û�йر��̵߳���,
           ��ô��ǰ�̱߳������������������,����ǿ���̵߳��� */
        else
        {
            if (option &IPC_OPT_WAIT)
            {
                /* �õ���ǰ�̵߳�IPC�����Ľṹ��ַ */
                pContext = &(uKernelVariable.CurrentThread->IpcContext);

                /* �����̹߳�����Ϣ */
                uIpcSaveContext(pContext, (void*)pMailbox, (TBase32)pMail2, sizeof(TBase32),
                                option | IPC_OPT_MAILBOX | IPC_OPT_READ_DATA,
                                &state, pError);

                /* ��ǰ�߳������ڸ��������������,��ȡ���������߳̽����̻߳�����������,
                   ע��IPC�̹߳������������߳�״̬ */
                uIpcBlockThread(pContext, &(pMailbox->Queue), timeo);

                /* ��ǰ�߳��������,�����̼߳�������ִ�� */
                uThreadSchedule();

                CpuLeaveCritical(*pIMask);
                /* ��Ϊ��ǰ�߳��Ѿ�������IPC������߳���������,���Դ�������Ҫִ�б���̡߳�
                   ���������ٴδ����߳�ʱ,�ӱ����������С�*/
                CpuEnterCritical(pIMask);

                /* ����߳�IPC������Ϣ */
                uIpcCleanContext(pContext);
            }
        }
    }
    return state;
}


/*************************************************************************************************
 *  ����: �̷߳����ʼ�                                                                           *
 *  ����: (1) pMailbox ����ṹ��ַ                                                              *
 *        (2) pMail2   �����ʼ��ṹ��ַ��ָ�����                                                *
 *        (3) option   ���������ģʽ                                                            *
 *        (4) timeo    ʱ������ģʽ�·��������ʱ�޳���                                          *
 *        (5) pIMask   �ж����μĴ���ֵ                                                          *
 *        (6) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static TState SendMail(TMailBox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo,
                       TReg32* pIMask, TError* pError)
{
    TState state = eError;
    TBool HiRP = eFalse;
    TIpcContext* pContext;

    /* HiRP���̻߳����²������� */
    state = TrySendMail(pMailbox, (void**)pMail2, &HiRP, pError);

    /* �����ISR�����µ��ñ�������ֱ�ӷ���;
       �����ǰ�̳߳ɹ������ʼ�,����ʱ��ֹ�̵߳���,��ʹ�����˸������ȼ����߳�Ҳ�Ӻ�������;
       �����ǰ�̳߳ɹ������ʼ�,���Ҵ�ʱû�н�ֹ�̵߳���,�����Ƿ���Ҫ�����̵߳���
       �����ǰ�̲߳��ܷ����ʼ�,����ʱ��ֹ�̵߳���,��ֱ�ӴӺ�������;
       �����ǰ�̲߳��ܷ����ʼ�,��ʹ��ʱû�н�ֹ�̵߳���,���ǲ��õ��ǲ���������,��ֱ�ӷ���;
       �����ǰ�̲߳��ܷ����ʼ�,���Ҵ�ʱû�н�ֹ�̵߳���,ͬʱ���õ�����������,�������߳�;

       ���ϸ��������
       ֻ�����̻߳����²��������̵߳��Ȳſɼ�������,
       ����ʹ֮ǰ�����˸������ȼ����߳�Ҳ������е��ȡ�
       ���ߵ�ǰ�̷߳����ʼ�ʧ��,Ҳ����������ǰ�߳� */
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
        /* �����ǰ�̲߳��ܵõ�����,���Ҳ��õ��ǵȴ���ʽ,�����ں�û�йر��̵߳���,
           ��ô��ǰ�̱߳������������������,����ǿ���̵߳��� */
        else
        {
            if (option &IPC_OPT_WAIT)
            {
                /* ���ͽ����̶Ȳ�ͬ���ʼ����߳̽��벻ͬ���������� */
                if (option &IPC_OPT_UARGENT)
                {
                    option |= IPC_OPT_USE_AUXIQ;
                }

                /* �õ���ǰ�̵߳�IPC�����Ľṹ��ַ */
                pContext = &(uKernelVariable.CurrentThread->IpcContext);

                /* �����̹߳�����Ϣ */
                uIpcSaveContext(pContext, (void*)pMailbox, (TBase32)pMail2, sizeof(TBase32),
                                option | IPC_OPT_MAILBOX | IPC_OPT_WRITE_DATA, &state, pError);

                /* ��ǰ�߳������ڸ�������������� */
                uIpcBlockThread(pContext, &(pMailbox->Queue), timeo);

                /* ��ǰ�߳��������,�����̼߳�������ִ�� */
                uThreadSchedule();

                CpuLeaveCritical(*pIMask);
                /* ��Ϊ��ǰ�߳��Ѿ�������IPC������߳���������,���Դ�������Ҫִ�б���̡߳�
                   ���������ٴδ����߳�ʱ,�ӱ����������С�*/
                CpuEnterCritical(pIMask);

                /* ����߳�IPC������Ϣ */
                uIpcCleanContext(pContext);
            }
        }
    }
    return state;
}


/*************************************************************************************************
 *  ����: �߳�/ISR�������ж�ȡ�ʼ�                                                               *
 *  ����: (1) pMailbox ����ṹ��ַ                                                              *
 *        (2) pMail2   �����ʼ��ṹ��ַ��ָ�����                                                *
 *        (3) option   ���������ģʽ                                                            *
 *        (4) timeo    ʱ������ģʽ�·��������ʱ�޳���                                          *
 *        (5) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xMailBoxReceive(TMailBox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo,
                       TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TReg32 imask;

    CpuEnterCritical(&imask);
    if (pMailbox->Property &IPC_PROP_READY)
    {
        /* ���ǿ��Ҫ����ISR�¶�ȡ�ʼ� */
        if (option &IPC_OPT_ISR)
        {
            /* �жϳ���ֻ���Է�������ʽ�������ж�ȡ�ʼ�,������ʱ�������̵߳������⡣
               ���ж���,��ǰ�߳�δ������߾������ȼ��߳�,Ҳδ�ش����ں˾����̶߳��С�
               �����ڴ˴��õ���HiRP������κ����塣*/
            KNL_ASSERT((uKernelVariable.State == eIntrState), "");
            state = TryReceiveMail(pMailbox, (void**)pMail2, &HiRP, &error);
        }
        else
        {
            /* �Զ��ж���ζ�ȡ�ʼ� */
            state = ReceiveMail(pMailbox, pMail2, option, timeo, &imask, &error);
        }
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}



/*************************************************************************************************
 *  ����: �߳�/ISR�����ʼ�                                                                       *
 *  ����: (1) pMailbox ����ṹ��ַ                                                              *
 *        (2) pMail2   �����ʼ��ṹ��ַ��ָ�����                                                *
 *        (3) option   ���������ģʽ                                                            *
 *        (4) timeo    ʱ������ģʽ�·��������ʱ�޳���                                          *
 *        (5) pErrno   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xMailBoxSend(TMailBox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo,
                    TError* pErrno)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TReg32 imask;

    CpuEnterCritical(&imask);
    if (pMailbox->Property &IPC_PROP_READY)
    {
        /* ���ǿ��Ҫ����ISR���ͷ��ʼ� */
        if (option &IPC_OPT_ISR)
        {
            /* �жϳ���ֻ���Է�������ʽ�������ж�ȡ�ʼ�,������ʱ�������̵߳�������
               ���ж���,��ǰ�߳�δ������߾������ȼ��߳�,Ҳδ�ش����ں˾����̶߳��С�
               �����ڴ˴��õ���HiRP������κ����塣*/
            KNL_ASSERT((uKernelVariable.State == eIntrState), "");
            state = TrySendMail(pMailbox, (void**)pMail2, &HiRP, &error);
        }
        else
        {
            /* �Զ��ж���η����ʼ� */
            state = SendMail(pMailbox, pMail2, option, timeo, &imask, &error);
        }
    }
    CpuLeaveCritical(imask);

    *pErrno = error;
    return state;
}

/*************************************************************************************************
 *  ����: ���������������                                                                       *
 *  ����: (1) pMailbox  ����ṹ��ַ                                                             *
 *        (2) pError    ��ϸ���ý��                                                             *
 *  ����: (1) eFailure  ����ʧ��                                                                 *
 *        (2) eSuccess  �����ɹ�                                                                 *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xMailboxReset(TMailBox* pMailbox, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);
    if (pMailbox->Property &IPC_PROP_READY)
    {
        /* �����������ϵ����еȴ��̶߳��ͷ�,�����̵߳ĵȴ��������IPC_ERR_RESET */
        uIpcUnblockAll(&(pMailbox->Queue), eFailure, IPC_ERR_RESET, (void**)0, &HiRP);

        /* ���������״̬Ϊ��,��������е��ʼ� */
        pMailbox->Property &= IPC_RESET_MBOX_PROP;
        pMailbox->Status = eMailBoxEmpty;
        pMailbox->Mail = (TMail*)0;

        /* ���Է����߳���ռ */
        uThreadPreempt(HiRP);

        error = IPC_ERR_NONE;
        state = eSuccess;
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}



/*************************************************************************************************
 *  ���ܣ�����������ֹ����,��ָ�����̴߳�����Ķ�������������ֹ����������                        *
 *  ����: (1) pMailbox ����ṹ��ַ                                                              *
 *        (2) pThread  �߳̽ṹ��ַ                                                              *
 *        (3) option   ����ѡ��                                                                  *
 *        (4) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵��?                                                                                        *
 *************************************************************************************************/
TState xMailBoxFlush(TMailBox* pMailbox, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pMailbox->Property &IPC_PROP_READY)
    {
        /* ���������������ϵ����еȴ��̶߳��ͷţ������̵߳ĵȴ��������TCLE_IPC_FLUSH  */
        uIpcUnblockAll(&(pMailbox->Queue), eFailure, IPC_ERR_FLUSH, (void**)0, &HiRP);

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
 *  ���ܣ�����㲥����,�����ж����������е��̹߳㲥�ʼ�                                          *
 *  ����: (1) pMailbox  ����ṹ��ַ                                                             *
 *        (2) pMail2    �����ʼ��ṹ��ַ��ָ�����                                               *
 *        (3) pError    ��ϸ���ý��                                                             *
 *  ����: (1) eFailure  ����ʧ��                                                                 *
 *        (2) eSuccess  �����ɹ�                                                                 *
 *  ˵����ֻ��������߳����������д��ڶ�������̵߳�ʱ��,���ܰ��ʼ����͸������е��߳�            *
 *************************************************************************************************/
TState xMailBoxBroadcast(TMailBox* pMailbox, TMail* pMail2, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pMailbox->Property &IPC_PROP_READY)
    {
        /* ֻ������ղ������̵߳ȴ���ȡ�ʼ���ʱ����ܽ��й㲥 */
        if (pMailbox->Status == eMailBoxEmpty)
        {
            uIpcUnblockAll(&(pMailbox->Queue), eSuccess, IPC_ERR_NONE, (void**)pMail2, &HiRP);

            /* ���Է����߳���ռ */
            uThreadPreempt(HiRP);

            error = IPC_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = IPC_ERR_INVALID_STATUS;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ���ʼ������                                                                             *
 *  ������(1) pMailbox   ����ĵ�ַ                                                              *
 *        (2) property   ����ĳ�ʼ����                                                          *
 *        (3) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eFailure   ����ʧ��                                                                *
 *        (2) eSuccess   �����ɹ�                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xMailBoxCreate(TMailBox* pMailbox, TProperty property, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    if (!(pMailbox->Property &IPC_PROP_READY))
    {
        property |= IPC_PROP_READY;
        pMailbox->Property = property;
        pMailbox->Status = eMailBoxEmpty;
        pMailbox->Mail = (void*)0;

        pMailbox->Queue.PrimaryHandle   = (TObjNode*)0;
        pMailbox->Queue.AuxiliaryHandle = (TObjNode*)0;
        pMailbox->Queue.Property        = &(pMailbox->Property);

        error = IPC_ERR_NONE;
        state = eSuccess;
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ���������                                                                               *
 *  ������(1) pMailbox   ����ĵ�ַ                                                              *
 *        (2) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eFailure   ����ʧ��                                                                *
 *        (2) eSuccess   �����ɹ�                                                                *
 *  ˵����ע���̵߳ĵȴ��������IPC_ERR_DELETE                                                   *
 *************************************************************************************************/
TState xMailBoxDelete(TMailBox* pMailbox, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pMailbox->Property &IPC_PROP_READY)
    {
        /* ���������������ϵ����еȴ��̶߳��ͷ�,�����̵߳ĵȴ��������IPC_ERR_DELETE  */
        uIpcUnblockAll(&(pMailbox->Queue), eFailure, IPC_ERR_DELETE, (void**)0, &HiRP);

        /* �����������ȫ������ */
        memset(pMailbox, 0U, sizeof(TMailBox));

        /* ���Է����߳���ռ */
        uThreadPreempt(HiRP);

        error = IPC_ERR_NONE;
        state = eSuccess;
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


#endif

