#pragma once
#include "Modules/ConCommandArgs.hpp"
#include "Modules/Console.hpp"
#include "Modules/Client.hpp"

namespace TAS
{
	struct TasFrame
	{
		int FramesLeft;
		std::string Command;
	};

	std::vector<TasFrame> Frames;
	bool IsRunning = false;
	int FrameDelay = 0;

	void Delay(int delay)
	{
		FrameDelay = delay;
	}
	void AddFrame(int framesLeft, std::string command)
	{
		Frames.push_back(TasFrame
		{
			framesLeft,
			command
		});
	}
	void Reset()
	{
		IsRunning = false;
		Frames.clear();
	}
	void Start()
	{
		IsRunning = true;
	}
}