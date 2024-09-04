extends GPUParticles3D

@export var autoEmit:bool = true;

func _ready()->void:
	emitting = autoEmit;
