#pragma once

#include "PrefabServerStaticMesh_Base.hpp"

namespace editor
{
class PrefabServerStaticMesh final : public PrefabServerStaticMesh_Base
{
	GDCLASS(PrefabServerStaticMesh, PrefabServerStaticMesh_Base)
public: // Godot bound functions
	PrefabServerStaticMesh();
	virtual ~PrefabServerStaticMesh();
	static void _bind_methods();

public:
	bool isTrullyStaticForMerge = true;
	inline bool get_isTrullyStaticForMerge() { return isTrullyStaticForMerge; }
	inline void set_isTrullyStaticForMerge(bool v)
	{
		isTrullyStaticForMerge = v;
	}
};
} // namespace editor
