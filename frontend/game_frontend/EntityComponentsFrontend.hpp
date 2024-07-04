#pragma once

#include "../../common/include/EntityComponents.hpp"

struct ComponentGodotNode {
	class EntityPrefabScript *node = nullptr;

	ComponentGodotNode(EntityPrefabScript *node) : node(node) {}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentGodotNode, MV(node));
};

struct ComponentStaticGodotNode {
	class EntityStaticPrefabScript *node = nullptr;

	ComponentStaticGodotNode(EntityStaticPrefabScript *node) : node(node) {}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentStaticGodotNode, MV(node));
};
