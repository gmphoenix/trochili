/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCLC_IPC_H
#define _TCLC_IPC_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"

#if (TCLC_IPC_ENABLE)

/* IPC�߳��������нṹ���� */
struct IpcBlockedQueueDef
{
    TProperty* Property;                         /* �߳�������������                           */
    TObjNode*  PrimaryHandle;                    /* �����л����̷ֶ߳���                       */
    TObjNode*  AuxiliaryHandle;                  /* �����и����̷ֶ߳���                       */
};
typedef struct IpcBlockedQueueDef TIpcQueue;


/* IPC����������ں˴���ʹ�� */
#define IPC_ERR_NONE             (TError)(0x0)          /* �����ɹ�                                   */
#define IPC_ERR_FAULT            (TError)(0x1<<0)       /* ����/�����÷�����                          */
#define IPC_ERR_UNREADY          (TError)(0x1<<1)       /* IPC����û�б���ʼ��                        */
#define IPC_ERR_TIMEO            (TError)(0x1<<2)       /* ������ʱ�޵����̱߳�����                   */
#define IPC_ERR_DELETE           (TError)(0x1<<3)       /* IPC�������٣��̱߳�����                  */
#define IPC_ERR_RESET            (TError)(0x1<<4)       /* IPC�������ã��̱߳�����                  */
#define IPC_ERR_FLUSH            (TError)(0x1<<5)       /* IPC���������ϵ��̱߳�����ֹ                */
#define IPC_ERR_ABORT            (TError)(0x1<<6)       /* IPC���������ϵ��̱߳�����ֹ                */
#define IPC_ERR_INVALID_VALUE    (TError)(0x1<<7)       /* �������ֵ��������                         */
#define IPC_ERR_INVALID_STATUS   (TError)(0x1<<8)       /* �����״̬���ܱ�����                       */
#define IPC_ERR_FLAGS            (TError)(0x1<<9)       /* �����͵��¼��Ѿ����� or 
                                                           �ڴ����¼����ܱ�����   */
#define IPC_ERR_FORBIDDEN        (TError)(0x1<<16)      /* �������ѱ������߳�ռ�� or
                                                           �����������ڵ�ǰ�߳�       */

/* IPC�������ԣ��ں˴���ʹ�� */
#define IPC_PROPERTY             (TProperty)(0x0)
#define IPC_PROP_READY           (TProperty)(0x1<<0)       /* IPC�����Ѿ�����ʼ��                        */
#define IPC_PROP_PREEMP_AUXIQ    (TProperty)(0x1<<1)       /* �����߳��������в������ȼ����ȷ���         */
#define IPC_PROP_PREEMP_PRIMIQ   (TProperty)(0x1<<2)       /* �����߳��������в������ȼ����ȷ���         */
#define IPC_PROP_AUXIQ_AVAIL     (TProperty)(0x1<<17)      /* �����߳�������������ڱ��������߳�         */
#define IPC_PROP_PRIMQ_AVAIL     (TProperty)(0x1<<18)      /* �����߳�������������ڱ��������߳�         */

#define IPC_VALID_SEMN_PROP      (IPC_PROP_PREEMP_PRIMIQ)
#define IPC_VALID_MUTEX_PROP     (IPC_PROP_PREEMP_PRIMIQ)
#define IPC_VALID_MBOX_PROP      (IPC_PROP_PREEMP_PRIMIQ | IPC_PROP_PREEMP_AUXIQ)
#define IPC_VALID_MQUE_PROP      (IPC_PROP_PREEMP_PRIMIQ | IPC_PROP_PREEMP_AUXIQ)
#define IPC_VALID_FLAG_PROP      (IPC_PROP_PREEMP_PRIMIQ)


#define IPC_RESET_SEMN_PROP      (IPC_PROP_READY | IPC_PROP_PREEMP_PRIMIQ)
#define IPC_RESET_MUTEX_PROP     (IPC_PROP_READY | IPC_PROP_PREEMP_PRIMIQ)
#define IPC_RESET_MBOX_PROP      (IPC_PROP_READY | IPC_PROP_PREEMP_PRIMIQ | IPC_PROP_PREEMP_AUXIQ)
#define IPC_RESET_MQUE_PROP      (IPC_PROP_READY | IPC_PROP_PREEMP_PRIMIQ | IPC_PROP_PREEMP_AUXIQ)
#define IPC_RESET_FLAG_PROP      (IPC_PROP_READY | IPC_PROP_PREEMP_PRIMIQ)


