#include <random>

#include "PrefabServerBase.hpp"

namespace editor
{
PrefabServerBase::PrefabServerBase() {}
PrefabServerBase::~PrefabServerBase() {}

void PrefabServerBase::ClearChildren()
{
	while (get_child_count() > 0) {
		auto child = get_child(0);
		remove_child(child);
		child->queue_free();
	}
	while (get_child_count(true) > 0) {
		auto child = get_child(0, true);
		remove_child(child);
		child->queue_free();
	}
}

void PrefabServerBase::_ready() { ClearChildren(); }

void PrefabServerBase::_process(double dt) {}

void PrefabServerBase::Serialize(uint16_t higherLevelComponentsCount,
								 icon7::ByteWriter &writer)
{
	writer.op(higherLevelComponentsCount);
}

String PrefabServerBase::GetRandomString()
{
	static std::random_device rd;
	static std::mt19937_64 mt64(rd());

	std::string str;
	str.resize(64);
	sprintf(str.data(), "Name%lX", (uint64_t)mt64());
	return String::utf8(str.c_str());
}
} // namespace editor
