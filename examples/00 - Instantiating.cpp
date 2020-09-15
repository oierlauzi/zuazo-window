/*
 * This example shows how instantiate an output window
 * 
 * How to compile:
 * c++ 00\ -\ Instantiating.cpp -std=c++17 -Wall -Wextra -lzuazo -lzuazo-window -lglfw -ldl -lpthread
 */

#include "TestSource.h"

#include <zuazo/Instance.h>
#include <zuazo/Modules/Window.h>
#include <zuazo/Outputs/Window.h>

#include <mutex>

int main() {
	//Instantiate Zuazo as usual. Note that we're loading the Window module
	Zuazo::Instance::ApplicationInfo appInfo(
		"Window Example 00",						//Application's name
		Zuazo::Version(0, 1, 0),					//Application's version
		Zuazo::Verbosity::GEQ_INFO,					//Verbosity 
		{ Zuazo::Modules::Window::get() }			//Modules
	);
	Zuazo::Instance instance(std::move(appInfo));
	std::unique_lock<Zuazo::Instance> lock(instance);

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
		videoMode,						//Video mode limits
		Zuazo::Math::Vec2i(1280, 720),	//Window size (in screen coordinates)
		Zuazo::Outputs::Window::NO_MONITOR //No monitor
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

	//Done!
	lock.unlock();
	getchar();
	lock.lock();
}