/* �߳�IPCѡ��ں˴���ʹ�� */
#define IPC_OPTION               (TOption)(0x0)
#define IPC_OPT_ISR              (TOption)(0x1<<0)       /* ̽�����ģʽ���̲߳�������������ISR�µ���  */
#define IPC_OPT_WAIT             (TOption)(0x1<<1)       /* ���÷�ʽ�ȴ�IPC                            */
#define IPC_OPT_TIMED            (TOption)(0x1<<2)       /* ʱ�޷�ʽ�ȴ����                           */
#define IPC_OPT_UARGENT          (TOption)(0x1<<3)       /* ��Ϣ���С��ʼ�ʹ��                         */
#define IPC_OPT_AND              (TOption)(0x1<<4)       /* ����¼���ǲ�����AND����                  */
#define IPC_OPT_OR               (TOption)(0x1<<5)       /* ����¼���ǲ�����OR����                   */
#define IPC_OPT_CONSUME          (TOption)(0x1<<6)       /* �¼����ʹ��                               */

#define IPC_OPT_SEMAPHORE        (TOption)(0x1<<16)      /* ����߳��������ź������߳�����������       */
#define IPC_OPT_MUTEX            (TOption)(0x1<<17)      /* ����߳������ڻ��������߳�����������       */
#define IPC_OPT_MAILBOX          (TOption)(0x1<<18)      /* ����߳�������������߳�����������         */
#define IPC_OPT_MSGQUEUE         (TOption)(0x1<<19)      /* ����߳���������Ϣ���е��߳�����������     */
#define IPC_OPT_FLAGS            (TOption)(0x1<<20)      /* ����߳��������¼���ǵ��߳�����������     */

#define IPC_OPT_USE_AUXIQ        (TOption)(0x1<<23)      /* ����߳����߳��������еĸ���������         */
#define IPC_OPT_READ_DATA        (TOption)(0x1<<24)      /* �����ʼ�������Ϣ                           */
#define IPC_OPT_WRITE_DATA       (TOption)(0x1<<25)      /* �����ʼ�������Ϣ                           */

#define IPC_VALID_SEMN_OPT       (IPC_OPT_ISR|IPC_OPT_WAIT|IPC_OPT_TIMED)
#define IPC_VALID_MUTEX_OPT      (IPC_OPT_WAIT|IPC_OPT_TIMED)
#define IPC_VALID_MBOX_OPT       (IPC_OPT_ISR|IPC_OPT_WAIT|IPC_OPT_TIMED|IPC_OPT_UARGENT)
#define IPC_VALID_MSGQ_OPT       (IPC_OPT_ISR|IPC_OPT_WAIT|IPC_OPT_TIMED|IPC_OPT_UARGENT)
#define IPC_VALID_FLAG_OPT       (IPC_OPT_ISR|IPC_OPT_WAIT|IPC_OPT_TIMED|\
                                  IPC_OPT_AND|IPC_OPT_OR|IPC_OPT_CONSUME)



/* NOTE: not compliant MISRA2004 18.4: Unions shall not be used. */
union IpcDataDef
{
    TBase32  Value;                                /* ���汻�������ݱ����ĵ�ֵַַ               */
    void*  Addr1;                                /* ָ���¼���ǵ�һ��ָ��                     */
    void** Addr2;                                /* ָ����Ϣ�����ʼ��Ķ���ָ��                 */
};
typedef union IpcDataDef TIpcData;

/* �߳����ڼ�¼IPC�������ϸ��Ϣ�ļ�¼�ṹ */
struct IpcContextDef
{
    void*        Object;                          /* ָ��IPC�����ַ��ָ��                      */
    TIpcQueue*   Queue;                           /* �߳�����IPC�̶߳���ָ��                    */
    TIpcData     Data;                            /* ��IPC���������ص�����ָ��                */
	TBase32      Length;                          /* ��IPC���������ص����ݳ���                */
    TOption      Option;                          /* ����IPC����Ĳ�������                      */
    TState*      State;                           /* IPC��������ķ���ֵ                        */
    TError*      Error;                           /* IPC��������Ĵ������                      */
    void*        Owner;                           /* IPC���������߳�                            */
    TObjNode     ObjNode;                         /* �߳�����IPC���е�����ڵ�                  */
};
typedef struct IpcContextDef TIpcContext;

extern void uIpcInitContext(TIpcContext* pContext, void* pOwner);
extern void uIpcSaveContext(TIpcContext* pContext, void* pIpc, TBase32 data, TBase32 len, TOption option,
                            TState* pState, TError* pError);
extern void uIpcCleanContext(TIpcContext* pContext);
extern void uIpcBlockThread(TIpcContext* pContext, TIpcQueue* pQueue, TTimeTick ticks);
extern void uIpcUnblockThread(TIpcContext* pContext,  TState state, TError error, TBool* pHiRP);
extern void uIpcUnblockAll(TIpcQueue* pQueue, TState state, TError error,
                           void** pData2, TBool* pHiRP);
extern void uIpcSetPriority(TIpcContext* pContext, TPriority priority);

#endif

#endif /* _TCLC_IPC_H */

