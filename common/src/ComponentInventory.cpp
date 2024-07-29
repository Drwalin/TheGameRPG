#include "../include/RegistryComponent.inl.hpp"

#include "../include/ComponentInventory.hpp"

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentInventory, "INV");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentEquippedInventory, "EQINV");

int RegisterEntityComponentsInventory(flecs::world &ecs)
{
	ecs.component<ComponentInventory>();
	ecs.component<ComponentEquippedInventory>();
	return 0;
}

ItemBase::ItemBase() {}
ItemBase::~ItemBase() {}

bitscpp::ByteReader<true> &
ItemStack::__ByteStream_op(bitscpp::ByteReader<true> &s)
{
	if (item) {
		s.op(item->uniqueNameShort);
		s.op(amount);
		s.op(tag);
	}
	return s;
}
bitscpp::ByteWriter<icon7::ByteBuffer> &
ItemStack::__ByteStream_op(bitscpp::ByteWriter<icon7::ByteBuffer> &s)
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
	return s;
}

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentInventory,
											{ s.op(items); });

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentEquippedInventory, {
	s.op(equippedItems, NUMBER_OF_SLOTS);
});

void ItemBase::SerializeClientInfo(bitscpp::ByteWriter<icon7::ByteBuffer> &s)
{
	s.op(uniqueNameShort);
	s.op(maxStackSize);
	s.op(fullName);
	s.op(description);
	s.op(modelName);
	s.op(usableItemSlots);
}
