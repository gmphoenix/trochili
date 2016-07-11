#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH15_RUBY_EXAMPLE)

#ifndef COLIBRI_H
#define COLIBRI_H
#include "colibri207bsp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* typedef data type  */
typedef unsigned int    UINT32;
typedef int             INT32;
typedef unsigned short  UINT16;
typedef short           INT16;
typedef unsigned char   UINT8;
typedef char            INT8;
typedef int             INT;
typedef unsigned int    UINT;



#define AIIO_UART_IND        (0xEA) /* ���ڽ��յ�����     */
#define AIIO_UART_CNF        (0xEB)
#define AIIO_UART_REQ        (0xEC) /* �����򴮿ڷ������� */
#define AIIO_UART_RSP        (0xED) 


#define ISR_KEY_UP           (0xAA)
#define ISR_KEY_DOWN         (0xAB)
#define ISR_KEY_LEFT         (0xAC)
#define ISR_KEY_RIGHT        (0xAD)
#define ISR_KEY_OK           (0xAE)
#define ISR_KEY_CANCEL       (0xAF)

/* ĳ���������ֻ��Ҫ��ʾһ���ʱ�䣬Ȼ���Զ��رա���ʱ����ͨ������1�����
   ��ʱ������ʱ���Զ����������Ϣ */
#define WIND_TIMER_IND       (0xBA)

/* ��Ļ�ϵ�ʱ����ʾ������ͨ��һ�������Ե������ʱ����ÿ�뷢һ�������Ϣ */
#define WIND_WALL_TIMER_IND  (0xBB)

/* IO������ص��¼���� */
#define IO_UART_RXIND_FLG   (0x1<<0)  /* uart rx availibel       */
#define IO_UART_RXCNF_FLG   (0x1<<1)  /* uart rx done            */
#define IO_UART_TXREQ_FLG   (0x1<<2)  /* uart tx require         */
#define IO_UART_TXRSP_FLG   (0x1<<3)  /* uart tx done            */


/* Colibri��ܵ�ͨ����Ϣͷ */
typedef struct
{
    TWord16 Sender;                    /* ��Ϣ��Դ */
    TWord16 Primitive;                 /* ��Ϣԭ�� */
    TWord16 Length;                    /* ��Ϣ���� */
} CoMsgHead;
#define SIZEOF_MSG_HEAD (sizeof (CoMsgHead))

/* Colibri��ܵ�ͨ��RSP��Ϣ�ṹ */
typedef struct
{
    TWord16   MsgQId;                  /* RSP��Ϣ���б��         */
    UINT8     Prim;                    /* RSPԭ��                 */
    UINT8     Send;                    /* �Ƿ���ҪRSP������Ϣ��� */
    UINT8*    Data;                    /* �����ڴ��ַ            */
} CoMsgRsp;
#define SIZEOF_MSG_RSP (sizeof (CoMsgRsp))

/* Colibri��ܵ�ͨ��CNF��Ϣ�ṹ */
typedef struct
{
    TWord16   MsgQId;                  /* CNF��Ϣ���б��         */
    UINT8     Prim;                    /* CNFԭ��                 */
    UINT8     Send;                    /* �Ƿ���ҪCNF������Ϣ��� */
    UINT8*    Data;                    /* �����ڴ��ַ            */
} CoMsgCnf;
#define SIZEOF_MSG_CNF (sizeof (CoMsgCnf))

/* Colibri��ܵ�ͨ��IND��Ϣ�ṹ */
typedef struct
{
    CoMsgHead Head;                     /* ͨ����Ϣͷ�ṹ         */
    CoMsgCnf  CnfInfo;                  /* CNF��Ϣ                */
    UINT8*    DataBuf;                  /* �����ڴ��ַ           */
    UINT8     DataLen;                  /* ���ݳ���               */
} CoDataIndMsg;
#define SIZEOF_DATAIND_MSG  (sizeof (CoDataIndMsg))

/* Colibri��ܵ�ͨ��CNF��Ϣ�ṹ */
typedef struct
{
    CoMsgHead Head;                    /* ͨ����Ϣͷ�ṹ          */
    UINT8*    DataBuf;                 /* �����ڴ��ַ            */
} CoDataCnfMsg;
#define SIZEOF_DATACNF_MSG (sizeof(CoDataCnfMsg))

/* Colibri��ܵ�ͨ��REQ��Ϣ�ṹ */
typedef struct
{
    CoMsgHead Head;                    /* ͨ����Ϣͷ�ṹ          */
    CoMsgRsp  RspInfo;                 /* RSP��Ϣ                 */
    UINT8*    DataBuf;                 /* �����ڴ��ַ            */
    UINT32    DataLen;                 /* ���ݳ���                */
} CoDataReqMsg;
#define SIZEOF_DATAREQ_MSG  (sizeof(CoDataReqMsg))

/* Colibri��ܵ�ͨ��RSP��Ϣ�ṹ */
typedef struct
{
    CoMsgHead Head;                    /* ͨ����Ϣͷ�ṹ          */
    UINT8*    DataBuf;                 /* �����ڴ��ַ            */
} CoDataRspMsg;
#define SIZEOF_DATARSP_MSG (sizeof(CoDataRspMsg))


#endif
#endif
