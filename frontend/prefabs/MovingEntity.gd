extends EntityPrefabScript;

func _process(dt: float)->void:
	_my_internal_process(dt);
	UpdateAnimationState();

func UpdateAnimationState()->void:
	var onGround = GetOnGround();
	var vel3:Vector3 = GetVelocity();
	var vel2:Vector2 = Vector2(vel3.x, vel3.z);
	
	var rot = GetRotation();
	vel2 = vel2.rotated(rot.y);
	vel2.x = -vel2.x;
	
	var animationTree:AnimationTree = GetAnimationTree();
	if animationTree != null:
		animationTree["parameters/conditions/falling"] = !onGround;
		animationTree["parameters/conditions/onGround"] = onGround;
		animationTree["parameters/FlatLocomotion/blend_position"] = vel2;
