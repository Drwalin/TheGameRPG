#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/project_settings.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/resource_saver.hpp"

#include <icon7/Debug.hpp>
#include <icon7/ByteWriter.hpp>

#include "../../../common/include/EntityComponents.hpp"
#include "../../../common/include/RegistryComponent.hpp"

#include "../GameClientFrontend.hpp"

#include "../GameFrontend.hpp"
#include "../GodotGlm.hpp"

#include "PrefabServerBase.hpp"
#include "PrefabServerStaticMesh.hpp"

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
	REGISTER_PROPERTY(GameEditorConfig, merge_static_objects,
					  Variant::Type::BOOL, "merge_static_objects");
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
	objectsToMerge.clear();
	otherNodesToMerge.clear();
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

	MergeObjects(writer);
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
			PrefabServerStaticMesh *stPref =
				dynamic_cast<PrefabServerStaticMesh *>(pref);
			PrefabServerStaticMesh_Base *stBasePref =
				dynamic_cast<PrefabServerStaticMesh_Base *>(pref);
			if (stPref && stPref->isTrullyStaticForMerge &&
				merge_static_objects) {
				objectsToMerge.push_back(stPref);
			} else if (stBasePref) {
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
	for (auto n : later) {
		bool v = SelectNodes(n);
		if (v == false && ret == true) {
			otherNodesToMerge.push_back(n);
		}
	}
	return ret;
}

void GameEditorConfig::MergeObjects(icon7::ByteWriter &writer)
{
	if (objectsToMerge.empty() && otherNodesToMerge.empty()) {
		return;
	}

	Node3D *rootNode = memnew(Node3D);
	rootNode->set_transform(
		Transform3D(Basis(Quaternion::from_euler(Vector3(0, 0, 0)), Vector3(1, 1, 1)),
					Vector3(0, 0, 0)));

	bool hasCollision = false;
	for (auto it : objectsToMerge) {
		Transform3D trans;
		Ref<Resource> graphicMeshOrPackedScene;
		Ref<Mesh> collisionMesh;
		trans = it->GetMergingData(&graphicMeshOrPackedScene, &collisionMesh);

		if (collisionMesh.is_valid() && !collisionMesh.is_null()) {
			hasCollision = true;
			break;
		}
	}

	String collisionPath =
		GenerateUniqueFileName("res://assets/generated/Col", ".obj.txt");
	String graphicScenePath =
		GenerateUniqueFileName("res://assets/generated/Scene", ".tscn");

	Ref<FileAccess> file;
	int32_t indexOffset = 1;
	if (hasCollision) {
		file = FileAccess::open(collisionPath, FileAccess::WRITE);
	}

	bool hasGraphics = false;
	for (auto it : objectsToMerge) {
		Transform3D trans;
		Ref<Resource> graphicMeshOrPackedScene;
		Ref<Mesh> collisionMesh;
		trans = it->GetMergingData(&graphicMeshOrPackedScene, &collisionMesh);

		Node3D *node = InstantiateGraphicMesh(graphicMeshOrPackedScene);
		if (node) {
			rootNode->add_child(node);
			node->set_owner(rootNode);
			node->set_transform(trans);
			hasGraphics = true;
		}

		if (hasCollision && collisionMesh.is_valid() &&
			!collisionMesh.is_null()) {

			std::string srcCollisionPath =
				collisionMesh->get_path().utf8().ptr();
			if (srcCollisionPath.find(RES_PREFIX) == 0) {
				srcCollisionPath =
					srcCollisionPath.replace(0, RES_PREFIX.size(), "");
			}
			TerrainCollisionData colData;
			if (GameClientFrontend::singleton->GetCollisionShape(
					srcCollisionPath, &colData)) {

				char line[128];
				for (auto v : colData.vertices) {
					v = ToGlm(trans.xform(::ToGodot(v)));
					sprintf(line, "v %f %f %f\n", v.x, v.y, v.z);
					for (char *c = line; *c; ++c) {
						file->store_8(*c);
					}
				}

				for (int i = 0; i + 2 < (int64_t)colData.indices.size(); i += 3) {
					int32_t id[3];
					for (int j = 0; j < 3; ++j) {
						id[j] = indexOffset + colData.indices[i + j];
					}

					sprintf(line, "f %i// %i// %i//\n", id[0], id[1], id[2]);
					for (char *c = line; *c; ++c) {
						file->store_8(*c);
					}
				}

				indexOffset += colData.vertices.size();
			}
		}
	}

	for (auto it : otherNodesToMerge) {
		Node *nd = it->duplicate();

		rootNode->add_child(nd);
		nd->set_owner(rootNode);

		Node3D *n3d = Object::cast_to<Node3D>(nd);
		Node3D *sn3d = Object::cast_to<Node3D>(it);
		if (n3d && sn3d) {
			n3d->set_transform(sn3d->get_global_transform());
		}
		hasGraphics = true;
	}

	if (hasCollision) {
		file->close();
	}

	if (hasGraphics) {
		Ref<PackedScene> packedScene;
		packedScene.instantiate();
		packedScene->pack(rootNode);
		ResourceSaver::get_singleton()->save(packedScene, graphicScenePath);
	}
	rootNode->queue_free();

	{
		std::string graphicPath;
		if (hasGraphics) {
			graphicPath = graphicScenePath.utf8().ptr();
			if (graphicPath.find(RES_PREFIX) == 0) {
				graphicPath = graphicPath.replace(0, RES_PREFIX.size(), "");
			}
		}

		String colPath = collisionPath;
		std::string collisionPath;
		if (hasCollision) {
			collisionPath = colPath.utf8().ptr();
			if (collisionPath.find(RES_PREFIX) == 0) {
				collisionPath = collisionPath.replace(0, RES_PREFIX.size(), "");
			}
		}

		reg::Registry::SerializePersistent(
			GameClientFrontend::singleton->realm,
			ComponentStaticTransform{{0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1}},
			writer);
		reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
										   ComponentModelName{graphicPath},
										   writer);
		reg::Registry::SerializePersistent(
			GameClientFrontend::singleton->realm,
			ComponentStaticCollisionShapeName{collisionPath}, writer);
		writer.op("");
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
