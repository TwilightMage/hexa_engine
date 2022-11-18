#include "hexa_engine/Game.h"

#include "hexa_engine/AudioChannel.h"
#include "hexa_engine/CameraComponent.h"
#include "hexa_engine/Material.h"
#include "hexa_engine/Mod.h"
#include "hexa_engine/OgreApp.h"
#include "hexa_engine/SaveGame.h"
#include "hexa_engine/Settings.h"
#include "hexa_engine/StaticMesh.h"
#include "hexa_engine/TableBase.h"
#include "hexa_engine/Texture.h"
#include "hexa_engine/World.h"
#include "hexa_engine/tools/Comp.h"
#include "hexa_engine/tools/Help.h"
#include "hexa_engine/tools/ITool.h"

#include <Bites/OgreTrays.h>
#include <OgreCompositorManager.h>
#include <OgreEntity.h>
#include <OgreRenderWindow.h>
#include <RTShaderSystem/OgreShaderGenerator.h>
#include <base_lib/File.h>
#include <base_lib/Logger.h>
#include <base_lib/Path.h>
#include <base_lib/Set.h>
#include <base_lib/performance.h>
#include <hexa_engine/Audio.h>
#include <hexa_engine/IControllable.h>
#include <reactphysics3d/reactphysics3d.h>
#include <soloud/soloud.h>

Game::Game(const String& name, int argc, char* argv[])
    : Module(Name(name))
{
    if (instance_)
    {
        throw std::runtime_error("Only one game instance may exist");
    }
    else
    {
        instance_ = Shared<Game>(this);
    }

    for (int i = 0; i < argc; i++)
    {
        args_.add(argv[i]);
    }

    Utils::init_base_lib(argv[0]);
    
    event_bus_ = MakeShared<EventBus>();
    physics_ = MakeShared<reactphysics3d::PhysicsCommon>();
    soloud_ = MakeShared<SoLoud::Soloud>();
    ogre_app_ = MakeShared<OgreApp>(name);

    ogre_app_->on_setup = std::bind(&Game::setup, this);

    ogre_app_->on_keyPressed = std::bind(&Game::keyPressed, this, std::placeholders::_1, std::placeholders::_2);
    ogre_app_->on_keyReleased = std::bind(&Game::keyReleased, this, std::placeholders::_1);
    ogre_app_->on_textInput = std::bind(&Game::textInput, this, std::placeholders::_1);
    ogre_app_->on_mousePressed = std::bind(&Game::mousePressed, this, std::placeholders::_1);
    ogre_app_->on_mouseReleased = std::bind(&Game::mouseReleased, this, std::placeholders::_1);
    ogre_app_->on_axisMoved = std::bind(&Game::axisMoved, this, std::placeholders::_1, std::placeholders::_2);
    ogre_app_->on_mouseMoved = std::bind(&Game::mouseMoved, this, std::placeholders::_1, std::placeholders::_2);
    ogre_app_->on_wheelRolled = std::bind(&Game::wheelRolled, this, std::placeholders::_1);
    ogre_app_->on_windowResized = std::bind(&Game::windowResized, this, std::placeholders::_1);
}

Path Game::get_module_path(const String& sub_path) const
{
    return Path("./" + sub_path);
}

void Game::quit() {
    ogre_app_->getRoot()->queueEndRendering();
}

void Game::launch()
{
    if (handle_tool()) return;

    verbose("Game", "Launching...");

    ogre_app_->initApp();

    initialization_stage();
    loading_stage();
    start();
    render_loop();
    unloading_stage();

    stage_ = GameStage::Unloaded;

    ogre_app_->close();

    instance_.reset();
}

void Game::possess(const Shared<IControllable>& controllable)
{
    if (instance_->current_controllable_)
    {
        instance_->current_controllable_->on_unpossess();
    }
    instance_->current_controllable_ = controllable;
    instance_->current_controllable_->on_possess();
}

