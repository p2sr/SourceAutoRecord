# vmthook
Basic class for hooking virtual methods.

# Example
```
/* define target function prototype */
typedef void (*FrameStageNotify)(void*, ClientFrameStage_t);

/* hook function in clientdll virtual method table */
clienthook = new VMTHook(clientdll);
clienthook->HookFunction((void*)hkFrameStageNotify, 36);
```

```
void hkFrameStageNotify(void* thisptr, ClientFrameStage_t Stage) {
    /* call the original function inside our hook */
    clienthook->GetOriginal<FrameStageNotify>(36)(thisptr, Stage);
}
```