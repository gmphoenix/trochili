/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "string.h"

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.debug.h"
#include "tcl.cpu.h"
#include "tcl.mem.pool.h"

#if ((TCLC_MEMORY_ENABLE) && (TCLC_MEMORY_POOL_ENABLE))
/*************************************************************************************************
 *  ����: ��ʼ���ڴ�ҳ��                                                                         *
 *  ����: (1) pPool      �ڴ�ҳ�ؽṹ��ַ                                                        *
 *        (2) pAddr      �ڴ����������ַ                                                        *
 *        (3) pages      �ڴ�����ڴ�ҳ��Ŀ                                                      *
 *        (4) pgsize     �ڴ�ҳ��С                                                              *
 *        (5) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xMemPoolCreate(TMemPool* pPool, void* pAddr, TBase32 pages, TBase32 pgsize, TError* pError)
{
    TState state = eFailure;
    TError error = MEM_ERR_FAULT;
    TReg32 imask;
    TIndex index;
    TChar* pTemp;

    CpuEnterCritical(&imask);

    if (!(pPool->Property & MEM_PROP_READY))
    {
        /* ��ձ�������ڴ�ռ� */
        memset(pAddr, 0U, pages * pgsize);

        /* ���������ڴ�ҳ���� */
        pTemp = (TChar*)pAddr;
        for (index = 0; index < pages; index++)
        {
            uObjListAddNode(&(pPool->PageList), (TObjNode*)pTemp, eQuePosTail);
            pTemp += pgsize;
        }

        /* ���������ڴ涼���ڿɷ���״̬ */
        for (index = 0; index < MEM_PAGE_TAGS; index++)
        {
            pPool->PageTags[index] = ~0U;
        }
        pPool->PageAddr  = pAddr;
        pPool->PageAvail = pages;
        pPool->PageNbr   = pages;
        pPool->PageSize  = pgsize;
        pPool->Property  = MEM_PROP_READY;

        error = MEM_ERR_NONE;
        state = eSuccess;
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: �����ڴ��                                                                             *
 *  ����: (1) pPool      �ڴ�ؽṹ��ַ                                                          *
 *        (2) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xMemPoolDelete(TMemPool* pPool, TError* pError)
{
    TReg32 imask;
    TState state = eFailure;
    TError error = MEM_ERR_UNREADY;

    CpuEnterCritical(&imask);
    if (pPool->Property & MEM_PROP_READY)
    {
        memset(pPool->PageAddr, 0, pPool->PageNbr * pPool->PageSize);
        memset(pPool, 0, sizeof(TMemPool));
        error = MEM_ERR_NONE;
        state = eSuccess;
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: ���ڴ������������ڴ�                                                                 *
 *  ����: (1) pPool      �ڴ�ؽṹ��ַ                                                          *
 *        (2) pAddr2     �������뵽���ڴ��ָ�����                                              *
 *        (3) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xPoolMemMalloc(TMemPool* pPool, void** pAddr2, TError* pError)
{
    TState state = eFailure;
    TError error = MEM_ERR_UNREADY;
    TReg32 imask;
    TIndex x;
    TIndex y;
    TIndex index;
    TChar* pTemp;

    CpuEnterCritical(&imask);

    if (pPool->Property & MEM_PROP_READY)
    {
        /* ����ڴ�ش��ڿ����ڴ�ҳ */
        if (pPool->PageAvail > 0U)
        {
            /* �����ڴ�ҳ�����ȥ */
            pTemp = (TChar*)(pPool->PageList);
            uObjListRemoveNode(&(pPool->PageList), (TObjNode*)pTemp);
            pPool->PageAvail--;
            *pAddr2 = (void*)pTemp;

            /* ��Ǹ��ڴ�ҳ�Ѿ������� */
            index = (pTemp - pPool->PageAddr)/(pPool->PageSize);
            y = (index >> 5);
            x = (index & 0x1f);
            pPool->PageTags[y]  &= ~(0x1 << x);

            /* ��ո��ڴ�ҳ���� */
            memset((void*)pTemp, 0U, pPool->PageSize);

            error = MEM_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = MEM_ERR_NO_MEM;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: ���ڴ�����ͷ��ڴ�                                                                     *
 *  ����: (1) pPool      �ڴ�ؽṹ��ַ                                                          *
 *        (2) pAddr      ���ͷ��ڴ�ĵ�ַ                                                        *
 *        (3) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState xPoolMemFree (TMemPool* pPool, void* pAddr, TError* pError)
{
    TState state = eFailure;
    TError error = MEM_ERR_UNREADY;
    TReg32 imask;
    TIndex index;
    TChar* pTemp;
    TBase32 x;
    TBase32 y;
	TBase32 tag;

    CpuEnterCritical(&imask);

    if (pPool->Property & MEM_PROP_READY)
    {
        /* ����ڴ��ȷʵ���ڴ�ҳ�������ȥ��δ���� */
        if (pPool->PageAvail < pPool->PageNbr)
        {
            /* ����ͷŵ��ڴ��ַ�Ƿ���Ĵ��ں��ʵĿ���ʼ��ַ�ϡ�
               �˴�����Ҫ�󱻹�����ڴ�ռ������������ */
            index = ((TChar*)pAddr - pPool->PageAddr) / (pPool->PageSize);
          //  index = (index < pPool->PageNbr) ? index : pPool->PageNbr;
            pTemp = pPool->PageAddr + index * pPool->PageSize;

            /* ����õ�ַ����������ȷʵ�Ǵ���ĳ���ڴ�ҳ���׵�ַ */
            if (pTemp == (TChar*)pAddr)
            {
                /* ����ڴ�ҳ�����ǣ������ٴ��ͷ��Ѿ��ͷŹ����ڴ�ҳ��ַ */
                y = (index >> 5);
                x = (index & 0x1f);
				tag = pPool->PageTags[y] & (0x1 << x);
                if (tag == 0)
                {
                    /* ��ո��ڴ�ҳ */
                    memset(pAddr, 0U, pPool->PageSize);

                    /* �ջظõ�ַ���ڴ�ҳ */
                    uObjListAddNode(&(pPool->PageList), (TObjNode*)pAddr, eQuePosTail);
                    pPool->PageAvail++;

                    /* ��Ǹ��ڴ�ҳ���Ա����� */
                    pPool->PageTags[y] |= (0x1 << x);

                    error = MEM_ERR_NONE;
                    state = eSuccess;
                }
                else
                {
                    error = MEM_ERR_DBL_FREE;
                }
            }
            else
            {
                error = MEM_ERR_BAD_ADDR;
            }
        }
        else
        {
            error = MEM_ERR_POOL_FULL;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}

#endif