void Game::use_camera(const Shared<CameraComponent>& camera)
{
    instance_->current_camera_ = camera;
    if (instance_->viewport_)
    {
        instance_->viewport_->setCamera(camera->ogre_camera_);
    }
    else
    {
        instance_->viewport_ = instance_->ogre_app_->getRenderWindow()->addViewport(camera->ogre_camera_);
        // Ogre::CompositorManager::getSingleton().addCompositor(instance_->viewport_,
        // "Hexa/Flip_X_comp");
        // Ogre::CompositorManager::getSingleton().setCompositorEnabled(instance_->viewport_,
        // "Hexa/Flip_X_comp", true);

        Ogre::CompositorManager::getSingleton().addCompositor(instance_->viewport_, "DeferredShading/GBuffer");
        Ogre::CompositorManager::getSingleton().setCompositorEnabled(instance_->viewport_, "DeferredShading/GBuffer", true);

        Ogre::CompositorManager::getSingleton().addCompositor(instance_->viewport_, "DeferredShading/ShowLit");
        Ogre::CompositorManager::getSingleton().setCompositorEnabled(instance_->viewport_, "DeferredShading/ShowLit", true);
    }
}

void Game::open_world(const Shared<World>& world)
{
    close_world();

    instance_->world_ = world;
    if (instance_->world_)
    {
        instance_->world_->init();
        instance_->shader_generator_->addSceneManager(instance_->world_->manager_);
        instance_->world_->manager_->addRenderQueueListener(&Ogre::OverlaySystem::getSingleton());
        instance_->world_->start();
        instance_->event_bus_->world_opened(world);
    }
    else
    {
        print_warning("Game", "Attempt to open nullptr world");
    }
}

void Game::close_world()
{
    if (instance_->world_)
    {
        instance_->current_camera_ = nullptr;

        instance_->world_->manager_->removeRenderQueueListener(&Ogre::OverlaySystem::getSingleton());
        instance_->shader_generator_->removeSceneManager(instance_->world_->manager_);
        instance_->world_->close();
        instance_->event_bus_->world_closed(instance_->world_);
        instance_->world_ = nullptr;
    }
}

const Shared<World>& Game::get_world()
{
    return instance_->world_;
}

const List<String>& Game::get_args()
{
    return instance_->args_;
}

Shared<Game> Game::get_instance()
{
    return instance_;
}

const GameInfo& Game::get_info()
{
    return instance_->info_;
}

const Shared<Settings>& Game::get_settings()
{
    return instance_->settings_;
}

const Shared<SaveGame>& Game::get_save_game()
{
    return instance_->save_game_;
}

const Shared<EventBus>& Game::get_event_bus()
{
    return instance_->event_bus_;
}

void Game::call_on_main_thread(std::function<void()> func)
{
    instance_->main_thread_calls_mutex_.lock();
    instance_->main_thread_calls_.add(func);
    instance_->main_thread_calls_mutex_.unlock();
}

Shared<Material> Game::get_basic_material()
{
    return instance_->load_material("basic");
    /*return instanced
            ? instance_->get_material("Engine/Basic_Instanced")
            : instance_->get_material("Engine/Basic");*/
}

const Shared<SpriteFont>& Game::get_default_font()
{
    return instance_->default_font_;
}

const Shared<AudioChannel>& Game::get_general_channel()
{
    return instance_->general_channel_;
}

const List<Shared<Mod>>& Game::get_mods()
{
    return instance_->mods_;
}

Shared<Module> Game::get_module_by_name(const Name& module_name)
{
    if (instance_->get_module_name() == module_name)
        return instance_;

    for (const auto& mod : instance_->mods_)
    {
        if (mod->get_module_name() == module_name)
            return mod;
    }

    return nullptr;
}

uint Game::get_screen_width()
{
    return GetSystemMetrics(SM_CXSCREEN);
}

uint Game::get_screen_height()
{
    return GetSystemMetrics(SM_CYSCREEN);
}

const Vector2& Game::get_mouse_pos()
{
    return instance_->mouse_pos_;
}

const Vector2& Game::get_mouse_delta()
{
    return instance_->mouse_delta_;
}

void Game::set_mouse_grab(bool state)
{
    instance_->ogre_app_->setWindowGrab(state);
}

