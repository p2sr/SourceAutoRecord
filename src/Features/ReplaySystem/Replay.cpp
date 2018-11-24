#include "Replay.hpp"

#include <cstring>

Replay::Replay(int viewSize)
    : views(viewSize)
    , source("[unsaved]")
{
}
Replay::Replay(int viewSize, const char* source)
    : Replay(viewSize)
{
    std::strncpy(this->source, source, sizeof(this->source));
}
ReplayView* Replay::GetView(int view)
{
    if (view > this->GetViewSize())
        return nullptr;

    return &this->views[view];
}
int Replay::GetViewSize()
{
    return this->views.size();
}
int Replay::GetFrameSize()
{
    return this->views[0].frames.size();
}
