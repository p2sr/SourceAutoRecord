#include "Module.hpp"

#include <vector>

void Modules::InitAll()
{
    for (const auto& m : this->list) {
        if (!m->hasLoaded) {
            m->Init();
        }
    }
}
void Modules::ShutdownAll()
{
    for (const auto& m : this->list) {
        m->Shutdown();
    }
}

std::vector<Module*> Modules::list;
Modules* modules;
