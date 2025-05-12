#pragma once

#include "ComponentBase.hpp"

namespace editor
{
class ComponentAITick : public ComponentBase
{
	GDCLASS(ComponentAITick, ComponentBase)
public:
	ComponentAITick();
	virtual ~ComponentAITick();
	static void _bind_methods();

	virtual void Serialize(icon7::ByteWriter &writer) override;

public:
	String aiTickName = "";
	String get_aiTickName() { return aiTickName; }
	void set_aiTickName(String v) { aiTickName = v; }
};
} // namespace editor
