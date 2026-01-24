#include <Event.hpp>
#include <Variable.hpp>
#include <Modules/Client.hpp>
#include <Modules/Engine.hpp>
#include <Features/Renderer.hpp>

Variable sar_demo_clean_start("sar_demo_clean_start", "0", 0,  
	"Attempts to minimize visual interpolation of some elements (like post-processing or lighting) when demo playback begins.\n", 0);
Variable sar_demo_clean_start_tonemap("sar_demo_clean_start_tonemap", "0", 0, 
	"Overrides initial tonemap scalar value used in auto-exposure.\n"
	"Setting it to 0 will attempt to skip over to target value for several ticks.\n", 0);


bool g_tonemapScaleDemoLookupInProgress = false;
int g_tonemapScaleDemoLookupTick;
bool g_tonemapScaleContinuousSearchEnabled = false;
int g_tonemapScaleTweakStartTick = 0;

const float TONEMAP_SCALE_SEARCH_DISABLE_THRESHOLD = 0.01f;
const int TONEMAP_SCALE_SEARCH_MAX_NOCHANGE_TICKS = 30;

void startTonemapScaleDemoLookup(int tick) {
	g_tonemapScaleDemoLookupInProgress = true;
	g_tonemapScaleDemoLookupTick = tick;
}

void stopTonemapScaleDemoLookup(bool finished = false) {
	if (!g_tonemapScaleDemoLookupInProgress) {
		return;
	}

	if (!finished) {
		console->Print("Failed to sample tonemap scale before the end of the demo.\n");
	}
	g_tonemapScaleDemoLookupInProgress = false;
	g_tonemapScaleDemoLookupTick = -1;
}

void updateTonemapScaleDemoLookup() {
	if (!g_tonemapScaleDemoLookupInProgress || !client->GetCurrentTonemappingSystem) {
		return;
	}

	if (!engine->demoplayer->IsPlaying() || engine->demoplayer->GetTick() < g_tonemapScaleDemoLookupTick) {
		return;
	}

	float tonemapScale = client->GetCurrentTonemappingSystem()->m_flCurrentTonemapScale;
	sar_demo_clean_start_tonemap.SetValue(tonemapScale);
	console->Print("Sampled tonemap scale value %.6f and stored it in \"sar_demo_clean_start_tonemap\" variable.\n", tonemapScale);
	
	stopTonemapScaleDemoLookup(true);
}

void triggerTonemapScaleTweak() {
	if (!client->GetCurrentTonemappingSystem || !client->ResetToneMapping) {
		return;
	}

	float tonemapScaleOverride = sar_demo_clean_start_tonemap.GetFloat();

	if (tonemapScaleOverride == 0.0f) {
		g_tonemapScaleContinuousSearchEnabled = true;
		auto tonemapSystem = client->GetCurrentTonemappingSystem();
		g_tonemapScaleTweakStartTick = tonemapSystem->m_nCurrentQueryFrame;
	}

	client->ResetToneMapping(tonemapScaleOverride);
}

void disableTonemapScaleContinuousSearch() {
	g_tonemapScaleContinuousSearchEnabled = false;
}

void updateTonemapScaleContinuousSearch() {
	if (!g_tonemapScaleContinuousSearchEnabled || !client->GetCurrentTonemappingSystem) {
		return;
	}

	auto tonemapSystem = client->GetCurrentTonemappingSystem();
	if (tonemapSystem == nullptr) {
		disableTonemapScaleContinuousSearch();
		return;
	}

	float diff = tonemapSystem->m_flTargetTonemapScale - tonemapSystem->m_flCurrentTonemapScale;

	if (fabsf(diff) > TONEMAP_SCALE_SEARCH_DISABLE_THRESHOLD) {
		// last sampled luminance was different enough to change target scale, reset it
		tonemapSystem->m_flCurrentTonemapScale = tonemapSystem->m_flTargetTonemapScale;
		return;
	}
	
	int searchDuration = tonemapSystem->m_nCurrentQueryFrame - g_tonemapScaleTweakStartTick;
	if (searchDuration > TONEMAP_SCALE_SEARCH_MAX_NOCHANGE_TICKS) {
		disableTonemapScaleContinuousSearch();
	}
}

void forceProjectedTexturesTargetColor() {
	for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
		auto clientEntity = client->GetEntity(i);
		if (!clientEntity) {
			continue;
		}

		if (strcmp(clientEntity->GetClientClass()->m_pNetworkName, "CEnvProjectedTexture") != 0) {
			continue;
		}

		auto targetColor = clientEntity->field<color32>("m_LightColor");
		auto &m_CurrentLinearFloatLightColor = clientEntity->fieldOff<Vector>("m_LightColor", 0x04);
		auto &m_flCurrentLinearFloatLightAlpha = clientEntity->fieldOff<float>("m_LightColor", 0x10);

		m_CurrentLinearFloatLightColor = Vector(targetColor.r, targetColor.g, targetColor.b);
		m_flCurrentLinearFloatLightAlpha = targetColor.a;
	}
}

CON_COMMAND(sar_demo_clean_start_tonemap_sample,
            "sar_demo_clean_start_tonemap_sample [tick] - samples tonemap scale from current demo "
            "at given tick and stores it in \"sar_demo_clean_start_tonemap\" variable. "
            "If no tick is given, sampling will happen when `__END__` is seen in demo playback.\n") {
	if (args.ArgC() > 2) {
		return console->Print(sar_demo_clean_start_tonemap_sample.ThisPtr()->m_pszHelpString);
	}

	if (!engine->demoplayer->IsPlaying()) {
		return console->Print("No demo is currently being played back.\n");
	}

	g_tonemapScaleDemoLookupInProgress = true;

	if (args.ArgC() == 2) {
		startTonemapScaleDemoLookup(std::atoi(args[1]));
		return console->Print("Tonemap sampling set up to trigger at tick %d\n", g_tonemapScaleDemoLookupTick);
	} else {
		startTonemapScaleDemoLookup(Renderer::segmentEndTick);
		return console->Print("Tonemap sampling set up to trigger at __END__ tick (%d)\n", g_tonemapScaleDemoLookupTick);
	}
}


ON_EVENT(FRAME) {
	updateTonemapScaleContinuousSearch();
	updateTonemapScaleDemoLookup();
}

ON_EVENT(DEMO_START) {
	if (!sar_demo_clean_start.GetBool()) {
		return;
	}

	triggerTonemapScaleTweak();
	forceProjectedTexturesTargetColor();
}

ON_EVENT(DEMO_STOP) {
	disableTonemapScaleContinuousSearch();
	stopTonemapScaleDemoLookup();
}
