#pragma once
#include "Utils.hpp"

#define SAR_PLUGIN_SIGNATURE \
	new char[26] { 65, 114, 101, 32, 121, 111, 117, 32, 104, 97, 112, 112, 121, 32, 110, 111, 119, 44, 32, 74, 97, 109, 101, 114, 63, 00 }

// CServerPlugin
#define CServerPlugin_m_Size 16
#define CServerPlugin_m_Plugins 4

class Plugin {
public:
	CPlugin *ptr;
	int index;

public:
	Plugin();
};
