@tool
extends Node3D

@export var meshes:Array[PackedScene] = [];
@export var execute:bool = false;

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
	if (randi()%2) == 0:
		node.rotate(node.basis.get_rotation_quaternion()*Vector3(1,0,0), PI);
	if (randi()%2) == 0:
		node.rotate(node.basis.get_rotation_quaternion()*Vector3(0,1,0), PI);
	if (randi()%2) == 0:
		node.rotate(node.basis.get_rotation_quaternion()*Vector3(0,0,1), PI);
	var mesh = meshes[randi()%meshes.size()].instantiate();
	node.graphic_Mesh_or_PackedScene = mesh.graphic_Mesh_or_PackedScene;
	node.collision_mesh = mesh.collision_mesh;
	mesh.queue_free();
