#pragma once
#include "Utils.h"

namespace Patterns
{
	// server.dll
	Pattern CheckJumpButton = Pattern
	{
		"server.dll",
		std::vector<Signature>
		{
			{
				// CPortalGameMovement::CheckJumpButton
				// \x55\x8B\xEC\x83\xEC\x0C\x56\x8B\xF1\x8B\x4E\x04 xxxxxxxxxxxx
				{ "Portal 2 Build 6879" },
				{ "55 8B EC 83 EC 0C 56 8B F1 8B 4E 04" }
			},
			{
				// CINFRAGameMovement::CheckJumpButton
				// \x55\x8B\xEC\x83\xEC\x18\x56\x8B\xF1\x8B\x4E\x04 xxxxxxxxxxxx
				{ "INFRA Build 6905" },
				{ "55 8B EC 83 EC 18 56 8B F1 8B 4E 04" }
			}
		}
	};

	// client.dll
	Pattern Paint = Pattern
	{
		"client.dll",
		std::vector<Signature>
		{
			{
				// CFPSPanel::Paint
				// \x53\x8B\xDC\x83\xEC\x08\x83\xE4\xF0\x83\xC4\x04\x55\x8B\x6B\x04\x89\x6C\x24\x04\x8B\xEC\x81\xEC\x00\x00\x00\x00\x56\x8B\xF1\x8B\x0D\x00\x00\x00\x00\x8B\x01\x8B\x96\x00\x00\x00\x00 xxxxxxxxxxxxxxxxxxxxxxxx????xxxxx????xxxx????
				{ "Portal 2 Build 6879" },
				{ "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 56 8B F1 8B 0D ? ? ? ? 8B 01 8B 96 ? ? ? ? " }
			}
		}
	};
	Pattern MatSystemSurfacePtr = Pattern
	{
		"client.dll",
		std::vector<Signature>
		{
			{
				// CNetGraphPanel::DrawTextFields
				// \x55\x8B\xEC\xA1\x00\x00\x00\x00\x81\xEC\x00\x00\x00\x00\x83\x78\x30\x00\x56\x8B\xF1\x0F\x84\x00\x00\x00\x00 xxxx????xx????xxxxxxxxx????
				{ "Portal 2 Build 6879" },
				{ "55 8B EC A1 ? ? ? ? 81 EC ? ? ? ? 83 78 30 00 56 8B F1 0F 84 ? ? ? ? " },
				{ 264 }
			}
		}
	};

	// engine.dll
	Pattern ConVar_Ctor3 = Pattern
	{
		"engine.dll",
		std::vector<Signature>
		{
			{
				// \x55\x8B\xEC\xF3\x0F\x10\x45\x00\x8B\x55\x14 xxxxxxx?xxx
				{ "Portal 2 Build 6879" },
				{ "55 8B EC F3 0F 10 45 ? 8B 55 14" }
			}
		}
	};
	Pattern CvarPtr = Pattern
	{
		"engine.dll",
		std::vector<Signature>
		{
			{
				// \x8B\x0D\x00\x00\x00\x00\x8B\x01\x8B\x50\x40\x68\x00\x00\x00\x00\xFF\xD2\x85\xC0\x74\x17 xx????xxxxxx????xxxxxxx
				{ "Portal 2 Build 6879" },
				{ "8B 0D ? ? ? ? 8B 01 8B 50 40 68 ? ? ? ? FF D2 85 C0 74 17" },
				{ 2 }
			}
		}
	};
	Pattern ConCommand_Ctor1 = Pattern
	{
		"engine.dll",
		std::vector<Signature>
		{
			{
				// 
				{ "Portal 2 Build 6879" },
				{ "55 8B EC 8B 45 0C 53 33 DB 56 8B F1 8B 4D 18 80 66 20 F9" }
			}
		}
	};
	Pattern SetSignonState = Pattern
	{
		"engine.dll",
		std::vector<Signature>
		{
			{
				// CGameClient::SetSignonState
				// \x55\x8B\xEC\x56\x57\x8B\x7D\x08\x8B\xF1\x83\xFF\x02 xxxxxxxxxxxxx
				{ "Portal 2 Build 6879" },
				{ "55 8B EC 56 57 8B 7D 08 8B F1 83 FF 02 " }
			}
		}
	};
	Pattern OnFileSelected = Pattern
	{
		"engine.dll",
		std::vector<Signature>
		{
			{
				// CDemoUIPanel::OnFileSelected
				// \x55\x8B\xEC\x8B\x45\x08\x81\xEC\x00\x00\x00\x00\x56\x8B\xF1\x85\xC0\x0F\x84\x00\x00\x00\x00\x80\x38\x00\x0F\x84\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x8B\x11\x57\x68\x00\x00\x00\x00\x8D\xBD\x00\x00\x00\x00\x57\x50\x8B\x82\x00\x00\x00\x00\xFF\xD0\x6A\x0A xxxxxxxx????xxxxxxx????xxxxx????xx????xxxx????xx????xxxx????xxxx
				{ "Portal 2 Build 6879" },
				{ "55 8B EC 8B 45 08 81 EC ? ? ? ? 56 8B F1 85 C0 0F 84 ? ? ? ? 80 38 00 0F 84 ? ? ? ? 8B 0D ? ? ? ? 8B 11 57 68 ? ? ? ? 8D BD ? ? ? ? 57 50 8B 82 ? ? ? ? FF D0 6A 0A" }
			}
		}
	};
}