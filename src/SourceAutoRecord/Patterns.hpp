#pragma once
#include "Utils.hpp"

namespace Patterns
{
	// server.dll
	namespace
	{
		Pattern CheckJumpButton = Pattern
		{
			"CGameMovement::CheckJumpButton",
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
		Pattern PlayerUse = Pattern
		{
			"C_BasePlayer::PlayerUse",
			"server.dll",
			std::vector<Signature>
			{
				{
					// CPortal_Player::PlayerUse
					// \x55\x8B\xEC\x8B\x15\x00\x00\x00\x00\x83\xEC\x0C\x56\x8B\xF1 xxxxx????xxxxxx
					{ "Portal 2 Build 6879" },
					{ "55 8B EC 8B 15 ? ? ? ? 83 EC 0C 56 8B F1" }
				}
			}
		};
	}

	// client.dll
	namespace
	{
		Pattern Paint = Pattern
		{
			"CFPSPanel::Paint",
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
		Pattern SetSize = Pattern
		{
			"VPanel::SetSize",
			"client.dll",
			std::vector<Signature>
			{
				{
					// VPanel::SetSize
					// \x55\x8B\xEC\x8B\x41\x04\x8B\x50\x04\x8B\x45\x0C\x56\x8B\x35\x00\x00\x00\x00\x57\x8B\x3E\x8D\x4C\x0A\x04\x8B\x55\x08\x50\x8B\x01\x52\x8B\x10\xFF\xD2\x50\x8B\x47\x10 xxxxxxxxxxxxxxx????xxxxxxxxxxxxxxxxxxxxxx
					{ "Portal 2 Build 6879" },
					{ "55 8B EC 8B 41 04 8B 50 04 8B 45 0C 56 8B 35 ? ? ? ? 57 8B 3E 8D 4C 0A 04 8B 55 08 50 8B 01 52 8B 10 FF D2 50 8B 47 10" }
				}
			}
		};
		Pattern ShouldDraw = Pattern
		{
			"CFPSPanel::ShouldDraw",
			"client.dll",
			std::vector<Signature>
			{
				{
					// CFPSPanel::ShouldDraw
					// \x80\x3D\x00\x00\x00\x00\x00\x75\x7C xx?????xx
					{ "Portal 2 Build 6879" },
					{ "80 3D ? ? ? ? ? 75 7C" }
				}
			}
		};
		Pattern MatSystemSurfacePtr = Pattern
		{
			"IMatSystemSurface *g_pMatSystemSurface",
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
	}

	// engine.dll
	namespace
	{
		Pattern ConVar_Ctor3 = Pattern
		{
			"ConVar_Ctor3",
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
			"CvarPtr",
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
		Pattern InputSystemPtr = Pattern
		{
			"IInputSystem *g_pInputSystem",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// \x55\x8B\xEC\x56\x8B\x75\x08\x83\x3E\x02\x74\x11\x68\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x83\xC4\x04\x5E\x5D\xC3\x8B\x86\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00 xxxxxxxxxxxxx????xx????xxxxxxxx????xx????
					{ "Portal 2 Build 6879" },
					{ "55 8B EC 56 8B 75 08 83 3E 02 74 11 68 ? ? ? ? FF 15 ? ? ? ? 83 C4 04 5E 5D C3 8B 86 ? ? ? ? 8B 0D ? ? ? ? " },
					{ 37 }
				}
			}
		};
		Pattern Key_SetBinding = Pattern
		{
			"Key_SetBinding",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// \x55\x8B\xEC\x56\x8B\x75\x08\x83\xFE\xFF xxxxxxxxxx
					{ "Portal 2 Build 6879" },
					{ "55 8B EC 56 8B 75 08 83 FE FF" }
				}
			}
		};
		Pattern ConCommand_Ctor1 = Pattern
		{
			"ConCommand_Ctor1",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// \x55\x8B\xEC\x8B\x45\x0C\x53\x33\xDB\x56\x8B\xF1\x8B\x4D\x18\x80\x66\x20\xF9 xxxxxxxxxxxxxxxxxxx
					{ "Portal 2 Build 6879" },
					{ "55 8B EC 8B 45 0C 53 33 DB 56 8B F1 8B 4D 18 80 66 20 F9" }
				}
			}
		};
		Pattern ConCommand_Ctor2 = Pattern
		{
			"ConCommand_Ctor2",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// \x55\x8B\xEC\x8B\x45\x0C\x53\x33\xDB\x56\x8B\xF1\x8B\x4D\x18\x80\x4E\x20\x02 xxxxxxxxxxxxxxxxxxx
					{ "Portal 2 Build 6879" },
					{ "55 8B EC 8B 45 0C 53 33 DB 56 8B F1 8B 4D 18 80 4E 20 02" }
				}
			}
		};
		Pattern SetSignonState = Pattern
		{
			"CGameClient::SetSignonState",
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
			"CDemoUIPanel::OnFileSelected",
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
		Pattern GetGameDir = Pattern
		{
			"CVEngineServer::GetGameDir",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// CVEngineServer::GetGameDir
					// \x55\x8B\xEC\x8B\x45\x08\x85\xC0\x74\x12\x8B\x4D\x0C xxxxxxxxxxxxx
					{ "Portal 2 Build 6879" },
					{ "55 8B EC 8B 45 08 85 C0 74 12 8B 4D 0C" }
				}
			}
		};
		Pattern EngineClientPtr = Pattern
		{
			"IVEngineClient *engineClient",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// IVEngineClient *engineClient
					{ "Portal 2 Build 6879" },
					{ "55 8B EC 83 EC 10 B8 ? ? ? ? 99" },
					{ 63 }
				}
			}
		};
		Pattern LoadgamePtr = Pattern
		{
			"CGameServer::m_bLoadgame",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// CGameClient::SpawnPlayer
					// \x55\x8B\xEC\x83\xEC\x14\x80\x3D\x00\x00\x00\x00\x00\x56 xxxxxxxx?????x
					{ "Portal 2 Build 6879" },
					{ "55 8B EC 83 EC 14 80 3D ? ? ? ? ? 56" },
					{ 8 }
				}
			}
		};
		Pattern CurtimePtr = Pattern
		{
			"CGlobalVarsBase::curtime",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// CGlobalVarsBase::curtime
					// \x89\x96\xC4\x00\x00\x00\x8B\x86\xC8\x00\x00\x00\x8B\xCE\xA3\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xD9\x1D\x00\x00\x00\x00\x8B\xCE\xE8\x00\x00\x00\x00\xD9\x1D xxxxxxxxxxxxxxx????x????xx????xxx????xx
					{ "Portal 2 Build 6879" },
					{ "89 96 C4 00 00 00 8B 86 C8 00 00 00 8B CE A3 ? ? ? ? E8 ? ? ? ? D9 1D ? ? ? ? 8B CE E8 ? ? ? ? D9 1D" },
					{ 26 }
				}
			}
		};
		Pattern MapnamePtr = Pattern
		{
			"CBaseServer::m_szMapname",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// CBaseServer::m_szMapname[64]
					// \xD9\x00\x2C\xD9\xC9\xDF\xF1\xDD\xD8\x76\x00\x80\x00\x00\x00\x00\x00\x00 x?xxxxxxxx?x?????x
					{ "Portal 2 Build 6879" },
					{ "D9 ? 2C D9 C9 DF F1 DD D8 76 ? 80 ? ? ? ? ? 00" },
					{ 13 }
				},
				{
					// CBaseServer::m_szMapname[64]
					// \x76\x50\x80\x3D\x00\x00\x00\x00\x00\xB8\x00\x00\x00\x00 xxxx?????x????
					{ "INFRA Build 6905" },
					{ "76 50 80 3D ? ? ? ? ? B8 ? ? ? ? " },
					{ 10 }
				}
			}
		};
		//Pattern CloseDemoFile = Pattern
		//{
		//	"CDemoRecorder::CloseDemoFile",
		//	"engine.dll",
		//	std::vector<Signature>
		//	{
		//		{
		//			// CDemoRecorder::CloseDemoFile
		//			// \x56\x8B\xF1\x57\x8D\x4E\x04\xE8\x00\x00\x00\x00\x84\xC0\x0F\x84\x00\x00\x00\x00\x80\xBE\x00\x00\x00\x00\x00 xxxxxxxx????xxxx????xx?????
		//			{ "Portal 2 Build 6879" },
		//			{ "56 8B F1 57 8D 4E 04 E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 80 BE ? ? ? ? ? " }
		//		}
		//	}
		//};
		Pattern DemoRecorderPtr = Pattern
		{
			"IDemoRecorder *demorecorder",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x83\x3D\x00\x00\x00\x00\x00\x53 xxxxx????xx?????x
					{ "Portal 2 Build 6879" },
					{ "55 8B EC 81 EC ? ? ? ? 83 3D ? ? ? ? ? 53" },
					{ 43 }
				}
			}
		};
		Pattern DemoPlayerPtr = Pattern
		{
			"IDemoPlayer *demoplayer",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x83\x3D\x00\x00\x00\x00\x00\x53 xxxxx????xx?????x
					{ "Portal 2 Build 6879" },
					{ "55 8B EC 81 EC ? ? ? ? 83 3D ? ? ? ? ? 53" },
					{ 79 }
				}
			}
		};
		Pattern PlayDemo = Pattern
		{
			"CL_PlayDemo_f",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x83\x3D\x00\x00\x00\x00\x00\x0F\x85\x00\x00\x00\x00\x8B\x45\x08\x83\x38\x02 xxxxx????xx?????xx????xxxxxx
					{ "Portal 2 Build 6879" },
					{ "55 8B EC 81 EC ? ? ? ? 83 3D ? ? ? ? ? 0F 85 ? ? ? ? 8B 45 08 83 38 02" }
				}
			}
		};
		Pattern StartPlayback = Pattern
		{
			"CDemoPlayer::StartPlayback",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// CDemoPlayer::StartPlayback
					// \x55\x8B\xEC\x53\x56\x57\x6A\x00\x8B\xF1 xxxxxxxxxx
					{ "Portal 2 Build 6879" },
					{ "55 8B EC 53 56 57 6A 00 8B F1" }
				}
			}
		};
		//Pattern StopPlayback = Pattern
		//{
		//	"CDemoPlayer::StopPlayback",
		//	"engine.dll",
		//	std::vector<Signature>
		//	{
		//		{
		//			// CDemoPlayer::StopPlayback
		//			// \x55\x8B\xEC\x51\x56\x8B\xF1\x8B\x06\x8B\x50\x18 xxxxxxxxxxxx
		//			{ "Portal 2 Build 6879" },
		//			{ "55 8B EC 51 56 8B F1 8B 06 8B 50 18" }
		//		}
		//	}
		//};
		Pattern StopRecording = Pattern
		{
			"CDemoRecorder::StopRecording",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// CDemoRecorder::StopRecording
					// \x55\x8B\xEC\x51\x56\x8B\xF1\x8B\x06\x8B\x50\x10 xxxxxxxxxxxx
					{ "Portal 2 Build 6879" },
					{ "55 8B EC 51 56 8B F1 8B 06 8B 50 10" }
				}
			}
		};
		Pattern StartupDemoFile = Pattern
		{
			"CDemoRecorder::StartupDemoFile",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// CDemoRecorder::StartupDemoFile
					// \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x53\x8B\xD9\x80\xBB\x00\x00\x00\x00\x00\x56 xxxxx????xxxxx?????x
					{ "Portal 2 Build 6879" },
					{ "55 8B EC 81 EC ? ? ? ? 53 8B D9 80 BB ? ? ? ? ? 56" }
				}
			}
		};
		Pattern ReadPacket = Pattern
		{
			"CDemoPlayer::ReadPacket",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// CDemoPlayer::ReadPacket
					// \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x57\x8B\xF9\x8D\x4F\x04\x89\x7D\xB0 xxxxx????xxxxxxxxx
					{ "Portal 2 Build 6879" },
					{ "55 8B EC 81 EC ? ? ? ? 57 8B F9 8D 4F 04 89 7D B0" },
					{ 414 }
				}
			}
		};
		Pattern Stop = Pattern
		{
			"CON_COMMAND stop",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// \x83\x3D\x00\x00\x00\x00\x00\x75\x1F xx?????xx
					{ "Portal 2 Build 6879" },
					{ "83 3D ? ? ? ? ? 75 1F" }
				}
			}
		};
		Pattern ConVar_PrintDescription = Pattern
		{
			"ConVar_PrintDescription",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// \x68\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x83\xC4\x0C\x8B\xE5\x5D\xC3\x8D\x8D\x00\x00\x00\x00 x????xx????xxxxxxxxx????
					{ "Portal 2 Build 6879" },
					{ "68 ? ? ? ? FF 15 ? ? ? ? 83 C4 0C 8B E5 5D C3 8D 8D ? ? ? ? " },
					{ 1 }
				}
			}
		};
		Pattern Disconnect = Pattern
		{
			"CBaseClientState::Disconnect",
			"engine.dll",
			std::vector<Signature>
			{
				{
					// \x55\x8B\xEC\xDD\x05\x00\x00\x00\x00\x83\xEC\x18 xxxxx????xxx
					{ "Portal 2 Build 6879" },
					{ "55 8B EC DD 05 ? ? ? ? 83 EC 18" }
				}
			}
		};
	}
}