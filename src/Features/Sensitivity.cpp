#include "Command.hpp"
#include "Modules/Engine.hpp"
#include "Variable.hpp"


auto sensitivity = Variable("sensitivity");
auto m_yaw = Variable("m_yaw");

CON_COMMAND(sar_sensitivity, "sar_sensitivity <cm/360> <dpi> - sets the sensitivity to the specified cm/360 value based on the supplied dpi value\n") {
	if (args.ArgC() != 3) {
		return console->Print(sar_sensitivity.ThisPtr()->m_pszHelpString);
	}

	double cm_per_three_sixty = std::atof(args[1]);
	double dpi = std::atof(args[2]);
	double new_sens = 914.4 / (cm_per_three_sixty * (dpi * m_yaw.GetFloat()));

	sensitivity.SetValue((float)new_sens);
}
