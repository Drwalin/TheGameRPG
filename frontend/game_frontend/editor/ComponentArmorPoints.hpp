#pragma once

#include "ComponentBase.hpp"

namespace editor
{
class ComponentArmorPoints : public ComponentBase
{
	GDCLASS(ComponentArmorPoints, ComponentBase)
public:
	ComponentArmorPoints();
	virtual ~ComponentArmorPoints();
	static void _bind_methods();

	virtual void Serialize(icon7::ByteWriter &writer) override;

public:
	int64_t armorPoints = 0;
	int64_t get_armorPoints() { return armorPoints; }
	void set_armorPoints(int64_t v) { armorPoints = v; }
};
} // namespace editor
