/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "trochili.h"


/*************************************************************************************************
 *  ���ܣ��ں�����API                                                                            *
 *  ������(1) pUserEntry  Ӧ�ó�ʼ������                                                         *
 *        (2) pCpuEntry   ��������ʼ������                                                       *
 *        (3) pBoardEntry �弶�豸��ʼ������                                                     *
 *        (4) pTraceEntry �����������                                                           *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void TclStartKernel(TUserEntry pUserEntry,
                    TCpuSetupEntry pCpuEntry,
                    TBoardSetupEntry pBoardEntry,
                    TTraceEntry pTraceEntry)
{
    KNL_ASSERT((pUserEntry  != (TUserEntry)0), "");
    KNL_ASSERT((pCpuEntry   != (TCpuSetupEntry)0), "");
    KNL_ASSERT((pBoardEntry != (TBoardSetupEntry)0), "");
    KNL_ASSERT((pTraceEntry != (TTraceEntry)0), "");

    xKernelStart(pUserEntry, pCpuEntry, pBoardEntry, pTraceEntry);
}



/*************************************************************************************************
 *  ���ܣ�����ϵͳIdle������IDLE�̵߳���                                                         *
 *  ������(1) pEntry ϵͳIdle����                                                                *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void TclSetSysIdleEntry(TSysIdleEntry pEntry)
{
    KNL_ASSERT((pEntry != (TSysIdleEntry)0), "");
    xKernelSetIdleEntry(pEntry);
}


/*************************************************************************************************
 *  ���ܣ����ϵͳ��ǰ�߳�ָ��                                                                   *
 *  ������(1) pThread2 ���ص�ǰ�߳�ָ��                                                          *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void TclGetCurrentThread(TThread** pThread2)
{
    KNL_ASSERT((pThread2 != (TThread**)0), "");
    xKernelGetCurrentThread(pThread2);
}


/*************************************************************************************************
 *  ���ܣ����ϵͳ������ʱ�ӽ�����                                                               *
 *  ������(1) pJiffies ����ϵͳ�����е�ʱ�ӽ�����                                                *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void TclGetTimeJiffies(TTimeTick* pJiffies)
{
    KNL_ASSERT((pJiffies != (TTimeTick*)0), "");
    xKernelGetJiffies(pJiffies);
}


/*************************************************************************************************
 *  ���ܣ����ϵͳ������ʱ���                                                                   *
 *  ������(1) pStamp ��΢��Ϊ��λ��ʱ�����ֵ                                                    *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void TclGetTimeStamp(TTimeStamp* pStamp)
{
    TTimeTick jiffies;
    KNL_ASSERT((pStamp != (TTimeStamp*)0), "");

    xKernelGetJiffies(&jiffies);
    *pStamp = (jiffies * 1000U * 1000U) / TCLC_TIME_TICK_RATE;
}


#if (TCLC_IRQ_ENABLE)
/*************************************************************************************************
 *  ���ܣ������ж���������                                                                       *
 *  ������(1) irqn     �жϺ�                                                                    *
 *        (2) pISR     ISR������                                                               *
 *        (3) pASR     �жϴ����߳�                                                              *
 *        (4) data     Ӧ���ṩ�Ļص�����                                                        *
 *        (5) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclSetIrqVector(TIndex irqn, TISR pISR, TThread* pASR, TArgument data, TError* pError)
{
    TState state;
    KNL_ASSERT((irqn < TCLC_CPU_IRQ_NUM), "");
    KNL_ASSERT((pISR != (TISR)0), "");

    state = xIrqSetVector(irqn, pISR, pASR, data, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ�����ж�����API����                                                                    *
 *  ������(1) irqn �жϱ��                                                                      *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵�����ں�����ʱ������ñ�����                                                               *
 *************************************************************************************************/
TState TclCleanIrqVector(TIndex irqn, TError* pError)
{
    TState state;
    KNL_ASSERT((irqn < TCLC_CPU_IRQ_NUM), "");

    state = xIrqCleanVector(irqn, pError);
    return state;
}


#endif

/*************************************************************************************************
 *  ���ܣ�ʹ���̵߳���API                                                                        *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclUnlockScheduler(void)
{
    TState state;
    state = xKernelUnlockSched();
    return state;
}


/*************************************************************************************************
 *  ���ܣ���ֹ�̵߳���API                                                                        *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclLockScheduler(void)
{
    TState state;
    state = xKernelLockSched();
    return state;
}

/*************************************************************************************************
 *  ���ܣ��弶�ַ�����ӡ����                                                                     *
 *  ������(1) pStr ����ӡ���ַ���                                                                *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void TclTrace(const char* pNote)
{
    /* ��Ҫ�Ĳ������ */
    KNL_ASSERT((pNote != (char*)0), "");

    xKernelTrace(pNote);
}

/*************************************************************************************************
 *  ���ܣ��߳̽ṹ��ʼ��API                                                                      *
 *  ������(1) pThread  �߳̽ṹ��ַ                                                              *
 *        (2) pEntry   �̺߳�����ַ                                                              *
 *        (3) argument �̲߳�����ַ(��������ʾ)                                                  *
 *        (4) pStack   �߳�ջ��ַ                                                                *
 *        (5) bytes    �߳�ջ��С�����ֽ�Ϊ��λ                                                  *
 *        (6) priority �߳����ȼ�                                                                *
 *        (7) ticks    �߳�ʱ��Ƭ����                                                            *
 *        (8) pError   ��ϸ���ý��                                                              *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclCreateThread(TThread* pThread,
                       TThreadEntry pEntry,
                       TArgument    data,
                       void*        pStack,
                       TBase32        bytes,
                       TPriority    priority,
                       TTimeTick    ticks,
                       TError*      pError)
{
    TState state;

    /* ��Ҫ�Ĳ������ */
    KNL_ASSERT((pThread != (TThread*)0), "");
    KNL_ASSERT((pEntry != (void*)0), "");
    KNL_ASSERT((pStack != (void*)0), "");
    KNL_ASSERT((bytes > 0U), "");
    KNL_ASSERT((priority <= TCLC_USER_PRIORITY_LOW), "");
    KNL_ASSERT((priority >= TCLC_USER_PRIORITY_HIGH), "");
    KNL_ASSERT((ticks > 0U), "");

    state = xThreadCreate(pThread,
                          eThreadDormant,
                          THREAD_PROP_PRIORITY_SAFE,
                          THREAD_ACAPI_ALL,
                          pEntry,
                          data,
                          pStack,
                          bytes,
                          priority,
                          ticks,
                          pError);

    return state;
}

