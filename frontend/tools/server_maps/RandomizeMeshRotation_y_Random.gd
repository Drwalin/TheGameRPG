@tool
extends Node3D

@export var execute:bool = false;
@export var rotateOnlyVertical:bool = false;

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
	if (!rotateOnlyVertical) || ((node.basis * Vector3(0,1,0)).y > 0.99):
		node.rotate(node.basis.get_rotation_quaternion()*Vector3(0,1,0), randf()*2.0*PI);
