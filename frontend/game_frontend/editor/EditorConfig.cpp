#include <flecs.h>

#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/project_settings.hpp"
#include "godot_cpp/classes/engine.hpp"

#include <icon7/Debug.hpp>

#include "PrefabServerBase.hpp"

#include "EditorConfig.hpp"

int RegisterEntityGameComponents(flecs::world &ecs);

namespace editor
{
bool GameEditorConfig::render_graphic = true;
bool GameEditorConfig::render_collision = false;
bool GameEditorConfig::render_triggers = false;

void GameEditorConfig::_bind_methods()
{
	if (Engine::get_singleton()->is_editor_hint() == true) {
		flecs::world ecs;
		RegisterEntityGameComponents(ecs);
	}

	REGISTER_PROPERTY(GameEditorConfig, render_graphic, Variant::Type::BOOL,
					  "render graphics");
	REGISTER_PROPERTY(GameEditorConfig, render_collision, Variant::Type::BOOL,
					  "render collision");
	REGISTER_PROPERTY(GameEditorConfig, render_triggers, Variant::Type::BOOL,
					  "render triggers");
	REGISTER_PROPERTY_WITH_HINT(
		GameEditorConfig, save_map_file_path, Variant::Type::STRING,
		PropertyHint::PROPERTY_HINT_GLOBAL_SAVE_FILE, "", "save_map_file_path");
	REGISTER_PROPERTY(GameEditorConfig, save_scene, Variant::Type::BOOL,
					  "save_scene");
}

void GameEditorConfig::_process(double dt)
{
	set_transform({});
	if (save_scene) {
		SaveScene();
		save_scene = false;
	}
}

void GameEditorConfig::SaveScene()
{
	std::string projectPath =
		ProjectSettings::get_singleton()->globalize_path("res://").utf8().ptr();
	std::string filePath = projectPath + "/" + save_map_file_path.utf8().ptr();

	FILE *file = fopen(filePath.c_str(), "wb");
	if (file == nullptr) {
		LOG_ERROR("Cannot write to file: `%s`",
				  save_map_file_path.utf8().ptr());
		godot::UtilityFunctions::print("Cannot write to file: `",
									   save_map_file_path, "`");
		return;
	}

	icon7::ByteWriter writer(1024 * 1024);
	SaveNode(this, writer);
	fwrite(writer.Buffer().data(), 1, writer.Buffer().size(), file);
	fclose(file);
}

void GameEditorConfig::SaveNode(Node3D *node, icon7::ByteWriter &writer)
{
	auto children = node->get_children();
	for (int i = 0; i < children.size(); ++i) {
		auto c = children[i];
		Node3D *n = Object::cast_to<Node3D>(c.operator Object *());
		if (PrefabServerBase *pref = Object::cast_to<PrefabServerBase>(n)) {
			pref->Serialize(writer);
			writer.op("");
		} else {
			SaveNode(n, writer);
		}
	}
}

} // namespace editor
