#include "ReplayProvider.hpp"

#include "Command.hpp"
#include "Event.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Replay.hpp"
#include "ReplayPlayer.hpp"
#include "ReplayRecorder.hpp"
#include "Variable.hpp"

#include <algorithm>
#include <fstream>

Variable sar_replay_mode("sar_replay_mode", "0", 0,
                         "Mode of replay system.\n"
                         "0 = Default,\n"
                         "1 = Automatic recording after a load,\n"
                         "2 = Automatic playback after a load.\n");
Variable sar_replay_viewmode("sar_replay_viewmode", "0", 0,
                             "Fallback mode of replay system.\n"
                             "0 = Default,\n"
                             "1 = Automatically records first view and plays second view after a load,\n"
                             "2 = Automatically records second view and plays first view after a load.\n");

ReplayProvider *replayProvider;

ReplayProvider::ReplayProvider()
	: replays() {
	this->hasLoaded = true;
}
ReplayProvider::~ReplayProvider() {
	this->DeleteAll();
}
void ReplayProvider::CreateNewReplay() {
	auto clients = engine->GetMaxClients();
	clients = std::max(clients, 1);
	clients = std::min(clients, 2);

	this->replays.push_back(new Replay(clients));
}
Replay *ReplayProvider::GetCurrentReplay() {
	if (!this->AnyReplaysLoaded()) {
		this->CreateNewReplay();
	}

	return this->replays.back();
}
bool ReplayProvider::AnyReplaysLoaded() {
	return !this->replays.empty();
}
void ReplayProvider::DeleteAll() {
	for (const auto &replay : this->replays) {
		delete replay;
	}
	this->replays.clear();
}
void ReplayProvider::MergeAll() {
	if (this->replays.size() < 2) {
		return console->Print("Need at least two replays for a merge!\n");
	}

	auto clients = engine->GetMaxClients();
	clients = std::max(clients, 1);
	clients = std::min(clients, 2);

	auto viewSize = this->GetCurrentReplay()->GetViewSize();
	auto baseReplay = new Replay(clients);

	for (const auto &replay : this->replays) {
		if (viewSize != replay->GetViewSize()) {
			console->Warning("Ignored different view size between replays!\n");
		}

		for (auto viewIndex = 0; viewIndex < (int)viewSize; ++viewIndex) {
			baseReplay->GetView(viewIndex)->frames.insert(
				baseReplay->GetView(viewIndex)->frames.end(),
				replay->GetView(viewIndex)->frames.begin(),
				replay->GetView(viewIndex)->frames.end());
		}
	}

	this->replays.push_back(baseReplay);
}
void ReplayProvider::MergeViews(int firstReplay, int secondReplay, int firstView, int secondView) {
	auto replay = replayProvider->replays[firstReplay];
	auto replay2 = replayProvider->replays[secondReplay];

	auto viewSize = replay->GetViewSize();
	if (viewSize != replay2->GetViewSize()) {
		return console->Print("Replays have different view size!\n");
	}

	if (viewSize != 2) {
		return console->Print("Replay view size of 2 is required!\n");
	}

	if ((firstView != 0 && firstView != 1) || (secondView != 0 && secondView != 1)) {
		return console->Print("Invalid view indexes!\n");
	}

	// Both views should have the same size, copy last frame multiple times to fill up empty frames
	auto frameDiff = replay->GetFrameSize() - replay2->GetFrameSize();
	if (frameDiff != 0) {
		auto view = replay->GetView((frameDiff < 0) ? secondView : firstView);
		auto last = view->frames.back();
		while (frameDiff--) {
			view->frames.push_back(last);
		}
	}

	auto baseReplay = new Replay(viewSize);
	baseReplay->GetView(0)->frames.insert(
		baseReplay->GetView(0)->frames.end(),
		replay2->GetView(firstView)->frames.begin(),
		replay2->GetView(firstView)->frames.end());
	baseReplay->GetView(1)->frames.insert(
		baseReplay->GetView(1)->frames.end(),
		replay->GetView(secondView)->frames.begin(),
		replay->GetView(secondView)->frames.end());

	this->replays.push_back(baseReplay);
}
void ReplayProvider::Export(const char *fileName, int index) {
	auto filePath = std::string(engine->GetGameDirectory()) + std::string("/") + std::string(fileName);
	if (filePath.substr(filePath.length() - 4, 4) != SAR_TAS_REPLAY_EXTENSION)
		filePath += SAR_TAS_REPLAY_EXTENSION;

	std::ofstream file(filePath, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!file.good()) {
		console->Print("File not found!\n");
		return file.close();
	}

	if (!this->AnyReplaysLoaded()) {
		console->Print("Nothing has been recorded or imported!\n");
		return file.close();
	}

	if (index < 0 && index >= (int)this->replays.size()) {
		console->Print("Invalid replay index!\n");
		return file.close();
	}

	file << SAR_TAS_REPLAY_HEADER002 << std::endl;

	auto replay = this->replays[index];
	auto viewSize = (int)replay->GetViewSize();
	file.write((char *)&viewSize, sizeof(viewSize));

	auto frameIndex = 0;
	auto frameSize = (int)replay->GetView(0)->frames.size();
	while (frameIndex < frameSize) {
		for (auto viewIndex = 0; viewIndex < viewSize; ++viewIndex) {
			auto frame = replay->GetView(viewIndex)->frames[frameIndex];
			file.write((char *)&frame.buttons, sizeof(frame.buttons));
			file.write((char *)&frame.forwardmove, sizeof(frame.forwardmove));
			file.write((char *)&frame.impulse, sizeof(frame.impulse));
			file.write((char *)&frame.mousedx, sizeof(frame.mousedx));
			file.write((char *)&frame.mousedy, sizeof(frame.mousedy));
			file.write((char *)&frame.sidemove, sizeof(frame.sidemove));
			file.write((char *)&frame.upmove, sizeof(frame.upmove));
			file.write((char *)&frame.viewangles, sizeof(frame.viewangles));
		}
		++frameIndex;
	}

	console->Print("Exported TAS replay!\n");

	file.close();
}
void ReplayProvider::Import(const char *fileName) {
	auto filePath = std::string(engine->GetGameDirectory()) + std::string("/") + std::string(fileName);
	if (filePath.substr(filePath.length() - 4, 4) != SAR_TAS_REPLAY_EXTENSION)
		filePath += SAR_TAS_REPLAY_EXTENSION;

	std::ifstream file(filePath, std::ios::in | std::ios::binary);
	if (!file.good()) {
		console->Print("File not found.\n");
		return file.close();
	}

	std::string buffer;
	std::getline(file, buffer);

	if (buffer == std::string(SAR_TAS_REPLAY_HEADER001)) {
		auto replay = new Replay(1, fileName);
		while (!file.eof() && !file.bad()) {
			auto frame = ReplayFrame();
			file.read((char *)&frame.buttons, sizeof(frame.buttons));
			file.read((char *)&frame.forwardmove, sizeof(frame.forwardmove));
			file.read((char *)&frame.impulse, sizeof(frame.impulse));
			file.read((char *)&frame.mousedx, sizeof(frame.mousedx));
			file.read((char *)&frame.mousedy, sizeof(frame.mousedy));
			file.read((char *)&frame.sidemove, sizeof(frame.sidemove));
			file.read((char *)&frame.upmove, sizeof(frame.upmove));
			file.read((char *)&frame.viewangles, sizeof(frame.viewangles));
			replay->GetView(0)->frames.push_back(frame);
		}
		this->replays.push_back(replay);
	} else if (buffer == std::string(SAR_TAS_REPLAY_HEADER002)) {
		auto viewSize = 0;
		file.read((char *)&viewSize, sizeof(viewSize));

		if (viewSize != 1 && viewSize != 2) {
			console->Print("Invalid view size: %i\n", viewSize);
			return file.close();
		}

		auto replay = new Replay(viewSize, fileName);
		while (!file.eof() && !file.bad()) {
			for (auto viewIndex = 0; viewIndex < viewSize; ++viewIndex) {
				auto frame = ReplayFrame();
				file.read((char *)&frame.buttons, sizeof(frame.buttons));
				file.read((char *)&frame.forwardmove, sizeof(frame.forwardmove));
				file.read((char *)&frame.impulse, sizeof(frame.impulse));
				file.read((char *)&frame.mousedx, sizeof(frame.mousedx));
				file.read((char *)&frame.mousedy, sizeof(frame.mousedy));
				file.read((char *)&frame.sidemove, sizeof(frame.sidemove));
				file.read((char *)&frame.upmove, sizeof(frame.upmove));
				file.read((char *)&frame.viewangles, sizeof(frame.viewangles));
				replay->GetView(viewIndex)->frames.push_back(frame);
			}
		}
		this->replays.push_back(replay);
	} else {
		console->Print("Invalid file format!\n");
		return file.close();
	}

	console->Print("Imported TAS replay!\n");
	file.close();
}

