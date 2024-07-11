@tool
extends GPUParticles3D

func _ready():
	if ProjectSettings.get_setting("rendering/renderer/rendering_method") == "gl_compatibility":
		visible = false;
		process_mode = PROCESS_MODE_DISABLED;
	#	get_parent().remove_child.call_deferred(self);
	#	queue_free();
	else:
		visible = true;
	global_basis = Basis();

var c:int = 0;
func _process(_dt: float)->void:
	c = c + 1;
	if c > 0:
		global_basis = Basis();
		c = -100;
