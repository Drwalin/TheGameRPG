@tool
extends Node3D;

@export var runCode:bool = false;
@export_file("*.tscn", "*.scn") var pathToSavePrefab;
@export_file("*.tres", "*.res") var pathToSaveAnimations;
@export_file("*.tres", "*.res") var pathToSaveMesh;

var rootNode:Node3D = null;
var animationLibrary:AnimationLibrary = null;

func _process(delta):
	if runCode:
		print("Merging files");
		runCode = false;
		# find model
		for c in get_children():
			if c.scene_file_path != "":
				if FindChildNode(c, "MeshInstance3D") != null:
					rootNode = c.duplicate();
					var m:MeshInstance3D = FindChildNode(rootNode, "MeshInstance3D");
					m.mesh.surface_get_material(0).resource_local_to_scene = true;
					ResourceSaver.save(m.mesh, pathToSaveMesh);
					m.mesh = ResourceLoader.load(pathToSaveMesh);
					m.mesh.surface_get_material(0).resource_local_to_scene = true;
					break;
		
		animationLibrary = AnimationLibrary.new();
		for c in get_children():
			if c.scene_file_path != "":
				var a = FindChildNode(c, "AnimationPlayer");
				if a != null:
					ProcessAnimationPlayer(c, a);
		
		ResourceSaver.save(animationLibrary, pathToSaveAnimations);
		
		
		
		var animationPlayer:AnimationPlayer = AnimationPlayer.new();
		animationPlayer.add_animation_library("", animationLibrary);
		animationPlayer.root_node = NodePath(rootNode.name);
		animationPlayer.clear_caches();
		rootNode.add_child(animationPlayer);
		
		var packedScene:PackedScene = PackedScene.new();
		packedScene.pack(rootNode);
		ResourceSaver.save(packedScene, pathToSavePrefab);
		rootNode.free();
		rootNode = null;
		
		#animationLibrary.free();
		animationLibrary = null;

static func GetAllNodes(node:Node)->Array[Node]:
	var nodes:Array[Node] = node.get_children();
	var i:int = 0;
	while i < nodes.size():
		var c = nodes[i].get_children();
		nodes.append_array(c);
		i = i+1;
	return nodes;

static func FindNode(nodes:Array[Node], typeName:String)->Node:
	for n in nodes:
		if n.is_class(typeName):
			return n;
	return null;

static func FindChildNode(rootNode:Node, typeName:String)->Node:
	var nodes:Array[Node] = GetAllNodes(rootNode);
	for n in nodes:
		if n.is_class(typeName):
			return n;
	return null;

static func Max3(a:Vector3, b:Vector3)->Vector3:
	return Vector3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));

static func Min3(a:Vector3, b:Vector3)->Vector3:
	return Vector3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));

func ProcessAnimationPlayer(node:Node, animPlayer:AnimationPlayer)->void:
	var animNames = animPlayer.get_animation_list();
	if animNames.size() > 0:
		var animation:Animation = animPlayer.get_animation(animNames[0]).duplicate(true);
		var elementName = GetElementName(node.scene_file_path);
		animation.resource_name = elementName;
		print("Adding animation ", elementName, "     -> ", animation);
		var trackId:int = animation.find_track("RootNode/Armature", Animation.TYPE_POSITION_3D);
		
		var keyCount:int = animation.track_get_key_count(trackId);
		var maxPos:Vector3 = Vector3.ZERO;
		var minPos:Vector3 = Vector3.ZERO;
		var maxTotalPos:Vector3 = Vector3.ZERO;
		for i in range(0, keyCount):
			var t:float = animation.track_get_key_time(trackId, i);
			var v:Vector3 = animation.position_track_interpolate(trackId, t);
			maxTotalPos = Max3(maxTotalPos, abs(v));
			maxPos = Max3(maxPos, v);
			minPos = Min3(minPos, v);
		
		var axis:int = 0;
		for i in range(0, 3):
			if maxTotalPos[i] > maxTotalPos[axis]:
				axis = i;
		
		var mainScale:Vector3 = node.get_child(0).get_child(0).scale;
		
		animation.track_set_path(animation.find_track("RootNode/Armature", Animation.TYPE_POSITION_3D), "RootNode/Armature/Skeleton3D");
		animation.track_set_path(animation.find_track("RootNode/Armature", Animation.TYPE_ROTATION_3D), "RootNode/Armature/Skeleton3D");
		
		print("Max axis: ", axis);
		if maxTotalPos[axis] > 0.6:
			var rootMotionId:int = animation.add_track(Animation.TYPE_POSITION_3D, 0);
			animation.track_set_path(rootMotionId, "RootNode/Armature");
			trackId = animation.find_track("RootNode/Armature/Skeleton3D", Animation.TYPE_POSITION_3D);
			
			var duration:float = animation.track_get_key_time(trackId, keyCount-1);
			var startPos:Vector3 = animation.position_track_interpolate(trackId, 0);
			var endPos:Vector3 = animation.position_track_interpolate(trackId, duration);
			
			for i in range(0, keyCount):
				var t:float = animation.track_get_key_time(trackId, i);
				var v:Vector3 = animation.position_track_interpolate(trackId, t);
				
				var mov:Vector3 = (endPos - startPos) * t / duration;
				
				var v2:Vector3 = mov;
				v2 /= mainScale;
				animation.position_track_insert_key(rootMotionId, t, v2);
				
				v -= mov;
				v /= mainScale;
				animation.position_track_insert_key(trackId, t, v);
		else:
			trackId = animation.find_track("RootNode/Armature/Skeleton3D", Animation.TYPE_POSITION_3D);
			for i in range(0, keyCount):
				var t:float = animation.track_get_key_time(trackId, i);
				var v:Vector3 = animation.position_track_interpolate(trackId, t);
				v /= mainScale;
				animation.position_track_insert_key(trackId, t, v);
		
		animationLibrary.add_animation(elementName, animation);

func GetElementName(path:String)->String:
	var a:int = path.rfind("/");
	if a < 0:
		a = 0;
	else:
		a = a+1;
	var b:int = path.rfind(".");
	if b < 0:
		b = path.length();
	var name:String = path.substr(a, b-a);
	name = name.replace(" ", "_");
	return name;
