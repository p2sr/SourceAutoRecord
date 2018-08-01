#pragma once

namespace Session {

int BaseTick = 0;
int LastSession = 0;

void Rebase(int from)
{
    BaseTick = from;
}
}