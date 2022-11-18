#pragma once

#include "EventBus.h"
#include "GameInfo.h"
#include "KeyCode.h"
#include "Module.h"
#include "SoundHandle.h"
#include "Table.h"

#include <base_lib/List.h>
#include <base_lib/Map.h>
#include <base_lib/Path.h>
#include <base_lib/String.h>
#include <base_lib/Vector2.h>
#include <base_lib/Vector3.h>
#include <base_lib/Version.h>
#include <mutex>

class ITool;
class TableBase;
class OgreApp;

namespace OgreBites {
    class TrayManager;
}

class CameraComponent;
class AudioChannel;
class Audio;
class Entity;
class StaticMesh;
class TextBlock;
class Animation;
class UIInputElement;
class SystemIO;
class Collision;
class ConvexMeshCollision;
class SaveGame;
class Settings;
class SpriteFont;
class UIElement;
class Image;
class Mod;
class GLFWwindow;
class GLFWcursor;
class IControllable;
class World;
class Texture;

namespace reactphysics3d {
    class PhysicsCommon;
}

namespace SoLoud {
    class Soloud;
}

namespace Ogre {
    class Viewport;
    class RenderWindow;
    class Root;
    class GpuProgram;

    namespace RTShader {
        class ShaderGenerator;
    }
} // namespace Ogre

enum class GameStage {
    Unloaded,
    Initialization,
    Loading,
    Starting,
    RenderLoop,
    Unloading
};

class EXPORT Game : public Module, public std::enable_shared_from_this<Game> {
    friend AudioChannel;
    friend Audio;
    friend SoundHandle;
    friend World;
    friend StaticMesh;
    friend Texture;
    friend Entity;
    friend UIElement;
    friend Collision;
    friend SystemIO;
    friend Animation;
    friend Module;

public:
    Game(const String& name, int argc, char* argv[]);

    Path get_module_path(const String& sub_path = "") const override;

    void quit();

    void launch();

    static void possess(const Shared<IControllable>& controllable);
    static void use_camera(const Shared<CameraComponent>& camera);

    static void open_world(const Shared<World>& world);
    static void close_world();
    static const Shared<World>& get_world();

    static const List<String>& get_args();

    static Shared<Game> get_instance();

    static const GameInfo& get_info();
    static const Shared<Settings>& get_settings();

    template<typename T>
    static Shared<T> get_settings() {
        return cast<T>(get_settings());
    }

    static const Shared<SaveGame>& get_save_game();

    template<typename T>
    static Shared<T> get_save_game() {
        return cast<T>(get_save_game());
    }

    static const Shared<EventBus>& get_event_bus();

    template<typename T>
    static Shared<T> get_event_bus() {
        return cast<T>(get_event_bus());
    }

    static void call_on_main_thread(std::function<void()> func);

    static Shared<Texture>& get_white_texture() { return instance_->white_texture_; }
    static Shared<Texture>& get_uv_test_texture() { return instance_->uv_test_texture_; }

    static Shared<Material> get_basic_material();
    static const Shared<SpriteFont>& get_default_font();
    static const Shared<AudioChannel>& get_general_channel();
    static const List<Shared<Mod>>& get_mods();
    static Shared<Module> get_module_by_name(const Name& module_name);

    static uint get_screen_width();
    static uint get_screen_height();

    static const Vector2& get_mouse_pos();
    static const Vector2& get_mouse_delta();
    static void set_mouse_grab(bool state);
    static void set_cursor_texture(const Shared<Texture>& tex, uint hotspot_x, uint hotspot_y);
    static void add_ui(const Shared<UIElement>& ui);
    static float get_ui_scale();
    static Vector3 get_un_projected_mouse();
    static float get_time();

    static GameStage get_stage();

    template<Convertible<Compound::Object> T>
    Shared<Table<T>> create_table(const Name& name) {
        if (!CheckError(get_stage() == GameStage::Initialization, "Database", "Table %s can be created only during initialization", name.c()))
            return nullptr;

        Shared<TableBase>& slot = tables_[name];

        if (!CheckError(!slot, "Database", "Table %s already exists in database", name.c()))
            return nullptr;

        slot = MakeSharedInternal(Table<T>, name.to_string());

        return cast<Table<T>>(slot);
    }

    template<Convertible<Compound::Object> T>
    Shared<Table<T>> get_table(const Name& name) {
        return cast<Table<T>>(tables_.find_or_default(name));
    }

    void on_add_resource_directories(Set<String>& local, Set<String>& global) override;

    const Map<Name, Shared<ITool>>& get_tools() const { return tools_; }

protected:
    virtual void init_game_info(GameInfo& out_info) = 0;
    virtual Shared<Settings> generate_settings_object();
    virtual Shared<SaveGame> generate_save_game_object(const String& profile_name);
    virtual Shared<EventBus> generate_event_bus_object();
    virtual void on_init();
    virtual void on_loading_stage();
    virtual void on_start();
    virtual void on_tick(float delta_time);
    virtual void on_unloading_stage();

    void setup();

private:
    bool handle_tool();
    void initialization_stage();
    void loading_stage();
    void start();
    void render_loop();
    void unloading_stage();

    void search_table_files(const Path& path);
    void load_tables(const Path& path);

    bool keyPressed(KeyCode key, bool repeat);
    bool keyReleased(KeyCode key);
    bool textInput(const char* chars);
    bool mousePressed(int button);
    bool mouseReleased(int button);
    bool axisMoved(int axis, float value);
    bool mouseMoved(const Vector2& new_pos, const Vector2& delta);
    bool wheelRolled(float y);
    void windowResized(Ogre::RenderWindow* rw);

    inline static Shared<Game> instance_ = nullptr;

    List<String> args_;

    GameInfo info_;
    Shared<Settings> settings_;
    Shared<SaveGame> save_game_;
    Shared<EventBus> event_bus_;

    // Common
    Vector2 mouse_pos_;
    Vector2 mouse_delta_;
    Vector3 un_projected_mouse_;
    float time_;

    // Assets
    Map<String, Shared<StaticMesh>> meshes_;
    Map<String, Shared<Animation>> animations_;
    Map<String, Shared<Audio>> audios_;
    List<Shared<AudioChannel>> audio_channels_;

    // Game
    Version game_version_ = {0, 1, 0};
    float ui_scale_ = 2;
    Shared<SpriteFont> default_font_;
    Shared<AudioChannel> general_channel_;
    List<Shared<Mod>> mods_;
    GameStage stage_ = GameStage::Unloaded;
    Map<Name, Shared<TableBase>> tables_;

    Shared<Texture> white_texture_;
    Shared<Texture> uv_test_texture_;

    // Gameplay
    Shared<CameraComponent> current_camera_;
    Shared<IControllable> current_controllable_;
    Shared<World> world_;

    // Core
    Shared<reactphysics3d::PhysicsCommon> physics_;
    Shared<SoLoud::Soloud> soloud_;
    Shared<OgreApp> ogre_app_;
    Ogre::RTShader::ShaderGenerator* shader_generator_;
    Ogre::Viewport* viewport_;
    Weak<UIElement> ui_under_mouse_;
    Weak<UIElement> pressed_ui_;
    List<std::function<void()>> main_thread_calls_;
    std::mutex main_thread_calls_mutex_;
    Map<Name, Shared<ITool>> tools_;
};
