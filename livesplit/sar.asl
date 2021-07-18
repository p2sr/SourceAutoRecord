/*
    SourceAutoRecord 1.8+

    Report issues at:
        https://github.com/NeKzor/SourceAutoRecord/issues
*/

state("portal2") { /* Portal 2 (7054) */ }
state("hl2") { /* Portal (1910503) */ }

startup
{
    vars.SAR = new MemoryWatcherList();
    vars.FindInterface = (Func<Process, bool>)((proc) =>
    {
        // TimerInterface
        var target = new SigScanTarget(16,
            "53 41 52 5F 54 49 4D 45 52 5F 53 54 41 52 54 00", // char start[16]
            "?? ?? ?? ??", // int total
            "?? ?? ?? ??", // float ipt
            "?? ?? ?? ??", // TimerAction action
            "53 41 52 5F 54 49 4D 45 52 5F 45 4E 44 00"); // char end[14]

        var result = IntPtr.Zero;
        foreach (var page in proc.MemoryPages(true))
        {
            var scanner = new SignatureScanner(proc, page.BaseAddress, (int)page.RegionSize);
            result = scanner.Scan(target);

            if (result != IntPtr.Zero)
            {
                print("[ASL] pubInterface = 0x" + result.ToString("X"));
                vars.Total = new MemoryWatcher<int>(result);
                vars.Ipt = new MemoryWatcher<float>(result + sizeof(int));
                vars.Action = new MemoryWatcher<int>(result + sizeof(int) + sizeof(float));

                vars.SAR.Clear();
                vars.SAR.AddRange(new MemoryWatcher[]
                {
                    vars.Total,
                    vars.Ipt,
                    vars.Action
                });
                vars.SAR.UpdateAll(proc);

                print("[ASL] pubInterface->ipt = " + vars.Ipt.Current.ToString());
                return true;
            }
        }

        print("[ASL] Memory scan failed!");
        return false;
    });

    vars.TimerModel = new TimerModel{ CurrentState = timer };

    vars.TimerAction = new Dictionary<string, int>()
    {
        { "none",    0 },
        { "start",   1 },
        { "restart", 2 },
        { "split",   3 },
        { "end",     4 },
        { "reset",   5 },
    };
}

init
{
    vars.Init = false;
}

update
{
    if (vars.Init)
    {
        timer.IsGameTimePaused = true;
        vars.SAR.UpdateAll(game);

        if (modules.FirstOrDefault(m => m.ModuleName == "sar.dll") == null)
        {
            vars.Init = false;
        }
    }
    else
    {
        if (modules.FirstOrDefault(m => m.ModuleName == "sar.dll") != null)
        {
            vars.Init = vars.FindInterface(game);
        }
    }

    if (vars.Init) {
        if (vars.Action.Changed && (vars.Action.Current == vars.TimerAction["start"] || vars.Action.Current == vars.TimerAction["restart"] || vars.Action.Current == vars.TimerAction["reset"])) {
            vars.TimerModel.Reset();
        }
    }

    return vars.Init;
}

gameTime
{
    return TimeSpan.FromSeconds(vars.Total.Current * vars.Ipt.Current);
}

start
{
    return vars.Action.Changed
        && (vars.Action.Current == vars.TimerAction["start"] || vars.Action.Current == vars.TimerAction["restart"]);
}

split
{
    return vars.Action.Changed
        && (vars.Action.Current == vars.TimerAction["split"] || vars.Action.Current == vars.TimerAction["end"]);
}
