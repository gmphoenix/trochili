/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_MAILBOX_H
#define _TCL_MAILBOX_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.ipc.h"
#include "tcl.thread.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MAILBOX_ENABLE))

/* �ʼ����Ͷ��� */
typedef enum
{
    eNormalMail,  /* ��ͨ�ʼ�  */
    eUrgentMail   /* �����ʼ�  */
} TMailType;

/* ����״̬���� */
typedef enum
{
    eMailBoxEmpty,  /* �������ݿ� */
    eMailBoxFull    /* ���������� */
} TMailBoxStatus;

/* �ʼ��ṹ���� */
typedef void* TMail;

/* ����ṹ���� */
struct MailBoxDef
{
    TProperty      Property;         /* �̵߳ĵ��Ȳ��Ե��������� */
    TMail          Mail;             /* ������ʼ�����           */
    TMailBoxStatus Status;           /* �����״̬               */
    TIpcQueue      Queue;            /* ������߳���������       */
};
typedef struct MailBoxDef TMailBox;

extern TState xMailBoxSend(TMailBox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo, TError* pError);
extern TState xMailBoxReceive(TMailBox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo, TError* pError);
extern TState xMailBoxCreate(TMailBox* pMailbox, TProperty property, TError* pError);
extern TState xMailBoxDelete(TMailBox* pMailbox, TError* pError);
extern TState xMailBoxFlush(TMailBox* pMailbox, TError* pError);
extern TState xMailboxReset(TMailBox* pMailbox, TError* pError);
extern TState xMailBoxBroadcast(TMailBox* pMailbox, TMail* pMail2, TError* pError);

#endif

#endif /* _TOCHILI_MAILBOX_H */

