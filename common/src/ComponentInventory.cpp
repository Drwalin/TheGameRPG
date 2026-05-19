#include "../include/RegistryComponent.inl.hpp"

#include "../include/ComponentInventory.hpp"

int RegisterEntityComponentsInventory(flecs::world &ecs)
{
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentInventory, "INV");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentEquippedInventory, "EQINV");
	return 0;
}

ItemBase::ItemBase() {}
ItemBase::~ItemBase() {}

void ItemStack::serialize(icon7::ByteWriterBase &s) const
{
	if (item) {
		s.op(item->uniqueNameShort);
		s.op(amount);
		s.op(tag);
	}
}
void ItemStack::serialize(icon7::ByteReaderBase &s)
{
	std::string name;
	s.op(name);
	if (name != "") {
		item = ItemBase::GetByName(name);
		s.op(amount);
		s.op(tag);
		if (item == nullptr) {
			amount = 0;
			tag = "";
		}
	}
}

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentInventory,
											{ s.op(items); });

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentEquippedInventory, {
	s.op(equippedItems, NUMBER_OF_SLOTS);
});

void ItemBase::SerializeClientInfo(icon7::ByteWriterBase &s)
{
	s.op(uniqueNameShort);
	s.op(maxStackSize);
	s.op(fullName);
	s.op(description);
	s.op(modelName);
	s.op(usableItemSlots);
}
