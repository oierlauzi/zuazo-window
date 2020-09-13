/*
 * This example shows how instantiate an output window and set it fullscreen
 * 
 * How to compile:
 * c++ 01\ -\ Monitor.cpp -std=c++17 -Wall -Wextra -lzuazo -lzuazo-window -lglfw -ldl -lpthread
 */

#include "TestSource.h"

#include <zuazo/Instance.h>
#include <zuazo/Outputs/Window.h>

#include <mutex>
#include <iostream>
#include <thread>

int main() {
	//Load all the required window components. This *MUST* be done before instantiating Zuazo
	Zuazo::Outputs::Window::init();

	//Instantiate Zuazo as usual
	Zuazo::Instance::ApplicationInfo appInfo {
		"Window Example 01",						//Application's name
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
		Zuazo::Utils::MustBe<Zuazo::Rate>(Zuazo::Rate(30, 1)), //Just specify the desired rate
		Zuazo::Utils::Any<Zuazo::Resolution>(),
		Zuazo::Utils::Any<Zuazo::AspectRatio>(),
		Zuazo::Utils::Any<Zuazo::ColorPrimaries>(),
		Zuazo::Utils::Any<Zuazo::ColorModel>(),
		Zuazo::Utils::Any<Zuazo::ColorTransferFunction>(),
		Zuazo::Utils::Any<Zuazo::ColorSubsampling>(),
		Zuazo::Utils::Any<Zuazo::ColorRange>(),
		Zuazo::Utils::Any<Zuazo::ColorFormat>()	
	);

	//Query the available monitors
	const auto monitors = Zuazo::Outputs::Window::getMonitors();

	if(monitors.size() == 0) {
		std::cerr << "No monitor is present!" << std::endl;
		std::terminate();
	}



	//Construct the window object
	Zuazo::Outputs::Window window(
		instance, 						//Instance
		"Output Window",				//Layout name
		videoMode,						//Video mode limits
		Zuazo::Math::Vec2i(1280, 720),	//Window size (in screen coordinates).
		Zuazo::Outputs::Window::NO_MONITOR //The monitor
		
	);

	//Open the window (now becomes visible)
	window.open();

	//Let's route a test source to it
	const Zuazo::VideoMode testVideoMode(
		Zuazo::Utils::MustBe<Zuazo::Rate>(Zuazo::Rate(25, 1)), //Just specify the desired rate
		Zuazo::Utils::MustBe<Zuazo::Resolution>(Zuazo::Resolution(1280, 720)),
		Zuazo::Utils::MustBe<Zuazo::AspectRatio>(Zuazo::AspectRatio(1, 1)),
		Zuazo::Utils::MustBe<Zuazo::ColorPrimaries>(Zuazo::ColorPrimaries::BT709),
		Zuazo::Utils::MustBe<Zuazo::ColorModel>(Zuazo::ColorModel::RGB),
		Zuazo::Utils::MustBe<Zuazo::ColorTransferFunction>(Zuazo::ColorTransferFunction::IEC61966_2_1),
		Zuazo::Utils::MustBe<Zuazo::ColorSubsampling>(Zuazo::ColorSubsampling::RB_444),
		Zuazo::Utils::MustBe<Zuazo::ColorRange>(Zuazo::ColorRange::FULL),
		Zuazo::Utils::MustBe<Zuazo::ColorFormat>(Zuazo::ColorFormat::B8G8R8A8)	
	);

	TestSource testSrc(instance, "", testVideoMode);
	testSrc.open();
	Zuazo::Signal::getInput<Zuazo::Video>(window) << Zuazo::Signal::getOutput<Zuazo::Video>(testSrc);


	//Display the available monitors for selection
	std::cout << "Select monitors:" << std::endl;
	for(size_t i = 0; i < monitors.size(); i++) {
		std::cout << "[" << i << "] " << monitors[i].getName() << " "<< Zuazo::Resolution(monitors[i].getSize()) << std::endl;
	}
	size_t monitorSelection;

	lock.unlock();
	std::cin >> monitorSelection;
	lock.lock();

	if(monitorSelection >= monitors.size()) {
		std::cerr << "Wrong selection!" << std::endl;
		std::terminate();
	}

	window.setMonitor(monitors[monitorSelection]);

	lock.unlock();
	std::this_thread::sleep_for(std::chrono::seconds(4));
	lock.lock();

	window.setMonitor(Zuazo::Outputs::Window::NO_MONITOR);

	lock.unlock();
	//Some strange behaviour, that's why this line is duplicate
	std::cin.get();
	std::cin.get();
	lock.lock();
}