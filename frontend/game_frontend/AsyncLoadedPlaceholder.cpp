#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/engine_debugger.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/animation_tree.hpp>
#include <godot_cpp/core/memory.hpp>

#include <icon7/Debug.hpp>

#include "AsyncLoadedPlaceholder.hpp"

AsyncLoadedPlaceholder3D::AsyncLoadedPlaceholder3D()
{
	beginTime = Time::get_singleton()->get_unix_time_from_system();
}

void AsyncLoadedPlaceholder3D::_bind_methods() {}

void AsyncLoadedPlaceholder3D::_ready()
{
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
}

void AsyncLoadedPlaceholder3D::_process(double)
{
	if (is_queued_for_deletion()) {
		return;
	}

	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	const auto res = rl->load_threaded_get_status(resourcePath.c_str());

	if (res == ResourceLoader::THREAD_LOAD_INVALID_RESOURCE) {
		double dt =
			Time::get_singleton()->get_unix_time_from_system() - beginTime;
		LOG_INFO("Invalid resource: `%s` after %f ms", resourcePath.c_str(),
				 dt * 1000.0);
		get_parent()->remove_child(this);
		queue_free();
	} else if (res == ResourceLoader::THREAD_LOAD_FAILED) {
		double dt =
			Time::get_singleton()->get_unix_time_from_system() - beginTime;
		LOG_INFO("Failed to load resource: `%s` after %f ms",
				 resourcePath.c_str(), dt * 1000.0);
		get_parent()->remove_child(this);
		queue_free();
	} else if (res == ResourceLoader::THREAD_LOAD_IN_PROGRESS) {
		// do nothing
	} else if (res == ResourceLoader::THREAD_LOAD_LOADED) {
		Ref<Resource> resource = rl->load_threaded_get(resourcePath.c_str());
		if (resource.is_null() == false && resource.is_valid()) {
			Ref<Mesh> mesh = resource;
			if (mesh.is_valid() && mesh.is_null() == false) {
				MeshInstance3D *meshInstance = memnew(MeshInstance3D);
				meshInstance->set_mesh(mesh);

				get_parent()->add_child(meshInstance);
				if (pointerHolderNode3d) {
					*pointerHolderNode3d = meshInstance;
				} else if (pointerHolderNode) {
					*pointerHolderNode = meshInstance;
				}

				get_parent()->remove_child(this);
				queue_free();
				return;
			}
			Ref<PackedScene> packedScene = resource;
			if (packedScene.is_valid() && packedScene.is_null() == false) {
				Node *node = packedScene->instantiate();

				get_parent()->add_child(node);

				if (pointerHolderNode) {
					*pointerHolderNode = node;
				} else if (pointerHolderNode3d) {
					Node3D *modelNode3D = Object::cast_to<Node3D>(node);
					if (modelNode3D) {
						*pointerHolderNode3d = modelNode3D;
					} else {
						LOG_WARN("Failed co convert scene '%s' to Node3D",
								 resourcePath.c_str());
					}
				}

				if (animationTreeHolder) {
					*animationTreeHolder =
						(AnimationTree *)(node->find_child("AnimationTree"));
				}

				get_parent()->remove_child(this);
				queue_free();
				return;
			}
		}
	}
}

void AsyncLoadedPlaceholder3D::Init(const std::string resourcePath,
									Node3D **const pointerHolderNode3d,
									Node **const pointerHolderNode,
									AnimationTree **const animationTreeHolder)
{
	rl = ResourceLoader::get_singleton();
	this->resourcePath = resourcePath;
	this->pointerHolderNode3d = pointerHolderNode3d;
	this->pointerHolderNode = pointerHolderNode;
	this->animationTreeHolder = animationTreeHolder;

	rl->load_threaded_request(resourcePath.c_str(), String(), false,
							  ResourceLoader::CACHE_MODE_REUSE);
}

AsyncLoadedPlaceholder3D *AsyncLoadedPlaceholder3D::CreateNew()
{
	return Object::cast_to<AsyncLoadedPlaceholder3D>(
		ClassDB::instantiate("AsyncLoadedPlaceholder3D"));
}
