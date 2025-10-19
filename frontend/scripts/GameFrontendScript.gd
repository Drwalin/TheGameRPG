extends GameFrontend;
class_name GameFrontendScript;

var enablePointShadows:bool = true;

var camera:Camera3D;

var SENSITIVITY:float = 0.01;
var CAMERA_KEY_SENSITIVITY:float = 1.0;
var SPEED_UP_CAMERA_KEY_SENSITIVITY_FACTOR:float = 3.0;

var animationTree:AnimationTree;
var isInHud:bool = false;

var renderPoints:MultiMeshInstance3D = preload("res://scripts/vfx/RenderPoints.tscn").instantiate();
var renderPointsMesh:MultiMesh = renderPoints.multimesh;

func _ready():
	InternalReady();
	camera = GetPlayerCamera();
	GetNodeToAddEntities().get_parent().add_child(renderPoints);

func Rotate(x: float, y: float)->void:
	var rot = GetPlayerRotation();
	rot.y += y;
	rot.x += x;
	if rot.x < -3.141592*0.5:
		rot.x = -3.141592*0.5
	if rot.x > 3.141592*0.5:
		rot.x = 3.141592*0.5
	SetPlayerRotation(rot);

func _input(event)->void:
	if event is InputEventMouseMotion:
		if Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
			Rotate(-event.relative.y * SENSITIVITY, -event.relative.x * SENSITIVITY);
	elif event is InputEventMouseButton:
		if event.button_index == MOUSE_BUTTON_RIGHT:
			if event.pressed:
				if event.shift_pressed == false:
					Input.action_press("attack_secondary");
			elif Input.is_action_pressed("attack_secondary"):
				Input.action_release("attack_secondary");

func IsDownOrJustReleased(action: StringName)->bool:
	return (Input.is_action_just_released(action) || Input.is_action_pressed(action)) && !Input.is_action_just_pressed(action);

func DrawTestRay(start:Vector3, end:Vector3):
	var res:Array = RayTest(start, end, FILTER_STATIC_OBJECT|FILTER_TERRAIN|FILTER_CHARACTER, true);
	if res.size() > 0:
		#print(res);
		if (res[2] < 0.00001):
			return;
		var pos:int = randi_range(0, renderPointsMesh.instance_count-1);
		renderPointsMesh.set_instance_transform(pos, Transform3D(Basis(), res[3]));
		renderPointsMesh.visible_instance_count = renderPointsMesh.instance_count;

func RandVec()->Vector3:
	return Vector3(randf_range(-20,20),randf_range(-30,20),randf_range(-20,20));

func TestSomeRandomRays():
	for i in range(0,20):
		DrawTestRay(RandVec(), RandVec());

func _process(dt: float)->void:
	InternalProcess();
	
	TestSomeRandomRays();
	
	if Input.is_mouse_button_pressed(MOUSE_BUTTON_LEFT):
		var camPos:Vector3 = GetCameraPosition();
		var camDir:Vector3 = GetCameraDirectionLook();
		var end = camPos + (camDir * 1000.0);
		DrawTestRay(camPos, end);
	
	if IsInPlayerControl() == false:
		return;
	
	var cameraDeltaRotation:Vector2 = Vector2(0,0);
	if IsDownOrJustReleased("camera_left"):
		cameraDeltaRotation.x -= 1;
	if IsDownOrJustReleased("camera_right"):
		cameraDeltaRotation.x += 1;
	if IsDownOrJustReleased("camera_up"):
		cameraDeltaRotation.y -= 1;
	if IsDownOrJustReleased("camera_down"):
		cameraDeltaRotation.y += 1;
	cameraDeltaRotation *= dt;
	cameraDeltaRotation *= CAMERA_KEY_SENSITIVITY;
	if IsDownOrJustReleased("camera_key_rotation_boost"):
		cameraDeltaRotation *= SPEED_UP_CAMERA_KEY_SENSITIVITY_FACTOR;
	Rotate(-cameraDeltaRotation.y, -cameraDeltaRotation.x);
	
	var d:Vector3;
	var rot:Vector3;
	if camera != null:
		var mov:Vector3 = Vector3.ZERO;
		var dir = Input.get_vector("movement_left", "movement_right", "movement_forward", "movement_backward");
		if dir.length_squared() > 0.01:
			d = Vector3(dir.x, 0, dir.y).normalized();
			rot = GetPlayerRotation();
			mov += d.rotated(Vector3(0,1,0), rot.y+PI);
		if Input.is_action_pressed("movement_jump"):
			PlayerTryJump();
		SetPlayerDirectionMovement(Vector2(mov.x, mov.z));
	else:
		SetPlayerDirectionMovement(Vector2(0,0));
	
	var pos = gameFrontend.GetPlayerPosition();
	rot = gameFrontend.GetPlayerRotation();
	camera.set_rotation(rot + Vector3(0, PI, 0));
	var height = gameFrontend.GetPlayerHeight();
	camera.position = pos + Vector3(0, height, 0);
	
	if isInHud:
		if Input.is_action_just_pressed("input_use"):
			gameFrontend.PerformInteractionUse();
		
		if Input.is_action_just_pressed("attack_primary", true):
			gameFrontend.PerformAttack(0, 0, 0);
		if Input.is_action_just_pressed("attack_primary_powerfull", true):
			gameFrontend.PerformAttack(0, 1, 0);
		if Input.is_action_just_pressed("attack_secondary", true):
			gameFrontend.PerformAttack(0, 2, 0);
		if Input.is_action_just_pressed("attack_secondary_powerfull", true):
			gameFrontend.PerformAttack(0, 3, 0);
		
		if Input.is_action_just_released("attack_primary", true):
			gameFrontend.PerformAttack(0, 0, 1);
		if Input.is_action_just_released("attack_primary_powerfull", true):
			gameFrontend.PerformAttack(0, 1, 1);
		if Input.is_action_just_released("attack_secondary", true):
			gameFrontend.PerformAttack(0, 2, 1);
		if Input.is_action_just_released("attack_secondary_powerfull", true):
			gameFrontend.PerformAttack(0, 3, 1);
		
		for i in range(0,128):
			if Input.is_action_just_pressed("skill_%d" % i):
				gameFrontend.PerformAttack(1, i, 0);
