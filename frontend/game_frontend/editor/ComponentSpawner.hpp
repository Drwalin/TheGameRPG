#pragma once

#include <godot_cpp/classes/csg_sphere3d.hpp>

#include "ComponentBase.hpp"

namespace editor
{
class ComponentSpawner : public ComponentBase
{
	GDCLASS(ComponentSpawner, ComponentBase)
public: // Godot bound functions
	ComponentSpawner();
	virtual ~ComponentSpawner();
	static void _bind_methods();

	void _ready() override;
	void _process(double dt) override;

	virtual void Serialize(icon7::ByteWriter &writer) override;

public: // variables
	class CSGSphere3D *renderSphere = nullptr;
	class CSGSphere3D *get_renderSphere() { return renderSphere; }
	void set_renderSphere(class CSGSphere3D *v) { renderSphere = v; }

	int64_t maxAmount = 10;
	int64_t get_maxAmount() { return maxAmount; }
	void set_maxAmount(int64_t v) { maxAmount = v; }

	int64_t spawnCooldown = 3000;
	int64_t get_spawnCooldown() { return spawnCooldown; }
	void set_spawnCooldown(int64_t v) { spawnCooldown = v; }

	double spawnRadius = 32.0f;
	double get_spawnRadius() { return spawnRadius; }
	void set_spawnRadius(double v) { spawnRadius = v; }

	double radiusToCheckExistingEntities = 128.0f;
	double get_radiusToCheckExistingEntities()
	{
		return radiusToCheckExistingEntities;
	}
	void set_radiusToCheckExistingEntities(double v)
	{
		radiusToCheckExistingEntities = v;
	}
};
} // namespace editor
