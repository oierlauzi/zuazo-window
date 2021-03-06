#pragma once

#include <zuazo/Macros.h>
#include <zuazo/ZuazoBase.h>
#include <zuazo/RendererBase.h>
#include <zuazo/Video.h>
#include <zuazo/ScalingMode.h>
#include <zuazo/ScalingFilter.h>
#include <zuazo/Keyboard.h>
#include <zuazo/Math/Vector.h>
#include <zuazo/Utils/Pimpl.h>
#include <zuazo/Utils/BufferView.h>
#include <zuazo/Signal/ConsumerLayout.h>

#include <tuple>
#include <vector>
#include <mutex>

namespace Zuazo::Renderers {

class Window final
	: public Utils::Pimpl<struct WindowImpl>
	, public ZuazoBase
	, public VideoBase
	, public RendererBase
{
	friend WindowImpl;
public:

	class Monitor {
	public:
		struct Mode {
			Math::Vec2i	size;
			Math::Vec3i colorDepth;
			int			frameRate;
		};

		Monitor();
		Monitor(const Monitor& other);
		~Monitor();

		Monitor&						operator=(const Monitor& other);

		bool							operator==(const Monitor& other) const noexcept;
		bool							operator!=(const Monitor& other) const noexcept;

		std::string_view				getName() const;
		Math::Vec2d						getPhysicalSize() const;
		Math::Vec2i						getPosition() const;
		const Mode&						getMode() const;
		Utils::BufferView<const Mode>	getModes() const;

	private:
		void*							m_handle;

	};


	using SizeCallback = std::function<void(Window&, Math::Vec2i)>;
	using PositionCallback = std::function<void(Window&, Math::Vec2i)>;
	using IconifyCallback = std::function<void(Window&, bool)>;
	using MaximizeCallback = std::function<void(Window&, bool)>;
	using FocusCallback = std::function<void(Window&, bool)>;
	using ScaleCallback = std::function<void(Window&, Math::Vec2f)>;
	using ShouldCloseCallback = std::function<void(Window&)>;
	using KeyboardCallback = std::function<void(Window&, KeyboardKey, KeyEvent, KeyModifiers)>;
	using CharacterCallback = std::function<void(Window&, uint)>;
	using MouseButtonCallback = std::function<void(Window&, MouseKey, KeyEvent, KeyModifiers)>;
	using MousePositionCallback = std::function<void(Window&, Math::Vec2d)>;
	using MouseScrollCallback = std::function<void(Window&, Math::Vec2d)>;
	using CursorEnterCallback = std::function<void(Window&, bool)>;


	struct Callbacks {
		SizeCallback				sizeCbk;
		PositionCallback			positionCbk;
		IconifyCallback				iconifyCbk;
		MaximizeCallback			maximizeCbk;
		FocusCallback				focusCbk;
		ScaleCallback				scaleCbk;
		ShouldCloseCallback			shouldCloseCbk;
		KeyboardCallback			keyboardCbk;
		CharacterCallback			characterCbk;
		MouseButtonCallback			mouseButtonCbk;
		MousePositionCallback		mousePositionCbk;
		MouseScrollCallback			mouseScrollCbk;
		CursorEnterCallback			cursorEnterCbk;
	};



	Window(	Instance& instance, 
			std::string name,
			Math::Vec2i size,
			const Monitor& mon = NO_MONITOR );
	Window(const Window& other) = delete;
	Window(Window&& other);
	virtual ~Window();

	Window&						operator=(const Window& other) = delete;
	Window&						operator=(Window&& other);

	void						setTitle(std::string name);
	const std::string&			getTitle() const;

	void						setSize(Math::Vec2i size);
	Math::Vec2i					getSize() const;
	void						setSizeCallback(SizeCallback cbk);
	const SizeCallback&			getSizeCallback() const;

	void						setPosition(Math::Vec2i pos);
	Math::Vec2i					getPosition() const;
	void						setPositionCallback(PositionCallback cbk);
	const PositionCallback&		getPositionCallback() const;

	Math::Vec2f					getScale() const;
	void						setScaleCallback(ScaleCallback cbk);
	const ScaleCallback&		getScaleCallback() const;

	bool						shouldClose() const;
	void						setShouldCloseCallback(ShouldCloseCallback cbk);
	const ShouldCloseCallback&	getShouldCloseCallback() const;

	void						setOpacity(float opa);
	float						getOpacity() const;

	void						setResizeable(bool resizeable);
	bool						getResizeable() const;

	void						setDecorated(bool deco);
	bool						getDecorated() const;

	void						setVisibility(bool visibility);
	bool						getVisibility() const;

	void						iconify();
	bool						isIconified() const;
	void						setIconifyCallback(IconifyCallback cbk);
	const IconifyCallback&		getIconifyCallback() const;

	void						maximize();
	bool						isMaximized() const;
	void						setMaximizeCallback(MaximizeCallback cbk);
	const MaximizeCallback&		getMaximizeCallback() const;

	void 						focus();
	bool						isFocused() const;
	void						setFocusCallback(FocusCallback cbk);
	const FocusCallback&		getFocusCallback() const;

	void						restore();

	void						setMonitor(const Monitor& mon, const Monitor::Mode* mode);
	Monitor						getMonitor() const;


	KeyEvent					getKeyState(KeyboardKey key) const;
	void						setKeyboardCallback(KeyboardCallback cbk);
	const KeyboardCallback&		getKeyboardCallback() const;

	void						setCharacterCallback(CharacterCallback cbk);
	const CharacterCallback&	getCharacterCallback() const;

	KeyEvent					getMouseButtonState(MouseKey but) const;
	void						setMouseButtonCallback(MouseButtonCallback cbk);
	const MouseButtonCallback&	getMouseButtonCallback() const;
	
	Math::Vec2d					getMousePosition() const;
	void						setMousePositionCallback(MousePositionCallback cbk);
	const MousePositionCallback& getMousePositionCallback() const;

	void						setMouseScrollCallback(MouseScrollCallback cbk);
	const MouseScrollCallback& 	getMouseScrollCallback() const;

	void						setCursorEnterCallback(CursorEnterCallback cbk);
	const CursorEnterCallback& 	getCursorEnterCallback() const;

	static Monitor							getPrimaryMonitor();
	static Utils::BufferView<const Monitor>	getMonitors();

	static const Monitor					NO_MONITOR;

};

}