extends GameFrontend;
class_name GameFrontendScript;

var camera:Camera3D;

var SENSITIVITY:float = 0.01;

var animationTree:AnimationTree;

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

var mouseCapturedPosition:Vector2;
var isDraggingCameraRotationButton: bool = false;
func _input(event)->void:
	if event is InputEventMouseButton:
		if event.button_index == MOUSE_BUTTON_RIGHT:
			if (event.pressed) && (isDraggingCameraRotationButton == false):
				mouseCapturedPosition = get_viewport().get_mouse_position();
				Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED);
				isDraggingCameraRotationButton = true;
			elif (!event.pressed) && (isDraggingCameraRotationButton == true):
				Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE);
				get_viewport().warp_mouse(mouseCapturedPosition);
				isDraggingCameraRotationButton = false;
	if event is InputEventMouseMotion:
		if isDraggingCameraRotationButton:
			Rotate(-event.relative.y * SENSITIVITY, -event.relative.x * SENSITIVITY);
	if event is InputEventKey:
		if event.keycode == KEY_ESCAPE:
			pass;

func _process(delta: float)->void:
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
	d = camera.basis * Vector3(0, 0, 0);
	var height = gameFrontend.GetPlayerHeight();
	camera.position = pos + d + Vector3(0, height, 0);
	
	
