#pragma once
#include <vector>

class Module {
public:
    bool hasLoaded;

public:
    virtual bool Init() = 0;
    virtual void Shutdown() = 0;
};

class Modules {
public:
    static std::vector<Module*> list;

public:
    template <typename T = Module>
    void AddModule(T** modulePtr)
    {
        *modulePtr = new T();
        list.push_back(*modulePtr);
    }
    template <typename T = Module>
    void RemoveModule(T** modulePtr)
    {
        list.erase(*modulePtr);
    }
    void InitAll();
    void ShutdownAll();
};

extern Modules* modules;
