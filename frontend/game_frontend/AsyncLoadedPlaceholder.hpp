#pragma once

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>
#include <godot_cpp/classes/node3d.hpp>

#include <godot_cpp/classes/time.hpp>

using namespace godot;
namespace godot
{
class ResourceLoader;
class AnimationTree;
} // namespace godot

class GameFrontend;

class AsyncLoadedPlaceholder3D final : public Node3D
{
	GDCLASS(AsyncLoadedPlaceholder3D, Node)
public: // Godot bound functions
	AsyncLoadedPlaceholder3D();
	static void _bind_methods();

	void Init(const std::string resourcePath,
			  Node3D **const pointerHolderNode3d,
			  Node **const pointerHolderNode,
			  AnimationTree **const animationTreeHolder);

	void _ready() override;
	void _process(double dt) override;

private: // variables
	std::string resourcePath;
	Node3D **pointerHolderNode3d = nullptr;
	Node **pointerHolderNode = nullptr;
	AnimationTree **animationTreeHolder = nullptr;
	godot::ResourceLoader *rl = nullptr;

public:
	static AsyncLoadedPlaceholder3D *CreateNew();

	double beginTime;
};
