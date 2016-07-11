#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH15_RUBY_EXAMPLE)
#include "ruby.h"
#include "ruby.os.h"

static void AIIO_SendUartReq(char* req, int len);
static void AIIO_ProcUartRsp(CoDataRspMsg* pRsp);
static void AIIO_ProcUartInd (CoDataIndMsg* pIndMsg);
static void AIIO_SendUartCnf(CoMsgCnf* pCnf);

/* ������UART����,����ʼUART���͹��� */
static void AIIO_SendUartReq(char* req, int len)
{
    CoDataReqMsg * pMsg = 0;
    UINT8* pData = 0;

    /* ������Ҫ�Ӵ��ڷ��͵����� */
    pData = (UINT8 *)OS_MallocMemory(len);
    memcpy(pData, req, len);

    /* �Ѵ��������ݴ������Ϣ */
    pMsg = (CoDataReqMsg*)OS_MallocMemory(SIZEOF_DATAREQ_MSG);
    pMsg->Head.Sender    = AI_THREAD_ID;
    pMsg->Head.Primitive = AIIO_UART_REQ;
    pMsg->Head.Length    = SIZEOF_DATAREQ_MSG;
    pMsg->DataBuf        = (UINT8*)pData;
    pMsg->DataLen        = len;

    /* ������Ϣ��Ӧ���� */
    pMsg->RspInfo.MsgQId = AI_QUEUE_ID ;
    pMsg->RspInfo.Prim   = AIIO_UART_RSP ;
    pMsg->RspInfo.Send   = 1;
    pMsg->RspInfo.Data   = pData;

    /* ����Ϣ����IO�������Ϣ���� */
    OS_SendMessage(IO_UART_REQ_QUEUE_ID, (void**)(&pMsg));

    /* ֪ͨIO���� */
    OS_SetEvent(IO_EVENT_GROUP_ID, IO_UART_TXREQ_FLG);
}


/* ���մ�����UART�����������Ӧ��Ϣ */
static void AIIO_ProcUartRsp(CoDataRspMsg* pRsp)
{
    if (pRsp->DataBuf!= 0)
    {
        OS_FreeMemory((void*)(pRsp->DataBuf));
    }
}


static char* teststring= "abcdefghijklmnopqrstuvwxyz";
static char teststring2[32];
/* ���մ�����UART���յ������� */
static void AIIO_ProcUartInd (CoDataIndMsg* pIndMsg)
{
    char* data;
    int len;
    CoMsgCnf CnfInfo;

    CnfInfo.Prim   = pIndMsg->CnfInfo.Prim;
    CnfInfo.Data   = pIndMsg->CnfInfo.Data;
    CnfInfo.Send   = pIndMsg->CnfInfo.Send;
    CnfInfo.MsgQId = pIndMsg->CnfInfo.MsgQId;

    /* ������յ������� */
    len  = pIndMsg->DataLen;
    data = (char*)OS_MallocMemory(len);
    memcpy(data, pIndMsg->DataBuf, len);
	memcpy(teststring2, pIndMsg->DataBuf, len);
    OS_FreeMemory(data);

    /* ��������ȷ����Ϣ */
    AIIO_SendUartCnf(&CnfInfo);

    AIIO_SendUartReq(teststring2, len);
}


/* ȷ��UART�����Ѿ����������,�������UART���չ��� */
static void AIIO_SendUartCnf(CoMsgCnf* pCnf)
{
    CoDataCnfMsg* pMsg = 0;

    if (pCnf->Send)
    {
        pMsg = (CoDataCnfMsg*)OS_MallocMemory(SIZEOF_DATACNF_MSG);
        pMsg->Head.Sender    = AI_THREAD_ID;
        pMsg->Head.Primitive = pCnf->Prim;
        pMsg->Head.Length    = SIZEOF_DATACNF_MSG;
        pMsg->DataBuf        = pCnf->Data;
        OS_SendMessage(pCnf->MsgQId, (void**)(&pMsg));
        OS_SetEvent(IO_EVENT_GROUP_ID, IO_UART_RXCNF_FLG);
    }
    else
    {
        if (pCnf->Data !=0)
        {
            OS_Error("error ");
        }
    }
}



/* ���մ����δ�IO������յ������� */
static void AI_ProcIOPrimitive(CoMsgHead* pMsg)
{
    switch (pMsg->Primitive)
    {
        case AIIO_UART_RSP:
        {
            AIIO_ProcUartRsp((CoDataRspMsg*)pMsg);
            break;
        }
        case AIIO_UART_IND:
        {
            AIIO_ProcUartInd((CoDataIndMsg*)pMsg);
            break;
        }
        default:
        {
            break;
        }
    }

}


/* ���մ����δ�FS������յ������� */
static void AI_ProcFSPrimitive(void* pMsg2)
{
}


/* ���մ����δ�UI������յ������� */
static void AI_ProcUIPrimitive(void* pMsg2)
{
}


/* AI�̵߳������� */
void AI_ThreadEntry(unsigned int arg)
{
    CoMsgHead* pMsg;

    while (eTrue)
    {
        /* ����Ϣ���������������Ϣ */
        OS_PendMessage(AI_QUEUE_ID, (void**)(&pMsg));
        if (pMsg)
        {
            switch (pMsg->Sender)
            {
                case IO_THREAD_ID:
                {
                    AI_ProcIOPrimitive(pMsg);
                    break;
                }
                case FS_THREAD_ID:
                {
                    AI_ProcFSPrimitive(pMsg);
                    break;
                }
                case UI_THREAD_ID:
                {
                    AI_ProcUIPrimitive(pMsg);
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

#endif

