/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.debug.h"
#include "tcl.object.h"

/*************************************************************************************************
 *  ���ܣ����������ȳ����򽫽ڵ���뵽ָ����˫��ѭ������                                         *
 *  ������(1) pHandle2 ָ������ͷָ���ָ��                                                      *
 *        (2) pNode    �ڵ�ָ��                                                                  *
 *        (3) pos      ���׶�β���                                                              *
 *  ���أ���                                                                                     *
 *  ˵������                                                                                     *
 *************************************************************************************************/
void uObjQueueAddFifoNode(TObjNode** pHandle2, TObjNode* pNode, TQueuePos pos)
{
    KNL_ASSERT((pNode->Handle == (TObjNode**)0), "");

    if (*pHandle2)
    {
        /* ������в��գ���ѽڵ�׷���ڶ�β */
        pNode->Prev = (*pHandle2)->Prev;
        pNode->Prev->Next = pNode;
        pNode->Next = *pHandle2;
        pNode->Next->Prev = pNode;
        if (pos == eQuePosHead)
        {
            (*pHandle2) = (*pHandle2)->Prev;
        }
    }
    else
    {
        /* �����ʼ����һ���ڵ��ͷ�ڵ�ָ�� */
        *pHandle2 = pNode;
        pNode->Prev = pNode;
        pNode->Next = pNode;
    }
    pNode->Handle = pHandle2;
}


/*************************************************************************************************
 *  ���ܣ����������ȼ�������򽫽ڵ���뵽ָ����˫��ѭ������                                     *
 *  ������(1) pHandle2 ָ������ͷָ���ָ��                                                      *
 *        (2) pNode    �ڵ�ָ��                                                                  *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void uObjQueueAddPriorityNode(TObjNode** pHandle2, TObjNode* pNode)
{
    TObjNode* pTemp = (TObjNode*)0;

    KNL_ASSERT((pNode->Handle == (TObjNode**)0), "");

    /* �������Ƿ�Ϊ�� */
    if (*pHandle2)
    {
        pTemp = (*pHandle2);

        /* �½ڵ��ͷ������ȼ�����(��ʱ��Ҫ����headָ��)  */
        if (*(pTemp->Data) > *(pNode->Data))
        {
            (*pHandle2) = pNode;
        }
        else
        {
            /* �ڶ����������½ڵ��λ�ã������½ڵ�����ȼ������нڵ�����ȼ�����
                (��ʱ����Ҫ����headָ��) */
            pTemp = pTemp->Next;
            while ((*(pTemp->Data) <= *(pNode->Data)) && (pTemp != (*pHandle2)))
            {
                pTemp = pTemp->Next;
            }
        }

        /* �����½ڵ㵽����  */
        pNode->Prev = pTemp->Prev;
        pNode->Prev->Next = pNode;
        pNode->Next = pTemp;
        pNode->Next->Prev = pNode;
    }
    else
    {
        (*pHandle2) = pNode;
        pNode->Prev = pNode;
        pNode->Next = pNode;
    }
    pNode->Handle = pHandle2;
}



/*************************************************************************************************
 *  ���ܣ����ڵ��ָ����˫��ѭ���������Ƴ�                                                       *
 *  ������(1) pHandle2 ָ������ͷָ���ָ��                                                      *
 *        (2) pNode    �ڵ�ָ��                                                                  *
 *        (3) pos      ���׶�β���                                                              *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void uObjQueueRemoveNode(TObjNode** pHandle2, TObjNode* pNode)
{
    KNL_ASSERT((pNode->Handle == pHandle2), "");

    /* ����Ƿ������ֻ��һ���ڵ㣬����ǾͰ�ͷ���ָ����� */
    if (pNode->Prev == pNode)
    {
        *pHandle2 = (TObjNode*)0;
    }
    else
    {
        /* ��������в�ֹһ���ڵ㣬��ѵ�ǰ�ڵ�ɾ�� */
        pNode->Prev->Next = pNode->Next;
        pNode->Next->Prev = pNode->Prev;

        /* ������ڵ���ͷ��㣬�����ͷ���ָ�룬�������� */
        if (pNode == *pHandle2)
        {
            *pHandle2 = pNode->Next;
        }
    }

    /* ��սڵ�ǰ���������Ϣ */
    pNode->Next = (TObjNode*)0;
    pNode->Prev = (TObjNode*)0;
    pNode->Handle = (TObjNode**)0;
}


