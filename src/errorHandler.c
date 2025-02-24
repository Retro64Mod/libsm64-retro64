// References: https://spin.atomicobject.com/2013/01/13/exceptions-stack-traces-c/

#include "errorHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <imagehlp.h>


void windows_print_stacktrace(CONTEXT* context,FILE* crashlog)
{
  SymInitialize(GetCurrentProcess(), 0, true);
 
  STACKFRAME frame = { 0 };
 
  /* setup initial stack frame */
  frame.AddrPC.Offset         = context->Rip;
  frame.AddrPC.Mode           = AddrModeFlat;
  frame.AddrStack.Offset      = context->Rsp;
  frame.AddrStack.Mode        = AddrModeFlat;
  frame.AddrFrame.Offset      = context->Rbp;
  frame.AddrFrame.Mode        = AddrModeFlat;
 int i = 0;
 void* baddr = GetModuleHandle("sm64.dll");
  while (StackWalk(IMAGE_FILE_MACHINE_AMD64 ,
                   GetCurrentProcess(),
                   GetCurrentThread(),
                   &frame,
                   context,
                   0,
                   SymFunctionTableAccess,
                   SymGetModuleBase,
                   0 ) )
  {
      fprintf(crashlog,"[%i]: PC:%p (PCR:%p) ST:%p FR:%p\n",i, 
      (void*)frame.AddrPC.Offset,
      ((void*)frame.AddrPC.Offset)-baddr,
      (void*)frame.AddrStack.Offset,
      (void*)frame.AddrFrame.Offset);
      i++;
  }
  SymCleanup( GetCurrentProcess() );
}

LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS * ExceptionInfo)
{
  FILE* crashlog = fopen("retro64_crash.log", "w");
  void* addr = GetModuleHandle("sm64.dll");
  fprintf(crashlog,"== LIBSM64 RETRO64 CRASH ==\n");
  fprintf(crashlog,"Exception: %p\n", ExceptionInfo->ExceptionRecord->ExceptionAddress);
  fprintf(crashlog,"Base: %p\n", addr);
  switch(ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
    case EXCEPTION_ACCESS_VIOLATION:
      fputs("Error: EXCEPTION_ACCESS_VIOLATION\n", stderr);
      break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
      fputs("Error: EXCEPTION_ARRAY_BOUNDS_EXCEEDED\n", stderr);
      break;
    case EXCEPTION_BREAKPOINT:
      fputs("Error: EXCEPTION_BREAKPOINT\n", stderr);
      break;
    case EXCEPTION_DATATYPE_MISALIGNMENT:
      fputs("Error: EXCEPTION_DATATYPE_MISALIGNMENT\n", stderr);
      break;
    case EXCEPTION_FLT_DENORMAL_OPERAND:
      fputs("Error: EXCEPTION_FLT_DENORMAL_OPERAND\n", stderr);
      break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      fputs("Error: EXCEPTION_FLT_DIVIDE_BY_ZERO\n", stderr);
      break;
    case EXCEPTION_FLT_INEXACT_RESULT:
      fputs("Error: EXCEPTION_FLT_INEXACT_RESULT\n", stderr);
      break;
    case EXCEPTION_FLT_INVALID_OPERATION:
      fputs("Error: EXCEPTION_FLT_INVALID_OPERATION\n", stderr);
      break;
    case EXCEPTION_FLT_OVERFLOW:
      fputs("Error: EXCEPTION_FLT_OVERFLOW\n", stderr);
      break;
    case EXCEPTION_FLT_STACK_CHECK:
      fputs("Error: EXCEPTION_FLT_STACK_CHECK\n", stderr);
      break;
    case EXCEPTION_FLT_UNDERFLOW:
      fputs("Error: EXCEPTION_FLT_UNDERFLOW\n", stderr);
      break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
      fputs("Error: EXCEPTION_ILLEGAL_INSTRUCTION\n", stderr);
      break;
    case EXCEPTION_IN_PAGE_ERROR:
      fputs("Error: EXCEPTION_IN_PAGE_ERROR\n", stderr);
      break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
      fputs("Error: EXCEPTION_INT_DIVIDE_BY_ZERO\n", stderr);
      break;
    case EXCEPTION_INT_OVERFLOW:
      fputs("Error: EXCEPTION_INT_OVERFLOW\n", stderr);
      break;
    case EXCEPTION_INVALID_DISPOSITION:
      fputs("Error: EXCEPTION_INVALID_DISPOSITION\n", stderr);
      break;
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
      fputs("Error: EXCEPTION_NONCONTINUABLE_EXCEPTION\n", stderr);
      break;
    case EXCEPTION_PRIV_INSTRUCTION:
      fputs("Error: EXCEPTION_PRIV_INSTRUCTION\n", stderr);
      break;
    case EXCEPTION_SINGLE_STEP:
      fputs("Error: EXCEPTION_SINGLE_STEP\n", stderr);
      break;
    case EXCEPTION_STACK_OVERFLOW:
      fputs("Error: EXCEPTION_STACK_OVERFLOW\n", stderr);
      break;
    default:
      fputs("Error: Unrecognized Exception\n", stderr);
      break;
  }
  fflush(stderr);
  /* If this is a stack overflow then we can't walk the stack, so just show
    where the error happened */
  if (EXCEPTION_STACK_OVERFLOW != ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
      windows_print_stacktrace(ExceptionInfo->ContextRecord,crashlog);
  }
  else
  {
      fprintf(crashlog,"[RIP]: %p ", (void*)ExceptionInfo->ContextRecord->Rip,crashlog);
  }
  fflush(crashlog);
  fclose(crashlog);
  return EXCEPTION_EXECUTE_HANDLER;
}
#include <signal.h>
void handleErrors(){
    //SetUnhandledExceptionFilter(windows_exception_handler);
    //signal( SIGSEGV, windows_exception_handler );
}
#else
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
void handler(int sig) {
  void *array[10];
  size_t size;
  FILE* crashlog = fopen("retro64_crash.log", "w");
  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(crashlog, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, crashlog);
  fflush(crashlog);
  fclose(crashlog);
  exit(1);
}
void handleErrors(){
    signal( SIGSEGV, handler );
}
#endif