void Game::set_cursor_texture(const Shared<Texture>& tex, uint hotspot_x, uint hotspot_y)
{
    uint scale = Math::round(instance_->ui_scale_);

    Array2D<Color> cursor_pixels =
        Array2D<Color>(tex->get_width() * scale, tex->get_height() * scale);

    for (uint x = 0; x < cursor_pixels.get_size_x(); x++)
    {
        for (uint y = 0; y < cursor_pixels.get_size_y(); y++)
        {
            cursor_pixels.at(x, y) = tex->get_pixel(x / scale, y / scale);
        }
    }

    uint len = cursor_pixels.get_size_x() * cursor_pixels.get_size_y() * 4;

    // TODO: Implement set_mouse_texture
    // instance_->ogre_app_->setMouseTexture((byte*)cursor_pixels.to_list().get_data(),
    // cursor_pixels.get_size_x(), cursor_pixels.get_size_y(), hotspot_x,
    // hotspot_y);
}

void Game::add_ui(const Shared<UIElement>& ui)
{
    // TODO: Implement Game::add_ui(const Shared<UIElement>& ui)
    //instance_->ui_root_->add_child(ui);
}

float Game::get_ui_scale()
{
    return instance_->ui_scale_;
}

Vector3 Game::get_un_projected_mouse()
{
    return instance_->un_projected_mouse_;
}

float Game::get_time()
{
    return instance_->time_;
}

GameStage Game::get_stage()
{
    return instance_->stage_;
}

void Game::on_add_resource_directories(Set<String>& local,
                                       Set<String>& global)
{
    local = {"textures/actions"};

    global = {
        "textures/ui", "textures/tiles", "textures/complex_tiles",
        "textures/characters", "meshes/characters", "meshes/items",
        "meshes/tiles", "audio/ambient", "audio/effects",
        "audio/music"};
}

Shared<Settings> Game::generate_settings_object()
{
    return MakeShared<Settings>();
}

Shared<SaveGame> Game::generate_save_game_object(const String& profile_name)
{
    return MakeShared<SaveGame>(profile_name);
}

Shared<EventBus> Game::generate_event_bus_object()
{
    return MakeShared<EventBus>();
}

void Game::on_init()
{
}

void Game::on_loading_stage()
{
}

void Game::on_start()
{
}

void Game::on_tick(float delta_time)
{
}

void Game::on_unloading_stage()
{
}

void Game::setup()
{
    shader_generator_ = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    reset_global_resources_directories();
    add_resource_directories();

    //Ogre::ResourceGroupManager::getSingletonPtr()->addResourceLocation(Path("resources/SdkTrays.zip").get_absolute_string().c(), "Zip", module_name.c());
}

bool Game::handle_tool()
{
    const static auto register_tool = [&](Shared<ITool> tool) -> void {
        tools_[tool->get_tool_name()] = tool;
    };
    if (tools_.size() == 0) {
        register_tool(MakeShared<Tools::Help>());
        register_tool(MakeShared<Tools::Comp>());
    }
    
    const List<String>& args = get_args();

    if (args.length() >= 2 && args[1] == "comp")
    {
        const Name tool_name = Name(args[1]);

        if (const auto tool = tools_.find_or_default(tool_name)) {
            auto tool_args = args;
            tool_args.remove_range(0, 2);
            tool->execute(tool_args);
            return true;
        }
    }

    return false;
}

void Game::initialization_stage()
{
    verbose("Game", "Initialization...");
    stage_ = GameStage::Initialization;

    info_ = GameInfo();

    info_.title = "Untitled Game";

    init_game_info(info_);

    settings_ = generate_settings_object();
    
    const Path settings_path = Path("settings.json");
    Compound::Object settings_json;
    Compound::Convert::try_load_object_from_file(settings_path, settings_json);
    settings_->read_settings(settings_json);
    Compound::Convert::save_to_file(settings_path, true, settings_->write_settings());

    soloud_->init();

    on_init();

    verbose("Mod Loader", "Searching for mods in mods folder");
    for (auto& path : Path("mods").list())
    {
        if (path.get_type() == EPathType::Directory && Mod::verify_signature(path))
        {
            if (auto mod = Mod::load(path.get_child(path.filename + ".dll")))
            {
                mod->info_ = mod->load_mod_info(path.get_child(path.filename + ".meta"));
                if (!mod->info_.target_game_version.match(game_version_))
                {
                    print_error("Mod Loader", "%s >>> Target game version %s doesn't match %s, skipping", 
                                mod->info_.get_full_display_name().c(),
                                mod->info_.target_game_version.to_string().c(),
                                game_version_.to_string().c());
                    continue;
                }
                mod->add_resource_directories();
                mods_.add(mod);
                verbose("Mod Loader", "%s >>> Initialized mod", mod->info_.get_full_display_name().c());
            }
            else
            {
                print_warning("Mod Loader", "%s >>> Failed to initialize", mod->info_.get_full_display_name().c());
            }
        }
    }

    search_table_files(get_resources_path());
    for (auto mod : mods_)
    {
        search_table_files(mod->get_resources_path());
    }
}

