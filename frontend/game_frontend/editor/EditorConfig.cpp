#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/project_settings.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/resource_saver.hpp"
#include "godot_cpp/classes/csg_primitive3d.hpp"

#include <icon7/Debug.hpp>
#include <icon7/ByteWriter.hpp>

#include "../GameClientFrontend.hpp"
#include "../GameFrontend.hpp"

#include "EntityBase.hpp"
#include "EditorConfig.hpp"

#include "ComponentTrigger.hpp"

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
	REGISTER_PROPERTY(GameEditorConfig, save_file, Variant::Type::BOOL,
					  "save file");
	REGISTER_PROPERTY_WITH_HINT(
		GameEditorConfig, save_map_file_path, Variant::Type::STRING,
		PropertyHint::PROPERTY_HINT_GLOBAL_SAVE_FILE, "", "save_map_file_path");
}

void GameEditorConfig::_ready()
{
	set_render_graphic(render_graphic);
	set_render_collision(render_collision);
	set_render_triggers(render_triggers);
}

void GameEditorConfig::_process(double dt)
{
	set_transform({});
	
	if (save_file) {
		SaveScene();
		save_file = false;
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

	icon7::ByteWriter writer(16 * 1024 * 1024);
	SerializeNode(this, writer);

	FILE *file = fopen(filePath.c_str(), "wb");
	if (file == nullptr) {
		LOG_ERROR("Cannot write to file: `%s`",
				  save_map_file_path.utf8().ptr());
		godot::UtilityFunctions::print("Cannot write to file: `",
									   save_map_file_path, "`");
		return;
	}
	fwrite(writer.Buffer().data(), 1, writer.Buffer().size(), file);
	fclose(file);
}

void GameEditorConfig::SerializeNode(Node *node, icon7::ByteWriter &writer)
{
	if (auto entity = Object::cast_to<EntityBase>(node)) {
		entity->Serialize(writer);
	} else {
		for (int i = 0; i < get_child_count(false); ++i) {
			SerializeNode(get_child(i, false), writer);
		}
	}
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
	return node;
}

void GameEditorConfig::ForEachNode(Node *node, bool isInsideEntity,
								   void(func)(Node *, bool isInsideEntity))
{
	func(node, isInsideEntity);
	if (Object::cast_to<EntityBase>(node)) {
		isInsideEntity = true;
	}
	for (int i = 0; i < node->get_child_count(true); ++i) {
		ForEachNode(node->get_child(i, true), isInsideEntity, func);
	}
}

void GameEditorConfig::set_render_graphic(bool v)
{
	render_graphic = v;
	ForEachNode(
		this, false, +[](Node *n, bool isInsideEntity) {
			if (isInsideEntity) {
				if (MeshInstance3D *mi = Object::cast_to<MeshInstance3D>(n)) {
					mi->set_visible(render_graphic);
				}
			}
		});
}

void GameEditorConfig::set_render_collision(bool v)
{
	render_collision = v;
	ForEachNode(
		this, false, +[](Node *n, bool isInsideEntity) {
			if (isInsideEntity) {// && !IsTrigger(n)) {
				if (auto csg = Object::cast_to<CSGPrimitive3D>(n)) {
					// TODO: if not trigger
					csg->set_visible(render_collision);
				}
			}
		});
}

void GameEditorConfig::set_render_triggers(bool v)
{
	render_triggers = v;
	ForEachNode(
		this, false, +[](Node *n, bool isInsideEntity) {
			if (isInsideEntity) {// && IsTrigger(n)) {
				if (auto csg = Object::cast_to<CSGPrimitive3D>(n)) {
					// TODO: if not trigger
					csg->set_visible(render_triggers);
				}
			}
		});
}

} // namespace editor
