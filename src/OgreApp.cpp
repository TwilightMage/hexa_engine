#include "hexa_engine/OgreApp.h"

#include "hexa_engine/Texture.h"

OgreApp::OgreApp(const String& name)
    : OgreBites::ApplicationContext(name.c())
{
}

void OgreApp::setup()
{
    OgreBites::ApplicationContext::setup();

    addInputListener(this);

    OgreBites::WindowEventUtilities::_addRenderWindow(getRenderWindow());
    OgreBites::WindowEventUtilities::addWindowEventListener(getRenderWindow(), this);

    on_setup();
}

void OgreApp::load()
{
    ui_ = MakeShared<OgreBites::TrayManager>("UI", getRenderWindow(), this);
    addInputListener(ui_.get());
    ui_->hideCursor();
}

void OgreApp::close()
{
    if (ui_)
    {
        removeInputListener(ui_.get());
        ui_ = nullptr;
    }

    closeApp();
}

bool OgreApp::keyPressed(const OgreBites::KeyboardEvent& evt)
{
    return on_keyPressed((KeyCode)evt.keysym.sym, evt.repeat);
}

bool OgreApp::keyReleased(const OgreBites::KeyboardEvent& evt)
{
    return on_keyReleased((KeyCode)evt.keysym.sym);
}

bool OgreApp::textInput(const OgreBites::TextInputEvent& evt)
{
    return on_textInput(evt.chars);
}

bool OgreApp::mousePressed(const OgreBites::MouseButtonEvent& evt)
{
    return on_mousePressed(evt.button);
}

bool OgreApp::mouseReleased(const OgreBites::MouseButtonEvent& evt)
{
    return on_mouseReleased(evt.button);
}

bool OgreApp::axisMoved(const OgreBites::AxisEvent& evt)
{
    return on_axisMoved(evt.axis, evt.value);
}

bool OgreApp::mouseMoved(const OgreBites::MouseMotionEvent& evt)
{
    return on_mouseMoved(Vector2((float)evt.x, (float)evt.y), Vector2((float)evt.xrel, (float)evt.yrel));
}

bool OgreApp::mouseWheelRolled(const OgreBites::MouseWheelEvent& evt)
{
    return on_wheelRolled(evt.y);
}

void OgreApp::windowResized(Ogre::RenderWindow* rw)
{
    on_windowResized(rw);
}

void OgreApp::set_window_icon(const Shared<Texture>& icon_texture)
{
    // TODO: Implement set_window_icon
    // setWindowIcon((byte*)icon_texture->get_pixels().begin(), icon_texture->get_width(), icon_texture->get_height(), mWindows[0].native);
}
