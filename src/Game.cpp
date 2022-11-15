#include "hexa_engine/Game.h"

#include "hexa_engine/AudioChannel.h"
#include "hexa_engine/CameraComponent.h"
#include "hexa_engine/Material.h"
#include "hexa_engine/Mod.h"
#include "hexa_engine/OgreApp.h"
#include "hexa_engine/Paths.h"
#include "hexa_engine/SaveGame.h"
#include "hexa_engine/Settings.h"
#include "hexa_engine/StaticMesh.h"
#include "hexa_engine/TableBase.h"
#include "hexa_engine/World.h"

#include <Bites/OgreTrays.h>
#include <OgreCompositorManager.h>
#include <OgreEntity.h>
#include <OgreGpuProgramManager.h>
#include <OgreRenderWindow.h>
#include <OgreTechnique.h>
#include <OgreTextureManager.h>
#include <RTShaderSystem/OgreShaderGenerator.h>
#include <base_lib/File.h>
#include <base_lib/Logger.h>
#include <base_lib/Path.h>
#include <base_lib/Set.h>
#include <base_lib/performance.h>
#include <base_lib/stb.h>
#include <hexa_engine/Audio.h>
#include <hexa_engine/IControllable.h>
#include <reactphysics3d/reactphysics3d.h>
#include <soloud/soloud.h>

FORCEINLINE Name name_instancing(const Name& name)
{
    return Name(name.to_string() + "_instancing");
}

Game::Game(const String& name, int argc, char* argv[])
    : app_path_(argv[0])
    , Module(Name(name))
    , event_bus_(new EventBus())
    , physics_(new reactphysics3d::PhysicsCommon)
    , soloud_(new SoLoud::Soloud)
    , ogre_app_(new OgreApp(name))
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

    Logger::init(argv[0]);

    ogre_app_->on_setup = std::bind(&Game::setup, this);

    ogre_app_->on_keyPressed = std::bind(
        &Game::keyPressed, this, std::placeholders::_1, std::placeholders::_2);
    ogre_app_->on_keyReleased =
        std::bind(&Game::keyReleased, this, std::placeholders::_1);
    ogre_app_->on_textInput =
        std::bind(&Game::textInput, this, std::placeholders::_1);
    ogre_app_->on_mousePressed =
        std::bind(&Game::mousePressed, this, std::placeholders::_1);
    ogre_app_->on_mouseReleased =
        std::bind(&Game::mouseReleased, this, std::placeholders::_1);
    ogre_app_->on_axisMoved = std::bind(
        &Game::axisMoved, this, std::placeholders::_1, std::placeholders::_2);
    ogre_app_->on_mouseMoved = std::bind(
        &Game::mouseMoved, this, std::placeholders::_1, std::placeholders::_2);
    ogre_app_->on_wheelRolled =
        std::bind(&Game::wheelRolled, this, std::placeholders::_1);
    ogre_app_->on_windowResized =
        std::bind(&Game::windowResized, this, std::placeholders::_1);
}

Path Game::get_module_path(const String& sub_path) const
{
    return "./" + sub_path;
}

