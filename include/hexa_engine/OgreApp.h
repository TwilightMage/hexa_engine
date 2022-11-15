#pragma once

#include "KeyCode.h"

#include <Bites/OgreApplicationContext.h>
#include <Bites/OgreTrays.h>
#include <Bites/OgreWindowEventUtilities.h>
#include <base_lib/Pointers.h>
#include <base_lib/String.h>
#include <base_lib/Vector2.h>

class Texture;

class OgreApp : public OgreBites::ApplicationContext, public OgreBites::InputListener, OgreBites::WindowEventListener, public OgreBites::TrayListener
{
public:
    explicit OgreApp(const String& name);

    void setup() override;
    void load();
    void close();

    bool keyPressed(const OgreBites::KeyboardEvent& evt) override;
    bool keyReleased(const OgreBites::KeyboardEvent& evt) override;
    bool textInput(const OgreBites::TextInputEvent& evt) override;
    bool mousePressed(const OgreBites::MouseButtonEvent& evt) override;
    bool mouseReleased(const OgreBites::MouseButtonEvent& evt) override;
    bool axisMoved(const OgreBites::AxisEvent& evt) override;
    bool mouseMoved(const OgreBites::MouseMotionEvent& evt) override;
    bool mouseWheelRolled(const OgreBites::MouseWheelEvent& evt) override;
    void windowResized(Ogre::RenderWindow* rw) override;

    void set_window_icon(const Shared<Texture>& icon_texture);

    std::function<void()> on_setup;

    std::function<bool(KeyCode key, bool repeat)> on_keyPressed;
    std::function<bool(KeyCode key)> on_keyReleased;
    std::function<bool(const char* chars)> on_textInput;
    std::function<bool(int button)> on_mousePressed;
    std::function<bool(int button)> on_mouseReleased;
    std::function<bool(int axis, float value)> on_axisMoved;
    std::function<bool(const Vector2& new_pos, const Vector2& delta)> on_mouseMoved;
    std::function<bool(float y)> on_wheelRolled;
    std::function<void(Ogre::RenderWindow*)> on_windowResized;

private:
    Shared<OgreBites::TrayManager> ui_;
};
