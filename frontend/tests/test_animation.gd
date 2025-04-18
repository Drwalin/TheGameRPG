@tool
extends Node3D

@export var texture:Texture2D;

# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.

static var BLOCK = 0;

var animtex:ImageTexture;

func _Col(v:Vector3)->Color:
	return Color(v.x, v.y, v.z);

func _process(_delta):
	#process_image();
	
	
	if animtex == null:
		var img:Image = Image.create(64, 64, false, Image.FORMAT_RGBF);
		
		var bones:Array[Transform3D] = [] as Array[Transform3D];
		var sk:Skeleton3D = $RootNode/Armature/Skeleton3D;
		
		bones.resize(sk.get_bone_count());
		
		for i in range(0, sk.get_bone_count()):
			#bones[i] = sk.get_bone_global_rest(i).affine_inverse() * sk.get_bone_global_pose(i);
			bones[i] = sk.get_bone_global_pose(i) * sk.get_bone_global_rest(i).affine_inverse();
			var t:Transform3D = bones[i];
			var basis:Basis = t.basis;
			var origin:Vector3 = t.origin;
			var m:Array[Color];
			m.resize(4);
			
			m[0] = _Col(basis.x);
			m[1] = _Col(basis.y);
			m[2] = _Col(basis.z);
			m[3] = _Col(origin);
			
			for j in range(4):
				img.set_pixel(i, j, m[j]);
		animtex = ImageTexture.create_from_image(img);
		
	
	
	$ciry_dwellers_2.custom_aabb = AABB(Vector3(-100,-100,-100), Vector3(100,100,100));
	
	
	var mat:ShaderMaterial = null;
	
	if $ciry_dwellers_2.mesh == null:
		var m:ArrayMesh = ArrayMesh.new();
		var ar = $RootNode/Armature/Skeleton3D/ciry_dwellers_1.mesh.surface_get_arrays(0).duplicate();
		var w = ar[Mesh.ARRAY_WEIGHTS].duplicate();
		var b = ar[Mesh.ARRAY_BONES].duplicate();
		
		
		
		ar[Mesh.ARRAY_TANGENT] = null;
		ar[Mesh.ARRAY_WEIGHTS] = null;
		ar[Mesh.ARRAY_BONES] = null;
		
		var bbbones:PackedByteArray;
		var weigbones:PackedByteArray;
		bbbones.resize(b.size()/2);
		weigbones.resize(w.size()/2);
		for i in range(0, bbbones.size()):
			bbbones[i] = b[(i&3) + ((i>>2)<<3)];
			weigbones[i] = w[(i&3) + ((i>>2)<<3)] * 255.0;
		ar[Mesh.ARRAY_CUSTOM0] = bbbones;
		ar[Mesh.ARRAY_CUSTOM1] = weigbones;
		
		
		
		
		m.clear_surfaces();
		m.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, ar, [], {},
			Mesh.ARRAY_FORMAT_VERTEX |
			Mesh.ARRAY_FORMAT_COLOR |
			Mesh.ARRAY_FORMAT_TEX_UV |
			Mesh.ARRAY_FORMAT_NORMAL |
			Mesh.ARRAY_FLAG_USE_DYNAMIC_UPDATE |
			Mesh.ARRAY_FORMAT_CUSTOM0 | (Mesh.ARRAY_CUSTOM_RGBA8_UNORM<<Mesh.ARRAY_FORMAT_CUSTOM0_SHIFT) |
			Mesh.ARRAY_FORMAT_CUSTOM1 | (Mesh.ARRAY_CUSTOM_RGBA8_UNORM<<Mesh.ARRAY_FORMAT_CUSTOM1_SHIFT) |
			Mesh.ARRAY_FORMAT_INDEX
			);
	
		mat = ShaderMaterial.new();
		mat.set_shader(load("res://tests/test_animation.gdshader"));
		
		m.surface_set_material(0, mat);
		$ciry_dwellers_2.mesh = m;
		animtex = null;
	else:
		mat = $ciry_dwellers_2.mesh.surface_get_material(0);
		
	
	#mat.set_shader_parameter("bbones", bones);
	#mat.set_shader_parameter("bonesCount", bones.size());
	mat.set_shader_parameter("tex", texture);
	mat.set_shader_parameter("animtex", animtex);
	
	
	var STOP = 6;
	
	if (BLOCK != STOP):
		var sk:Skeleton3D = $RootNode/Armature/Skeleton3D;
		var ar = $RootNode/Armature/Skeleton3D/ciry_dwellers_1.mesh.surface_get_arrays(0).duplicate();
		var w = ar[Mesh.ARRAY_WEIGHTS].duplicate();
		var b = ar[Mesh.ARRAY_BONES].duplicate();
		
		b.sort();
		w.sort();
		var ids = {};
		var ws = {};
		
		for B in b:
			ids[B] = 1;
		for W in w:
			ws[int(W*100.0)] = 1;
	
		
		####################################################
		BLOCK = STOP;
		var s:String;
		for B in ids.keys():
			s = "%s %s" %[ s, B];
		print("IDS:", s);
		print("IDS min: ", ids.keys().min(), "    max: ", ids.keys().max());
		
		s = "";
		for W in ws.keys():
			s = "%s %s" %[ s, W];
		print("WS:", s);
		print("WS min: ", ws.keys().min(), "    max: ", ws.keys().max());
		
		print("Bones count = ", sk.get_bone_count());









func getValue(x:int, y:int, img:Image)->int:
	var value:int = 0;
	for i in range(0, 8):
		for j in range(0, 4):
			var px:Color = img.get_pixel(x+i, y+j);
			if px.r != px.g || px.r != px.b:
				return -1;
			if px.r > 0.99 && px.g > 0.99 && px.b > 0.99:
				var bit:int = (i + (j*8));
				value |= (1 << bit);
			elif px.r > 0.01 || px.g > 0.01 && px.b > 0.01:
				return -1;
	#if value & (1<<15):
	#	value |= (-1) ^ (0xFFFF);
	#value /= 8;
	return value %41;

@export var executeTest:bool = false;
var cntr_:int = 90;
func process_image():
	cntr_ += 1;
	if (cntr_ > 100):
		cntr_ = 0;
		if !Engine.is_editor_hint():
			executeTest = true;
	if executeTest:
		executeTest = false;
	else:
		return;
	
	var img:Image = get_viewport().get_texture().get_image();
	
	var size:Vector2i = Vector2i(img.get_width(), img.get_height());
	
	print(size);
	
	var uniq:Dictionary = {};
	
	for x in range(0, size.x-8, 8):
		for y in range(0, size.y-12, 4):
			var v:int = getValue(x, y, img);
			if (v == -1):
				continue;
			if (v != getValue(x, y+4, img)):
				continue;
			if (v != getValue(x, y+8, img)):
				continue;
			if (v != getValue(x+8, y, img)):
				continue;
			if (v != getValue(x+8, y+4, img)):
				continue;
			if (v != getValue(x+8, y+8, img)):
				continue;
			uniq[v] = 1;
	
	var s:String = "";
	
	var keys:Array = uniq.keys();
	keys.sort()
	
	for v in keys:
		if s == "":
			s = "%d" % [v];
		else:
			s = "%s, %d" % [s, v];
	
	print("");
	print("");
	print("");
	print("Values: ", s);
	print("");
	print("");
	print("");
	
	pass;