void Game::launch()
{
    if (handle_tool())
        return;

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

void Game::focus_ui(const Shared<UIInputElement>& ui_input_reciever)
{
    if (instance_->ui_input_element_)
    {
        instance_->ui_input_element_->on_unfocus();
    }

    instance_->ui_input_element_ = ui_input_reciever;

    if (instance_->ui_input_element_)
    {
        instance_->ui_input_element_->on_focus();
    }
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
        instance_->viewport_ = instance_->ogre_app_->getRenderWindow()->addViewport(
            camera->ogre_camera_);
        // Ogre::CompositorManager::getSingleton().addCompositor(instance_->viewport_,
        // "Hexa/Flip_X_comp");
        // Ogre::CompositorManager::getSingleton().setCompositorEnabled(instance_->viewport_,
        // "Hexa/Flip_X_comp", true);

        Ogre::CompositorManager::getSingleton().addCompositor(
            instance_->viewport_, "DeferredShading/GBuffer");
        Ogre::CompositorManager::getSingleton().setCompositorEnabled(
            instance_->viewport_, "DeferredShading/GBuffer", true);

        Ogre::CompositorManager::getSingleton().addCompositor(
            instance_->viewport_, "DeferredShading/ShowLit");
        Ogre::CompositorManager::getSingleton().setCompositorEnabled(
            instance_->viewport_, "DeferredShading/ShowLit", true);
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
        instance_->world_->manager_->addRenderQueueListener(
            &Ogre::OverlaySystem::getSingleton());
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

        instance_->world_->manager_->removeRenderQueueListener(
            &Ogre::OverlaySystem::getSingleton());
        instance_->shader_generator_->removeSceneManager(
            instance_->world_->manager_);
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

const Path& Game::get_app_path()
{
    return instance_->app_path_;
}

Shared<Texture> Game::load_texture(const ModuleAssetID& id)
{
    if (!id.is_valid())
    {
        print_error("Texture", "Invalid id %s, unable to load texture",
                    id.to_string().c());
        return nullptr;
    }

    if (const auto slot = instance_->textures_.find_or_default(id))
    {
        return slot;
    }

    if (get_stage() <= GameStage::Loading)
        print_warning("Texture",
                      "Texture loading outside of loading stage is not "
                      "recommended! Texture id: %s",
                      id.to_string().c());

    const auto path = id.evaluate_asset_path().with_extension(".png");

    if (const auto result =
            load_texture_path(path, id.asset_name, id.module_name))
    {
        result->id_ = id;
        instance_->textures_[id] = result;

        return result;
    }

    return nullptr;
}

Shared<Texture> Game::create_texture(const Array2D<Color>& pixels,
                                     const ModuleAssetID& id)
{
    if (!id.is_valid())
    {
        print_error("Texture", "Invalid id %s, unable to create texture",
                    id.to_string().c());
        return nullptr;
    }

    Ogre::Image ogre_image(Ogre::PF_R5G6B5, pixels.get_size_x(),
                           pixels.get_size_y());

    for (uint x = 0; x < pixels.get_size_x(); x++)
    {
        for (uint y = 0; y < pixels.get_size_y(); y++)
        {
            ogre_image.setColourAt(Ogre::ColourValue((byte*)&pixels.at(x, y)), x, y,
                                   0);
        }
    }

    Shared<Texture> result;

    if (result = instance_->textures_.find_or_default(id))
    {
        result->ogre_texture_->loadImage(ogre_image);

        print_warning("Texture", "Texture %s was rewritten manually",
                      id.to_string().c());
    }
    else
    {
        result = MakeShared<Texture>();
        result->ogre_texture_ = Ogre::TextureManager::getSingleton().loadImage(
            id.asset_name.c(), id.module_name.c(), ogre_image);
        result->id_ = id;

        instance_->textures_[id] = result;

        verbose("Texture", "Texture %s was created manually", id.to_string().c());
    }

    if (get_stage() <= GameStage::Loading)
        print_warning("Texture",
                      "Texture creation outside of loading stage is not "
                      "recommended! Texture path: %s",
                      id.to_string().c());

    return result;
}

Shared<Texture> Game::get_texture(const ModuleAssetID& id)
{
    return instance_->textures_.find_or_default(id);
}

Shared<Material> Game::load_material(const ModuleAssetID& id)
{
    if (!CheckError(id.is_valid(), "Material",
                    "Invalid id %s, unable to load material", id.to_string().c()))
        return nullptr;

    if (const auto slot = instance_->materials_.find_or_default(id))
        return slot;

    Check(get_stage() <= GameStage::Loading, "Material",
          "material loading outside of loading stage is not recommended! "
          "Material id: %s",
          id.to_string().c());

    const auto path = id.evaluate_asset_path().with_extension(".mat");

    if (!CheckError(path.exists(), "Texture",
                    "Failed to load texture %s, file does not exist",
                    id.to_string().c()))
        return nullptr;

    Compound::Object material_json;
    if (!CheckError(Compound::Converters::JSON().try_parse_value(
                        File::read_file(path), material_json),
                    "Material", "Failed to read material %s info",
                    id.to_string().c()))
        return nullptr;

    const auto vertex_program_name =
        ModuleAssetID(material_json.get_string("vertex"), id.module_name);
    const auto fragment_program_name =
        ModuleAssetID(material_json.get_string("fragment"), id.module_name);

    if (!CheckError(
            vertex_program_name.is_valid(), "Material",
            "Unable to load material %s because vertex program is not specified",
            id.to_string().c()))
        return nullptr;
    if (!CheckError(fragment_program_name.is_valid(), "Material",
                    "Unable to load material %s because fragment program is not "
                    "specified",
                    id.to_string().c()))
        return nullptr;

    bool vertex_instancing;
    bool fragment_instancing;

    if (!CheckError(
            load_shader_program(vertex_program_name, vertex_instancing),
            "Material",
            "Unable to load material %s because of vertex shader loading fail"))
        return nullptr;
    if (!CheckError(
            load_shader_program(fragment_program_name, fragment_instancing),
            "Material",
            "Unable to load material %s because of fragment shader loading fail"))
        return nullptr;

    Shared<Ogre::Material> ogre_material = nullptr;
    Shared<Ogre::Material> ogre_material_instancing = nullptr;

    for (uint i = 0; i < (vertex_instancing || fragment_instancing ? 2 : 1);
         i++)
    {
        const bool inst_pass = i == 1;

        auto& mat = inst_pass ? ogre_material_instancing : ogre_material;
        mat = Ogre::MaterialManager::getSingleton().create(
            inst_pass ? name_instancing(id.asset_name).c() : id.asset_name.c(),
            id.module_name.c());
        const auto technique = mat->createTechnique();
        const auto pass = technique->createPass();

        // TODO: make ogre accept shaders from another modules
        pass->setVertexProgram(
            vertex_instancing && inst_pass
                ? name_instancing(vertex_program_name.asset_name).c()
                : vertex_program_name.asset_name.c());
        pass->setFragmentProgram(
            fragment_instancing && inst_pass
                ? name_instancing(fragment_program_name.asset_name).c()
                : fragment_program_name.asset_name.c());

        for (const auto& texture : material_json.get_array("textures"))
        {
            if (texture.get_type() == Compound::Type::Object)
            {
                const auto texture_data = texture.get_object();

                auto unit_state = pass->createTextureUnitState();

                if (const auto param_default = texture_data.find("default"))
                {
                    if (const auto default_texture = load_texture(
                            ModuleAssetID(param_default->get_string(), id.module_name)))
                    {
                        unit_state->setTexture(default_texture->ogre_texture_);
                    }
                }

                if (const auto param_filtering = texture_data.find("filtering"))
                {
                    const static Map<String, Ogre::TextureFilterOptions> filters = {
                        {"", Ogre::TextureFilterOptions::TFO_NONE},
                        {"bilinear", Ogre::TextureFilterOptions::TFO_BILINEAR},
                        {"trilinear", Ogre::TextureFilterOptions::TFO_TRILINEAR},
                        {"anisotropic", Ogre::TextureFilterOptions::TFO_ANISOTROPIC}};
                    const auto filter =
                        filters.find_or_default(param_filtering->get_string(),
                                                Ogre::TextureFilterOptions::TFO_NONE);
                    unit_state->setTextureFiltering(filter);
                }
            }
        }

        mat->load();
    }

    Shared<Material> result = MakeShared<Material>();
    result->ogre_material_ = ogre_material;
    result->ogre_material_instancing_ = ogre_material_instancing;
    result->id_ = id;

    instance_->materials_[id] = result;

    return result;
}

Shared<Material> Game::clone_material(const Shared<Material>& material,
                                      const ModuleAssetID& new_id)
{
    if (material == nullptr)
        return nullptr;

    const ModuleAssetID actual_new_id = ModuleAssetID(
        new_id.module_name.is_valid() ? new_id.module_name
                                      : material->get_id().module_name,
        new_id.asset_name.is_valid()
            ? new_id.asset_name
            : Name(material->get_id().asset_name.to_string() + "_clone"));
    auto result = MakeShared<Material>();
    result->ogre_material_ = material->ogre_material_->clone(
        Ogre::String(actual_new_id.asset_name.c()),
        Ogre::String(actual_new_id.module_name.c()));
    result->id_ = actual_new_id;

    instance_->materials_[actual_new_id] = result;

    return result;
}

Shared<Material> Game::get_material(const ModuleAssetID& id)
{
    return instance_->materials_.find_or_default(id);
}

const Shared<Material>& Game::get_basic_material(bool instanced)
{
    // TODO: Work later
    assert(false);
    return nullptr;
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

const Shared<Module>& Game::get_module_by_name(const Name& module_name)
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

void Game::set_cursor_texture(const Shared<Texture>& tex, uint hotspot_x,
                              uint hotspot_y)
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
    instance_->ui_root_->add_child(ui);
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

    Ogre::ResourceGroupManager::getSingletonPtr()->addResourceLocation(
        Path("resources/SdkTrays.zip").get_absolute_string().c(), "Zip",
        module_name.c());

    Ogre::MaterialManager::getSingleton().addListener(new GBufferSchemeHandler,
                                                      "GBuffer");
    Ogre::CompositorManager::getSingleton().registerCustomCompositionPass(
        "DeferredLight", new DeferredLightCompositionPass());
}

bool Game::handle_tool()
{
    const List<String>& args = get_args();

    if (args.length() >= 2 && args[1] == "comp")
    {
        if (args.length() >= 3)
        {
            const String converter_name = args[2];
            if (args.length() >= 4)
            {
                const Path src = args[3];
                const Path out = (args.length() >= 5)
                                     ? args[4]
                                     : src.parent + "/" + src.filename + ".comp";

                if (src.exists())
                {
                    Shared<Compound::Converter> converter = nullptr;

                    if (converter_name == "json")
                        converter = MakeShared<Compound::Converters::JSON>();
                    // else if (converter_name == "yaml") converter =
                    // MakeShared<Compound::Converters::YAML>(); else if (converter_name
                    // == "xml") converter = MakeShared<Compound::Converters::XML>();

                    if (converter)
                    {
                        Compound::Object json;
                        if (converter->try_parse_value(File::read_file(src), json))
                        {
                            auto os = std::ofstream(out.get_absolute_string().c(),
                                                    std::ios::binary);
                            StreamUtils::write<Compound::Object>(os, json);
                            os.close();

                            verbose("comp", "Conversion done!");
                        }
                        else
                            print_error("comp",
                                        "Provided file contains broken compound value");
                    }
                    else
                        print_error("comp", "Converter %s is not a valid converter",
                                    converter_name.c());
                }
                else
                    print_error("comp", "Unable to read from provided source file");
            }
            else
                print_error("comp", "Source file path is required");
        }
        else
            print_error("comp", "Source format is required");

        return true;
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
    auto converter = Compound::Converters::JSON();
    converter.separate_with_new_line = true;
    const Path settings_path = "settings.json";
    Compound::Object settings_json;
    if (settings_path.exists())
    {
        settings_json = converter.parse_value(File::read_file(settings_path));
    }
    settings_->read_settings(settings_json);
    File::write_file(settings_path,
                     converter.format_value(settings_->write_settings()));

    soloud_->init();

    on_init();

    verbose("Mod Loader", "Searching for mods in %s/...",
            Path("mods").get_absolute_string().c());
    for (auto& path : Path("mods").list())
    {
        if (path.get_type() == EPathType::Directory &&
            Mod::verify_signature(path))
        {
            if (auto mod = Mod::load(path.get_child(path.filename + ".dll")))
            {
                mod->info_ =
                    mod->load_mod_info(path.get_child(path.filename + ".meta"));
                if (mod->info_.target_game_version != game_version_)
                {
                    print_error("Mod Loader",
                                "Mod %s target game version is %s, but current game "
                                "version is %s. This mod will be skipped",
                                mod->info_.display_name.c(),
                                mod->info_.target_game_version.to_string().c(),
                                game_version_.to_string().c());
                    continue;
                }
                mod->add_resource_directories();
                mods_.add(mod);
                verbose("Mod Loader", "Initialized mod %s",
                        mod->info_.display_name.c());
            }
            else
            {
                print_warning("Mod Loader", "Failed to initialize mod %s",
                              path.get_absolute_string().c());
            }
        }
    }

    search_table_files(get_module_path("resources"));
    for (auto mod : mods_)
    {
        search_table_files(mod->get_module_path("resources"));
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

        white_texture_ = create_texture(white, get_asset_id("white"));
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
                Color c = Color((byte)(1.0f / cc * (x + 0.5f) * 255),
                                (byte)(1.0f / cc * (y + 0.5f) * 255), 0);
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

        uv_test_texture_ = create_texture(uv_test, get_asset_id("uv_test"));
    }

    for (const auto& mod : mods_)
    {
        mod->register_resource_directories();
    }

    Ogre::ResourceGroupManager::getSingletonPtr()->initialiseResourceGroup(
        get_module_name().c());
    for (const auto& mod : mods_)
    {
        if (Ogre::ResourceGroupManager::getSingletonPtr()->resourceGroupExists(
                mod->get_module_name().c()))
        {
            Ogre::ResourceGroupManager::getSingletonPtr()->initialiseResourceGroup(
                mod->get_module_name().c());
        }
    }

    ogre_app_->load();

    // Fonts
    default_font_ = SpriteFont::load_fnt(RESOURCES_FONTS + "arial.fnt");

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

    float fps_delta_time_stack = 0;
    uint fps_count = 0;
    uint fps_last_count = 0;

    auto fps_display = MakeShared<TextBlock>();
    fps_display->set_z(10);
    add_ui(fps_display);

    const auto start_time = std::chrono::system_clock::now();

    auto tick_start = start_time;

    while (!ogre_app_->getRoot()->endRenderingQueued())
    {
        // while
        // (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()
        // - tick_start).count() / 1000.0f < 1.0f / settings_->fps_limit);
        const float tick_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now() - tick_start)
                .count() /
            1000.0f;
        tick_start = std::chrono::system_clock::now();

        time_ += tick_time;

        main_thread_calls_mutex_.lock();
        for (auto& func : main_thread_calls_)
        {
            func();
        }
        main_thread_calls_.clear();
        main_thread_calls_mutex_.unlock();

        fps_delta_time_stack += tick_time;
        fps_count++;
        if (fps_delta_time_stack >= 1)
        {
            fps_delta_time_stack -= 1;
            fps_last_count = fps_count;
            fps_count = 0;
        }

        fps_display->set_text(String::format("FPS: %i", fps_last_count));

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
                const auto cam_to =
                    current_camera_->get_owner()->get_location() +
                    current_camera_->get_owner()->get_rotation().forward();

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

    Texture::unload_all_static();

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
        valid = Compound::Converters::JSON().try_parse_value(File::read_file(path),
                                                             import_data);
    }
    else if (path.extension == ".bdb")
    {
        std::ifstream fin(path.get_absolute_string().c(),
                          std::ios::binary | std::ios::in);
        import_data = StreamUtils::read<Compound::Object>(fin);
        valid = true;
    }

    if (!valid)
        return;

    for (const auto& table_data : import_data)
    {
        if (const auto table = tables_.find_or_default(Name(table_data->key)))
        {
            if (table_data->value.get_type() == Compound::Type::Object)
            {
                table->add_record_compounds(shared_from_this(),
                                            (Compound::Object)table_data->value, false);
            }
        }
        else
        {
            print_error("Database",
                        "Table named %s was not registered (declared in file %s)",
                        table_data->key.c(), path.get_absolute_string().c());
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
            if (instance_->ui_input_element_)
            {
                instance_->ui_input_element_->key_hold(key);
            }
        }
        else
        {
            if (key == KeyCode::Escape)
            {
                ogre_app_->getRoot()->queueEndRendering();
            }
            else if (instance_->ui_input_element_)
            {
                instance_->ui_input_element_->key_down(key);
            }
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
        if (instance_->ui_input_element_)
        {
            instance_->ui_input_element_->key_up(key);
        }
        else if (instance_->current_controllable_)
        {
            instance_->current_controllable_->key_up(key);
        }
    }

    return true;
}

bool Game::textInput(const char* chars)
{
    if (instance_ && instance_->ui_input_element_)
    {
        instance_->ui_input_element_->text_input(chars[0]);
    }

    return true;
}

bool Game::mousePressed(int button)
{
    if (instance_ && instance_->current_controllable_)
    {
        if (auto ui_under_mouse = instance_->ui_under_mouse_.lock())
        {
            if (instance_->ui_input_element_ &&
                instance_->ui_input_element_ != ui_under_mouse)
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
        else
        {
            if (instance_->ui_input_element_)
            {
                instance_->ui_input_element_->on_unfocus();
                instance_->ui_input_element_ = nullptr;
            }

            instance_->current_controllable_->mouse_button_down(button);
        }
    }

    return true;
}

bool Game::mouseReleased(int button)
{
    if (instance_->current_controllable_)
    {
        if (auto released_ui = instance_->pressed_ui_.lock())
        {
            released_ui->on_release();
            released_ui->is_pressed_ = false;
            instance_->pressed_ui_.reset();
        }
        else
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

    Shared<UIElement> ui_under_mouse;
    float pressed_z = 0.0f;
    instance_->ui_root_->detect_topmost_under_mouse(instance_->mouse_pos_, 0.0f,
                                                    ui_under_mouse, pressed_z);

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
    }

    return true;
}

bool Game::wheelRolled(float y)
{
    if (instance_->ui_input_element_ == nullptr &&
        instance_->current_controllable_)
    {
        instance_->current_controllable_->scroll(y);
    }

    return true;
}

void Game::windowResized(Ogre::RenderWindow* rw)
{
    instance_->ui_root_->set_size(
        Vector2(static_cast<float>(rw->getWidth()) / get_ui_scale(),
                static_cast<float>(rw->getHeight()) / get_ui_scale()));
}

Shared<Texture> Game::load_texture_path(const Path& path, const Name& name,
                                        const Name& module_name)
{
    const Shared<Ogre::FileStreamDataStream> stream =
        MakeShared<Ogre::FileStreamDataStream>(
            new std::ifstream(path.get_absolute_string().c()));
    Ogre::Image image;
    image.load(stream, path.extension.substring(1).c());
    const auto ogre_texture = Ogre::TextureManager::getSingleton().loadImage(
        name.c(), module_name.c(), image);

    Shared<Texture> result = MakeShared<Texture>();
    result->ogre_texture_ = ogre_texture;
    return result;

    /*uint tex_width, tex_height;
    const auto pixels = stb::load(path, tex_width, tex_height);
    if (pixels.length() > 0)
    {
            if (tex_width * tex_height > 0)
            {
                    Ogre::Image ogre_image(Ogre::PF_R5G6B5, tex_width,
    tex_height);

                    for (uint x = 0; x < tex_width; x++)
                    {
                            for (uint y = 0; y < tex_height; y++)
                            {
                                    ogre_image.setColourAt(Ogre::ColourValue((byte*)&pixels[y
    * tex_width + x]), x, y, 0);
                            }
                    }


                    auto result = MakeShared<Texture>();
                    result->ogre_texture_ =
    Ogre::TextureManager::getSingleton().load result->pixels_ = Array2D(tex_width,
    tex_height, pixels);

                    verbose("Texture", "Loaded texture %ix%i %s", tex_width,
    tex_height, path.get_absolute_string().c());

                    return result;
            }
            else
            {
                    print_error("Texture", "Texture size is invalid: %ix%i %s",
    tex_width, tex_height, path.get_absolute_string().c());
            }
    }
    else
    {
            print_error("Texture", "Unknown error on loading texture %s",
    path.get_absolute_string().c());
    }
    return nullptr;*/
}

bool Game::load_shader_program(const ModuleAssetID& id, bool& instancing)
{
    if (Ogre::GpuProgramManager::getSingleton().getResourceByName(
            id.asset_name.c(), id.module_name.c()))
        return true;

    const auto definition_path = id.evaluate_asset_path().with_extension(".pro");

    if (definition_path.exists())
    {
        Compound::Object program_json;
        if (Compound::Converters::JSON().try_parse_value(
                File::read_file(definition_path), program_json))
        {
            const auto source_name =
                ModuleAssetID(program_json.get_string("source"), id.module_name);
            if (source_name.is_valid())
            {
                const auto source_path =
                    source_name.evaluate_asset_path().with_extension(".hlsl");
                if (source_path.exists())
                {
                    const static List<String> program_type_names = {"vertex", "fragment"};
                    const int program_type_index =
                        program_type_names.index_of(program_json.get_string("type"));
                    if (program_type_index > -1)
                    {
                        const static List<String> entry_points = {"vert", "frag"};
                        const static List<String> syntax_codes = {"vs_2_0", "ps_2_0"};
                        const static Map<String,
                                         Ogre::GpuProgramParameters::AutoConstantType>
                            param_types = {
                                {"world_matrix", Ogre::GpuProgramParameters::
                                                     AutoConstantType::ACT_WORLD_MATRIX},
                                {"inverse_world_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_INVERSE_WORLD_MATRIX},
                                {"transpose_world_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_TRANSPOSE_WORLD_MATRIX},
                                {"inverse_transpose_world_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_INVERSE_TRANSPOSE_WORLD_MATRIX},
                                {"world_matrix_array_3x4",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_WORLD_MATRIX_ARRAY_3x4},
                                {"world_matrix_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_WORLD_MATRIX_ARRAY},
                                {"world_dualquaternion_array_2x4",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_WORLD_DUALQUATERNION_ARRAY_2x4},
                                {"world_scale_shear_matrix_array_3x4",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_WORLD_SCALE_SHEAR_MATRIX_ARRAY_3x4},
                                {"view_matrix", Ogre::GpuProgramParameters::
                                                    AutoConstantType::ACT_VIEW_MATRIX},
                                {"inverse_view_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_INVERSE_VIEW_MATRIX},
                                {"transpose_view_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_TRANSPOSE_VIEW_MATRIX},
                                {"inverse_transpose_view_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_INVERSE_TRANSPOSE_VIEW_MATRIX},
                                {"projection_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_PROJECTION_MATRIX},
                                {"inverse_projection_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_INVERSE_PROJECTION_MATRIX},
                                {"transpose_projection_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_TRANSPOSE_PROJECTION_MATRIX},
                                {"inverse_transpose_projection_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_INVERSE_TRANSPOSE_PROJECTION_MATRIX},
                                {"viewproj_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_VIEWPROJ_MATRIX},
                                {"inverse_viewproj_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_INVERSE_VIEWPROJ_MATRIX},
                                {"transpose_viewproj_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_TRANSPOSE_VIEWPROJ_MATRIX},
                                {"inverse_transpose_viewproj_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_INVERSE_TRANSPOSE_VIEWPROJ_MATRIX},
                                {"worldview_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_WORLDVIEW_MATRIX},
                                {"inverse_worldview_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_INVERSE_WORLDVIEW_MATRIX},
                                {"transpose_worldview_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_TRANSPOSE_WORLDVIEW_MATRIX},
                                {"inverse_transpose_worldview_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX},
                                {"normal_matrix", Ogre::GpuProgramParameters::
                                                      AutoConstantType::ACT_NORMAL_MATRIX},
                                {"worldviewproj_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_WORLDVIEWPROJ_MATRIX},
                                {"inverse_worldviewproj_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_INVERSE_WORLDVIEWPROJ_MATRIX},
                                {"transpose_worldviewproj_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_TRANSPOSE_WORLDVIEWPROJ_MATRIX},
                                {"inverse_transpose_worldviewproj_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_INVERSE_TRANSPOSE_WORLDVIEWPROJ_MATRIX},
                                {"render_target_flipping",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_RENDER_TARGET_FLIPPING},
                                {"vertex_winding",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_VERTEX_WINDING},
                                {"fog_colour", Ogre::GpuProgramParameters::
                                                   AutoConstantType::ACT_FOG_COLOUR},
                                {"fog_params", Ogre::GpuProgramParameters::
                                                   AutoConstantType::ACT_FOG_PARAMS},
                                {"surface_ambient_colour",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_SURFACE_AMBIENT_COLOUR},
                                {"surface_diffuse_colour",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_SURFACE_DIFFUSE_COLOUR},
                                {"surface_specular_colour",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_SURFACE_SPECULAR_COLOUR},
                                {"surface_emissive_colour",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_SURFACE_EMISSIVE_COLOUR},
                                {"surface_shininess",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_SURFACE_SHININESS},
                                {"surface_alpha_rejection_value",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_SURFACE_ALPHA_REJECTION_VALUE},
                                {"light_count", Ogre::GpuProgramParameters::
                                                    AutoConstantType::ACT_LIGHT_COUNT},
                                {"ambient_light_colour",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_AMBIENT_LIGHT_COLOUR},
                                {"light_diffuse_colour",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_DIFFUSE_COLOUR},
                                {"light_specular_colour",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_SPECULAR_COLOUR},
                                {"light_attenuation",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_ATTENUATION},
                                {"spotlight_params",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_SPOTLIGHT_PARAMS},
                                {"light_position",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_POSITION},
                                {"light_position_object_space",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_POSITION_OBJECT_SPACE},
                                {"light_position_view_space",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_POSITION_VIEW_SPACE},
                                {"light_direction",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_DIRECTION},
                                {"light_direction_object_space",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_DIRECTION_OBJECT_SPACE},
                                {"light_direction_view_space",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_DIRECTION_VIEW_SPACE},
                                {"light_distance_object_space",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_DISTANCE_OBJECT_SPACE},
                                {"light_power_scale",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_POWER_SCALE},
                                {"light_diffuse_colour_power_scaled",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED},
                                {"light_specular_colour_power_scaled",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED},
                                {"light_diffuse_colour_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_DIFFUSE_COLOUR_ARRAY},
                                {"light_specular_colour_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_SPECULAR_COLOUR_ARRAY},
                                {"light_diffuse_colour_power_scaled_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED_ARRAY},
                                {"light_specular_colour_power_scaled_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED_ARRAY},
                                {"light_attenuation_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_ATTENUATION_ARRAY},
                                {"light_position_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_POSITION_ARRAY},
                                {"light_position_object_space_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_POSITION_OBJECT_SPACE_ARRAY},
                                {"light_position_view_space_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_POSITION_VIEW_SPACE_ARRAY},
                                {"light_direction_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_DIRECTION_ARRAY},
                                {"light_direction_object_space_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_DIRECTION_OBJECT_SPACE_ARRAY},
                                {"light_direction_view_space_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_DIRECTION_VIEW_SPACE_ARRAY},
                                {"light_distance_object_space_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_DISTANCE_OBJECT_SPACE_ARRAY},
                                {"light_power_scale_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_POWER_SCALE_ARRAY},
                                {"spotlight_params_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_SPOTLIGHT_PARAMS_ARRAY},
                                {"derived_ambient_light_colour",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_DERIVED_AMBIENT_LIGHT_COLOUR},
                                {"derived_scene_colour",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_DERIVED_SCENE_COLOUR},
                                {"derived_light_diffuse_colour",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_DERIVED_LIGHT_DIFFUSE_COLOUR},
                                {"derived_light_specular_colour",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_DERIVED_LIGHT_SPECULAR_COLOUR},
                                {"derived_light_diffuse_colour_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_DERIVED_LIGHT_DIFFUSE_COLOUR_ARRAY},
                                {"derived_light_specular_colour_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_DERIVED_LIGHT_SPECULAR_COLOUR_ARRAY},
                                {"light_number", Ogre::GpuProgramParameters::
                                                     AutoConstantType::ACT_LIGHT_NUMBER},
                                {"light_casts_shadows",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_CASTS_SHADOWS},
                                {"light_casts_shadows_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LIGHT_CASTS_SHADOWS_ARRAY},
                                {"shadow_extrusion_distance",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_SHADOW_EXTRUSION_DISTANCE},
                                {"camera_position",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_CAMERA_POSITION},
                                {"camera_position_object_space",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_CAMERA_POSITION_OBJECT_SPACE},
                                {"camera_relative_position",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_CAMERA_RELATIVE_POSITION},
                                {"texture_viewproj_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_TEXTURE_VIEWPROJ_MATRIX},
                                {"texture_viewproj_matrix_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_TEXTURE_VIEWPROJ_MATRIX_ARRAY},
                                {"texture_worldviewproj_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_TEXTURE_WORLDVIEWPROJ_MATRIX},
                                {"texture_worldviewproj_matrix_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_TEXTURE_WORLDVIEWPROJ_MATRIX_ARRAY},
                                {"spotlight_viewproj_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_SPOTLIGHT_VIEWPROJ_MATRIX},
                                {"spotlight_viewproj_matrix_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_SPOTLIGHT_VIEWPROJ_MATRIX_ARRAY},
                                {"spotlight_worldviewproj_matrix_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX_ARRAY},
                                {"custom",
                                 Ogre::GpuProgramParameters::AutoConstantType::ACT_CUSTOM},
                                {"time",
                                 Ogre::GpuProgramParameters::AutoConstantType::ACT_TIME},
                                {"time_0_x", Ogre::GpuProgramParameters::AutoConstantType::
                                                 ACT_TIME_0_X},
                                {"costime_0_x", Ogre::GpuProgramParameters::
                                                    AutoConstantType::ACT_COSTIME_0_X},
                                {"sintime_0_x", Ogre::GpuProgramParameters::
                                                    AutoConstantType::ACT_SINTIME_0_X},
                                {"tantime_0_x", Ogre::GpuProgramParameters::
                                                    AutoConstantType::ACT_TANTIME_0_X},
                                {"time_0_x_packed",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_TIME_0_X_PACKED},
                                {"time_0_1", Ogre::GpuProgramParameters::AutoConstantType::
                                                 ACT_TIME_0_1},
                                {"costime_0_1", Ogre::GpuProgramParameters::
                                                    AutoConstantType::ACT_COSTIME_0_1},
                                {"sintime_0_1", Ogre::GpuProgramParameters::
                                                    AutoConstantType::ACT_SINTIME_0_1},
                                {"tantime_0_1", Ogre::GpuProgramParameters::
                                                    AutoConstantType::ACT_TANTIME_0_1},
                                {"time_0_1_packed",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_TIME_0_1_PACKED},
                                {"time_0_2pi", Ogre::GpuProgramParameters::
                                                   AutoConstantType::ACT_TIME_0_2PI},
                                {"costime_0_2pi", Ogre::GpuProgramParameters::
                                                      AutoConstantType::ACT_COSTIME_0_2PI},
                                {"sintime_0_2pi", Ogre::GpuProgramParameters::
                                                      AutoConstantType::ACT_SINTIME_0_2PI},
                                {"tantime_0_2pi", Ogre::GpuProgramParameters::
                                                      AutoConstantType::ACT_TANTIME_0_2PI},
                                {"time_0_2pi_packed",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_TIME_0_2PI_PACKED},
                                {"frame_time", Ogre::GpuProgramParameters::
                                                   AutoConstantType::ACT_FRAME_TIME},
                                {"fps",
                                 Ogre::GpuProgramParameters::AutoConstantType::ACT_FPS},
                                {"viewport_width",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_VIEWPORT_WIDTH},
                                {"viewport_height",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_VIEWPORT_HEIGHT},
                                {"inverse_viewport_width",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_INVERSE_VIEWPORT_WIDTH},
                                {"inverse_viewport_height",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_INVERSE_VIEWPORT_HEIGHT},
                                {"viewport_size", Ogre::GpuProgramParameters::
                                                      AutoConstantType::ACT_VIEWPORT_SIZE},
                                {"view_direction",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_VIEW_DIRECTION},
                                {"view_side_vector",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_VIEW_SIDE_VECTOR},
                                {"view_up_vector",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_VIEW_UP_VECTOR},
                                {"fov",
                                 Ogre::GpuProgramParameters::AutoConstantType::ACT_FOV},
                                {"near_clip_distance",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_NEAR_CLIP_DISTANCE},
                                {"far_clip_distance",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_FAR_CLIP_DISTANCE},
                                {"pass_number", Ogre::GpuProgramParameters::
                                                    AutoConstantType::ACT_PASS_NUMBER},
                                {"pass_iteration_number",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_PASS_ITERATION_NUMBER},
                                {"animation_parametric",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_ANIMATION_PARAMETRIC},
                                {"texel_offsets", Ogre::GpuProgramParameters::
                                                      AutoConstantType::ACT_TEXEL_OFFSETS},
                                {"scene_depth_range",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_SCENE_DEPTH_RANGE},
                                {"shadow_scene_depth_range",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_SHADOW_SCENE_DEPTH_RANGE},
                                {"shadow_scene_depth_range_array",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_SHADOW_SCENE_DEPTH_RANGE_ARRAY},
                                {"shadow_colour", Ogre::GpuProgramParameters::
                                                      AutoConstantType::ACT_SHADOW_COLOUR},
                                {"texture_size", Ogre::GpuProgramParameters::
                                                     AutoConstantType::ACT_TEXTURE_SIZE},
                                {"inverse_texture_size",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_INVERSE_TEXTURE_SIZE},
                                {"packed_texture_size",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_PACKED_TEXTURE_SIZE},
                                {"texture_matrix",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_TEXTURE_MATRIX},
                                {"lod_camera_position",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LOD_CAMERA_POSITION},
                                {"lod_camera_position_object_space",
                                 Ogre::GpuProgramParameters::AutoConstantType::
                                     ACT_LOD_CAMERA_POSITION_OBJECT_SPACE},
                                {"light_custom", Ogre::GpuProgramParameters::
                                                     AutoConstantType::ACT_LIGHT_CUSTOM},
                                {"point_params", Ogre::GpuProgramParameters::
                                                     AutoConstantType::ACT_POINT_PARAMS}};

                        instancing = program_json.get_bool("instancing");

                        bool success = true;
                        Shared<Ogre::GpuProgram> programs[2] = {nullptr, nullptr};

                        for (uint i = 0; i < (instancing ? 2 : 1); i++)
                        {
                            const bool inst_pass = i == 1;

                            const auto program =
                                Ogre::HighLevelGpuProgramManager::getSingleton()
                                    .createProgram(inst_pass
                                                       ? name_instancing(id.asset_name).c()
                                                       : id.asset_name.c(),
                                                   id.module_name.c(), "hlsl",
                                                   Ogre::GpuProgramType(program_type_index));
                            programs[i] = program;
                            program->setSource(
                                File::read_file(source_path)
                                    .c()); // TODO: Fix ogre to accept global path
                            program->setParameter("entry_point",
                                                  entry_points[program_type_index].c());
                            program->setParameter("target",
                                                  syntax_codes[program_type_index].c());

                            List<String> defines =
                                program_json.get_array("defines").convert<String>().without(
                                    "");
                            if (inst_pass)
                                defines.add("INSTANCING");
                            if (defines.length() > 0)
                                program->setParameter("preprocessor_defines",
                                                      String::join(defines, " ").c());

                            auto& params = program->getDefaultParameters();
                            for (auto& param : program_json.get_object(
                                     inst_pass ? "instancing_params" : "params"))
                            {
                                const auto param_type_name = param->value.get_string();
                                if (const auto param_type = param_types.find(param_type_name))
                                {
                                    params->setNamedAutoConstant(param->key.c(), *param_type);
                                }
                                else
                                {
                                    print_warning(
                                        "Shader Program",
                                        "Parameter %s in shader program %s has invalid type: %s",
                                        param->key.c(), id.to_string().c(), param_type_name.c());
                                }
                            }

                            program->load();

                            if (program->hasCompileError())
                            {
                                print_error("Shader Program",
                                            "Failed to create shader program %s, aborting",
                                            program->getName().c_str());
                                success = false;
                                for (const auto& prog : programs)
                                    Ogre::HighLevelGpuProgramManager::getSingleton().remove(prog);
                                break;
                            }
                        }

                        return success;
                    }
                }
            }
        }
    }

    return false;
}
