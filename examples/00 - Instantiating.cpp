/*
 * This example shows how instantiate an output window
 * 
 * How to compile:
 * c++ 00\ -\ Instantiating.cpp -std=c++17 -Wall -Wextra -lzuazo -ldl -lpthread
 */

#include <zuazo/Instance.h>
#include <zuazo/Outputs/Window.h>

#include <mutex>

int main() {
	//Load all the required window components. This *MUST* be done before instantiating Zuazo
	Zuazo::Outputs::Window::init();

	//Instantiate Zuazo as usual
	Zuazo::Instance::ApplicationInfo appInfo {
		"Window Example 00",						//Application's name
		Zuazo::Version(0, 1, 0),					//Application's version
		Zuazo::Verbosity::GEQ_INFO,					//Verbosity 
		Zuazo::VideoMode::ANY,						//Default video-mode
		Zuazo::Instance::defaultInstanceLogFunc,	//Instance log callback
		Zuazo::Instance::defaultElementLogFunc		//Element log callback
	};
	Zuazo::Instance instance(std::move(appInfo));
	std::unique_lock<Zuazo::Instance> lock(instance);

	//Generaly, you want to enable event polling
	Zuazo::Outputs::Window::enableRegularEventPolling(instance);

	//Construct the desired video mode
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

	//Construct the window object
	Zuazo::Outputs::Window window(
		instance, 						//Instance
		"Output Window",				//Layout name
		Zuazo::Math::Vec2i(1280, 720),	//Window size (in screen coordinates)
		videoMode						//Video mode limits
	);

	//Open the window (now becomes visible)
	window.open();

	//Done!
	lock.unlock();
	getchar();
	lock.lock();

}