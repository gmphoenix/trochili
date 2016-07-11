#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH9_TIMER_CONFIG_EXAMPLE)

/* �����߳�ʱ��Ҫ�Ĳ��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (32U)

/* Led�߳̽ṹ��ջ */
static TThread ThreadLed;
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];

/* �û���ʱ���ṹ */
static TTimer Led1Timer;
static TTimer Led2Timer;

static TTimeTick ticks1 = 500;
static TTimeTick ticks2 = 500;

/* �û���ʱ��1�Ļص����������1�룬������Ϩ��Led1 */
static void BlinkLed1(TArgument data)
{
    static TIndex index = 0;
    if (index % 2)
    {
        EvbLedControl(LED1, LED_OFF);
    }
    else
    {
        EvbLedControl(LED1, LED_ON);
    }
    index++;
}

/* �û���ʱ��2�Ļص����������1�룬������Ϩ��Led2 */
static void BlinkLed2(TArgument data)
{
    static TIndex index = 0;
    if (index % 2)
    {
        EvbLedControl(LED2, LED_OFF);
    }
    else
    {
        EvbLedControl(LED2, LED_ON);
    }
    index++;
}


/* �����尴���жϴ����� */
static TBitMask EvbKeyISR(TArgument data)
{
    TState state;
    TError error;

    TBase32 keyid;

  	keyid = EvbKeyScan();
    if (keyid == 1)
    {
        /* �ر��û���ʱ�� */
        state = TclStopTimer(&Led1Timer, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        ticks1 *= 2;
        ticks1 = (ticks1 > 2000)? 2000:ticks1;

        /* �����û���ʱ�� */
        state = TclConfigTimer(&Led1Timer, TCLM_MLS2TICKS(ticks1), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        /* �����û���ʱ�� */
        state = TclStartTimer(&Led1Timer, 0U, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

    }
    else
    {
        /* �ر��û���ʱ�� */
        state = TclStopTimer(&Led1Timer, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        ticks1 /= 2;
        ticks1 = (ticks1 < 500)? 500:ticks1;

        /* �����û���ʱ�� */
        state = TclConfigTimer(&Led1Timer, TCLM_MLS2TICKS(ticks1), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        /* �����û���ʱ�� */
        state = TclStartTimer(&Led1Timer, 0U, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");
    }

    return TCLR_IRQ_DONE;
}


/* Led�߳������� */
static void ThreadLedEntry(TArgument data)
{
    TState state;
    TError error;

    /* ��ʼ���û���ʱ��1 */
    state = TclCreateTimer(&Led1Timer,
                           TCLP_TIMER_PERIODIC,
                           TCLM_MLS2TICKS(ticks1),
                           &BlinkLed1,
                           (TArgument)0,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

    /* ��ʼ���û���ʱ��2 */
    state = TclCreateTimer(&Led2Timer,
                           TCLP_TIMER_PERIODIC,
                           TCLM_MLS2TICKS(ticks2),
                           &BlinkLed2,
                           (TArgument)0,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");


    /* �����û���ʱ��1 */
    state = TclStartTimer(&Led1Timer, TCLM_MLS2TICKS(500), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

    /* �����û���ʱ��2 */
    state = TclStartTimer(&Led2Timer, TCLM_MLS2TICKS(500), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");


    /* ���ú�KEY��ص��ⲿ�ж����� */
    state = TclSetIrqVector(KEY_IRQ_ID, &EvbKeyISR, (TThread*)0, (TArgument)0, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IRQ_NONE), "");

    while (eTrue)
    {
    }
}


/* �û�Ӧ����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ��ʼ��Led�豸�����߳� */
    state = TclCreateThread(&ThreadLed,
                            &ThreadLedEntry,
                            (TArgument)0,
                            ThreadLedStack,
                            THREAD_LED_STACK_BYTES,
                            THREAD_LED_PRIORITY,
                            THREAD_LED_SLICE,
                            &error);
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