void Game::loading_stage()
{
    verbose("Game", "Loading stage...");
    stage_ = GameStage::Loading;

    register_resource_directories();

    // Engine/White texture
    {
        Array2D<Color> white(1, 1);
        white.at(0, 0) = Color::white();

        white_texture_ = create_texture(white, Name("white"));
    }

    // Engine/UV_Test texture
    {
        const uint cc = 32;
        const uint s = 512;
        const uint cs = s / cc;
        Array2D<Color> uv_test(s, s);
        for (uint x = 0; x < s; x++)
        {
            for (uint y = 0; y < s; y++)
            {
                uv_test.at(x, y) = ((x / cs) % 2 == 0) != ((y / cs) % 2 == 0)
                                       ? Color(178, 178, 178)
                                       : Color(76, 76, 76);
            }
        }

        for (uint x = 0; x < cc; x++)
        {
            for (uint y = 0; y < cc; y++)
            {
                Color c = Color((byte)(1.0f / cc * (x + 0.5f) * 255), (byte)(1.0f / cc * (y + 0.5f) * 255), 0);
                uint xx = x * cs;
                uint yy = y * cs;

                uv_test.at(xx + cs / 2, yy + cs / 2) = c;

                uv_test.at(xx + cs / 2 - 1, yy + cs / 2) = c;
                uv_test.at(xx + cs / 2, yy + cs / 2 - 1) = c;
                uv_test.at(xx + cs / 2 + 1, yy + cs / 2) = c;
                uv_test.at(xx + cs / 2, yy + cs / 2 + 1) = c;

                uv_test.at(xx + cs / 2 - 2, yy + cs / 2) = c;
                uv_test.at(xx + cs / 2, yy + cs / 2 - 2) = c;
                uv_test.at(xx + cs / 2 + 2, yy + cs / 2) = c;
                uv_test.at(xx + cs / 2, yy + cs / 2 + 2) = c;
            }
        }

        uv_test_texture_ = create_texture(uv_test, Name("uv_test"));
    }

    for (const auto& mod : mods_)
    {
        mod->register_resource_directories();
    }

    Ogre::ResourceGroupManager::getSingletonPtr()->initialiseResourceGroup(get_module_name().c());
    for (const auto& mod : mods_)
    {
        if (Ogre::ResourceGroupManager::getSingletonPtr()->resourceGroupExists(mod->get_module_name().c()))
        {
            Ogre::ResourceGroupManager::getSingletonPtr()->initialiseResourceGroup(mod->get_module_name().c());
        }
    }

    ogre_app_->load();

    // Fonts
    // TODO: Remove our fonts
    //default_font_ = SpriteFont::load_fnt(RESOURCES_FONTS + "arial.fnt");

    // Audio channels
    general_channel_ = AudioChannel::create();
    general_channel_->set_volume(settings_->audio_general);

    on_loading_stage();

    // Call loading stage in mods
    for (auto& mod : mods_)
    {
        try
        {
            mod->on_loading_stage();
            verbose("Mod Loader", "Loaded mod %s", mod->info_.name.c());
        }
        catch (std::runtime_error err)
        {
            print_error("Mod Loader", "Failed to load mod %s: %s",
                        mod->info_.name.c(), err.what());
            return;
        }
    }

    for (const auto& table_entry : tables_)
    {
        table_entry.value->post_load();
    }

    for (const auto& table_entry : tables_)
    {
        table_entry.value->init_assets();
    }
}

void Game::start()
{
    verbose("Game", "Starting...");
    stage_ = GameStage::Starting;

    save_game_ = generate_save_game_object("soft fur dragon");

    soloud_->set3dListenerUp(0, 0, 1);

    for (auto& mod : mods_)
    {
        try
        {
            mod->on_start(event_bus_);
            verbose("Mod Loader", "started mod %s", mod->info_.name.c());
        }
        catch (std::runtime_error err)
        {
            print_error("Mod Loader", "Failed to start mod %s: %s",
                        mod->info_.name.c(), err.what());
            return;
        }
    }

    on_start();
}

