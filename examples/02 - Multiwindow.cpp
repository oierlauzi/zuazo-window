/*
 * This example shows how instantiate an output window
 * 
 * How to compile:
 * c++ 02\ -\ Multiwindow.cpp -std=c++17 -Wall -Wextra -lzuazo -lzuazo-window -lzuazo-ffmpeg -lzuazo-compositor -lglfw -ldl -lpthread -lavutil -lavformat -lavcodec -lswscale
 */

#include <zuazo/Instance.h>
#include <zuazo/Player.h>
#include <zuazo/Modules/Window.h>
#include <zuazo/Consumers/WindowRenderer.h>
#include <zuazo/Processors/Layers/VideoSurface.h>
#include <zuazo/Sources/FFmpegClip.h>

#include <mutex>
#include <iostream>

int main(int argc, const char* argv[]) {
	if(argc != 3) {
		std::cerr << "Usage: " << *argv << " <window_count> <video_file>" << std::endl;
		std::terminate();
	}

	const size_t windowCount = std::atoi(argv[1]);
	const std::string filePath = argv[2];

	//Instantiate Zuazo as usual. Note that we're loading the Window module
	Zuazo::Instance::ApplicationInfo appInfo(
		"Window Example 02",						//Application's name
		Zuazo::Version(0, 1, 0),					//Application's version
		Zuazo::Verbosity::GEQ_INFO,					//Verbosity 
		{ Zuazo::Modules::Window::get() }			//Modules
	);
	Zuazo::Instance instance(std::move(appInfo));
	std::unique_lock<Zuazo::Instance> lock(instance);

	std::vector<Zuazo::Consumers::WindowRenderer> windows;
	windows.reserve(windowCount);
	for(size_t i = 0; i < windowCount; ++i) {
		//Create a window
		windows.emplace_back(
			instance, 								//Instance
			"Output Window " + Zuazo::toString(i),	//Layout name
			Zuazo::Math::Vec2i(1280, 720)			//Window size (in screen coordinates)
		);
		auto& window = windows.back();

		//Set the negotiation callback
		window.setVideoModeNegotiationCallback(
			[] (Zuazo::VideoBase&, const std::vector<Zuazo::VideoMode>& compatibility) -> Zuazo::VideoMode {
				auto result = compatibility.front();
				result.setFrameRate(Zuazo::Utils::MustBe<Zuazo::Rate>(result.getFrameRate().highest()));
				return result;
			}
		);

		//Configure it
		window.setTitle(window.getName());
		window.setResizeable(false); //Disable resizeing, as extra care needs to be taken
		window.asyncOpen(lock);
	}

	//In order to use the same surface across all windows, they need to have the same renderpass.
	//Otherwise, a surface needs to be created for each one
	for(size_t i = 1; i < windows.size(); ++i) {
		assert(windows[i].getRenderPass() == windows[i-1].getRenderPass());
	}

	//Create a layer for rendering to the window
	Zuazo::Processors::Layers::VideoSurface videoSurface(
		instance,
		"Video Surface",
		&windows.front(),
		windows.front().getVideoMode().getResolutionValue()
	);

	videoSurface.setScalingMode(Zuazo::ScalingMode::BOXED);
	videoSurface.setScalingFilter(Zuazo::ScalingFilter::NEAREST);
	videoSurface.asyncOpen(lock);

	//Add the surface to all windows
	for(size_t i = 0; i < windows.size(); ++i) {
		windows[i].setLayers({videoSurface});
	}

	//Create a video source
	Zuazo::Sources::FFmpegClip videoClip(
		instance,
		"Video Source",
		filePath
	);

	videoClip.play();
	videoClip.setRepeat(Zuazo::ClipBase::Repeat::REPEAT);
	videoClip.asyncOpen(lock);

	//Create a player for playing the clip
	Zuazo::Player clipPlayer(instance, &videoClip);
	clipPlayer.enable();

	//Route the signal
	videoSurface << videoClip;

	//Done!
	lock.unlock();
	getchar();
	lock.lock();
}