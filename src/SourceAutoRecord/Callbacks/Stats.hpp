#pragma once
#include "Modules/Client.hpp"
#include "Modules/ConCommandArgs.hpp"
#include "Modules/Console.hpp"
#include "Modules/DemoRecorder.hpp"
#include "Modules/Engine.hpp"
#include "Modules/InputSystem.hpp"

#include "Features/Demo.hpp"
#include "Features/Rebinder.hpp"
#include "Features/Stats.hpp"
#include "Features/Summary.hpp"
#include "Features/Teleporter.hpp"
#include "Features/Timer.hpp"
#include "Features/TimerAverage.hpp"
#include "Features/TimerCheckPoints.hpp"

#pragma once
#include "Features/Stats.hpp"

namespace Callbacks
{
	void ResetJumps()
	{
		Stats::TotalJumps = 0;
	}
}