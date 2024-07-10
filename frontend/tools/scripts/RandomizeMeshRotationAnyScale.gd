@tool
extends Node3D

@export var meshes:Array[PackedScene] = [];
@export var execute:bool = false;

@export var scaleMin:float = 1.0;
@export var scaleMax:float = 2.0;

func _process(delta):
	if execute:
		print("_process Randomizing");
		execute = false;
		RandomizeNodes(self);

func RandomizeNodes(node:Node3D):
	for n in node.get_children():
		if n is PrefabServerStaticMesh:
			RandomizeSingleNode(n);
		else:
			RandomizeNodes(n);

func RandomizeSingleNode(node:PrefabServerStaticMesh):
	node.rotate(node.basis.get_rotation_quaternion()*Vector3(1,0,0), PI*2.0*randf());
	node.rotate(node.basis.get_rotation_quaternion()*Vector3(0,1,0), PI*2.0*randf());
	node.rotate(node.basis.get_rotation_quaternion()*Vector3(0,0,1), PI*2.0*randf());
	var s:float = (scaleMax-scaleMin)*randf() + scaleMin;
	node.scale = Vector3(s, s, s);
	var mesh = meshes[randi()%meshes.size()].instantiate();
	node.graphic_Mesh_or_PackedScene = mesh.graphic_Mesh_or_PackedScene;
	node.collision_mesh = mesh.collision_mesh;
	mesh.queue_free();
