#pragma once

#include <string>

#include <icon7/ByteBuffer.hpp>
#include <icon7/ByteReader.hpp>
#include <icon7/ByteWriter.hpp>

#include "ComponentsUtility.hpp"

class ItemBase
{
public:
	ItemBase();
	virtual ~ItemBase();

	std::string uniqueNameFull;
	std::string uniqueNameShort;
	int64_t maxStackSize = 1;

	std::string fullName;
	std::string description;
	std::string modelName;

	static ItemBase *GetByName(const std::string &name) { return nullptr; }
};

enum EquippedItemSlotId {
	LEFT_HAND = 0,
	RIGHT_HAND = 1,

	HEAD = 2,
	TORSO = 3,
	LEGGS = 4,
	BOOTS = 5,
	SHOULDERS = 6,
	HANDS = 7,

	BACK = 8,
	HIP = 9,
};

struct ItemStack {
	ItemBase *item = nullptr;
	int64_t amount = 0;
	std::string tag;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ItemStack, {MV(item) MV(amount) MV(tag)});
};





struct ComponentInventory {
	std::vector<ItemStack> items;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentInventory, MV(items));
};

struct ComponentEquippedInventory {
	std::vector<ItemStack> items;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentEquippedInventory, MV(items));
};
