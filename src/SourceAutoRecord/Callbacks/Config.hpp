#pragma once
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Utils.hpp"

namespace Callbacks
{
	void SaveCvars()
	{
		std::ofstream file(Engine::GetDir() + std::string("\\cfg\\_sar_cvars.cfg"), std::ios::out | std::ios::trunc);
		if (!file.good()) {
			Console::Print("Failed to create config file!\n");
			return;
		}

		auto spacing = sar_hud_default_spacing.GetInt();
		auto xpadding = sar_hud_default_padding_x.GetInt();
		auto ypadding = sar_hud_default_padding_y.GetInt();
		auto index = sar_hud_default_font_index.GetInt();
		auto size = sar_hud_default_font_size.GetInt();
		auto color = sar_hud_default_font_color.GetString();

		file << "sar_hud_default_spacing " << spacing << "\n";
		file << "sar_hud_default_padding_x " << xpadding << "\n";
		file << "sar_hud_default_padding_y " << ypadding << "\n";
		file << "sar_hud_default_font_index " << index << "\n";
		file << "sar_hud_default_font_size " << size << "\n";
		file << "sar_hud_default_font_color " << color;

		Console::Print("Saved important settings in /cfg/_sar_cvars.cfg!\n");
	}
	void LoadCvars()
	{
		std::ifstream file(Engine::GetDir() + std::string("\\cfg\\_sar_cvars.cfg"), std::ios::in);
		if (!file.good()) {
			Console::Print("Config file not found!\n");
			return;
		}
		Engine::ExecuteCommand("exec _sar_cvars.cfg");
	}
}