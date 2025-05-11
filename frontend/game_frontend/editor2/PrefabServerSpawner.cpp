#include "../../../server/include/EntityGameComponents.hpp"
#include "../../../common/include/RegistryComponent.hpp"

#include <icon7/ByteWriter.hpp>
#include <icon7/ByteReader.hpp>

#include "../GameClientFrontend.hpp"

#include "EditorConfig.hpp"

#include "PrefabServerSpawner.hpp"

namespace editor
{
PrefabServerSpawner::PrefabServerSpawner() {}
PrefabServerSpawner::~PrefabServerSpawner() {}

void PrefabServerSpawner::Serialize(icon7::ByteWriter &writer)
{
	PrefabServerBase::Serialize(writer);

	ComponentSpawner spawner;
	spawner.maxAmount = maxAmount;
	spawner.spawnCooldown = spawnCooldown;
	spawner.spawnRadius = spawnRadius;
	spawner.radiusToCheckAmountEntities = radiusToCheckExistingEntities;
	spawner.lastSpawnedTimestamp = 0;
	spawner.prefabsOffset.push_back(0);

	icon7::ByteWriter _writer(1024);
	std::function<void(Node *)> traverseFunction = [&](Node *node) {
		if (node != this) {
			if (auto psb = Object::cast_to<PrefabServerBase>(node)) {
				psb->Serialize(_writer);
				_writer.op("");
				spawner.prefabsOffset.emplace_back(_writer.GetSize());
				return;
			}
		}

		auto ch = node->get_children();
		for (int i = 0; i < ch.size(); ++i) {
			traverseFunction(Object::cast_to<Node>((Object *)(ch[i])));
		}
	};

	traverseFunction(this);

	spawner.prefabsData.clear();
	spawner.prefabsData.insert(spawner.prefabsData.begin(), _writer.data(),
							   _writer.data() + _writer.GetSize());

	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   spawner, writer);

	reg::Registry::SerializePersistent(
		GameClientFrontend::singleton->realm,
		ComponentStaticTransform{ToGame(get_global_transform())}, writer);
}

void PrefabServerSpawner::_bind_methods()
{
	REGISTER_PROPERTY(PrefabServerSpawner, maxAmount, Variant::Type::INT,
					  "maxAmount");
	REGISTER_PROPERTY(PrefabServerSpawner, spawnCooldown, Variant::Type::INT,
					  "spawnCooldown");
	REGISTER_PROPERTY(PrefabServerSpawner, spawnRadius, Variant::Type::FLOAT,
					  "spawnRadius");
	REGISTER_PROPERTY(PrefabServerSpawner, radiusToCheckExistingEntities,
					  Variant::Type::FLOAT, "radiusToCheckExistingEntities");
}

void PrefabServerSpawner::_ready()
{
	PrefabServerBase::_ready();
	if (renderSphere == nullptr) {
		renderSphere = memnew(CSGSphere3D);
		renderSphere->set_use_collision(true);
		add_child(renderSphere);
		renderSphere->set_owner(this);
	}
}

void PrefabServerSpawner::_process(double dt)
{
	PrefabServerBase::_process(dt);
	renderSphere->set_visible(GameEditorConfig::render_triggers);
	if (GameEditorConfig::render_triggers) {
		renderSphere->set_radius(0.5);
	}
}
} // namespace editor
