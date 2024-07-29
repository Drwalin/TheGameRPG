#pragma once

#include <string>
#include <vector>

#include "ComponentsUtility.hpp"

enum EquippedItemSlotId : int16_t {
	LEFT_HAND = 0,
	RIGHT_HAND = 1,

	HEAD = 2,
	TORSO = 3,
	LEGGS = 4,
	BOOTS = 5,
	SHOULDERS = 6,
	GLOVES = 7,

	BELT = 8,
	BACK = 9,
	HIP_LEFT = 10,
	HIP_RIGHT = 11,

	FINGER_LEFT = 12,
	FINGER_RIGHT = 13,
	NECK = 14,
	WRIST_LEFT = 15,
	WRIST_RIGHT = 16,

	NUMBER_OF_SLOTS = 17,
	NONE = NUMBER_OF_SLOTS,
};

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

	std::vector<int16_t> usableItemSlots;

	void SerializeClientInfo(bitscpp::ByteWriter<icon7::ByteBuffer> &s);

	static ItemBase *GetByName(const std::string &name) { return nullptr; }
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
	ItemStack equippedItems[NUMBER_OF_SLOTS];

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentEquippedInventory, {
		for (int i = 0; i < NUMBER_OF_SLOTS; ++i) {
			MV(equippedItems[i]);
		}
	});
};
