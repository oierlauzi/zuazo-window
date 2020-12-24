/*
 * This example shows how instantiate an output window
 * 
 * How to compile:
 * c++ 00\ -\ Instantiating.cpp -std=c++17 -Wall -Wextra -lzuazo -lzuazo-window -lzuazo-ffmpeg -lzuazo-compositor -lglfw -ldl -lpthread
 */

#include <zuazo/Instance.h>
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

	//Construct the desired parameters
	const Zuazo::VideoMode videoMode(
		Zuazo::Utils::MustBe<Zuazo::Rate>(Zuazo::Rate(25, 1)), //Just specify the desired rate
		Zuazo::Utils::Any<Zuazo::Resolution>(),
		Zuazo::Utils::Any<Zuazo::AspectRatio>(),
		Zuazo::Utils::Any<Zuazo::ColorPrimaries>(),
		Zuazo::Utils::Any<Zuazo::ColorModel>(),
		Zuazo::Utils::Any<Zuazo::ColorTransferFunction>(),
		Zuazo::Utils::Any<Zuazo::ColorSubsampling>(),
		Zuazo::Utils::Any<Zuazo::ColorRange>(),
		Zuazo::Utils::Any<Zuazo::ColorFormat>()	
	);

	const Zuazo::Utils::Limit<Zuazo::DepthStencilFormat> depthStencil(
		Zuazo::Utils::MustBe<Zuazo::DepthStencilFormat>(Zuazo::DepthStencilFormat::NONE) //Not interested in the depth buffer
	);

	const auto windowSize = Zuazo::Math::Vec2i(1280, 720);

	const auto& monitor = Zuazo::Consumers::WindowRenderer::NO_MONITOR; //Not interested in the full-screen mode

	//Construct the window object
	Zuazo::Consumers::WindowRenderer window(
		instance, 						//Instance
		"Output Window",				//Layout name
		videoMode,						//Video mode limits
		depthStencil,					//Depth buffer limits
		windowSize,						//Window size (in screen coordinates)
		monitor							//Monitor for setting fullscreen
	);

	//Open the window (now becomes visible)
	window.setResizeable(false); //Disable resizeing, as extra care needs to be taken
	window.open();

	//Create a layer for rendering to the window
	Zuazo::Processors::Layers::VideoSurface videoSurface(
		instance,
		"Video Surface",
		&window,
		static_cast<Zuazo::Math::Vec2f>(window.getVideoMode().getResolutionValue())
	);

	window.setLayers({videoSurface});
	videoSurface.setScalingMode(Zuazo::ScalingMode::BOXED);
	videoSurface.setScalingFilter(Zuazo::ScalingFilter::CUBIC);
	videoSurface.open();

	//Create a video source
	Zuazo::Sources::FFmpegClip videoClip(
		instance,
		"Video Source",
		Zuazo::VideoMode::ANY,
		std::string(argv[1])
	);

	videoClip.play();
	videoClip.setRepeat(Zuazo::ClipBase::Repeat::REPEAT);
	videoClip.open();

	//Route the signal
	videoSurface << videoClip;

	std::cout << "\nSupported video-modes:\n";
	for(const auto& videoMode : window.getVideoModeCompatibility()) {
		std::cout << "\t-" << videoMode << "\n";
	}

	std::cout << "\nSelected video-mode:\n";
	std::cout << "\t-" << window.getVideoMode() << "\n";

	std::cout << "\nSupported depth-stencil formats:\n";
	std::cout << "\t-" << window.getDepthStencilFormatCompatibility() << "\n";

	std::cout << "\nSelected depth-stencil format:\n";
	std::cout << "\t-" << window.getDepthStencilFormat() << "\n";

	//Done!
	lock.unlock();
	getchar();
	lock.lock();
}