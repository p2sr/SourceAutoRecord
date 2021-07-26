#include "CrashHandler.hpp"

#include "SAR.hpp"
#include "Version.hpp"

#include <stdint.h>
#include <stdio.h>

#define CRASH_REPORTS_DIR "crash_reports"

#ifdef _WIN32
// clang-format off
#	include <Windows.h>
#	include <DbgHelp.h>
#	include <direct.h>
// clang-format on
#else
#	include <execinfo.h>
#	include <signal.h>
#	include <string.h>
#	include <sys/stat.h>
#endif

#ifdef _WIN32
static LONG WINAPI handler(EXCEPTION_POINTERS *ExceptionInfo)
#else
static void handler(int signal, siginfo_t *info, void *ucontext)
#endif
{
#ifdef _WIN32
#	define finish return EXCEPTION_EXECUTE_HANDLER
#else
#	define finish exit(1)
#endif

	const char *signame;
	void *faultaddr;

#ifdef _WIN32
	switch (ExceptionInfo->ExceptionRecord->ExceptionCode) {
	case EXCEPTION_ACCESS_VIOLATION:
	case EXCEPTION_STACK_OVERFLOW:
		signame = "Segmentation fault";
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		signame = "Illegal instruction";
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_INEXACT_RESULT:
	case EXCEPTION_FLT_INVALID_OPERATION:
	case EXCEPTION_FLT_OVERFLOW:
	case EXCEPTION_FLT_STACK_CHECK:
	case EXCEPTION_FLT_UNDERFLOW:
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_INT_OVERFLOW:
		signame = "Arithmetic exception";
		break;
	default:
		finish;
	}
	faultaddr = ExceptionInfo->ExceptionRecord->ExceptionAddress;
#else
	if (signal != SIGSEGV && signal != SIGILL && signal != SIGFPE) {
		// We shouldn't be getting this
		finish;
	}
	signame = strsignal(signal);
	faultaddr = info->si_addr;
#endif

	char filename[64];
	time_t t = time(NULL);
	struct tm *tm = gmtime(&t);
	strftime(filename, sizeof filename, CRASH_REPORTS_DIR "/%Y-%m-%d_%H.%M.%S.txt", tm);

#ifdef _WIN32
	_mkdir(CRASH_REPORTS_DIR);
#else
	mkdir(CRASH_REPORTS_DIR, 0777);
#endif

	FILE *f = fopen(filename, "w");

	if (!f) {
		finish;
	}

	fputs("SAR " SAR_VERSION " (Built " SAR_BUILT ")\n", f);
	fprintf(f, "%s caused by address 0x%08x\n", signame, (uint32_t)faultaddr);

#ifdef _WIN32
	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();

	CONTEXT *ctx = ExceptionInfo->ContextRecord;
	SymInitialize(process, 0, true);

	STACKFRAME frame = {0};
	frame.AddrPC.Offset = ctx->Eip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrStack.Offset = ctx->Esp;
	frame.AddrStack.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = ctx->Ebp;
	frame.AddrFrame.Mode = AddrModeFlat;

	while (StackWalk(
		IMAGE_FILE_MACHINE_I386,
		process,
		thread,
		&frame,
		ctx,
		0,
		SymFunctionTableAccess,
		SymGetModuleBase,
		0)) {
		void *addr = (void *)frame.AddrPC.Offset;
		DWORD64 displacement;

		char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
		PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = MAX_SYM_NAME;
		const char *symname;
		char modulefile[MAX_PATH + 1];
		modulefile[0] = 0;
		if (SymFromAddr(process, (ULONG64)addr, &displacement, symbol)) {
			symname = symbol->NameLen ? symbol->Name : "";
			displacement = (uint32_t)addr - (uint32_t)symbol->ModBase;
			GetModuleFileNameA((HMODULE)symbol->ModBase, modulefile, MAX_PATH);
		} else {
			symname = "";
		}


		fprintf(f, "\t%s(%s+0x%x) [0x%08x]\n", modulefile, symname, (uint32_t)displacement, (uint32_t)addr);
	}

	SymCleanup(process);
#else
	void *frames[128];
	int nframes = backtrace(frames, sizeof frames / sizeof frames[0]);
	char **symbols = backtrace_symbols(frames, nframes);

	// The first 2 frames are this function and linux-gate's signal
	// handler
	if (nframes <= 2) {
		fputs("\t<no stack frames>\n", f);
	} else {
		for (int i = 2; i < nframes; ++i) {
			fprintf(f, "\t%s\n", symbols[i]);
		}
	}

	if (nframes == sizeof frames / sizeof frames[0]) {
		fputs("\t<more frames may follow>\n", f);
	}
#endif

	fclose(f);

	finish;
#undef finish
}

#ifdef _WIN32

void CrashHandler::Init() {
	AddVectoredExceptionHandler(1, &handler);
}

void CrashHandler::Cleanup() {
	RemoveVectoredExceptionHandler(&handler);
}

#else

static struct sigaction g_old_segv, g_old_ill, g_old_fpe;

void CrashHandler::Init() {
	struct sigaction action = {0};

	action.sa_sigaction = &handler;
	action.sa_flags = SA_SIGINFO;
	sigfillset(&action.sa_mask);

	sigaction(SIGSEGV, &action, &g_old_segv);
	sigaction(SIGILL, &action, &g_old_ill);
	sigaction(SIGFPE, &action, &g_old_fpe);
}

void CrashHandler::Cleanup() {
	sigaction(SIGSEGV, &g_old_segv, NULL);
	sigaction(SIGILL, &g_old_ill, NULL);
	sigaction(SIGFPE, &g_old_fpe, NULL);
}

#endif
