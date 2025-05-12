#pragma once

#include "ComponentBase.hpp"

namespace editor
{
class ComponentCharacterSheet : public ComponentBase
{
	GDCLASS(ComponentCharacterSheet, ComponentBase)
public:
	ComponentCharacterSheet();
	virtual ~ComponentCharacterSheet();
	static void _bind_methods();

	virtual void Serialize(icon7::ByteWriter &writer) override;

public:
	double height = 1.85;
	double get_height() { return height; }
	void set_height(double v) { height = v; }

	double width = 0.5;
	double get_width() { return width; }
	void set_width(double v) { width = v; }

	double movementSpeed = 5.0;
	double get_movementSpeed() { return movementSpeed; }
	void set_movementSpeed(double v) { movementSpeed = v; }

	double stepHeight = 0.25;
	double get_stepHeight() { return stepHeight; }
	void set_stepHeight(double v) { stepHeight = v; }

	int64_t initialXp = 0;
	int64_t get_initialXp() { return initialXp; }
	void set_initialXp(int64_t v) { initialXp = v; }
};
} // namespace editor
