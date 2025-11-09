#include "Command.hpp"
#include "Modules/Engine.hpp"
#include "Variable.hpp"


#ifdef _WIN32
#	include <winuser.h>
#endif

// Liquipedia Counterstrike https://liquipedia.net/counterstrike/Mouse_Settings#Windows_Sensitivity
const float multiplierEnhancedOff[] = {0.03125, 0.0625, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875, 1, 1.25, 1.5, 1.75, 2, 2.25, 2.5, 2.75, 3, 3.25, 3.5};

CON_COMMAND(sar_sensitivity, "sar_sensitivity <cm|in> <distance> <dpi> - changes the sensitivity to match the amount of distance the mouse needs to travel to turn one whole 360.\n") {
	double distanceConstant;
	if (args.ArgC() == 4) {
		if (!strcmp(args[1], "cm")) {
			distanceConstant = 914.4;
		} else if (!strcmp(args[1], "in")) {
			distanceConstant = 360;
		} else {
			return console->Print("Cannot parse unit entered, options are cm or in\n");
		}
	} else {
		return console->Print(sar_sensitivity.ThisPtr()->m_pszHelpString);
	}

	static auto sensitivity = Variable("sensitivity");
	static auto m_yaw = Variable("m_yaw");
	static auto m_rawinput = Variable("m_rawinput");

	double distance = std::atof(args[2]);
	double dpi = std::atof(args[3]);
	

	if (!m_rawinput.GetBool()) {
#ifdef _WIN32
		// whole thing relies on the m_rawinput if that shit is on then it do not matter
		// first two values of this area are lowkey not useful in this situation i think we just need the last one
		int getMouseInfo[3];
		int winMouseSens;
		bool mouseAccel = SystemParametersInfo(SPI_GETMOUSE, 0, &getMouseInfo, 0);
		bool mouseSpeed = SystemParametersInfo(SPI_GETMOUSESPEED, 0, &winMouseSens, 0);

		if (mouseAccel && mouseSpeed) {
			if (getMouseInfo[2]) {
				/* if true then Enhanced Pointer Performance is on.
				 * conveniently the multiplier when this is the case is just 1/10th the setting
				 * AS FAR AS I AM AWARE: this is how it essentially works, because at default setting (10, or 1.0)
				 * every dot the cursor is moved 1 pixel
				 * and such at non default values it is getting scaled
				 */
				dpi = dpi * (winMouseSens / 10);
			} else {
				// otherwise its easier to just look it up in this array
				dpi = dpi * multiplierEnhancedOff[winMouseSens - 1];
			}
		} else {
			console->Print("Could not retrieve windows mouse settings, sens may not be calculated correctly\n");
		}
	
#else
		console->Print("m_rawinput 0 may make sar_sensitivity inaccurate on linux\n");
#endif
	}
	double new_sens = distanceConstant / (distance * (dpi * m_yaw.GetFloat()));
	sensitivity.SetValue((float)new_sens);
}
