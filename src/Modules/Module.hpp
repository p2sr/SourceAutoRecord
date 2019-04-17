#pragma once
#include <vector>

class Module {
public:
    bool hasLoaded = false;

public:
    virtual ~Module() = default;
    virtual bool Init() = 0;
    virtual void Shutdown() = 0;
    virtual const char* Name() = 0;
};

class Modules {
public:
    std::vector<Module*> list;

public:
    Modules();
    template <typename T = Module>
    void AddModule(T** modulePtr)
    {
        *modulePtr = new T();
        this->list.push_back(*modulePtr);
    }
    template <typename T = Module>
    void RemoveModule(T** modulePtr)
    {
        this->list.erase(*modulePtr);
    }
    void InitAll();
    void ShutdownAll();
    void DeleteAll();
    ~Modules();
};
