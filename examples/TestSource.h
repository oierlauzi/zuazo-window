#pragma once

#include <zuazo/ZuazoBase.h>
#include <zuazo/Video.h>
#include <zuazo/Graphics/Uploader.h>
#include <zuazo/Signal/Layout.h>
#include <zuazo/Signal/Output.h>

class TestSource 
	: public Zuazo::ZuazoBase
{
public:
	TestSource(Zuazo::Instance& instance, std::string name, Zuazo::VideoMode videoMode)
		: ZuazoBase(
			instance,
			std::move(name),
			std::bind(&TestSource::openImpl, std::ref(*this), std::placeholders::_1),
			std::bind(&TestSource::closeImpl, std::ref(*this), std::placeholders::_1),
			std::bind(&TestSource::update, std::ref(*this)),
			{ videoOut }
		)
		, videoMode(std::move(videoMode))
		, uploader(instance.getVulkan(), videoMode.getFrameDescriptor())
		, videoOut(std::string(Zuazo::Signal::makeOutputName<Zuazo::Video>()))
	{
	}

private:
	Zuazo::VideoMode videoMode;
	Zuazo::Graphics::Uploader uploader;
	Zuazo::Signal::Output<Zuazo::Video> videoOut;
	int time;

	void openImpl(ZuazoBase&) {
		enablePeriodicUpdate(Zuazo::Instance::INPUT_PRIORITY, Zuazo::getPeriod(videoMode.getFrameRateValue()));
	}

	void closeImpl(ZuazoBase&) {
		disablePeriodicUpdate();
	}

	void update() {
		const auto frame = uploader.acquireFrame();

		const auto& pixels = frame->getPixelData();
		for(size_t i = 0; i < pixels[0].size(); i+=4){
			pixels[0][i + 0] = static_cast<std::byte>(time % 0x100);
			pixels[0][i + 1] = static_cast<std::byte>(time % 0x100);
			pixels[0][i + 2] = static_cast<std::byte>(time % 0x100);
			pixels[0][i + 3] = static_cast<std::byte>(0xff);
		}
        time++;

		frame->flush();

        videoOut.push(frame);
    }

};