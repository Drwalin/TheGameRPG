#pragma once

#include "ComponentBase.hpp"

namespace editor
{
class ComponentHealthRegen : public ComponentBase
{
	GDCLASS(ComponentHealthRegen, ComponentBase)
public:
	ComponentHealthRegen();
	virtual ~ComponentHealthRegen();
	static void _bind_methods();

	virtual void Serialize(icon7::ByteWriter &writer) override;

public:
	int64_t regenAmount = 1;
	int64_t get_regenAmount() { return regenAmount; }
	void set_regenAmount(int64_t v) { regenAmount = v; }
	
	int64_t regenCooldown = 60*1000;
	int64_t get_regenCooldown() { return regenCooldown; }
	void set_regenCooldown(int64_t v) { regenCooldown = v; }
};
} // namespace editor
