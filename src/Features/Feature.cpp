#include "Feature.hpp"

Features::Features()
	: list() {
}
void Features::DeleteAll() {
	for (auto &feature : this->list) {
		if (feature) {
			delete feature;
		}
	}
	this->list.clear();
}
Features::~Features() {
	this->DeleteAll();
}