// Commands

CON_COMMAND(sar_replay_record, "sar_replay_record - starts recording a replay\n") {
	auto replay = replayProvider->GetCurrentReplay();
	auto coop = engine->GetMaxClients() > 1 && replay->GetViewSize() > 1;

	auto recordingActive = (coop)
		? replayRecorder1->IsRecording() && replayRecorder2->IsRecording()
		: replayRecorder1->IsRecording();

	if (recordingActive) {
		return console->Print("Recording active!\n");
	}

	auto playbackActive = replayPlayer1->IsPlaying() || replayPlayer2->IsPlaying();
	if (playbackActive && replayProvider->AnyReplaysLoaded()) {
		replay->GetView(replayRecorder1->GetViewId())->Resize();
		if (coop) {
			replay->GetView(replayRecorder2->GetViewId())->Resize();
		}
	}

	replayRecorder1->StartRecording();
	if (coop) {
		replayRecorder2->StartRecording();
	}

	console->Print((playbackActive) ? "Recording!\n" : "Re-Recording!\n");
}
CON_COMMAND(sar_replay_record_view, "sar_replay_record_view <view_index> - starts recording a specific view for a replay\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_replay_record_view.ThisPtr()->m_pszHelpString);
	}

	auto view = std::atoi(args[1]);
	if (view != 0 && view != 1) {
		return console->Print("Invalid view index!\n");
	}

	auto replay = replayProvider->GetCurrentReplay();
	auto coop = engine->GetMaxClients() > 1 && replay->GetViewSize() > 1;
	if (!coop) {
		return console->Print("Cannot record in single player or a single player replay!\n");
	}

	if ((view == 0 && replayRecorder1->IsRecording()) || (view == 1 && replayRecorder2->IsRecording())) {
		return console->Print("Recording active!\n");
	}

	auto playbackActive = false;
	if (view == 0) {
		replayRecorder1->StartRecording();
		playbackActive = replayPlayer1->IsPlaying();
	} else {
		replayRecorder2->StartRecording();
		playbackActive = replayPlayer2->IsPlaying();
	}

	console->Print((playbackActive) ? "Recording!\n" : "Re-Recording!\n");
}
CON_COMMAND(sar_replay_play, "sar_replay_play - plays back a replay\n") {
	if (!replayProvider->AnyReplaysLoaded()) {
		return console->Print("Nothing has been recorded or imported!\n");
	}

	auto replay = replayProvider->GetCurrentReplay();
	auto coop = engine->GetMaxClients() > 1 && replay->GetViewSize() > 1;

	auto recordingActive = (coop)
		? replayRecorder1->IsRecording() && replayRecorder2->IsRecording()
		: replayRecorder1->IsRecording();

	if (recordingActive) {
		return console->Print("Recording active!\n");
	}

	replayPlayer1->StartPlaying(replay);
	if (coop) {
		replayPlayer2->StartPlaying(replay);
	}

	console->Print("Playing!\n");
}
CON_COMMAND(sar_replay_play_view, "sar_replay_play_view <view_index> - plays back a specific view of a replay\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_replay_play_view.ThisPtr()->m_pszHelpString);
	}

	auto view = std::atoi(args[1]);
	if (view != 0 && view != 1) {
		return console->Print("Invalid view index!\n");
	}

	if (!replayProvider->AnyReplaysLoaded()) {
		return console->Print("Nothing has been recorded or imported!\n");
	}

	auto replay = replayProvider->GetCurrentReplay();
	auto coop = engine->GetMaxClients() > 1 && replay->GetViewSize() > 1;
	if (!coop) {
		return console->Print("Cannot play in single player or a single player replay!\n");
	}

	if ((view == 0 && replayRecorder1->IsRecording()) || (view == 1 && replayRecorder2->IsRecording())) {
		return console->Print("Recording active!\n");
	}

	if (view == 0) {
		replayPlayer1->StartPlaying(replay);
	} else {
		replayPlayer2->StartPlaying(replay);
	}

	console->Print("Playing view %i!\n", view);
}
CON_COMMAND(sar_replay_stop, "sar_replay_stop - stops recording or playing user inputs\n") {
	replayRecorder1->StopRecording();
	replayRecorder2->StopRecording();
	replayPlayer1->StopPlaying();
	replayPlayer2->StopPlaying();
	console->Print("Stopped playing and recording!\n");
}
CON_COMMAND(sar_replay_merge_all, "sar_replay_merge_all - merges all replays into one\n") {
	replayProvider->MergeAll();
}
CON_COMMAND(sar_replay_merge_views, "sar_replay_merge_views <replay_index1> <replay_index2> <view_index1> <view_index2> - merges one view to another of two replays\n") {
	if (args.ArgC() != 5) {
		return console->Print(sar_replay_merge_views.ThisPtr()->m_pszHelpString);
	}

	auto loadedReplays = (int)replayProvider->replays.size();
	if (loadedReplays < 2) {
		return console->Print("Need at least two replays for a merge!\n");
	}

	auto replayIndex1 = std::atoi(args[1]);
	auto replayIndex2 = std::atoi(args[2]);

	if (replayIndex1 >= loadedReplays || replayIndex2 >= loadedReplays || replayIndex1 < 0 || replayIndex2 < 0 || replayIndex1 == replayIndex2) {
		return console->Print("Invalid replay indexes!\n");
	}

	auto viewIndex1 = std::atoi(args[3]);
	auto viewIndex2 = std::atoi(args[4]);

	if (viewIndex1 == viewIndex2) {
		return console->Print("View indexes cannot be the same! Use sar_replay_clone_views for that.\n");
	}

	replayProvider->MergeViews(replayIndex1, replayIndex2, viewIndex1, viewIndex2);
}
CON_COMMAND(sar_replay_clone_views, "sar_replay_clone_views <replay_index> <view_index> - clones view to another of a replay\n") {
	if (args.ArgC() != 3) {
		return console->Print(sar_replay_clone_views.ThisPtr()->m_pszHelpString);
	}

	auto loadedReplays = (int)replayProvider->replays.size();
	if (loadedReplays < 1) {
		return console->Print("Need at least one replay for cloning!\n");
	}

	auto replaxIndex = std::atoi(args[1]);
	auto viewIndex = std::atoi(args[2]);

	if (replaxIndex >= loadedReplays || replaxIndex < 0) {
		return console->Print("Invalid replay index!\n");
	}

	replayProvider->MergeViews(replaxIndex, replaxIndex, viewIndex, viewIndex);
}
CON_COMMAND(sar_replay_export, "sar_replay_export <file> - exports replay to a file\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_replay_export.ThisPtr()->m_pszHelpString);
	}

	replayProvider->Export(args[1]);
}
CON_COMMAND(sar_replay_export_at, "sar_replay_export_at <index> <file> - exports specific replay to a file\n") {
	if (args.ArgC() != 3) {
		return console->Print(sar_replay_export_at.ThisPtr()->m_pszHelpString);
	}

	replayProvider->Export(args[2], std::atoi(args[1]));
}
CON_COMMAND_AUTOCOMPLETEFILE(sar_replay_import, "sar_replay_import <file> - imports replay file\n", 0, 0, str) {
	if (args.ArgC() != 2) {
		return console->Print(sar_replay_import.ThisPtr()->m_pszHelpString);
	}

	replayProvider->DeleteAll();
	replayProvider->Import(args[1]);
}
CON_COMMAND_AUTOCOMPLETEFILE(sar_replay_import_add, "sar_replay_import_add <file> - imports replay file but doesn't delete already added replays\n", 0, 0, str) {
	if (args.ArgC() != 2) {
		return console->Print(sar_replay_import_add.ThisPtr()->m_pszHelpString);
	}

	replayProvider->Import(args[1]);
}
CON_COMMAND(sar_replay_list, "sar_replay_list - lists all currently imported replays\n") {
	if (!replayProvider->AnyReplaysLoaded()) {
		return console->Print("No replays have been recorded or imported!\n");
	}

	auto index = 0;
	for (const auto &replay : replayProvider->replays) {
		console->Print("[%i] %s\n", index++, replay->GetSource());
		console->Msg("  -> ");
		console->Print("views: %i | frames: %i\n", replay->GetViewSize(), replay->GetFrameSize());
	}
}

ON_EVENT(SESSION_START) {
	if (sar_replay_mode.GetBool()) {
		if (sar_replay_mode.GetInt() == 1) {
			replayProvider->CreateNewReplay();
			replayRecorder1->StartRecording();

			if (replayProvider->GetCurrentReplay()->GetViewSize() > 1) {
				replayRecorder2->StartRecording();
			}
		} else if (replayProvider->AnyReplaysLoaded()) {
			auto replay = replayProvider->GetCurrentReplay();
			replayPlayer1->StartPlaying(replay);

			if (engine->GetMaxClients() > 1 && replay->GetViewSize() > 1) {
				replayPlayer2->StartPlaying(replay);
			}
		}
	} else if (sar_replay_viewmode.isRegistered && sar_replay_viewmode.GetBool() && replayProvider->AnyReplaysLoaded()) {
		auto replay = replayProvider->GetCurrentReplay();
		if (engine->GetMaxClients() > 1 && replay->GetViewSize() > 1) {
			if (sar_replay_viewmode.GetInt() == 1) {
				replayRecorder1->StartRecording();
				replayPlayer2->StartPlaying(replay);
			} else {
				replayRecorder2->StartRecording();
				replayPlayer1->StartPlaying(replay);
			}
		}
	}
}
