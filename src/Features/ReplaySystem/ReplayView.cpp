#include "ReplayView.hpp"

ReplayView::ReplayView()
	: frames()
	, playIndex(0) {
}
void ReplayView::Reset() {
	this->playIndex = 0;
}
bool ReplayView::Ended() {
	return this->playIndex + 1 >= (int)this->frames.size();
}
void ReplayView::Resize() {
	this->frames.resize(this->playIndex);
}
