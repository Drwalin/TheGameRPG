extends MeshInstance3D

func _ready():
	if ProjectSettings.get_setting("rendering/renderer/rendering_method") == "gl_compatibility":
		get_mesh().surface_get_material(1).emission_enabled = false;

