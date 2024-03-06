extends EntityPrefabScript

var SENSITIVITY:float = 0.01;

func Rotate(x: float, y: float)->void:
	var rot = GetRotation();
	rot.y += y;
	rot.x += x;
#	if rot.x < -3.141592*0.5:
#		rot.x = -3.141592*0.5
#	if rot.x > 3.141592*0.5:
#		rot.x = 3.141592*0.5
	SetRotation(rot);

var isDraggingCameraRotationButton: bool = false;
func _input(event)->void:
	if IsPlayer():
		if event is InputEventMouseButton:
			if event.button_index == MOUSE_BUTTON_MIDDLE:
				if (event.pressed) && (isDraggingCameraRotationButton == false):
					Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED);
					isDraggingCameraRotationButton = true;
				elif (!event.pressed) && (isDraggingCameraRotationButton == true):
					Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE);
					isDraggingCameraRotationButton = false;
		if event is InputEventMouseMotion:
			if isDraggingCameraRotationButton:
				Rotate(-event.relative.y * SENSITIVITY, -event.relative.x * SENSITIVITY);
		if event is InputEventKey:
			if event.keycode == KEY_ESCAPE:
				Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE);
				if OS.is_debug_build():
					get_tree().quit();

func _process(delta: float)->void:
	_my_internal_process(delta);
	var cam:Camera3D = playerCamera;
	if cam != null:
		var mov:Vector3 = Vector3.ZERO;
		var dir = Input.get_vector("movement_left", "movement_right", "movement_forward", "movement_backward");
		if dir != Vector2.ZERO:
			var d = Vector3(dir.x, 0, dir.y).normalized();
			var rot = GetRotation();
			mov += d.rotated(Vector3(0,1,0), rot.y) * (GetMovementSpeed() * delta);
		if Input.is_action_just_pressed("movement_jump"):
			Jump();
		AddInputMovement(mov);
	pass;
