#pragma once

#include "GLFW.h"

#include <zuazo/Keyboard.h>

namespace Zuazo {

constexpr GLFW::Window::KeyboardKey toGLFW(KeyboardKey key);
constexpr KeyboardKey fromGLFW(GLFW::Window::KeyboardKey key);

constexpr GLFW::Window::MouseButton toGLFW(MouseKey key);
constexpr MouseKey fromGLFW(GLFW::Window::MouseButton key);

constexpr GLFW::Window::KeyboardModifiers toGLFW(KeyModifiers modifiers);
constexpr KeyModifiers fromGLFW(GLFW::Window::KeyboardModifiers modifiers);

constexpr GLFW::Window::KeyboardEvent toGLFW(KeyEvent event);
constexpr KeyEvent fromGLFW(GLFW::Window::KeyboardEvent event);

}

#include "GLFWConversions.inl"