void Game::render_loop()
{
    verbose("Game", "Entering render loop...");
    stage_ = GameStage::RenderLoop;

    /*float fps_delta_time_stack = 0;
    uint fps_count = 0;
    uint fps_last_count = 0;*/

    /*auto fps_display = MakeShared<TextBlock>();
    fps_display->set_z(10);
    add_ui(fps_display);*/

    const auto start_time = std::chrono::system_clock::now();

    auto tick_start = start_time;

    while (!ogre_app_->getRoot()->endRenderingQueued())
    {
        // while
        // (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()
        // - tick_start).count() / 1000.0f < 1.0f / settings_->fps_limit);
        const float tick_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - tick_start).count() / 1000.0f;
        tick_start = std::chrono::system_clock::now();

        time_ += tick_time;

        main_thread_calls_mutex_.lock();
        for (auto& func : main_thread_calls_)
        {
            func();
        }
        main_thread_calls_.clear();
        main_thread_calls_mutex_.unlock();

        /*fps_delta_time_stack += tick_time;
        fps_count++;
        if (fps_delta_time_stack >= 1)
        {
            fps_delta_time_stack -= 1;
            fps_last_count = fps_count;
            fps_count = 0;
        }

        fps_display->set_text(String::format("FPS: %i", fps_last_count));*/

        if (world_)
        {
            // ticking
            if (tick_time > 0.0f)
            {
                on_tick(tick_time);
                world_->tick(tick_time);
            }

            if (current_camera_)
            {
                const auto cam_from = current_camera_->get_owner()->get_location();
                const auto cam_to = current_camera_->get_owner()->get_location() + current_camera_->get_owner()->get_rotation().forward();

                soloud_->set3dListenerPosition(cam_from.x, cam_from.y, cam_from.z);
                soloud_->set3dListenerAt(cam_to.x, cam_to.y, cam_to.z);

                mouse_delta_ = Vector2::zero();
                ogre_app_->getRoot()->renderOneFrame();
            }
        }
    }

    close_world();
}

void Game::unloading_stage()
{
    verbose("Game", "Unloading stage...");
    stage_ = GameStage::Unloading;

    for (const auto& table_entry : tables_)
    {
        table_entry.value->clear();
    }

    on_unloading_stage();

    meshes_.clear();

    soloud_->stopAll();

    for (auto& audio : audios_)
    {
        audio.value->sample_.reset();
    }

    for (auto& audio_channel : audio_channels_)
    {
        audio_channel->bus_.reset();
    }

    soloud_->deinit();
}

void Game::search_table_files(const Path& path)
{
    if (path.exists())
    {
        const auto contents = path.list();
        for (const auto& sub_path : contents)
        {
            if (sub_path.get_type() == EPathType::Directory)
            {
                search_table_files(sub_path);
            }
            else if (sub_path.get_type() == EPathType::Regular)
            {
                if (sub_path.extension == ".db" || sub_path.extension == ".bdb")
                {
                    load_tables(sub_path);
                }
            }
        }
    }
}

void Game::load_tables(const Path& path)
{
    Compound::Object import_data;
    bool valid = false;
    if (path.extension == ".db")
    {
        valid = Compound::Convert::try_load_object_from_file(path, import_data, "json");
    }
    else if (path.extension == ".bdb")
    {
        valid = Compound::Convert::try_load_object_from_file(path, import_data, "cmp");
    }

    if (!valid) return;

    for (const auto& table_data : import_data)
    {
        if (const auto table = tables_.find_or_default(Name(table_data->key)))
        {
            if (table_data->value.get_type() == Compound::Type::Object)
            {
                table->add_record_compounds(shared_from_this(), (Compound::Object)table_data->value, false);
            }
        }
        else
        {
            print_error("Database", "Table named %s is not registered: %S", table_data->key.c(), path.get_absolute_string().c());
        }
    }

    // std::ofstream of((path.get_absolute_string() + ".bdb").c(), std::ios::out |
    // std::ios::binary);
    //
    // StreamUtils::write(of, import_data);
    //
    // of.close();
}

