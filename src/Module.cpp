#include "hexa_engine/Module.h"

#include <base_lib/File.h>
#include "hexa_engine/Game.h"
#include "hexa_engine/Material.h"
#include "hexa_engine/Shader.h"
#include "hexa_engine/Texture.h"

#include <OgreGpuProgramManager.h>
#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreTechnique.h>
#include <OgreTextureManager.h>
#include <base_lib/Logger.h>

const static Map<Name, Ogre::GpuProgramParameters::AutoConstantType> shader_param_types = {
    {"world_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_WORLD_MATRIX},
    {"inverse_world_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_INVERSE_WORLD_MATRIX},
    {"transpose_world_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_TRANSPOSE_WORLD_MATRIX},
    {"inverse_transpose_world_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX},
    {"bone_matrix_array_3x4", Ogre::GpuProgramParameters::AutoConstantType::ACT_BONE_MATRIX_ARRAY_3x4},
    {"world_matrix_array_3x4", Ogre::GpuProgramParameters::AutoConstantType::ACT_WORLD_MATRIX_ARRAY_3x4},
    {"bone_matrix_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_BONE_MATRIX_ARRAY},
    {"world_matrix_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_WORLD_MATRIX_ARRAY},
    {"bone_dualquaternion_array_2x4", Ogre::GpuProgramParameters::AutoConstantType::ACT_BONE_DUALQUATERNION_ARRAY_2x4},
    {"world_dualquaternion_array_2x4", Ogre::GpuProgramParameters::AutoConstantType::ACT_WORLD_DUALQUATERNION_ARRAY_2x4},
    {"bone_scale_shear_matrix_array_3x4", Ogre::GpuProgramParameters::AutoConstantType::ACT_BONE_SCALE_SHEAR_MATRIX_ARRAY_3x4},
    {"world_scale_shear_matrix_array_3x4", Ogre::GpuProgramParameters::AutoConstantType::ACT_WORLD_SCALE_SHEAR_MATRIX_ARRAY_3x4},
    {"view_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_VIEW_MATRIX},
    {"inverse_view_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_INVERSE_VIEW_MATRIX},
    {"transpose_view_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_TRANSPOSE_VIEW_MATRIX},
    {"inverse_transpose_view_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_INVERSE_TRANSPOSE_VIEW_MATRIX},
    {"projection_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_PROJECTION_MATRIX},
    {"inverse_projection_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_INVERSE_PROJECTION_MATRIX},
    {"transpose_projection_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_TRANSPOSE_PROJECTION_MATRIX},
    {"inverse_transpose_projection_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_INVERSE_TRANSPOSE_PROJECTION_MATRIX},
    {"viewproj_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_VIEWPROJ_MATRIX},
    {"inverse_viewproj_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_INVERSE_VIEWPROJ_MATRIX},
    {"transpose_viewproj_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_TRANSPOSE_VIEWPROJ_MATRIX},
    {"inverse_transpose_viewproj_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_INVERSE_TRANSPOSE_VIEWPROJ_MATRIX},
    {"worldview_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_WORLDVIEW_MATRIX},
    {"inverse_worldview_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_INVERSE_WORLDVIEW_MATRIX},
    {"transpose_worldview_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_TRANSPOSE_WORLDVIEW_MATRIX},
    {"inverse_transpose_worldview_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX},
    {"normal_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_NORMAL_MATRIX},
    {"worldviewproj_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_WORLDVIEWPROJ_MATRIX},
    {"inverse_worldviewproj_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_INVERSE_WORLDVIEWPROJ_MATRIX},
    {"transpose_worldviewproj_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_TRANSPOSE_WORLDVIEWPROJ_MATRIX},
    {"inverse_transpose_worldviewproj_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_INVERSE_TRANSPOSE_WORLDVIEWPROJ_MATRIX},
    {"render_target_flipping", Ogre::GpuProgramParameters::AutoConstantType::ACT_RENDER_TARGET_FLIPPING},
    {"vertex_winding", Ogre::GpuProgramParameters::AutoConstantType::ACT_VERTEX_WINDING},
    {"fog_colour", Ogre::GpuProgramParameters::AutoConstantType::ACT_FOG_COLOUR},
    {"fog_params", Ogre::GpuProgramParameters::AutoConstantType::ACT_FOG_PARAMS},
    {"surface_ambient_colour", Ogre::GpuProgramParameters::AutoConstantType::ACT_SURFACE_AMBIENT_COLOUR},
    {"surface_diffuse_colour", Ogre::GpuProgramParameters::AutoConstantType::ACT_SURFACE_DIFFUSE_COLOUR},
    {"surface_specular_colour", Ogre::GpuProgramParameters::AutoConstantType::ACT_SURFACE_SPECULAR_COLOUR},
    {"surface_emissive_colour", Ogre::GpuProgramParameters::AutoConstantType::ACT_SURFACE_EMISSIVE_COLOUR},
    {"surface_shininess", Ogre::GpuProgramParameters::AutoConstantType::ACT_SURFACE_SHININESS},
    {"surface_alpha_rejection_value", Ogre::GpuProgramParameters::AutoConstantType::ACT_SURFACE_ALPHA_REJECTION_VALUE},
    {"light_count", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_COUNT},
    {"ambient_light_colour", Ogre::GpuProgramParameters::AutoConstantType::ACT_AMBIENT_LIGHT_COLOUR},
    {"light_diffuse_colour", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_DIFFUSE_COLOUR},
    {"light_specular_colour", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_SPECULAR_COLOUR},
    {"light_attenuation", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_ATTENUATION},
    {"spotlight_params", Ogre::GpuProgramParameters::AutoConstantType::ACT_SPOTLIGHT_PARAMS},
    {"light_position", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_POSITION},
    {"light_position_object_space", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_POSITION_OBJECT_SPACE},
    {"light_position_view_space", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_POSITION_VIEW_SPACE},
    {"light_direction", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_DIRECTION},
    {"light_direction_object_space", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_DIRECTION_OBJECT_SPACE},
    {"light_direction_view_space", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_DIRECTION_VIEW_SPACE},
    {"light_distance_object_space", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_DISTANCE_OBJECT_SPACE},
    {"light_power_scale", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_POWER_SCALE},
    {"light_diffuse_colour_power_scaled", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED},
    {"light_specular_colour_power_scaled", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED},
    {"light_diffuse_colour_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_DIFFUSE_COLOUR_ARRAY},
    {"light_specular_colour_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_SPECULAR_COLOUR_ARRAY},
    {"light_diffuse_colour_power_scaled_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED_ARRAY},
    {"light_specular_colour_power_scaled_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED_ARRAY},
    {"light_attenuation_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_ATTENUATION_ARRAY},
    {"light_position_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_POSITION_ARRAY},
    {"light_position_object_space_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_POSITION_OBJECT_SPACE_ARRAY},
    {"light_position_view_space_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_POSITION_VIEW_SPACE_ARRAY},
    {"light_direction_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_DIRECTION_ARRAY},
    {"light_direction_object_space_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_DIRECTION_OBJECT_SPACE_ARRAY},
    {"light_direction_view_space_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_DIRECTION_VIEW_SPACE_ARRAY},
    {"light_distance_object_space_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_DISTANCE_OBJECT_SPACE_ARRAY},
    {"light_power_scale_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_POWER_SCALE_ARRAY},
    {"spotlight_params_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_SPOTLIGHT_PARAMS_ARRAY},
    {"derived_ambient_light_colour", Ogre::GpuProgramParameters::AutoConstantType::ACT_DERIVED_AMBIENT_LIGHT_COLOUR},
    {"derived_scene_colour", Ogre::GpuProgramParameters::AutoConstantType::ACT_DERIVED_SCENE_COLOUR},
    {"derived_light_diffuse_colour", Ogre::GpuProgramParameters::AutoConstantType::ACT_DERIVED_LIGHT_DIFFUSE_COLOUR},
    {"derived_light_specular_colour", Ogre::GpuProgramParameters::AutoConstantType::ACT_DERIVED_LIGHT_SPECULAR_COLOUR},
    {"derived_light_diffuse_colour_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_DERIVED_LIGHT_DIFFUSE_COLOUR_ARRAY},
    {"derived_light_specular_colour_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_DERIVED_LIGHT_SPECULAR_COLOUR_ARRAY},
    {"light_number", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_NUMBER},
    {"light_casts_shadows", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_CASTS_SHADOWS},
    {"light_casts_shadows_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_CASTS_SHADOWS_ARRAY},
    {"shadow_extrusion_distance", Ogre::GpuProgramParameters::AutoConstantType::ACT_SHADOW_EXTRUSION_DISTANCE},
    {"camera_position", Ogre::GpuProgramParameters::AutoConstantType::ACT_CAMERA_POSITION},
    {"camera_position_object_space", Ogre::GpuProgramParameters::AutoConstantType::ACT_CAMERA_POSITION_OBJECT_SPACE},
    {"camera_relative_position", Ogre::GpuProgramParameters::AutoConstantType::ACT_CAMERA_RELATIVE_POSITION},
    {"texture_viewproj_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_TEXTURE_VIEWPROJ_MATRIX},
    {"texture_viewproj_matrix_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_TEXTURE_VIEWPROJ_MATRIX_ARRAY},
    {"texture_worldviewproj_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX},
    {"texture_worldviewproj_matrix_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX_ARRAY},
    {"spotlight_viewproj_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_SPOTLIGHT_VIEWPROJ_MATRIX},
    {"spotlight_viewproj_matrix_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_SPOTLIGHT_VIEWPROJ_MATRIX_ARRAY},
    {"spotlight_worldviewproj_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX},
    {"spotlight_worldviewproj_matrix_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX_ARRAY},
    {"custom", Ogre::GpuProgramParameters::AutoConstantType::ACT_CUSTOM},
    {"time", Ogre::GpuProgramParameters::AutoConstantType::ACT_TIME},
    {"time_0_x", Ogre::GpuProgramParameters::AutoConstantType::ACT_TIME_0_X},
    {"costime_0_x", Ogre::GpuProgramParameters::AutoConstantType::ACT_COSTIME_0_X},
    {"sintime_0_x", Ogre::GpuProgramParameters::AutoConstantType::ACT_SINTIME_0_X},
    {"tantime_0_x", Ogre::GpuProgramParameters::AutoConstantType::ACT_TANTIME_0_X},
    {"time_0_x_packed", Ogre::GpuProgramParameters::AutoConstantType::ACT_TIME_0_X_PACKED},
    {"time_0_1", Ogre::GpuProgramParameters::AutoConstantType::ACT_TIME_0_1},
    {"costime_0_1", Ogre::GpuProgramParameters::AutoConstantType::ACT_COSTIME_0_1},
    {"sintime_0_1", Ogre::GpuProgramParameters::AutoConstantType::ACT_SINTIME_0_1},
    {"tantime_0_1", Ogre::GpuProgramParameters::AutoConstantType::ACT_TANTIME_0_1},
    {"time_0_1_packed", Ogre::GpuProgramParameters::AutoConstantType::ACT_TIME_0_1_PACKED},
    {"time_0_2pi", Ogre::GpuProgramParameters::AutoConstantType::ACT_TIME_0_2PI},
    {"costime_0_2pi", Ogre::GpuProgramParameters::AutoConstantType::ACT_COSTIME_0_2PI},
    {"sintime_0_2pi", Ogre::GpuProgramParameters::AutoConstantType::ACT_SINTIME_0_2PI},
    {"tantime_0_2pi", Ogre::GpuProgramParameters::AutoConstantType::ACT_TANTIME_0_2PI},
    {"time_0_2pi_packed", Ogre::GpuProgramParameters::AutoConstantType::ACT_TIME_0_2PI_PACKED},
    {"frame_time", Ogre::GpuProgramParameters::AutoConstantType::ACT_FRAME_TIME},
    {"fps", Ogre::GpuProgramParameters::AutoConstantType::ACT_FPS},
    {"viewport_width", Ogre::GpuProgramParameters::AutoConstantType::ACT_VIEWPORT_WIDTH},
    {"viewport_height", Ogre::GpuProgramParameters::AutoConstantType::ACT_VIEWPORT_HEIGHT},
    {"inverse_viewport_width", Ogre::GpuProgramParameters::AutoConstantType::ACT_INVERSE_VIEWPORT_WIDTH},
    {"inverse_viewport_height", Ogre::GpuProgramParameters::AutoConstantType::ACT_INVERSE_VIEWPORT_HEIGHT},
    {"viewport_size", Ogre::GpuProgramParameters::AutoConstantType::ACT_VIEWPORT_SIZE},
    {"view_direction", Ogre::GpuProgramParameters::AutoConstantType::ACT_VIEW_DIRECTION},
    {"view_side_vector", Ogre::GpuProgramParameters::AutoConstantType::ACT_VIEW_SIDE_VECTOR},
    {"view_up_vector", Ogre::GpuProgramParameters::AutoConstantType::ACT_VIEW_UP_VECTOR},
    {"fov", Ogre::GpuProgramParameters::AutoConstantType::ACT_FOV},
    {"near_clip_distance", Ogre::GpuProgramParameters::AutoConstantType::ACT_NEAR_CLIP_DISTANCE},
    {"far_clip_distance", Ogre::GpuProgramParameters::AutoConstantType::ACT_FAR_CLIP_DISTANCE},
    {"pass_number", Ogre::GpuProgramParameters::AutoConstantType::ACT_PASS_NUMBER},
    {"pass_iteration_number", Ogre::GpuProgramParameters::AutoConstantType::ACT_PASS_ITERATION_NUMBER},
    {"animation_parametric", Ogre::GpuProgramParameters::AutoConstantType::ACT_ANIMATION_PARAMETRIC},
    {"texel_offsets", Ogre::GpuProgramParameters::AutoConstantType::ACT_TEXEL_OFFSETS},
    {"scene_depth_range", Ogre::GpuProgramParameters::AutoConstantType::ACT_SCENE_DEPTH_RANGE},
    {"shadow_scene_depth_range", Ogre::GpuProgramParameters::AutoConstantType::ACT_SHADOW_SCENE_DEPTH_RANGE},
    {"shadow_scene_depth_range_array", Ogre::GpuProgramParameters::AutoConstantType::ACT_SHADOW_SCENE_DEPTH_RANGE_ARRAY},
    {"shadow_colour", Ogre::GpuProgramParameters::AutoConstantType::ACT_SHADOW_COLOUR},
    {"texture_size", Ogre::GpuProgramParameters::AutoConstantType::ACT_TEXTURE_SIZE},
    {"inverse_texture_size", Ogre::GpuProgramParameters::AutoConstantType::ACT_INVERSE_TEXTURE_SIZE},
    {"packed_texture_size", Ogre::GpuProgramParameters::AutoConstantType::ACT_PACKED_TEXTURE_SIZE},
    {"texture_matrix", Ogre::GpuProgramParameters::AutoConstantType::ACT_TEXTURE_MATRIX},
    {"lod_camera_position", Ogre::GpuProgramParameters::AutoConstantType::ACT_LOD_CAMERA_POSITION},
    {"lod_camera_position_object_space", Ogre::GpuProgramParameters::AutoConstantType::ACT_LOD_CAMERA_POSITION_OBJECT_SPACE},
    {"light_custom", Ogre::GpuProgramParameters::AutoConstantType::ACT_LIGHT_CUSTOM},
    {"point_params", Ogre::GpuProgramParameters::AutoConstantType::ACT_POINT_PARAMS}
};

const static Map<String, Ogre::TextureFilterOptions> texture_filters = {
    {"", Ogre::TextureFilterOptions::TFO_NONE},
    {"bilinear", Ogre::TextureFilterOptions::TFO_BILINEAR},
    {"trilinear", Ogre::TextureFilterOptions::TFO_TRILINEAR},
    {"anisotropic", Ogre::TextureFilterOptions::TFO_ANISOTROPIC}
};

FORCEINLINE Name name_instancing(const Name& name) {
    return Name(name.to_string() + "_instancing");
}

Module::Module(const Name& module_name)
    : module_name(module_name) {
}

void Module::on_add_resource_directories(Set<String>& local, Set<String>& global) {
}

Shared<Texture> Module::load_texture(const Name& name) {
    const ModuleAssetID asset_id(module_name, name);

    if (!name.is_valid()) {
        print_error("Texture", "Invalid name, unable to load: %s", asset_id.to_string().c());
        return nullptr;
    }

    if (const auto slot = textures_.find_or_default(name)) {
        return slot;
    }

    if (Game::get_instance()->get_stage() != GameStage::Loading) {
        print_warning("Texture", "Loading outside of loading stage is not recommended: %s", asset_id.to_string().c());
    }

    const auto path =  (get_textures_path() + asset_id.asset_name.c()).with_extension("png");

    if (!path.exists()) {
        print_error("Texture", "Does not present on disk, unable to load: %s", asset_id.to_string().c());
        return nullptr;
    }

    const Shared<Ogre::FileStreamDataStream> stream = MakeShared<Ogre::FileStreamDataStream>(new std::ifstream(path.get_absolute_string().c()));
    Ogre::Image image;
    image.load(stream, path.extension.c());
    const auto ogre_texture = Ogre::TextureManager::getSingleton().loadImage(name.c(), module_name.c(), image, Ogre::TEX_TYPE_2D, Ogre::MIP_DEFAULT, 1, false, Ogre::PF_R8G8B8A8);

    Shared<Texture> result = MakeSharedInternal(Texture);
    result->ogre_texture_ = ogre_texture;
    result->id_ = asset_id;

    textures_[name] = result;

    return result;
}

Shared<Texture> Module::create_texture(const Array2D<Color>& pixels, const Name& name) {
    const ModuleAssetID asset_id(module_name, name);

    if (!name.is_valid()) {
        print_error("Texture", "Invalid name, unable to load: %s", asset_id.to_string().c());
        return nullptr;
    }

    if (textures_.contains(name)) {
        print_error("Texture", "Name is already taken, unable to create: %s", asset_id.to_string().c());
        return nullptr;
    }

    if (Game::get_instance()->get_stage() != GameStage::Loading) {
        print_warning("Texture", "Creating outside of loading stage is not recommended: %s", asset_id.to_string().c());
    }

    Shared<Texture> result = MakeSharedInternal(Texture);
    Ogre::Image image = Ogre::Image(Ogre::PF_R8G8B8A8, pixels.get_size_x(), pixels.get_size_y(), 1, (byte*)pixels.begin(), false);
    result->ogre_texture_ = Ogre::TextureManager::getSingleton().loadImage(name.c(), module_name.c(), image);
    result->id_ = asset_id;

    textures_[name] = result;

    return result;
}

Shared<Material> Module::load_material(const Name& name) {
    const ModuleAssetID asset_id(module_name, name);

    if (!name.is_valid()) {
        print_error("Material", "Invalid name, unable to load: %s", asset_id.to_string().c());
        return nullptr;
    }

    if (const auto slot = materials_.find_or_default(name)) {
        return slot;
    }

    if (Game::get_instance()->get_stage() != GameStage::Loading) {
        print_warning("Material", "Loading outside of loading stage is not recommended: %s", asset_id.to_string().c());
    }

    const auto path =  (get_materials_path() + asset_id.asset_name.c()).with_extension("mat");

    if (!path.exists()) {
        print_error("Material", "Does not present on disk, unable to load: %s", asset_id.to_string().c());
        return nullptr;
    }

    Compound::Object material_json;
    if (!Compound::Convert::JSON().try_parse_object(File::read_file(path), material_json)) {
        print_error("Material", "Failed parsing: %s", asset_id.to_string().c());
        return nullptr;
    }

    const auto vertex_program_name = ModuleAssetID(material_json.get_string("vertex"), module_name);
    const auto fragment_program_name = ModuleAssetID(material_json.get_string("fragment"), module_name);

    if (!vertex_program_name.is_valid()) {
        print_error("Material", "Invalid vertex program name: %s", asset_id.to_string().c());
        return nullptr;
    }

    if (!fragment_program_name.is_valid()) {
        print_error("Material", "Invalid fragment program name: %s", asset_id.to_string().c());
        return nullptr;
    }

    const auto vertex_program = vertex_program_name.get_module_reference()->load_shader_program(vertex_program_name.asset_name);

    if (!vertex_program->is_valid()) {
        print_error("Material", "Vertex program failed to load: %s", asset_id.to_string().c());
        return nullptr;
    }

    const auto fragment_program = fragment_program_name.get_module_reference()->load_shader_program(fragment_program_name.asset_name);
    if (!fragment_program->is_valid()) {
        print_error("Material", "Fragment program failed to load: %s", asset_id.to_string().c());
        return nullptr;
    }

    Shared<Ogre::Material> ogre_material = nullptr;
    Shared<Ogre::Material> ogre_material_instancing = nullptr;

    for (uint i = 0; i < (vertex_program->has_instanced() || fragment_program->has_instanced() ? 2 : 1); i++) {
        const bool inst_pass = i == 1;

        auto& mat = inst_pass ? ogre_material_instancing : ogre_material;
        mat = Ogre::MaterialManager::getSingleton().create(
            inst_pass ? name_instancing(name).c() : name.c(),
            module_name.c());
        const auto technique = mat->createTechnique();
        const auto pass = technique->createPass();

        pass->setGpuProgram(Ogre::GPT_VERTEX_PROGRAM, inst_pass ? vertex_program->regular_ : vertex_program->try_get_instanced());
        pass->setGpuProgram(Ogre::GPT_FRAGMENT_PROGRAM, inst_pass ? fragment_program->regular_ : fragment_program->try_get_instanced());

        for (const auto& texture : material_json.get_array("textures")) {
            if (texture.get_type() == Compound::Type::Object) {
                const auto texture_data = texture.get_object();

                auto unit_state = pass->createTextureUnitState();

                if (const auto param_default = texture_data.find("default")) {
                    const auto default_id = ModuleAssetID(param_default->get_string(), module_name);
                    if (const auto default_texture = default_id.get_module_reference()->load_texture(default_id.asset_name)) {
                        unit_state->setTexture(default_texture->ogre_texture_);
                    }
                }

                if (const auto param_filtering = texture_data.find("filtering")) {

                    const auto filter = texture_filters.find_or_default(param_filtering->get_string(), Ogre::TextureFilterOptions::TFO_NONE);
                    unit_state->setTextureFiltering(filter);
                }
            }
        }

        mat->load();
    }

    Shared<Material> result = MakeSharedInternal(Material);
    result->ogre_material_ = ogre_material;
    result->ogre_material_instancing_ = ogre_material_instancing;
    result->id_ = asset_id;

    materials_[name] = result;

    return result;
}

Shared<Material> Module::clone_material(const Shared<Material>& material, const Name& new_name) {
    if (material == nullptr) return nullptr;
    
    const ModuleAssetID new_id = ModuleAssetID(
        module_name,
        new_name.is_valid() ? new_name : Name(material->get_id().asset_name.to_string() + "_clone")
    );

    if (const auto slot = materials_.contains(new_id.asset_name)) {
        print_error("Material", "Name is already taken, unable to clone: %s", new_id.to_string().c());
        return nullptr;
    }
    
    auto result = MakeSharedInternal(Material);
    result->ogre_material_ = material->ogre_material_->clone(new_id.asset_name.c(), new_id.module_name.c());
    result->id_ = new_id;

    materials_[new_id.asset_name] = result;

    return result;
}

Path Module::get_resources_path() const {
    return get_module_path("resources");
}

Path Module::get_textures_path() const {
    return get_resources_path() + "textures";
}

Path Module::get_materials_path() const {
    return get_resources_path() + "materials";
}

Path Module::get_shaders_path() const {
    return get_resources_path() + "shaders";
}

void Module::add_resource_directories() {
    Set<String> local;
    Set<String> global;

    on_add_resource_directories(local, global);

    local_directories += local;
    global_directories += global;
}

void Module::register_resource_directories() {
    const Set<String> directories = local_directories + global_directories;
    for (const auto& directory : directories) {
        const auto path = get_module_path("resources/" + directory);
        if (path.exists()) {
            Ogre::ResourceGroupManager::getSingletonPtr()->addResourceLocation(path.get_absolute_string().c(), "FileSystem", module_name.c());
        }
    }
}

void Module::unregister_resource_directories() {
    const auto locations = Ogre::ResourceGroupManager::getSingletonPtr()->listResourceLocations(module_name.c());
    for (uint i = 0; i < locations->size(); i++) {
        Ogre::ResourceGroupManager::getSingletonPtr()->removeResourceLocation(locations->at(i), module_name.c());
    }
}

void Module::reset_global_resources_directories() {
    global_directories = global_directories_default;
}

Shared<Shader> Module::load_shader_program(const Name& name) {
    const ModuleAssetID asset_id(module_name, name);

    if (!name.is_valid()) {
        print_error("Shader Program", "Invalid name, unable to load: %s", asset_id.to_string().c());
        return nullptr;
    }

    if (const auto slot = shader_.find_or_default(name)) {
        return slot;
    }

    const auto path =  (get_shaders_path() + asset_id.asset_name.c()).with_extension("sha");

    if (!path.exists()) {
        print_error("Shader Program", "Does not present on disk, unable to load: %s", asset_id.to_string().c());
        return nullptr;
    }

    Compound::Object program_json;
    if (!Compound::Convert::JSON().try_parse_object(File::read_file(path), program_json)) return nullptr;

    const auto source_name = ModuleAssetID(program_json.get_string("source"), module_name);
    if (!source_name.is_valid()) return nullptr;

    const auto source_path = (get_shaders_path() + asset_id.asset_name.c()).with_extension("hlsl");
    if (!source_path.exists()) return nullptr;

    const static List<String> program_type_names = {"vertex", "fragment"};
    const int program_type_index = program_type_names.index_of(program_json.get_string("type"));
    if (program_type_index < 0) return nullptr;

    const static List<String> entry_points = {"vert", "frag"};
    const static List<String> syntax_codes = {"vs_2_0", "ps_2_0"};

    const bool instancing = program_json.get_bool("instancing");

    Shared<Ogre::GpuProgram> programs[2] = {nullptr, nullptr};

    for (uint i = 0; i < (instancing ? 2 : 1); i++) {
        const bool inst_pass = i == 1;

        const auto program = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(inst_pass
                                                                                                ? name_instancing(name).c()
                                                                                                : name.c(),
                                                                                            module_name.c(), "hlsl",
                                                                                            Ogre::GpuProgramType(program_type_index));
        programs[i] = program;
        program->setSource(File::read_file(source_path).c());
        program->setParameter("entry_point", entry_points[program_type_index].c());
        program->setParameter("target", syntax_codes[program_type_index].c());

        List<String> defines = program_json.get_array("defines").convert<String>().without("");
        if (inst_pass) defines.add("INSTANCING");
        if (defines.length() > 0) program->setParameter("preprocessor_defines", String::join(defines, " ").c());

        auto& params = program->getDefaultParameters();
        for (const auto& param : program_json.get_object(inst_pass ? "instancing_params" : "params")) {
            const auto param_type_name = param->value.get_string();
            if (const auto param_type = shader_param_types.find(Name(param_type_name))) {
                params->setNamedAutoConstant(param->key.c(), *param_type);
            } else {
                print_warning("Shader Program", "Parameter %s has invalid type: %s", param->key.c(), asset_id.to_string().c(), param_type_name.c());
            }
        }

        program->load();

        if (program->hasCompileError()) {
            print_error("Shader Program", "Failed to create: %s", program->getName().c_str());

            for (const auto& prog : programs) Ogre::HighLevelGpuProgramManager::getSingleton().remove(prog);

            return nullptr;
        }
    }

    auto result = MakeSharedInternal(Shader, programs[0], programs[1]);
    shader_[name] = result;
    
    return result;
}
