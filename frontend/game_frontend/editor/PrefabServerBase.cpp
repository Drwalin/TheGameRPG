#include "PrefabServerBase.hpp"

namespace editor
{
PrefabServerBase::PrefabServerBase() {}
PrefabServerBase::~PrefabServerBase() {}
void PrefabServerBase::Serialize(uint16_t higherLevelComponentsCount,
									   icon7::ByteWriter &writer)
{
	writer.op(higherLevelComponentsCount);
}
}
