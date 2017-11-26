using System;
using System.Diagnostics;
using System.IO;
using System.Text;

namespace Injector
{
	// Dll injection method
	internal static class Program
	{
		private const string _defaultProc = "portal2";
		private const string _defaultLib = "SourceAutoRecord.dll";

		private static void Main(string[] args)
		{
			string Inject(string processName, string dllPath)
			{
				try
				{
					var process = Array.Find(Process.GetProcesses(), p => p.ProcessName == processName);
					if (process == null)
						return $"Cannot find process named {processName}!";

					if (!File.Exists(dllPath))
						return $"Cannot find dll named {dllPath}!";

					var kernel = Kernel32.LoadLibraryA("kernel32.dll");
					var load = Kernel32.GetProcAddress(kernel, "LoadLibraryA");
					Kernel32.FreeLibrary(kernel);
					if (load == UIntPtr.Zero)
						return "Cannot load LoadLibraryA!";

					var handle = Kernel32.OpenProcess(ProcessAccess.AllAccess, false, process.Id);
					if (handle == IntPtr.Zero)
						return "Cannot open process!";

					var bytes = Encoding.ASCII.GetBytes(dllPath);
					var gate = Kernel32.VirtualAllocEx(handle, IntPtr.Zero, (uint)bytes.Length, (uint)VirtualAllocExTypes.MEM_COMMIT_OR_RESERVE, (uint)PageProtection.PAGE_READWRITE);
					if (gate == IntPtr.Zero)
						return "Cannot reserve memory!";

					if (!Kernel32.WriteProcessMemory(handle, gate, bytes, (uint)bytes.Length, out _))
						return "Cannot write memory to process!";

					if (Kernel32.CreateRemoteThread(handle, IntPtr.Zero, 0u, load, gate, 0u, out _) == IntPtr.Zero)
						return "Cannot open remote thread!";
				}
				catch (Exception ex)
				{
					return ex.ToString();
				}
				return null;
			}

			// Defaults
			var proc = _defaultProc;
			var dll = Path.Combine(Path.GetDirectoryName(typeof(Program).Assembly.Location), _defaultLib);

			// Parse optional arguments
			if (args.Length == 2)
			{
				if (!string.IsNullOrEmpty(args[0]))
					proc = args[0];
				if (args[1].EndsWith(".dll"))
					dll = Path.Combine(Path.GetDirectoryName(typeof(Program).Assembly.Location), args[1]);
			}
			else if (args.Length == 1)
			{
				if (args[0].EndsWith(".dll"))
					dll = Path.Combine(Path.GetDirectoryName(typeof(Program).Assembly.Location), args[0]);
				else if (!string.IsNullOrEmpty(args[0]))
					proc = args[0];
			}

			var result = Inject(proc, dll);
			if (!string.IsNullOrEmpty(result))
				Console.WriteLine($"Error: {result}");
			else
				Console.WriteLine($"Successfully injected {dll} into process {proc}!");

			Console.ReadKey();
		}
	}
}