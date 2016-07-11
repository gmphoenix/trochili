#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH15_RUBY_EXAMPLE)
#include "ruby.h"
#include "ruby.os.h"

/* ���ļ�����ȫ��ͼ����ʾ�Ĺ�����
   �����������ISRͨ�����Ͳ�ͬ������Ϣ������
   1 �����жϴ������ᷢ�Ͳ�ͬ������ű����µ���Ϣ
   2 ��ʱ������ᷢ�Ͳ�ͬ��ʱ����Ϣ����
   ����������Щ��Ϣ���û�����ͨ����չ��Ϣ��������������

   ���ļ��еĴ�����Ҫ�û�
   1 ���°�����������ʵ�֡��ο�colibri.dev.key.c�ļ����������bsp
   2 �������TFT��������.
   */
#define KEY_UP_ID    0x1
#define KEY_DOWN_ID  0x2
#define KEY_LEFT_ID  0x3
#define KEY_RIGHT_ID 0x4

/* �������� */
#define NORMAL_WINDOW_VISIABLE  (1)
#define WINDOW1_VISIABLE         (2)
#define WINDOW2_VISIABLE         (3)

/* ָʾ������ʾ�����ĸ����� */
static int CurrentWinID = 0;

/* ȷ��USB�����Ѿ����������,�������USB���ν��չ��� */
static void  AIIO_ProcKeyMsg(int keyid)
{
    switch (CurrentWinID)
    {
        case NORMAL_WINDOW_VISIABLE:
        {
            /* ����פ�����յ��İ��� */
            switch (keyid)
            {
                    /* ����פ�����յ������ϰ��� */
                case KEY_UP_ID:
                {
                    break;
                }
                case KEY_DOWN_ID:
                {
                    break;
                }
                case KEY_LEFT_ID:
                {
                    break;
                }
                case KEY_RIGHT_ID:
                {
                    break;
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        case WINDOW1_VISIABLE:
        {
            /* ������1�յ������ϰ��� */
            break;
        }
        case WINDOW2_VISIABLE:
        {
            /* ������2�յ������ϰ��� */
            break;
        }
        default:
        {
            break;
        }
    }
}




/* �軭��פ���� */
static void DrawNormalWindow (void)
{

}

/* �軭����1 */
static void DrawWindow1 (void)
{

}

/* �軭����2 */
static void DrawWindow2 (void)
{

}


/* ���մ����δ�IO������յ������� */
static void UI_ProcKeyPrimitive(CoMsgHead* pMsg)
{
    switch (pMsg->Primitive)
    {
        case ISR_KEY_UP:
        {
            AIIO_ProcKeyMsg(KEY_UP_ID);
            break;
        }

        case ISR_KEY_DOWN:
        {
            AIIO_ProcKeyMsg(KEY_DOWN_ID);
            break;
        }
        case ISR_KEY_LEFT:
        {
            AIIO_ProcKeyMsg(KEY_LEFT_ID);
            break;
        }

        case ISR_KEY_RIGHT:
        {
            AIIO_ProcKeyMsg(KEY_RIGHT_ID);
            break;
        }
        default:
        {
            break;
        }
    }
}

static void UI_ProcTimerPrimitive(CoMsgHead* pMsg)
{
    switch (pMsg->Primitive)
    {
        case WIND_TIMER_IND:
        {
            /* ��ǰ���ڵ���ʾʱ�䵽����Թر��� */
            break;
        }
        case WIND_WALL_TIMER_IND:
        {
            /* ����"ǽ��ʱ��",��ʮ�����Ǹ���ǰ��ʾʱ�� */
            break;
        }
        break;
    }
}


/* AI�̵߳������� */
void UI_ThreadEntry(unsigned int arg)
{
    CoMsgHead* pMsg;

    /* ������ʾ��פ���� */
    DrawNormalWindow();

    while (eTrue)
    {
        /* ����Ϣ���������������Ϣ */
        OS_PendMessage(UI_QUEUE_ID, (void**)(&pMsg));
        if (pMsg)
        {
            switch (pMsg->Sender)
            {
                case KEY_ISR_ID:
                {
                    UI_ProcKeyPrimitive(pMsg);
                    break;
                }
                case TIMER_ISR_ID:
                {
                    UI_ProcTimerPrimitive(pMsg);
                    break;
                }
                default:
                {
                    break;
                }
            }

            /* �������յ�����Ϣ���ͷŸ���Ϣռ�õ��ڴ� */
            OS_FreeMemory(pMsg);
        }
    }
}

/* ����ǽ��ʱ�亯�� */
void WallTimerFunc(TArgument data)
{
    /* ��ȡ��ǰʱ�䣬������Ļ */
    CoMsgHead * pMsg = 0;

    /* �Ѵ��������ݴ������Ϣ */
    pMsg = (CoMsgHead *)OS_MallocMemory(SIZEOF_MSG_HEAD);
    pMsg->Sender    = TIMER_ISR_ID;
    pMsg->Primitive = WIND_WALL_TIMER_IND;
    pMsg->Length    = SIZEOF_MSG_HEAD;

    /* ����Ϣ����UI�������Ϣ���� */
    OS_SendMessage(UI_QUEUE_ID, (void**)(&pMsg));
}

/* ������ʾ����ʱ�䵽 */
void WinTimerFunc(TArgument data)
{
    CoMsgHead * pMsg = 0;

    /* �Ѵ��������ݴ������Ϣ */
    pMsg = (CoMsgHead *)OS_MallocMemory(SIZEOF_MSG_HEAD);
    pMsg->Sender    = TIMER_ISR_ID;
    pMsg->Primitive = WIND_TIMER_IND;
    pMsg->Length    = SIZEOF_MSG_HEAD;

    /* ����Ϣ����UI�������Ϣ���� */
    OS_SendMessage(UI_QUEUE_ID, (void**)(&pMsg));
}

#endif

