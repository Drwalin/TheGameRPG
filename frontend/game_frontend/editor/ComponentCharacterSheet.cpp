#include "godot_cpp/variant/utility_functions.hpp"

#include "../../../common/include/RegistryComponent.hpp"
#include "../../../common/include/ComponentCharacterSheet.hpp"

#include "../GodotGlm.hpp"
#include "../GameClientFrontend.hpp"

#include "EditorConfig.hpp"
#include "ComponentCharacterSheet.hpp"

namespace editor
{
ComponentCharacterSheet::ComponentCharacterSheet() {}
ComponentCharacterSheet::~ComponentCharacterSheet() {}

void ComponentCharacterSheet::Serialize(icon7::ByteWriter &writer)
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

	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   ComponentCharacterSheet_Ranges{},
									   writer);
	reg::Registry::SerializePersistent(
		GameClientFrontend::singleton->realm,
		ComponentCharacterSheet_LevelXP{0, initialXp}, writer);
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   ComponentCharacterSheet_Strength{},
									   writer);
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   ComponentCharacterSheet_AttackCooldown{},
									   writer);
}

void ComponentCharacterSheet::_bind_methods()
{
	REGISTER_PROPERTY(ComponentCharacterSheet, height, Variant::Type::FLOAT, "height");
	REGISTER_PROPERTY(ComponentCharacterSheet, width, Variant::Type::FLOAT, "width");
	REGISTER_PROPERTY(ComponentCharacterSheet, movementSpeed, Variant::Type::FLOAT,
					  "movementSpeed");
	REGISTER_PROPERTY(ComponentCharacterSheet, stepHeight, Variant::Type::FLOAT,
					  "stepHeight");

	REGISTER_PROPERTY(ComponentCharacterSheet, initialXp, Variant::Type::INT,
					  "initialXp");
}
} // namespace editor
