@tool
extends Node3D

@export var meshes:Array[PackedScene] = [];
@export var execute:bool = false;

@export var scaleMin:float = 1.0;
@export var scaleMax:float = 2.0;

var rayCast:RayCast3D;

func _ready()->void:
	rayCast = RayCast3D.new();
	add_child(rayCast, false, Node.INTERNAL_MODE_BACK);
	rayCast.owner = self;
	rayCast.hit_back_faces = true;
	rayCast.hit_from_inside = true;
	rayCast.enabled = false;
	rayCast.visible = false;

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

func IgnoreNodeCollision(node:Node3D):
	for c in node.get_children(true):
		IgnoreNodeCollision(c);
	if node is CollisionObject3D:
		rayCast.add_exception(node);

func DoRayCast(node:PrefabServerStaticMesh)->bool:
	rayCast.clear_exceptions();
	IgnoreNodeCollision(node);
	rayCast.global_transform = Transform3D(Basis(), node.get_global_transform().origin + Vector3(0, 0.5, 0));
	rayCast.target_position = Vector3(0, -1000, 0);
	rayCast.force_raycast_update();
	return rayCast.is_colliding();

func RandomizeSingleNode(node:PrefabServerStaticMesh):
	node.rotation = Vector3(0,0,0);
	node.rotate(Vector3(0,1,0), randf()*2.0*PI);
	
	if DoRayCast(node):
		var rot:Quaternion = Quaternion(Vector3(0,1,0), rayCast.get_collision_normal());
		node.basis = Basis(rot) * node.basis;
	
	var s:float = (scaleMax-scaleMin)*randf() + scaleMin;
	node.scale = Vector3(s, s, s);
	var mesh = meshes[randi()%meshes.size()].instantiate();
	node.graphic_Mesh_or_PackedScene = mesh.graphic_Mesh_or_PackedScene;
	node.collision_mesh = mesh.collision_mesh;
	mesh.queue_free();
