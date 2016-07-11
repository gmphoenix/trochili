/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.kernel.h"
#include "tcl.debug.h"


/*************************************************************************************************
 *  ���ܣ��ں˳������������ֹͣ��������                                                         *
 *  ������(1) pNote  ��ʾ�ַ���                                                                  *
 *        (2) pFile  ������ļ���                                                                *
 *        (3) pFunc  ����ĺ�����                                                                *
 *        (4) line   ����Ĵ�����                                                                *
 *  ���أ���                                                                                     *
 *  ˵���������ں���ϵ�����������ʱ��ȡ���ж�                                                 *
 *************************************************************************************************/
void uDebugPanic(const char* pNote, const char* pFile, const char* pFunc, int line)
{
    CpuDisableInt();		
    uKernelTrace(pNote);
    uKernelVariable.DBGLog.File = pFile;
    uKernelVariable.DBGLog.Func = pFunc;
    uKernelVariable.DBGLog.Line = line;
    uKernelVariable.DBGLog.Note = pNote;
    while (eTrue)
    {
        ;
    }
}

