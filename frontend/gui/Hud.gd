extends Control;

var mouseCapturedPosition:Vector2;
var isMouseCapturedInHUD:bool = true;

func _ready():
	pass;

func _process(_dt: float)->void:
	gameFrontend.isInHud = visible;
	
	var targetMode = Input.MOUSE_MODE_CAPTURED;
	if !visible || !isMouseCapturedInHUD:
		targetMode = Input.MOUSE_MODE_VISIBLE;
	
	if Input.mouse_mode != targetMode:
		if targetMode == Input.MOUSE_MODE_CAPTURED:
			mouseCapturedPosition = get_viewport().get_mouse_position();
			Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED);
		elif targetMode == Input.MOUSE_MODE_VISIBLE:
			Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE);
			get_viewport().warp_mouse(mouseCapturedPosition);
	
	var csText:String = "";
	var cs:Dictionary = gameFrontend.GetCharacterSheet();
	for s in cs:
		csText = "%s%s: %s\n" % [csText, s, cs[s]];
	$CharacterSheet.text = csText;
