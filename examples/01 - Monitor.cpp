/*
 * This example shows how instantiate an output window
 * 
 * How to compile:
 * c++ 01\ -\ Monitor.cpp -std=c++17 -Wall -Wextra -lzuazo -lzuazo-window -lzuazo-ffmpeg -lzuazo-compositor -lglfw -ldl -lpthread -lavutil -lavformat -lavcodec -lswscale
 */

#include <zuazo/Instance.h>
#include <zuazo/Player.h>
#include <zuazo/Modules/Window.h>
#include <zuazo/Consumers/WindowRenderer.h>
#include <zuazo/Processors/Layers/VideoSurface.h>
#include <zuazo/Sources/FFmpegClip.h>

#include <mutex>
#include <iostream>
#include <thread>

int main(int argc, const char* argv[]) {
	if(argc != 2) {
		std::cerr << "Usage: " << *argv << " <video_file>" << std::endl;
		std::terminate();
	}

	//Instantiate Zuazo as usual. Note that we're loading the Window module
	Zuazo::Instance::ApplicationInfo appInfo(
		"Window Example 01",						//Application's name
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
	//window.setResizeable(false); //Disable resizeing, as extra care needs to be taken
	window.asyncOpen(lock);

	//Create a layer for rendering to the window
	Zuazo::Processors::Layers::VideoSurface videoSurface(
		instance,
		"Video Surface",
		&window,
		window.getVideoMode().getResolutionValue()
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



	//List all the monitors
	const auto monitors = Zuazo::Consumers::WindowRenderer::getMonitors();
	std::cout << "Available monitors:\n";
	for(const auto& monitor : monitors) {
		std::cout << "\t-" << monitor.getName() << "\n";
	}

	//Ask the user to select a monitor
	size_t monitorSelection;
	std::cout << "Select monitor: ";
	lock.unlock();
	std::cin >> monitorSelection;
	lock.lock();

	//Check if the selection is valid
	if(monitorSelection < monitors.size()) {
		//Display available modes
		const auto modes = monitors[monitorSelection].getModes();

		std::cout << "Available modes:\n";
		for(const auto& mode : modes) {
			std::cout << "-\t" << mode.size.x << "x" << mode.size.y << "@" << mode.frameRate << "Hz" << "\n";
		}

		//Ask the user to select a mode for the monitor
		size_t modeSelection;
		std::cout << "Select mode: ";
		lock.unlock();
		std::cin >> modeSelection;
		lock.lock();

		//Check if the selection is valid
		if(modeSelection < modes.size()) {
			window.setMonitor(monitors[monitorSelection], &modes[modeSelection]);
			videoSurface.setSize(window.getVideoMode().getResolutionValue());
		} else {
			std::cout << "Invalid mode" << std::endl;
		}
	} else {
		std::cout << "Invalid monitor" << std::endl;
	}

	//Wait for 10
	lock.unlock();
	std::this_thread::sleep_for(std::chrono::seconds(10));
	lock.lock();

	//Unset the fullscreen
	window.setMonitor(Zuazo::Consumers::WindowRenderer::NO_MONITOR, nullptr);
	videoSurface.setSize(window.getVideoMode().getResolutionValue());

	//Done!
	lock.unlock();
	getchar();
	lock.lock();
}