#if (TCLC_IRQ_ENABLE)
/*************************************************************************************************
 *  ���ܣ��߳̽ṹ��ʼ��API                                                                      *
 *  ������(1) pThread  �߳̽ṹ��ַ                                                              *
 *        (2) pEntry   �̺߳�����ַ                                                              *
 *        (3) pArg     �̲߳�����ַ                                                              *
 *        (4) pStack   �߳�ջ��ַ                                                                *
 *        (5) bytes    �߳�ջ��С�����ֽ�Ϊ��λ                                                  *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclCreateAsyISR(TThread* pThread,
                       TThreadEntry pEntry,
                       TBase32 argument,
                       void* pStack,
                       TBase32 bytes,
                       TError* pError)
{
    TState state;

    /* ��Ҫ�Ĳ������ */
    KNL_ASSERT((pThread != (TThread*)0), "");
    KNL_ASSERT((pEntry != (void*)0), "");
    KNL_ASSERT((pStack != (void*)0), "");
    KNL_ASSERT((bytes > 0U), "");

    state = xThreadCreate(pThread,
                          eThreadSuspended,
                          THREAD_PROP_RUNASR | THREAD_PROP_PRIORITY_FIXED,
                          THREAD_ACAPI_ASR,
                          pEntry,
                          argument,
                          pStack,
                          bytes,
                          TCLC_IRQ_ASR_PRIORITY,
                          TCLC_IRQ_ASR_SLICE,
                          pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ���ǰ�߳�ע��                                                                           *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵������ʼ���̺߳Ͷ�ʱ���̲߳��ܱ�����                                                       *
 *************************************************************************************************/
TState TclDeleteAsyISR(TThread* pThread, TError* pError)
{
    TState state;

    /* ��Ҫ�Ĳ������ */
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadDelete(pThread, pError);
    return state;
}


#endif

/*************************************************************************************************
 *  ���ܣ���ǰ�߳�ע��                                                                           *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵������ʼ���̺߳Ͷ�ʱ���̲߳��ܱ�����                                                       *
 *************************************************************************************************/
TState TclDeleteThread(TThread* pThread, TError* pError)
{
    TState state;

    /* ��Ҫ�Ĳ������ */
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadDelete(pThread, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ��̼߳���API                                                                            *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *  ���أ��ο��߳���ط���ֵ����                                                                 *
 *  ˵���������̣߳�ʹ���߳��ܹ������ں˵���                                                     *
 *************************************************************************************************/
TState TclActivateThread(TThread* pThread, TError* pError)
{
    TState state;
    KNL_ASSERT((pThread != (TThread*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadActivate(pThread, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳�����API                                                                            *
 *  ������(1) thread �߳̽ṹ��ַ                                                                *
 *  ���أ��ο��߳���ط���ֵ����                                                                 *
 *  ˵������ʼ���̺߳Ͷ�ʱ���̲߳��ܱ�����                                                       *
 *************************************************************************************************/
TState TclDeactivateThread(TThread* pThread, TError* pError)
{
    TState state;
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadDeactivate(pThread, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ��̹߳���API                                                                            *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *  ���أ��ο��߳���ط���ֵ����                                                                 *
 *  ˵�����ں˳�ʼ���̲߳��ܱ�����                                                               *
 *************************************************************************************************/
TState TclSuspendThread(TThread* pThread, TError* pError)
{
    TState state;
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadSuspend(pThread, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ��ָ̻߳�API                                                                            *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *  ���أ��ο��߳���ط���ֵ����                                                                 *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclResumeThread(TThread* pThread, TError* pError)
{
    TState state;
	KNL_ASSERT((pThread != (TThread*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadResume(pThread, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ��̼߳��̵߳���API                                                                      *
 *  ��������                                                                                     *
 *  ���أ��ο��߳���ط���ֵ����                                                                 *
 *  ˵��������ں˹ر�ʱ����ȣ�ֻʹ�ñ����������ȵĻ����Ϳ���ʵ��Э��ʽ��������ں�             *
 *************************************************************************************************/
TState TclYieldThread(TError* pError)
{
    TState state = eFailure;
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadYield(pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ������߳����ȼ�API                                                                      *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) priority   �߳����ȼ�                                                              *
 *  ���أ��ο��߳���ط���ֵ����                                                                 *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclSetThreadPriority(TThread* pThread, TPriority priority, TError* pError)
{
    TState state;
    KNL_ASSERT((priority < TCLC_PRIORITY_NUM), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadSetPriority(pThread, priority, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ��޸��߳�ʱ��Ƭ����API                                                                  *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) slice   �߳�ʱ��Ƭ����                                                             *
 *  ���أ��ο��߳���ط���ֵ����                                                                 *
 *  ˵�����´ε���ʱ�Ż���Ч                                                                     *
 *************************************************************************************************/
TState TclSetThreadSlice(TThread* pThread, TTimeTick ticks, TError* pError)
{
    TState state;
    KNL_ASSERT((ticks > 0U), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadSetTimeSlice(pThread, ticks, pError);
    return state;
}

#if (TCLC_IPC_ENABLE)
/*************************************************************************************************
 *  ���ܣ�ǿ�ƽ���߳�����API                                                                    *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *  ���أ��ο��߳���ط���ֵ����                                                                 *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclUnblockThread(TThread* pThread, TError* pError)
{
    TState state;
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadUnblock(pThread, pError);
    return state;
}
#endif

#if (TCLC_TIMER_ENABLE)
/*************************************************************************************************
 *  ���ܣ��߳���ʱģ��ӿں���                                                                   *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) ticks   ��Ҫ��ʱ�ĵδ���Ŀ                                                         *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclDelayThread(TThread* pThread, TTimeTick ticks, TError* pError)
{
    TState state;
    KNL_ASSERT((ticks > 0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadDelay(pThread, ticks, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳���ʱȡ��API                                                                        *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *  ���أ��ο��߳���ط���ֵ����                                                                 *
 *  ˵���������������ʱ�޵ȴ���ʽ������IPC�߳����������ϵ��߳���Ч                              *
 *************************************************************************************************/
TState TclUnDelayThread(TThread* pThread, TError* pError)
{
    TState state;
    KNL_ASSERT((pThread != (TThread*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadUndelay(pThread, pError);
    return state;
}
#endif


#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_SEMAPHORE_ENABLE))
/*************************************************************************************************
 *  ����: ��ʼ�������ź���                                                                       *
 *  ����: (1) pSemaphore �����ź����ṹ��ַ                                                      *
 *        (2) value      �����ź�����ʼֵ                                                        *
 *        (3) mvalue     �����ź���������ֵ                                                    *
 *        (4) property   �ź����ĳ�ʼ����                                                        *
 *        (5) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclCreateSemaphore(TSemaphore* pSemaphore, TBase32 value, TBase32 mvalue,
                          TProperty property, TError* pError)
{
    TState state;
    KNL_ASSERT((pSemaphore != (TSemaphore*)0), "");
    KNL_ASSERT((mvalue >= 1U), "");
    KNL_ASSERT((mvalue >= value), "");
    KNL_ASSERT((pError != (TError*)0), "");

    property &= IPC_VALID_SEMN_PROP;
    state = xSemaphoreCreate(pSemaphore, value, mvalue, property, pError);
    return state;
}


/*************************************************************************************************
 *  ����: �߳�/ISR ����ź�����û����ź���                                                      *
 *  ����: (1) pSemaphore �ź����ṹ��ַ                                                          *
 *        (2) option     �����ź����ĵ�ģʽ                                                      *
 *        (3) timeo      ʱ������ģʽ�·����ź�����ʱ�޳���                                      *
 *        (5) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclObtainSemaphore(TSemaphore* pSemaphore, TOption option, TTimeTick timeo,
                          TError* pError)
{
    TState state;

    KNL_ASSERT((pSemaphore != (TSemaphore*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    /* ��������ѡ����β���Ҫ֧�ֵ�ѡ�� */
    option &= IPC_VALID_SEMN_OPT;

    /* ȡ��ISR��ǣ��Զ��жϵ��÷�ʽ */
    option &= ~IPC_OPT_ISR;
    state = xSemaphoreObtain(pSemaphore, option, timeo, pError);

    return state;
}


/*************************************************************************************************
 *  ����: �߳�/ISR�����ͷ��ź���                                                                 *
 *  ����: (1) pSemaphore �ź����ṹ��ַ                                                          *
 *        (2) option     �߳��ͷ��ź����ķ�ʽ                                                    *
 *        (3) timeo      �߳��ͷ��ź�����ʱ��                                                    *
 *        (4) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclReleaseSemaphore(TSemaphore* pSemaphore, TOption option, TTimeTick timeo, TError* pError)
{
    TState state;
    KNL_ASSERT((pSemaphore != (TSemaphore*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    /* ��������ѡ����β���Ҫ֧�ֵ�ѡ�� */
    option &= IPC_VALID_SEMN_OPT;

    /* ȡ��ISR��ǣ��Զ��жϵ��÷�ʽ */
    option &= ~IPC_OPT_ISR;
    state = xSemaphoreRelease(pSemaphore, option, timeo, pError);
    return state;
}


/*************************************************************************************************
 *  ����: ISR�ͷ��ź���                                                                          *
 *  ����: (1) pSemaphore �ź����ṹ��ַ                                                          *
 *        (2) option     �߳��ͷ��ź����ķ�ʽ                                                    *
 *        (3) timeo      �߳��ͷ��ź�����ʱ��                                                    *
 *        (4) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclIsrReleaseSemaphore(TSemaphore* pSemaphore)
{
    TState state;
    TError error;
    KNL_ASSERT((pSemaphore != (TSemaphore*)0), "");

    /* ����ISR��ǣ�ǿ��ָ�����÷�ʽ */
    state = xSemaphoreRelease(pSemaphore, IPC_OPT_ISR, 0U, &error);
    return state;
}


/*************************************************************************************************
 *  ����: ���ü����ź���                                                                         *
 *  ����: (1) pSemaphore �ź����ṹ��ַ                                                          *
 *        (2) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclDeleteSemaphore(TSemaphore* pSemaphore, TError* pError)
{
    TState state;
    KNL_ASSERT((pSemaphore != (TSemaphore*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xSemaphoreDelete(pSemaphore, pError);
    return state;
}


/*************************************************************************************************
 *  ����: ���ü����ź���                                                                         *
 *  ����: (1) pSemaphore �ź����ṹ��ַ                                                          *
 *        (2) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclResetSemaphore(TSemaphore* pSemaphore, TError* pError)
{
    TState state;
    KNL_ASSERT((pSemaphore != (TSemaphore*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xSemaphoreReset(pSemaphore, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ��ź���������ֹ����,��ָ�����̴߳��ź������߳�������������ֹ����������                  *
 *  ������(1) pSemaphore �ź����ṹ��ַ                                                          *
 *        (2) option     ����ѡ��                                                                *
 *        (3) pThread    �̵߳�ַ                                                                *
 *        (4) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclFlushSemaphore(TSemaphore* pSemaphore, TError* pError)
{
    TState state;
    KNL_ASSERT((pSemaphore != (TSemaphore*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xSemaphoreFlush(pSemaphore, pError);
    return state;
}
#endif


#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MUTEX_ENABLE))
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
TState TclCreateMutex(TMutex* pMutex, TPriority priority, TProperty property, TError* pError)
{
    TState state;
    KNL_ASSERT((pMutex != (TMutex*)0), "");
    KNL_ASSERT((priority < TCLC_LOWEST_PRIORITY), "");
    KNL_ASSERT((pError != (TError*)0), "");

    property &= IPC_VALID_MUTEX_PROP;
    state = xMutexCreate(pMutex, priority, property, pError);
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
TState TclDeleteMutex(TMutex* pMutex, TError* pError)
{
    TState state;
    KNL_ASSERT((pMutex != (TMutex*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMutexDelete(pMutex, pError);
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
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclLockMutex(TMutex* pMutex, TOption option, TTimeTick timeo, TError* pError)
{
    TState state;
    KNL_ASSERT((pMutex != (TMutex*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    /* ��������ѡ����β���Ҫ֧�ֵ�ѡ�� */
    option &= IPC_VALID_MUTEX_OPT;
    /* option &= ~IPC_OPT_ISR; */
    state = xMutexLock(pMutex, option, timeo, pError);
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
TState TclFreeMutex(TMutex* pMutex, TError* pError)
{
    TState state;
    KNL_ASSERT((pMutex != (TMutex*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMutexFree(pMutex, pError);
    return state;
}


/*************************************************************************************************
 *  ����: �����������������                                                                     *
 *  ����: (1) pMutex   �������ṹ��ַ                                                            *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclResetMutex(TMutex* pMutex, TError* pError)
{
    TState state;
    KNL_ASSERT((pMutex != (TMutex*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMutexReset(pMutex, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ�������������ֹ����,��ָ�����̴߳ӻ��������߳�������������ֹ����������                  *
 *  ������(1) pMutex  �������ṹ��ַ                                                             *
 *        (2) option     ����ѡ��                                                                *
 *        (3) pThread �̵߳�ַ                                                                   *
 *        (4) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eSuccess                                                                           *
 *        (2) eFailure                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclFlushMutex(TMutex* pMutex, TError* pError)
{
    TState state;
    KNL_ASSERT((pMutex != (TMutex*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMutexFlush(pMutex, pError);
    return state;
}
#endif


#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MAILBOX_ENABLE))
/*************************************************************************************************
 *  ���ܣ���ʼ������                                                                             *
 *  ������(1) pMailbox   ����ĵ�ַ                                                              *
 *        (2) property   ����ĳ�ʼ����                                                          *
 *        (3) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eFailure   ����ʧ��                                                                *
 *        (2) eSuccess   �����ɹ�                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclCreateMailBox(TMailBox* pMailbox, TProperty property, TError* pError)
{
    TState state;
    KNL_ASSERT((pMailbox != (TMailBox*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    property &= IPC_VALID_MBOX_PROP;
    state = xMailBoxCreate(pMailbox, property, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ���������                                                                               *
 *  ������(1) pMailbox   ����ĵ�ַ                                                              *
 *        (2) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eFailure   ����ʧ��                                                                *
 *        (2) eSuccess   �����ɹ�                                                                *
 *  ˵����ע���̵߳ĵȴ��������TCLE_IPC_DELETE                                                   *
 *************************************************************************************************/
TState TclDeleteMailBox(TMailBox* pMailbox, TError* pError)
{
    TState state;
    KNL_ASSERT((pMailbox != (TMailBox*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMailBoxDelete(pMailbox, pError);
    return state;
}


/*************************************************************************************************
 *  ����: �߳�/ISR�������н����ʼ�                                                               *
 *  ����: (1) pMailbox ����ṹ��ַ                                                              *
 *        (2) pMail2   �����ʼ��ṹ��ַ��ָ�����                                                *
 *        (3) option   ���������ģʽ                                                            *
 *        (4) timeo    ʱ������ģʽ�·��������ʱ�޳���                                          *
 *        (5) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclReceiveMail(TMailBox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo,
                      TError* pError)
{
    TState state;
    KNL_ASSERT((pMailbox != (TMailBox*)0), "");
    KNL_ASSERT((pMail2 != (TMail*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    /* ��������ѡ����β���Ҫ֧�ֵ�ѡ�� */
    option &= IPC_VALID_MBOX_OPT;

    /* ȡ��ISR��ǣ��Զ��жϵ��÷�ʽ */
    option &= ~IPC_OPT_ISR;
    state = xMailBoxReceive(pMailbox, pMail2, option, timeo, pError);
    return state;
}


/*************************************************************************************************
 *  ����: �߳�/ISR�����ʼ�                                                                       *
 *  ����: (1) pMailbox ����ṹ��ַ                                                              *
 *        (2) pMail2   �����ʼ��ṹ��ַ��ָ�����                                                *
 *        (3) option   ���������ģʽ                                                            *
 *        (4) timeo    ʱ������ģʽ�·��������ʱ�޳���                                          *
 *        (5) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclSendMail(TMailBox* pMailbox, TMail* pMail2, TOption option,
                   TTimeTick timeo, TError* pError)
{
    TState state;

    KNL_ASSERT((pMailbox != (TMailBox*)0), "");
    KNL_ASSERT((pMail2 != (TMail*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");
    if ((pMailbox != (TMailBox*)0) &&  (pMail2 != (TMail*)0) && (pError != (TError*)0))
    {
        /* ��������ѡ����β���Ҫ֧�ֵ�ѡ�� */
        option &= IPC_VALID_MBOX_OPT;

        /* ȡ��ISR��ǣ��Զ��жϵ��÷�ʽ */
        option &= ~IPC_OPT_ISR;
        state = xMailBoxSend(pMailbox, pMail2, option, timeo, pError);
    }

    return state;
}


/*************************************************************************************************
 *  ���ܣ�ISR�����䷢���ʼ�                                                                      *
 *  ����: (1) pMailbox ����ṹ��ַ                                                              *
 *        (2) pMail2   �����ʼ��ṹ��ַ��ָ�����                                                *
 *        (3) option   ���������ģʽ                                                            *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclIsrSendMail(TMailBox* pMailbox, TMail* pMail2, TOption option)
{
    TState state;
    TError error;
    KNL_ASSERT((pMailbox != (TMailBox*)0), "");
    KNL_ASSERT((pMail2 != (TMail*)0), "");

    /* ��������ѡ����β���Ҫ֧�ֵ�ѡ�� */
    option &= IPC_VALID_MBOX_OPT;

    /* ����ISR��ǣ�ǿ��ָ�����÷�ʽ */
    option |= IPC_OPT_ISR;
    state = xMailBoxSend(pMailbox, pMail2, option, 0U, &error);
    return state;
}


/*************************************************************************************************
 *  ���ܣ�����㲥����,�����ж����������е��̹߳㲥�ʼ�                                          *
 *  ����: (1) pMailbox  ����ṹ��ַ                                                             *
 *        (2) pMail2   �����ʼ��ṹ��ַ��ָ�����                                                *
 *        (3) pError    ��ϸ���ý��                                                             *
 *  ���أ�(1) eSuccess  �ɹ�                                                                     *
 *        (2) eFailure  ʧ��                                                                     *
 *  ˵����ֻ��������߳����������д��ڶ�������̵߳�ʱ�򣬲��ܰ���Ϣ���͸������е��߳�           *
 *************************************************************************************************/
TState TclBroadcastMail(TMailBox* pMailbox, TMail* pMail2, TError* pError)
{
    TState state;
    KNL_ASSERT((pMailbox != (TMailBox*)0), "");
    KNL_ASSERT((pMail2 != (TMail*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMailBoxBroadcast(pMailbox, pMail2, pError);
    return state;
}


/*************************************************************************************************
 *  ����: ���������������                                                                       *
 *  ����: (1) pMailbox ����ṹ��ַ                                                              *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclResetMailBox(TMailBox* pMailbox, TError* pError)
{
    TState state;
    KNL_ASSERT((pMailbox != (TMailBox*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMailboxReset(pMailbox, pError);
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
TState TclFlushMailBox(TMailBox* pMailbox, TError* pError)
{
    TState state;
    KNL_ASSERT((pMailbox != (TMailBox*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMailBoxFlush(pMailbox, pError);
    return state;
}
#endif


#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MQUE_ENABLE))
/*************************************************************************************************
 *  ���ܣ���Ϣ���г�ʼ������                                                                     *
 *  ���룺(1) pMsgQue   ��Ϣ���нṹ��ַ                                                         *
 *        (2) pPool2    ��Ϣ�����ַ                                                             *
 *        (3) capacity  ��Ϣ��������������Ϣ�����С                                             *
 *        (4) policy    ��Ϣ�����̵߳��Ȳ���                                                     *
 *        (5) pError    ��ϸ���ý��                                                             *
 *  ���أ�(1) eSuccess  �����ɹ�                                                                 *
 *        (2) eFailure  ����ʧ��                                                                 *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclCreateMsgQueue(TMsgQueue* pMsgQue, void** pPool2, TBase32 capacity, TProperty property,
                         TError* pError)
{
    TState state;
    KNL_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    KNL_ASSERT((pPool2 != (void*)0), "");
    KNL_ASSERT((capacity != 0U), "");
    KNL_ASSERT((pError != (TError*)0), "");

    property &= IPC_VALID_MQUE_PROP;
    state = xMQCreate(pMsgQue, pPool2, capacity, property, pError);
    return state;
}


/*************************************************************************************************
 *  ����: �����߳�/ISR������Ϣ�����е���Ϣ                                                       *
 *  ����: (1) pMsgQue  ��Ϣ���нṹ��ַ                                                          *
 *        (2) pMsg2    ������Ϣ�ṹ��ַ��ָ�����                                                *
 *        (3) option   ������Ϣ���е�ģʽ                                                        *
 *        (4) timeo    ʱ������ģʽ�·��������ʱ�޳���                                          *
 *        (5) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure   ����ʧ��                                                                *
 *  ����: (2) eSuccess   �����ɹ�                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclReceiveMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TOption option, TTimeTick timeo,
                         TError* pError)
{
    TState state;
    KNL_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    KNL_ASSERT((pMsg2 != (TMessage*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    /* ��������ѡ����β���Ҫ֧�ֵ�ѡ�� */
    option &= IPC_VALID_MSGQ_OPT;

    /* ȡ��ISR��ǣ��Զ��жϵ��÷�ʽ */
    option &= ~IPC_OPT_ISR;
    state = xMQReceive(pMsgQue, pMsg2, option, timeo, pError);
    return state;
}


/*************************************************************************************************
 *  ����: �����߳�/ISR����Ϣ�����з�����Ϣ                                                       *
 *  ����: (1) pMsgQue  ��Ϣ���нṹ��ַ                                                          *
 *        (2) pMsg2    ������Ϣ�ṹ��ַ��ָ�����                                                *
 *        (3) option   ������Ϣ���е�ģʽ                                                        *
 *        (4) timeo    ʱ������ģʽ�·��������ʱ�޳���                                          *
 *        (5) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure   ����ʧ��                                                                *
 *  ����: (2) eSuccess   �����ɹ�                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclSendMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TOption option, TTimeTick timeo,
                      TError* pError)
{
    TState state;
    KNL_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    KNL_ASSERT((pMsg2 != (TMessage*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    /* ��������ѡ����β���Ҫ֧�ֵ�ѡ�� */
    option &= IPC_VALID_MSGQ_OPT;

    /* ȡ��ISR��ǣ��Զ��жϵ��÷�ʽ */
    option &= ~IPC_OPT_ISR;
    state = xMQSend(pMsgQue, pMsg2, option, timeo, pError);
    return state;
}


/*************************************************************************************************
 *  ����: ����ISR����Ϣ�����з�����Ϣ                                                            *
 *  ����: (1) pMsgQue  ��Ϣ���нṹ��ַ                                                          *
 *        (2) pMsg2    ������Ϣ�ṹ��ַ��ָ�����                                                *
 *        (3) option   ������Ϣ���е�ģʽ                                                        *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclIsrSendMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TOption option)
{
    TState state;
    TError error;
    KNL_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    KNL_ASSERT((pMsg2 != (TMessage*)0), "");

    /* ��������ѡ����β���Ҫ֧�ֵ�ѡ�� */
    option &= IPC_VALID_MSGQ_OPT;

    /* ����ISR��ǣ�ǿ��ָ�����÷�ʽ */
    option |= IPC_OPT_ISR;
    state = xMQSend(pMsgQue, pMsg2, option, 0U, &error);
    return state;
}


/*************************************************************************************************
 *  ���ܣ���Ϣ���й㲥����,�����ж����������е��̹߳㲥��Ϣ                                      *
 *  ������(1) pMsgQue    ��Ϣ���нṹ��ַ                                                        *
 *        (2) pMsg2      ������Ϣ�ṹ��ַ��ָ�����                                              *
 *        (3) pError     ��ϸ���ý��                                                            *
 *  ���أ�(1) eSuccess   �ɹ��㲥������Ϣ                                                        *
 *        (2) eFailure   �㲥������Ϣʧ��                                                        *
 *  ˵����ֻ�ж����ж���Ϣ���е�ʱ�򣬲��ܰ���Ϣ���͸������е��߳�                               *
 *************************************************************************************************/
TState TclBroadcastMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TError* pError)
{
    TState state;
    KNL_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    KNL_ASSERT((pMsg2 != (TMessage*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMQBroadcast(pMsgQue, pMsg2, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ���Ϣ�������ú���                                                                       *
 *  ���룺(1) pMsgQue   ��Ϣ���нṹ��ַ                                                         *
 *        (2) pError    ��ϸ���ý��                                                             *
 *  ���أ�(1) eSuccess  �����ɹ�                                                                 *
 *        (2) eFailure  ����ʧ��                                                                 *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclDeleteMsgQueue(TMsgQueue* pMsgQue, TError* pError)
{
    TState state;
    KNL_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMQDelete(pMsgQue, pError);
    return state;
}


/*************************************************************************************************
 *  ����: �����Ϣ������������                                                                   *
 *  ����: (1) pMsgQue  ��Ϣ���нṹ��ַ                                                          *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclResetMsgQueue(TMsgQueue* pMsgQue, TError* pError)
{
    TState state;
    KNL_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMQReset(pMsgQue, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ���Ϣ����������ֹ����,��ָ�����̴߳���Ϣ���е�������������ֹ����������                  *
 *  ������(1) pMsgQue  ��Ϣ���нṹ��ַ                                                          *
 *        (2) option   ����ѡ��                                                                  *
 *        (3) pThread  �̵߳�ַ                                                                  *
 *        (4) pError   ��ϸ���ý��                                                              *
 *  ���أ�(1) eSuccess �ɹ�                                                                      *
 *        (2) eFailure ʧ��                                                                      *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclFlushMsgQueue(TMsgQueue* pMsgQue, TError* pError)
{
    TState state;
    KNL_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMQFlush(pMsgQue, pError);
    return state;
}
#endif


#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_FLAGS_ENABLE))
/*************************************************************************************************
 *  ���ܣ���ʼ���¼����                                                                         *
 *  ������(1) pFlags     �¼���ǵĵ�ַ                                                          *
 *        (2) property   �¼���ǵĳ�ʼ����                                                      *
 *        (3) pError     ����������ϸ����ֵ                                                      *
 *  ����: (1) eFailure   ����ʧ��                                                                *
 *        (2) eSuccess   �����ɹ�                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclCreateFlags(TFlags* pFlags, TProperty property, TError* pError)
{
    TState state;
    KNL_ASSERT((pFlags != (TFlags*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    property &= IPC_VALID_FLAG_PROP;
    state = xFlagsCreate(pFlags, property, pError);
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
TState TclDeleteFlags(TFlags* pFlags, TError* pError)
{
    TState state;
    KNL_ASSERT((pFlags != (TFlags*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xFlagsDelete(pFlags, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳�/ISR���¼���Ƿ����¼�                                                             *
 *  ������(1) pFlags   �¼���ǵĵ�ַ                                                            *
 *        (2) pPattern ��Ҫ���յı�ǵ����                                                      *
 *        (3) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure   ����ʧ��                                                                *
 *        (2) eSuccess   �����ɹ�                                                                *
 *  ˵������������������ǰ�߳�����                                                             *
 *************************************************************************************************/
TState TclSendFlags(TFlags* pFlags, TBitMask pattern, TError* pError)
{
    TState state;
    KNL_ASSERT((pFlags != (TFlags*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xFlagsSend(pFlags, pattern, pError);
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
TState TclReceiveFlags(TFlags* pFlags, TBitMask* pPattern, TOption option, TTimeTick timeo,
                       TError* pError)
{
    TState state;

    KNL_ASSERT((pFlags != (TFlags*)0), "");
    KNL_ASSERT((option & (IPC_OPT_AND | IPC_OPT_OR)) != 0U, "");
    KNL_ASSERT((pError != (TError*)0), "");

    /* ��������ѡ����β���Ҫ֧�ֵ�ѡ�� */
    option &= IPC_VALID_FLAG_OPT;

    /* ȡ��ISR��ǣ��Զ��жϵ��û��� */
    option &= ~IPC_OPT_ISR;
    state = xFlagsReceive(pFlags, pPattern, option, timeo, pError);
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
TState TclResetFlags(TFlags* pFlags, TError* pError)
{
    TState state;

    KNL_ASSERT((pFlags != (TFlags*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xFlagsReset(pFlags, pError);
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
TState TclFlushFlags(TFlags* pFlags, TError* pError)
{
    TState state = eFailure;
    KNL_ASSERT((pFlags != (TFlags*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xFlagsFlush(pFlags, pError);
    return state;
}
#endif


#if (TCLC_TIMER_ENABLE)
/*************************************************************************************************
 *  ���ܣ��û���ʱ����ʼ������                                                                   *
 *  ������(1) pTimer   ��ʱ����ַ                                                                *
 *        (2) property ��ʱ������                                                                *
 *        (3) ticks    ��ʱ���δ���Ŀ                                                            *
 *        (4) pRoutine �û���ʱ���ص�����                                                        *
 *        (5) pData    �û���ʱ���ص���������                                                    *
 *        (6) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵��                                                                                         *
 *************************************************************************************************/
TState TclCreateTimer(TTimer* pTimer, TProperty property, TTimeTick ticks,
                      TTimerRoutine pRoutine, TArgument data, TError* pError)
{
    TState state;
    KNL_ASSERT((pTimer != (TTimer*)0), "");
    KNL_ASSERT((pRoutine != (TTimerRoutine)0), "");
	
    state = xTimerCreate(pTimer, property, ticks, pRoutine, data, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ��ں˶�ʱ��ȡ����ʼ��                                                                   *
 *  ������(1) pTimer   ��ʱ���ṹ��ַ                                                            *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *        (3) pError   ��ϸ���ý��                                                              *
 *  ˵��                                                                                         *
 *************************************************************************************************/
TState TclDeleteTimer(TTimer* pTimer, TError* pError)
{
    TState state;
    KNL_ASSERT((pTimer != (TTimer*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xTimerDelete(pTimer, pError);
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
TState TclStartTimer(TTimer* pTimer, TTimeTick lagticks, TError* pError)
{
    TState state;
    KNL_ASSERT((pTimer != (TTimer*)0), "");

    state = xTimerStart(pTimer, lagticks, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ�ֹͣ�û���ʱ������                                                                     *
 *  ������(1) pTimer   ��ʱ����ַ                                                                *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *        (3) pError   ��ϸ���ý��                                                              *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclStopTimer(TTimer* pTimer, TError* pError)
{
    TState state;
    KNL_ASSERT((pTimer != (TTimer*)0), "");

    state = xTimerStop(pTimer, pError);
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
TState TclConfigTimer(TTimer* pTimer, TTimeTick ticks, TError* pError)
{
    TState state;
    KNL_ASSERT((pTimer != (TTimer*)0), "");
    KNL_ASSERT((ticks > 0U), "");

    state = xTimerConfig(pTimer, ticks, pError);
    return state;
}
#endif


#if ((TCLC_IRQ_ENABLE) && (TCLC_IRQ_DAEMON_ENABLE))
/*************************************************************************************************
 *  ���ܣ��ύ�ж�����                                                                           *
 *  ������(1) pIRQ      �ж�����ṹ��ַ                                                         *
 *        (2) priority  �ж��������ȼ�                                                           *
 *        (3) pEntry    �жϴ���ص�����                                                         *
 *        (4) data      �жϴ���ص�����                                                         *
 *  ����: (1) eFailure  ����ʧ��                                                                 *
 *        (2) eSuccess  �����ɹ�                                                                 *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclPostIRQ(TIrq* pIRQ, TPriority priority, TIrqEntry pEntry, TArgument data, TError* pError)
{
    TState state;
    KNL_ASSERT((pIRQ != (TIrq*)0), "");

    state = xIrqPostRequest(pIRQ, priority, pEntry, data, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ������ж�����                                                                           *
 *  ������(1) TIrq      �ж�����ṹ��ַ                                                         *
 *  ����: (1) eFailure  ����ʧ��                                                                 *
 *        (2) eSuccess  �����ɹ�                                                                 *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclCancelIRQ(TIrq* pIRQ, TError* pError)
{
    TState state;
    KNL_ASSERT((pIRQ != (TIrq*)0), "");

    state = xIrqCancelRequest(pIRQ, pError);
    return state;
}
#endif


#if ((TCLC_MEMORY_ENABLE) && (TCLC_MEMORY_POOL_ENABLE))
/*************************************************************************************************
 *  ����: ��ʼ���ڴ�ҳ��                                                                         *
 *  ����: (1) pPool      �ڴ�ҳ�ؽṹ��ַ                                                        *
 *        (2) pAddr      �ڴ����������ַ                                                        *
 *        (3) pages      �ڴ�����ڴ�ҳ��Ŀ                                                      *
 *        (4) pgsize     �ڴ�ҳ��С                                                              *
 *        (5) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclCreateMemoryPool(TMemPool* pPool, void* pAddr, TBase32 pages, TBase32 pgsize, TError* pError)
{
    TState state;
    KNL_ASSERT((pPool != (TMemPool*)0), "");
    KNL_ASSERT((pAddr != (void*)0), "");
    KNL_ASSERT((pages != 0U), "");
    KNL_ASSERT((pgsize > 0U), "");
    KNL_ASSERT((pages <= TCLC_MEMORY_POOL_PAGES), "");
    KNL_ASSERT((pError != (TError*)0), "");


    state = xMemPoolCreate(pPool, pAddr, pages, pgsize, pError);
    return state;
}


/*************************************************************************************************
 *  ����: �����ڴ��                                                                             *
 *  ����: (1) pPool      �ڴ�ؽṹ��ַ                                                          *
 *        (2) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclDeleteMemoryPool(TMemPool* pPool, TError* pError)
{
    TState state;
    KNL_ASSERT((pPool != (TMemPool*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMemPoolDelete(pPool, pError);
    return state;
}


/*************************************************************************************************
 *  ����: ���ڴ������������ڴ�                                                                 *
 *  ����: (1) pPool      �ڴ�ؽṹ��ַ                                                          *
 *        (2) pObject2   �������뵽���ڴ��ָ�����                                              *
 *        (3) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclMallocPoolMemory(TMemPool* pPool, void** pAddr2, TError* pError)
{
    TState state;
    KNL_ASSERT((pPool != (TMemPool*)0), "");
    KNL_ASSERT((pAddr2 != (void**)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xPoolMemMalloc(pPool, pAddr2, pError);
    return state;
}


/*************************************************************************************************
 *  ����: ���ڴ�����ͷ��ڴ�                                                                     *
 *  ����: (1) pPool      �ڴ�ؽṹ��ַ                                                          *
 *        (2) pObject    ���ͷ��ڴ�ĵ�ַ                                                        *
 *        (3) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclFreePoolMemory(TMemPool* pPool, void* pAddr, TError* pError)
{
    TState state;
    KNL_ASSERT((pPool != (TMemPool*)0), "");
    KNL_ASSERT((pAddr != (void*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xPoolMemFree(pPool, pAddr, pError);
    return state;
}
#endif


#if ((TCLC_MEMORY_ENABLE) && (TCLC_MEMORY_BUDDY_ENABLE))
/*************************************************************************************************
 *  ���ܣ���ʼ������ڴ������ƽṹ                                                             *
 *  ������(1) pBuddy    ���ϵͳ�������ڴ��ַ                                                   *
 *        (2) pAddr     �ɹ�������ڴ��ַ                                                       *
 *        (3) pagesize  �ڴ�ҳ��С                                                               *
 *        (4) pages     �ɹ�������ڴ�ҳ����                                                     *
 *        (5) pError    ��ϸ���ý��                                                             *
 *  ����: (1) eSuccess  �����ɹ�                                                                 *
 *        (2) eFailure  ����ʧ��                                                                 *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclCreateMemBuddy(TMemBuddy* pBuddy, TChar* pAddr, TBase32 pages, TBase32 pagesize, TError* pError)
{
    TState state;
    KNL_ASSERT((pBuddy != (TMemBuddy*)0), "");
    KNL_ASSERT((pAddr != (TChar*)0), "");
    KNL_ASSERT((pages > 0U), "");
    KNL_ASSERT((pages <= TCLC_MEMORY_BUDDY_PAGES), "");
    KNL_ASSERT((pagesize > 0U), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xBuddyCreate(pBuddy, pAddr, pages, pagesize, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ����ٻ���ڴ������ƽṹ                                                               *
 *  ������(1) pBuddy     ���ϵͳ�������ڴ��ַ                                                  *
 *        (2) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclDeleteMemBuddy(TMemBuddy* pBuddy, TError* pError)
{
    TState state;
    KNL_ASSERT((pBuddy != (TMemBuddy*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xBuddyDelete(pBuddy, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ��ӻ��ϵͳ�������ڴ�                                                                   *
 *  ������(1) pBuddy    ���ϵͳ�����������ַ                                                   *
 *        (2) len       ��Ҫ������ڴ泤��                                                       *
 *        (3) pAddr2    ����õ����ڴ��ַָ��                                                   *
 *        (4) pError    ��ϸ���ý��                                                             *
 *  ����: (1) eSuccess  �����ɹ�                                                                 *
 *        (2) eFailure  ����ʧ��                                                                 *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclMallocBuddyMem(TMemBuddy* pBuddy, int len, void** pAddr2, TError* pError)
{
    TState state;
    KNL_ASSERT((pBuddy != (TMemBuddy*)0), "");
    KNL_ASSERT((len > 0U), "");
    KNL_ASSERT((pAddr2 != (void**)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xBuddyMemMalloc(pBuddy, len, pAddr2, pError);
    return state;
}


/*************************************************************************************************
 *  ���ܣ�����ϵͳ���ͷ��ڴ�                                                                   *
 *  ������(1) pBuddy    ���ϵͳ�����������ַ                                                   *
 *        (2) pAddr     ���ͷŵ��ڴ��ַ                                                         *
 *        (3) pError    ��ϸ���ý��                                                             *
 *  ����: (1) eSuccess  �����ɹ�                                                                 *
 *        (2) eFailure  ����ʧ��                                                                 *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclFreeBuddyMem(TMemBuddy* pBuddy,  void* pAddr, TError* pError)
{
    TState state;
    KNL_ASSERT((pBuddy != (TMemBuddy*)0), "");
    KNL_ASSERT((pAddr != (char*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xBuddyMemFree(pBuddy, pAddr, pError);
    return state;
}
#endif

