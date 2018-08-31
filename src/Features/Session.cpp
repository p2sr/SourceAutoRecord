#include "Session.hpp"

Session::Session()
    : baseTick(0)
    , lastSession(0)
{
    this->hasLoaded = true;
}
void Session::Rebase(const int from)
{
    this->baseTick = from;
}

Session* session;
