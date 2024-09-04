#include "godot_cpp/variant/utility_functions.hpp"

#include "../../../server/include/EntityGameComponents.hpp"
#include "../../../server/include/callbacks/CallbackAiBehaviorTick.hpp"
#include "../../../common/include/RegistryComponent.hpp"
#include "../../../common/include/ComponentCharacterSheet.hpp"

#include "../GodotGlm.hpp"
#include "../GameClientFrontend.hpp"

#include "EditorConfig.hpp"

#include "PrefabServerNPC.hpp"

namespace editor
{
PrefabServerNPC::PrefabServerNPC() {}
PrefabServerNPC::~PrefabServerNPC() {}

void PrefabServerNPC::Serialize(icon7::ByteWriter &writer)
{
	if (height < 0.1) {
		godot::UtilityFunctions::print(this, " height cannot be below 0.1");
		return;
	}
	if (width < 0.1) {
		godot::UtilityFunctions::print(this, " width cannot be below 0.1");
		return;
	}
	if (movementSpeed <= 0) {
		godot::UtilityFunctions::print(
			this, " movementSpeed cannot be below or equal 0");
		return;
	}
	if (stepHeight < 0) {
		godot::UtilityFunctions::print(this, " stepHeight cannot be below 0");
		return;
	}

	ComponentShape shape;
	shape.height = height;
	shape.width = width;
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   shape, writer);

	ComponentMovementParameters movementParameters;
	movementParameters.maxMovementSpeedHorizontal = movementSpeed;
	movementParameters.stepHeight = stepHeight;
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   movementParameters, writer);

	ComponentLastAuthoritativeMovementState s;
	s.oldState.vel = {0, 0, 0};
	s.oldState.timestamp = 0;
	s.oldState.rot = {0, get_global_rotation().y * M_PI / 180.0, 0};
	s.oldState.pos = ToGlm(this->get_global_position());
	s.oldState.onGround = false;
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm, s,
									   writer);

	ComponentMovementState state = s.oldState;
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   state, writer);

	std::string graphicPath;
	if (graphic_Mesh_or_PackedScene.is_null() == false &&
		graphic_Mesh_or_PackedScene.is_valid()) {
		graphicPath = graphic_Mesh_or_PackedScene->get_path().utf8().ptr();
		if (graphicPath.find(RES_PREFIX) == 0) {
			graphicPath = graphicPath.replace(0, RES_PREFIX.size(), "");
		}
	}
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   ComponentModelName{graphicPath}, writer);

	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   ComponentName{nameNPC.utf8().ptr()},
									   writer);

	std::string onAiTick = aiTickName.utf8().ptr();
	named_callbacks::registry_entries::AiBehaviorTick aiTickEntry{
		onAiTick, onAiTick, {}, nullptr, nullptr};
	ComponentAITick aiTick;
	aiTick.aiTick = &aiTickEntry;
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   aiTick, writer);

	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   ComponentCharacterSheet_Ranges{},
									   writer);
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   ComponentCharacterSheet_Health{},
									   writer);
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   ComponentCharacterSheet_HealthRegen{},
									   writer);
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   ComponentCharacterSheet_LevelXP{},
									   writer);
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   ComponentCharacterSheet_Strength{},
									   writer);
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   ComponentCharacterSheet_AttackCooldown{},
									   writer);
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   ComponentCharacterSheet_Protection{},
									   writer);
}

void PrefabServerNPC::_bind_methods()
{
	REGISTER_PROPERTY_RESOURCE(PrefabServerNPC, graphic_Mesh_or_PackedScene,
							   Variant::Type::OBJECT, "Resource", "scene");
	REGISTER_PROPERTY(PrefabServerNPC, nameNPC, Variant::Type::STRING,
					  "NPC Name");
	REGISTER_PROPERTY(PrefabServerNPC, aiTickName, Variant::Type::STRING,
					  "AI Tick");
	REGISTER_PROPERTY(PrefabServerNPC, OnUse, Variant::Type::STRING,
					  "On Use (optional)");
	REGISTER_PROPERTY(PrefabServerNPC, height, Variant::Type::FLOAT, "height");
	REGISTER_PROPERTY(PrefabServerNPC, width, Variant::Type::FLOAT, "width");
	REGISTER_PROPERTY(PrefabServerNPC, movementSpeed, Variant::Type::FLOAT,
					  "movementSpeed");
	REGISTER_PROPERTY(PrefabServerNPC, stepHeight, Variant::Type::FLOAT,
					  "stepHeight");
}

void PrefabServerNPC::_ready()
{
	PrefabServerBase::_ready();
	graph = nullptr;
}

void PrefabServerNPC::_process(double dt)
{
	PrefabServerBase::_process(dt);

	if ((graph != nullptr) != GameEditorConfig::render_graphic) {
		RecreateGraphic();
	}
}

Ref<Resource> PrefabServerNPC::get_graphic_Mesh_or_PackedScene()
{
	return graphic_Mesh_or_PackedScene;
}
void PrefabServerNPC::set_graphic_Mesh_or_PackedScene(Ref<Resource> v)
{
	graphic_Mesh_or_PackedScene = Ref<Resource>{};

	Ref<PackedScene> packedScene = v;
	if (packedScene.is_null() == false && packedScene.is_valid()) {
		graphic_Mesh_or_PackedScene = v;
	}

	Ref<Mesh> mesh = v;
	if (mesh.is_null() == false && mesh.is_valid()) {
		graphic_Mesh_or_PackedScene = v;
	}

	RecreateGraphic();
}

void PrefabServerNPC::RecreateGraphic()
{
	RecreateResourceRenderer(&graph, &graphic_Mesh_or_PackedScene,
							 GameEditorConfig::render_graphic);
}
} // namespace editor
