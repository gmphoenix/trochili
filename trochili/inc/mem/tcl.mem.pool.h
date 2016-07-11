/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCLC_MEMORY_POOL_H
#define _TCLC_MEMORY_POOL_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.memory.h"

#if ((TCLC_MEMORY_ENABLE) && (TCLC_MEMORY_POOL_ENABLE))

#define MEM_PAGE_TAGS  ((TCLC_MEMORY_POOL_PAGES + 31U) >> 5u)

/* �ڴ�ؿ��ƿ�ṹ */
struct MemPoolDef
{
    TProperty Property;                   /* �ڴ�ҳ������                      */
    TChar*    PageAddr;                   /* ��������ڴ����ʼ��ַ            */
    TBase32   PageSize;                   /* �ڴ�ҳ��С                        */
    TBase32   PageNbr;                    /* �ڴ�ҳ��Ŀ                        */
    TBase32   PageAvail;                  /* �����ڴ�ҳ��Ŀ                    */
    TBase32   PageTags[MEM_PAGE_TAGS];    /* �ڴ�ҳ�Ƿ���ñ��                */
    TObjNode* PageList;                   /* �����ڴ�ҳ����ͷָ��              */
};
typedef struct MemPoolDef TMemPool;

extern TState xMemPoolCreate(TMemPool* pPool, void* pData, TBase32 pages, TBase32 pgsize, TError* pError);
extern TState xMemPoolDelete(TMemPool* pPool, TError* pError);
extern TState xPoolMemMalloc(TMemPool* pPool, void** pAddr2, TError* pError);
extern TState xPoolMemFree (TMemPool* pPool, void* pAddr, TError* pError);
#endif

#endif /* _TCLC_MEMORY_POOL_H  */

