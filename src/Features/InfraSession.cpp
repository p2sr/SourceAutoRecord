#include "InfraSession.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Utils/SDK.hpp"

InfraSession::InfraSession()
    : Session()
{
    this->prevState = INFRA_HS_RUN;
}
void InfraSession::Changed()
{
    console->DevMsg("m_currentState = %i\n", engine->hoststate->m_currentState);

    if (engine->hoststate->m_currentState == INFRA_HS_LOAD_GAME_WITHOUT_RESTART
        || engine->hoststate->m_currentState == INFRA_HS_CHANGE_LEVEL_SP
        || engine->hoststate->m_currentState == INFRA_HS_CHANGE_LEVEL_MP
        || engine->hoststate->m_currentState == INFRA_HS_GAME_SHUTDOWN) {
        this->Ended();
    } else if (this->prevState == INFRA_HS_LOAD_GAME_WITHOUT_RESTART
        && engine->hoststate->m_currentState == INFRA_HS_RUN) {
        this->Started();
    } else if (engine->hoststate->m_currentState == INFRA_HS_RUN
        && !engine->hoststate->m_activeGame
        && engine->GetMaxClients() <= 1) {
        this->Started(true);
    }
}
