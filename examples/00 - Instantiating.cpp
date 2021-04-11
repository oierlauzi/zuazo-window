/*
 * This example shows how instantiate an output window
 * 
 * How to compile:
 * c++ 00\ -\ Instantiating.cpp -std=c++17 -Wall -Wextra -lzuazo -lzuazo-window -lzuazo-ffmpeg -lzuazo-compositor -lglfw -ldl -lpthread -lavutil -lavformat -lavcodec -lswscale
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
	if(argc != 2) {
		std::cerr << "Usage: " << *argv << " <video_file>" << std::endl;
		std::terminate();
	}

	//Instantiate Zuazo as usual. Note that we're loading the Window module
	Zuazo::Instance::ApplicationInfo appInfo(
		"Window Example 00",						//Application's name
		Zuazo::Version(0, 1, 0),					//Application's version
		Zuazo::Verbosity::GEQ_INFO,					//Verbosity 
		{ Zuazo::Modules::Window::get() }			//Modules
	);
	Zuazo::Instance instance(std::move(appInfo));
	std::unique_lock<Zuazo::Instance> lock(instance);

	//Construct the window object
	Zuazo::Consumers::WindowRenderer window(
		instance, 						//Instance
		"Output Window",				//Layout name
		Zuazo::Math::Vec2i(1280, 720)	//Window size (in screen coordinates)
	);

	//Set the negotiation callback
	window.setVideoModeNegotiationCallback(
		[] (Zuazo::VideoBase&, const std::vector<Zuazo::VideoMode>& compatibility) -> Zuazo::VideoMode {
			auto result = compatibility.front();
			result.setFrameRate(Zuazo::Utils::MustBe<Zuazo::Rate>(result.getFrameRate().highest()));
			return result;
		}
	);

	//Open the window (now becomes visible)
	window.asyncOpen(lock);

	//Create a layer for rendering to the window
	Zuazo::Processors::Layers::VideoSurface videoSurface(
		instance,
		"Video Surface",
		&window,
		window.getViewportSize()
	);

	window.setViewportSizeCallback(
		std::bind(
			&Zuazo::Processors::Layers::VideoSurface::setSize, 
			&videoSurface, 
			std::placeholders::_2
		)
	);

	window.setLayers({videoSurface});
	videoSurface.setScalingMode(Zuazo::ScalingMode::BOXED);
	videoSurface.setScalingFilter(Zuazo::ScalingFilter::CUBIC);
	videoSurface.asyncOpen(lock);

	//Create a video source
	Zuazo::Sources::FFmpegClip videoClip(
		instance,
		"Video Source",
		std::string(argv[1])
	);

	videoClip.play();
	videoClip.setRepeat(Zuazo::ClipBase::Repeat::REPEAT);
	videoClip.asyncOpen(lock);

	//Create a player for playing the clip
	Zuazo::Player clipPlayer(instance, &videoClip);
	clipPlayer.enable();

	//Route the signal
	videoSurface << videoClip;

	//Show some data about the preferences to the user
	std::cout << "\nSupported video-modes:\n";
	for(const auto& videoMode : window.getVideoModeCompatibility()) {
		std::cout << "\t-" << videoMode << "\n";
	}

	std::cout << "\nSelected video-mode:\n";
	std::cout << "\t-" << window.getVideoMode() << "\n";

	//Done!
	lock.unlock();
	getchar();
	lock.lock();
}