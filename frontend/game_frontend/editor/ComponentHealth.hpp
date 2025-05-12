#pragma once

#include "ComponentBase.hpp"

namespace editor
{
class ComponentHealth : public ComponentBase
{
	GDCLASS(ComponentHealth, ComponentBase)
public:
	ComponentHealth();
	virtual ~ComponentHealth();
	static void _bind_methods();

	virtual void Serialize(icon7::ByteWriter &writer) override;

public:
	int64_t initialMaxHp = 10;
	int64_t get_initialMaxHp() { return initialMaxHp; }
	void set_initialMaxHp(int64_t v) { initialMaxHp = v; }

	int64_t initialHp = 10;
	int64_t get_initialHp() { return initialHp; }
	void set_initialHp(int64_t v) { initialHp = v; }
};
} // namespace editor
