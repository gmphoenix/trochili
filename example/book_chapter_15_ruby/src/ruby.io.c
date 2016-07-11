#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH15_RUBY_EXAMPLE)
#include "ruby.h"
#include "ruby.os.h"
#include "ruby.dev.h"
#include "gd32f20x.h"

#define UART_TX_PEND (0x1<<1)
#define UART_RX_PEND (0x1<<2)

struct IoXmitCtrlDef
{
    TBitMask State;
    CoMsgRsp UartRsp;
};

static struct IoXmitCtrlDef IoXmitInfo;

/* �����ڽ��չ��������������������� */
void IO_SendUartInd(void)
{
    UINT8* ptr;
    UINT32 len;
    UINT8* buf;
    CoIODevCtrl* pDev = &UartDev;
    CoDataIndMsg* pMsg =0;
    TReg32 imask;

    /* Get the current receive data byte count */
    if (pDev->DATA.RxBufCount > 0)
    {
        //IoXmitInfo.State |= UART_RX_PEND;

        /* Disable IRQ interrupts while flip Ping-Pong pointer. */
        CpuEnterCritical(&imask);

        /* �����յ��Ĵ������ݸ��Ƴ��� */
        buf = pDev->DATA.RxBufRdPtr;
        len = pDev->DATA.RxBufCount;
        ptr = (UINT8*)OS_MallocMemory(len);
        memcpy(ptr, buf, len);

        pDev->DATA.RxBufCount = 0;

        /* Re-enable interrupts */
        CpuLeaveCritical(imask);


        pMsg = (CoDataIndMsg*)OS_MallocMemory(SIZEOF_DATAIND_MSG);
        pMsg->Head.Sender    = IO_THREAD_ID;
        pMsg->Head.Primitive = AIIO_UART_IND;
        pMsg->Head.Length    = SIZEOF_DATAIND_MSG;
        pMsg->DataLen        = len;
        pMsg->DataBuf        = ptr;

        /* ������Ϣ��Ӧ���� */
        pMsg->CnfInfo.MsgQId = IO_UART_CNF_QUEUE_ID ;
        pMsg->CnfInfo.Prim   = AIIO_UART_CNF ;
        pMsg->CnfInfo.Send   = 1;
        pMsg->CnfInfo.Data   = ptr;

        OS_SendMessage(AI_QUEUE_ID, (void**)(&pMsg));
    }
}


/* �����ڷ��͹��������������������� */
static void IO_ProcUartReq(void)
{
    CoDataReqMsg* pReqMsg = (CoDataReqMsg*)0;
    CoIODevCtrl * pUartDev = 0;

    OS_GetMessage(IO_UART_REQ_QUEUE_ID, (void**)(&pReqMsg));
    if (pReqMsg)
    {
        IoXmitInfo.State |= UART_TX_PEND;
        IoXmitInfo.UartRsp.Prim   = pReqMsg->RspInfo.Prim;
        IoXmitInfo.UartRsp.Data   = pReqMsg->RspInfo.Data;
        IoXmitInfo.UartRsp.Send   = pReqMsg->RspInfo.Send;
        IoXmitInfo.UartRsp.MsgQId = pReqMsg->RspInfo.MsgQId;

        /* Read pointer points to the first byte of data in message. */
        pUartDev = &UartDev;
        pUartDev->DATA.TxBufPtr = pReqMsg->DataBuf;
        pUartDev->DATA.TxBufCount = (INT16)(pReqMsg->DataLen);

        /* Enable the USART1 Transmoit interrupt */
#if 1
        USART_INT_Set(USART1, USART_INT_TBE, ENABLE );
#else
        USART_ITConfig(USART1, USART_IT_TXE, ENABLE );
#endif
        OS_FreeMemory(pReqMsg);
        pReqMsg = 0;
    }
}


/* �����ڽ��չ������������ν������� */
static void IO_ProcUartCnf(void)
{
    CoDataCnfMsg* pCnf = 0;

    OS_GetMessage(IO_UART_CNF_QUEUE_ID, (void**)(&pCnf));
    if (pCnf)
    {
        if (pCnf->DataBuf != 0)
        {
            OS_FreeMemory((void *)(pCnf->DataBuf));
        }
        OS_FreeMemory(pCnf);
    }
}

/* �����ڷ��͹������������η������� */
static void IO_SendUartRsp(void)
{
    CoDataRspMsg* pMsg = 0;

    if (IoXmitInfo.UartRsp.Send)
    {
        pMsg = (CoDataRspMsg*)OS_MallocMemory(SIZEOF_DATARSP_MSG);
        pMsg->Head.Sender    = IO_THREAD_ID;
        pMsg->Head.Primitive = IoXmitInfo.UartRsp.Prim;
        pMsg->Head.Length    = SIZEOF_DATARSP_MSG;
        pMsg->DataBuf        = IoXmitInfo.UartRsp.Data;
        OS_SendMessage(IoXmitInfo.UartRsp.MsgQId, (void**)(&pMsg));
    }
    else
    {
        if (IoXmitInfo.UartRsp.Data !=0)
        {
            OS_Error("error ");
        }
    }
}



/* IO �̵߳������� */
void IO_ThreadEntry(unsigned int arg)
{
    TBitMask pattern;
    DevInitUart();

    while (eTrue)
    {
        /* IO����������������¼� */
        pattern = IO_UART_TXREQ_FLG| IO_UART_TXRSP_FLG| \
                  IO_UART_RXIND_FLG| IO_UART_RXCNF_FLG;
        OS_WaitEvent(IO_EVENT_GROUP_ID, &pattern);

        /* ����Uart����¼���ǣ�ע�����ͬʱ���յ�TXD��TXR�¼���
        ��Ӳ���������ݹ����У��µķ���������Ҫ������ */
        if (pattern & IO_UART_TXREQ_FLG)
        {
            if (!(IoXmitInfo.State & UART_TX_PEND))
            {
                IO_ProcUartReq();
            }
        }

        /* �ڴ���ǰ��Ϣ���͵Ĺ����У��������������������ʹ������ݣ�
        ���ǲ��ܼ�ʱ����ֻ�÷�����Ϣ������ȴ�����
        ��io�����֪��ǰ���ݷ������֮����Ҫ����Ƿ���Ҫ������ܵĹ��� */
        if (pattern & IO_UART_TXRSP_FLG)
        {
            /* ��ԭ��Ϣ������������Ϣ�Ѿ�ͨ�����ڷ������ */
            IO_SendUartRsp();

            IoXmitInfo.State &= ~UART_TX_PEND;

            /* ����Ƿ��л��ܵ���Ϣ��Ҫ��������оͰ���Щ��Ϣ���͵����� */
            IO_ProcUartReq();
        }



        if (pattern & IO_UART_RXIND_FLG)
        {
            IO_SendUartInd();
        }

        /* ����Uart����¼���� */
        if (pattern & IO_UART_RXCNF_FLG)
        {
            IO_ProcUartCnf();
        }


    }
}

#endif