bool Game::keyPressed(KeyCode key, bool repeat)
{
    if (instance_)
    {
        if (repeat)
        {
            /*if (instance_->ui_input_element_)
            {
                instance_->ui_input_element_->key_hold(key);
            }*/
        }
        else
        {
            if (key == KeyCode::Escape)
            {
                quit();
            }
            /*else if (instance_->ui_input_element_)
            {
                instance_->ui_input_element_->key_down(key);
            }*/
            else if (instance_->current_controllable_)
            {
                instance_->current_controllable_->key_down(key);
            }
        }
    }

    return true;
}

bool Game::keyReleased(KeyCode key)
{
    if (instance_)
    {
        /*if (instance_->ui_input_element_)
        {
            instance_->ui_input_element_->key_up(key);
        }
        else */if (instance_->current_controllable_)
        {
            instance_->current_controllable_->key_up(key);
        }
    }

    return true;
}

bool Game::textInput(const char* chars)
{
    /*if (instance_ && instance_->ui_input_element_)
    {
        instance_->ui_input_element_->text_input(chars[0]);
    }*/

    return true;
}

bool Game::mousePressed(int button)
{
    if (instance_ && instance_->current_controllable_)
    {
        /*if (auto ui_under_mouse = instance_->ui_under_mouse_.lock())
        {
            if (instance_->ui_input_element_ && instance_->ui_input_element_ != ui_under_mouse)
            {
                instance_->ui_input_element_->on_unfocus();
                instance_->ui_input_element_ = nullptr;
            }

            auto inv_mat = ui_under_mouse->get_ui_matrix().inverse();
            auto q1 = inv_mat * ui_under_mouse->get_position();
            auto q2 = inv_mat *
                      (ui_under_mouse->get_position() + ui_under_mouse->get_size());
            auto q = inv_mat * instance_->mouse_pos_ / instance_->ui_scale_;

            const auto rel_pos_aspect = (q - q1) / (q2 - q1);
            const auto rel_pos = rel_pos_aspect * ui_under_mouse->get_size();

            ui_under_mouse->on_press(rel_pos);
            ui_under_mouse->is_pressed_ = true;
            instance_->pressed_ui_ = ui_under_mouse;
        }
        else*/
        {
            /*if (instance_->ui_input_element_)
            {
                instance_->ui_input_element_->on_unfocus();
                instance_->ui_input_element_ = nullptr;
            }*/

            instance_->current_controllable_->mouse_button_down(button);
        }
    }

    return true;
}

bool Game::mouseReleased(int button)
{
    if (instance_->current_controllable_)
    {
        /*if (auto released_ui = instance_->pressed_ui_.lock())
        {
            released_ui->on_release();
            released_ui->is_pressed_ = false;
            instance_->pressed_ui_.reset();
        }
        else*/
        {
            instance_->current_controllable_->mouse_button_up(button);
        }
    }

    return true;
}

bool Game::axisMoved(int axis, float value)
{
    return false;
}

bool Game::mouseMoved(const Vector2& new_pos, const Vector2& delta)
{
    instance_->mouse_pos_ = new_pos;

    mouse_delta_ += delta;

    /*Shared<UIElement> ui_under_mouse;
    float pressed_z = 0.0f;
    instance_->ui_root_->detect_topmost_under_mouse(instance_->mouse_pos_, 0.0f, ui_under_mouse, pressed_z);

    auto current_ui_under_mouse = instance_->ui_under_mouse_.lock();

    if (current_ui_under_mouse != ui_under_mouse)
    {
        if (current_ui_under_mouse)
        {
            current_ui_under_mouse->on_mouse_leave();
            current_ui_under_mouse->is_mouse_over_ = false;
        }
        if (ui_under_mouse)
        {
            ui_under_mouse->on_mouse_enter();
            ui_under_mouse->is_mouse_over_ = true;
        }

        instance_->ui_under_mouse_ = ui_under_mouse;
    }*/

    return true;
}

bool Game::wheelRolled(float y)
{
    if (/*instance_->ui_input_element_ == nullptr && */instance_->current_controllable_)
    {
        instance_->current_controllable_->scroll(y);
    }

    return true;
}

void Game::windowResized(Ogre::RenderWindow* rw)
{
    //instance_->ui_root_->set_size(Vector2(static_cast<float>(rw->getWidth()) / get_ui_scale(), static_cast<float>(rw->getHeight()) / get_ui_scale()));
}
