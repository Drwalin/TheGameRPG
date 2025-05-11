#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/project_settings.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/resource_saver.hpp"

#include <icon7/Debug.hpp>
#include <icon7/ByteWriter.hpp>

#include "../GameClientFrontend.hpp"

#include "../GameFrontend.hpp"

#include "PrefabServerBase.hpp"
#include "PrefabServerStaticMesh_Base.hpp"

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

	if (Engine::get_singleton()->is_editor_hint() == false) {
		++frameCounter;
		if (frameCounter == 5) {
			SaveScene();
			get_tree()->quit();
		}
	}
}

void GameEditorConfig::SaveScene()
{
	otherObjects.clear();
	staticToGoSecond.clear();

	if (GameClientFrontend::singleton == nullptr) {
		GameFrontend::singleton->InternalReady();
	}

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
	SelectNodes(this);

	WriteSecondStatic(writer);
	WriteOtherObjects(writer);

	fwrite(writer.Buffer().data(), 1, writer.Buffer().size(), file);
	fclose(file);
}

bool CheckIfPrefab(Node *node)
{
	auto children = node->get_children();
	for (int i = 0; i < children.size(); ++i) {
		auto c = children[i];
		Node *n = Object::cast_to<Node>(c.operator Object *());
		if (Object::cast_to<PrefabServerBase>(n)) {
			return true;
		} else {
			if (CheckIfPrefab(n)) {
				return true;
			}
		}
	}
	return false;
}

bool GameEditorConfig::SelectNodes(Node *node)
{
	std::vector<Node *> later;
	bool ret = false;
	auto children = node->get_children();
	for (int i = 0; i < children.size(); ++i) {
		auto c = children[i];
		Node *n = Object::cast_to<Node>(c.operator Object *());
		if (PrefabServerBase *pref = Object::cast_to<PrefabServerBase>(n)) {
			PrefabServerStaticMesh_Base *stBasePref =
				dynamic_cast<PrefabServerStaticMesh_Base *>(pref);
			if (stBasePref) {
				staticToGoSecond.push_back(stBasePref);
			} else {
				otherObjects.push_back(pref);
			}
			ret = true;
		} else if (CheckIfPrefab(n)) {
			ret |= SelectNodes(n);
		} else {
			later.push_back(n);
		}
	}
	return ret;
}

Node3D *GameEditorConfig::InstantiateGraphicMesh(Ref<Resource> res)
{
	Node3D *node = nullptr;
	if (res.is_valid() && !res.is_null()) {

		Ref<PackedScene> packedScene = res;
		if (packedScene.is_null() == false && packedScene.is_valid()) {
			node = Object::cast_to<Node3D>(packedScene->instantiate());
		}

		Ref<Mesh> mesh = res;
		if (mesh.is_null() == false && mesh.is_valid()) {
			MeshInstance3D *m = memnew(MeshInstance3D);
			m->set_mesh(mesh);
		}
	}
	node->set_name(PrefabServerBase::GetRandomString());
	return node;
}

String GameEditorConfig::GenerateUniqueFileName(String prefix, String suffix)
{
	while (true) {
		String path = prefix + PrefabServerBase::GetRandomString() + suffix;
		if (FileAccess::file_exists(path) == false) {
			return path;
		}
	}
	return "";
}

void GameEditorConfig::WriteSecondStatic(icon7::ByteWriter &writer)
{
	for (auto it : staticToGoSecond) {
		it->Serialize(writer);
		writer.op("");
	}
}

void GameEditorConfig::WriteOtherObjects(icon7::ByteWriter &writer)
{
	for (auto it : otherObjects) {
		it->Serialize(writer);
		writer.op("");
	}
}

} // namespace editor
