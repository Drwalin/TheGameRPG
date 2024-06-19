
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/project_settings.hpp"

#include <icon7/Debug.hpp>

#include "PrefabServerBase.hpp"

#include "EditorConfig.hpp"

namespace editor
{
bool GameEditorConfig::render_graphic = true;
bool GameEditorConfig::render_collision = false;

void GameEditorConfig::_process(double dt)
{
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

	auto children = get_children();
	for (int i = 0; i < children.size(); ++i) {
		auto c = children[i];
		Object *o = c.operator Object *();
		if (PrefabServerBase *pref = Object::cast_to<PrefabServerBase>(o)) {
			pref->Serialize(0, writer);
		}
	}

	fwrite(writer.Buffer().data(), 1, writer.Buffer().size(), file);
	fclose(file);
}
} // namespace editor
