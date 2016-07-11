#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH10_IRQ_ASR_EXAMPLE)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

#define THREAD_KEY_ASR_STACK_BYTES    (512)
#define THREAD_KEY_ASR_PRIORITY       (5)
#define THREAD_KEY_ASR_SLICE          (20)

/* �û��߳�ջ���� */
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadKeyASRStack[THREAD_KEY_ASR_STACK_BYTES/4];

/* �û��̶߳��� */
static TThread ThreadLed;
static TThread ThreadKeyASR;

/* �û��ź������� */
static TSemaphore LedSemaphore;


/* �����尴���жϴ����� */
static TBitMask EvbKeyISR(TArgument data)
{
    if (EvbKeyScan())
    {
        return  TCLR_IRQ_ASR;
    }

    return TCLR_IRQ_DONE;
}


/* �����尴���жϴ����߳���������
   ������ִ����Ϻ��Զ����ں˹������Բ�������ѭ����
   �ȵ���һ���ж�ʱ���ٴα�ISR���  */
static void KeyAsyncIsrEntry(TArgument data)
{
    TState state;
    TError error;

    /* ��������ʽ�ͷ��ź��� */
    state = TclReleaseSemaphore((TSemaphore*)data, TCLO_IPC_DUMMY, 0, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");
}


/* Led�̵߳������� */
static void ThreadLedEntry(TArgument arg)
{
    TError error;
    TState state;

    while (eTrue)
    {
        /* Led�߳���������ʽ��ȡ�ź���������ɹ������Led */
        state = TclObtainSemaphore((TSemaphore*)arg, TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            TCLM_ASSERT((error == TCLE_IPC_NONE), "");
            EvbLedControl(LED2, LED_ON);
        }

        /* Led�߳���������ʽ��ȡ�ź���������ɹ���Ϩ��Led */
        state = TclObtainSemaphore((TSemaphore*)arg, TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            TCLM_ASSERT((error == TCLE_IPC_NONE), "");
            EvbLedControl(LED2, LED_OFF);
        }
    }
}


/* �û�Ӧ�ó�����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ��ʼ���ź��� */
    state = TclCreateSemaphore(&LedSemaphore, 0U, 1u, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ���ú�KEY��ص��ⲿ�ж����� */
    state = TclSetIrqVector(KEY_IRQ_ID, &EvbKeyISR, &ThreadKeyASR, (TArgument)0, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IRQ_NONE), "");

    /* ��ʼ��Led�߳� */
    state = TclCreateThread(&ThreadLed,
                          &ThreadLedEntry,
                          (TArgument)(&LedSemaphore),
                          ThreadLedStack,
                          THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY,
                          THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��KEY�첽�ж��̣߳����߳��Զ�������ڹ���״̬,����Ҫ�û�������� */
    state = TclCreateAsyISR(&ThreadKeyASR,
                            &KeyAsyncIsrEntry,
                            (TArgument)(&LedSemaphore),
                            ThreadKeyASRStack,
                            THREAD_KEY_ASR_STACK_BYTES,
                            &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�߳� */
    state = TclActivateThread(&ThreadLed, &error);
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
    TCLM_ASSERT((state == eSuccess), "");
}


/* ������BOOT֮������main�����������ṩ */
int main(void)
{
    /* ע������ں˺���,�����ں� */
    TclStartKernel(&AppSetupEntry,
                   &CpuSetupEntry,
                   &EvbSetupEntry,
                   &EvbTraceEntry);
    return 1;
}

#endif

