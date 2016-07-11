/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_MQUEUE_H
#define _TCL_MQUEUE_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.ipc.h"
#include "tcl.thread.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MQUE_ENABLE))

/* ��Ϣ����״̬���� */
enum MQStatus
{
    eMQEmpty,  /* ��Ϣ���п�       */
    eMQFull,   /* ��Ϣ������       */
    eMQPartial /* ��Ϣ���зǿշ��� */
};
typedef enum MQStatus TMQStatus;

/* ��Ϣ���Ͷ��� */
enum MsgTypeDef
{
    eNormalMessage,  /* ��ͨ��Ϣ,������Ϣ����ͷ */
    eUrgentMessage   /* ������Ϣ,������Ϣ����β */
};
typedef enum MsgTypeDef TMsgType;

/* ��Ϣ���ݽṹ���� */
typedef void* TMessage;

/* ��Ϣ���нṹ���� */
struct MessageQueueCB
{
    TProperty Property;      /* ��Ϣ������������       */
    void**    MsgPool;       /* ��Ϣ�����             */
    TBase32     Capacity;      /* ��Ϣ��������           */
    TBase32     MsgEntries;    /* ��Ϣ��������Ϣ����Ŀ   */
    TIndex    Head;          /* ��Ϣ����дָ��λ��     */
    TIndex    Tail;          /* ��Ϣ���ж�ָ��λ��     */
    TMQStatus Status;        /* ��Ϣ����״̬           */
    TIpcQueue Queue;         /* ��Ϣ���е��߳��������� */
};
typedef struct MessageQueueCB TMsgQueue;


/* ��Ϣ���в������� */
extern TState xMQCreate(TMsgQueue* pMsgQue, void** pPool2, TBase32 capacity,
                            TProperty property, TError* pError);
extern TState xMQReceive(TMsgQueue* pMsgQue, TMessage* pMsg2,
                                TOption option, TTimeTick timeo, TError* pError);
extern TState xMQSend(TMsgQueue* pMsgQue, TMessage* pMsg2,
                             TOption option, TTimeTick timeo, TError* pError);
extern TState xMQBroadcast(TMsgQueue* pMsgQue, TMessage* pMsg2, TError* pError);
extern TState xMQDelete(TMsgQueue* pMsgQue, TError* pError);
extern TState xMQReset(TMsgQueue* pMsgQue, TError* pError);
extern TState xMQFlush(TMsgQueue* pMsgQue, TError* pError);

#endif

#endif /* _TCL_MQUEUE_H */

