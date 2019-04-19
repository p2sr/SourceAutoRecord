#pragma once
#include "Session.hpp"

class InfraSession : public Session {
public:
    InfraSession();
    void Changed() override;
};
