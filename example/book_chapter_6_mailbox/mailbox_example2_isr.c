#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH6_MAILBOX_EXAMPLE2)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

/* �û��̶߳��� */
static TThread ThreadLed;

/* �û��߳�ջ���� */
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];


/* �û��ʼ����Ͷ��� */
typedef struct
{
    TIndex Index;
    TByte Value;
} TLedMail;


/* �û�������ʼ����� */
static TMailBox LedMailbox;
static TLedMail LedMail;

/* Led�̵߳������� */
static void ThreadLedEntry(TArgument data)
{
    TError error;
    TState state;

    TLedMail* pMail;
    while (eTrue)
    {
        /* Led�߳���������ʽ�����ʼ� */
        state = TclReceiveMail(&LedMailbox, (TMail*)(&pMail),
                               TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(pMail->Index, pMail->Value);
        }
    }
}

/* �����尴���жϴ����� */
static TBitMask EvbKeyISR(TArgument data)
{
    TState state;
    TLedMail* pMail = &LedMail;
    static int turn = 0;
    int keyid;

    keyid = EvbKeyScan();
    if (keyid)
    {
        LedMail.Index = keyid;
        /* �ʼ����ݽ���ON��OFF*/
        turn++;
        pMail->Value = (turn % 2) ? LED_ON : LED_OFF;

        /* Key ISR�Է�������ʽ�����ʼ� */
        state = TclIsrSendMail(&LedMailbox, (TMail*)(&pMail), 0);
        TCLM_ASSERT((state == eTrue), "");
    }

    return 0;
}

/* �û�Ӧ�ó�����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ���ú�KEY��ص��ⲿ�ж����� */
    state = TclSetIrqVector(KEY_IRQ_ID, &EvbKeyISR, (TThread*)0, (TArgument)0, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IRQ_NONE), "");

    /* ��ʼ������ */
    state = TclCreateMailBox(&LedMailbox, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ��ʼ��Led�豸�����߳� */
    state = TclCreateThread(&ThreadLed,
                          &ThreadLedEntry, (TArgument)(&LedMailbox),
                          ThreadLedStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�豸�����߳� */
    state = TclActivateThread(&ThreadLed, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
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
