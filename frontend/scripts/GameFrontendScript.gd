extends GameFrontend;
class_name GameFrontendScript;

var enablePointShadows:bool = true;

var camera:Camera3D;

var SENSITIVITY:float = 0.01;

var animationTree:AnimationTree;
var isInHud:bool = false;

func _ready():
	InternalReady();
	camera = GetPlayerCamera();

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

func _process(_delta: float)->void:
	InternalProcess();
	
	if IsInPlayerControl() == false:
		return;
	
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
			
		if Input.is_action_just_pressed("attack_primary"):
			gameFrontend.PerformAttack(0, 0, "", 0);
		if Input.is_action_just_pressed("attack_primary_powerfull"):
			gameFrontend.PerformAttack(0, 1, "", 0);
		if Input.is_action_just_pressed("attack_secondary"):
			gameFrontend.PerformAttack(0, 2, "", 0);
		if Input.is_action_just_pressed("attack_secondary_powerfull"):
			gameFrontend.PerformAttack(0, 3, "", 0);
		
		for i in range(0,128):
			if Input.is_action_just_pressed("skill_%d" % i):
				gameFrontend.PerformAttack(1, i, "", 0);
