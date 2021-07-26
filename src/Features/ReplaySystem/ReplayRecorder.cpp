#include "ReplayRecorder.hpp"

#include "Replay.hpp"

ReplayRecorder *replayRecorder1;
ReplayRecorder *replayRecorder2;

ReplayRecorder::ReplayRecorder()
	: viewIndex(ReplayRecorder::count++)
	, isRecording(false) {
}
void ReplayRecorder::Record(Replay *replay, CUserCmd *cmd) {
	auto view = replay->GetView(this->viewIndex);
	view->frames.push_back(ReplayFrame{
		cmd->viewangles,
		cmd->forwardmove,
		cmd->sidemove,
		cmd->upmove,
		cmd->buttons,
		cmd->impulse,
		cmd->mousedx,
		cmd->mousedy});
}
void ReplayRecorder::StartRecording() {
	this->isRecording = true;
}
void ReplayRecorder::StopRecording() {
	this->isRecording = false;
}
bool ReplayRecorder::IsRecording() {
	return this->isRecording;
}
int ReplayRecorder::GetViewId() {
	return this->viewIndex;
}

int ReplayRecorder::count;