/*************************************************************************************************
 *  ���ܣ����ڵ���뵽˫�������ָ��λ��                                                         *
 *  ��Χ��ȫ��                                                                                   *
 *  ������(1) pHandle2 ָ������ͷָ���ָ��                                                      *
 *        (2) pNode    �ڵ�ָ��                                                                  *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void uObjListAddNode(TObjNode** pHandle2, TObjNode* pNode, TQueuePos pos)
{
    TObjNode* pTail;
    KNL_ASSERT((pNode->Handle == (TObjNode**)0), "");

    /* ����������и��ڵ� */
    if (*pHandle2)
    {
        if (pos == eQuePosHead)
        {
            pNode->Next = *pHandle2;
            pNode->Prev = (TObjNode*)0;
            (*pHandle2)->Prev = pNode;
            *pHandle2 = pNode;
        }
        else
		{
            pTail= *pHandle2;
            while(pTail->Next)
            {
                pTail = pTail->Next;
            }

            pNode->Next = (TObjNode*)0;
            pNode->Prev = pTail;
            pTail->Next = pNode;
        }
    }
    /* ���������û�нڵ� */
    else
    {
        *pHandle2 = pNode;
        pNode->Next = (TObjNode*)0;
        pNode->Prev = (TObjNode*)0;
    }
    pNode->Handle = pHandle2;
}


/*************************************************************************************************
 *  ���ܣ����������ȼ�������򽫽ڵ���뵽ָ����˫������                                         *
 *  ������(1) pHandle2 ָ������ͷָ���ָ��                                                      *
 *        (2) pNode    �ڵ�ָ��                                                                  *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void uObjListAddPriorityNode(TObjNode** pHandle2, TObjNode* pNode)
{
    TObjNode* pCursor = (TObjNode*)0;
    TObjNode* pTail   = (TObjNode*)0;

    /* �������Ϊ�գ����½ڵ���Ϊͷ��� */
    if ((*pHandle2) == (TObjNode*)0)
    {
        *pHandle2   = pNode;
        pNode->Next = (TObjNode*)0;
        pNode->Prev = (TObjNode*)0;
    }
    else
    {
        /* ����������У����Һ���λ�� */
        pCursor = *pHandle2;
        while (pCursor != (TObjNode*)0)
        {
            pTail = pCursor;
            if (*(pNode->Data) >= *(pCursor->Data))
            {
                pCursor = pCursor->Next;
            }
            else
            {
                break;
            }
        }

        /* �α겻Ϊ��˵�����������ҵ����ʵĽڵ㣬��Ҫ���½ڵ���뵽�ýڵ�֮ǰ */
        if (pCursor != (TObjNode*)0)
        {
            /* ���������ͷ�ڵ� */
            if (pCursor->Prev == (TObjNode*)0)
            {
                *pHandle2 = pNode;
                pCursor->Prev = pNode;
                pNode->Prev   = (TObjNode*)0;
                pNode->Next    = pCursor;
            }
            /* ��������������ڵ�(�����м�Ľڵ����β�ڵ�) */
            else
            {
                pNode->Prev         = pCursor->Prev;
                pNode->Next         = pCursor;
                pCursor->Prev->Next = pNode;
                pCursor->Prev       = pNode;
            }
        }
        /* ����α�Ϊ��˵��û���ҵ����ʵĽڵ㣬����ֻ�ܰ��½ڵ����������� */
        else
        {
            pTail->Next = pNode;
            pNode->Prev = pTail;
            pNode->Next = (TObjNode*)0;
        }
    }
    pNode->Handle = pHandle2;
}


/*************************************************************************************************
 *  ���ܣ����ڵ��ָ����˫���������Ƴ�                                                           *
 *  ��Χ��ȫ��                                                                                   *
 *  ������(1) pHandle2 ָ������ͷָ���ָ��                                                      *
 *        (2) pNode    �ڵ�ָ��                                                                  *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void uObjListRemoveNode(TObjNode** pHandle2, TObjNode* pNode)
{
    KNL_ASSERT((pNode->Handle == pHandle2), "");

    /* ���������ֻ��һ���ڵ� */
    if ((pNode->Prev == (TObjNode*)0) && (pNode->Next == (TObjNode*)0))
    {
        *pHandle2 = (TObjNode*)0;
    }
    /* ����������ж���ڵ㲢����Ҫɾ������β���Ľڵ� */
    else if ((pNode->Prev != (TObjNode*)0) && (pNode->Next == (TObjNode*)0))
    {
        pNode->Prev->Next = (TObjNode*)0;
    }
    /* ����������ж���ڵ㲢����Ҫɾ������ͷ���Ľڵ� */
    else if ((pNode->Prev == (TObjNode*)0) && (pNode->Next != (TObjNode*)0))
    {
        pNode->Next->Prev = (TObjNode*)0;
        *pHandle2 = pNode->Next;
    }
    /* ����������ж���ڵ㲢����Ҫɾ�������в��Ľڵ� */
    else
    {
        pNode->Prev->Next = pNode->Next;
        pNode->Next->Prev = pNode->Prev;
    }

    /* ���ñ�ɾ���ڵ������ */
    pNode->Next = (TObjNode*)0;
    pNode->Prev = (TObjNode*)0;
    pNode->Handle = (TObjNode**)0;
}



