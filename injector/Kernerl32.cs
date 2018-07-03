using System;
using System.Runtime.InteropServices;

namespace Injector
{
	[Flags]
	public enum ProcessAccess
	{
		Terminate = 0x1,
		CreateThread = 0x2,
		VMOperation = 0x8,
		VMRead = 0x10,
		VMWrite = 0x20,
		DuplicateHandle = 0x40,
		SetInformation = 0x200,
		QueryInformation = 0x400,
		Synchronize = 0x0010_0000,
		AllAccess = CreateThread | DuplicateHandle | QueryInformation | SetInformation | Terminate | VMOperation | VMRead | VMWrite | Synchronize
	}

	[Flags]
	public enum PageProtection : uint
	{
		PAGE_NOACCESS = 0x01,
		PAGE_READONLY = 0x02,
		PAGE_READWRITE = 0x04,
		PAGE_WRITECOPY = 0x08,
		PAGE_EXECUTE = 0x10,
		PAGE_EXECUTE_READ = 0x20,
		PAGE_EXECUTE_READWRITE = 0x40,
		PAGE_EXECUTE_WRITECOPY = 0x80,
		PAGE_GUARD = 0x100,
		PAGE_NOCACHE = 0x200,
		PAGE_WRITECOMBINE = 0x400,
	}

	public enum VirtualAllocExTypes
	{
		WRITE_WATCH_FLAG_RESET = 0x0000_0001,
		MEM_COMMIT = 0x0000_1000,
		MEM_RESERVE = 0x0000_2000,
		MEM_COMMIT_OR_RESERVE = 0x0000_3000,
		MEM_DECOMMIT = 0x0000_4000,
		MEM_RELEASE = 0x0000_8000,
		MEM_FREE = 0x000_10000,
		MEM_protected = 0x000_20000,
		MEM_MAPPED = 0x000_40000,
		MEM_RESET = 0x000_80000,
		MEM_TOP_DOWN = 0x0010_0000,
		MEM_WRITE_WATCH = 0x0020_0000,
		MEM_PHYSICAL = 0x0040_0000,
		SEC_IMAGE = 0x01000000,
		MEM_IMAGE = SEC_IMAGE
	}

	public static class Kernel32
	{
		[DllImport("kernel32", SetLastError = true, CharSet = CharSet.Ansi)]
		public static extern IntPtr LoadLibraryA(string lpFileName);

		[DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
		public static extern UIntPtr GetProcAddress(IntPtr hModule, string procName);

		[DllImport("kernel32.dll", SetLastError = true)]
		[return: MarshalAs(UnmanagedType.Bool)]
		public static extern bool FreeLibrary(IntPtr hModule);

		[DllImport("kernel32.dll")]
		public static extern IntPtr OpenProcess(ProcessAccess dwDesiredAccess, [MarshalAs(UnmanagedType.Bool)] bool bInheritHandle, int dwProcessId);

		[DllImport("kernel32.dll", SetLastError = true, ExactSpelling = true)]
		public static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, UInt32 flAllocationType, UInt32 flProtect);

		[DllImport("kernel32.dll", SetLastError = true)]
		public static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out UIntPtr lpNumberOfBytesWritten);

		[DllImport("kernel32.dll")]
		public static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, uint dwStackSize, UIntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, out IntPtr lpThreadId);
	}
}