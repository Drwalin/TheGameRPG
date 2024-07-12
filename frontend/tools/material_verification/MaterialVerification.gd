@tool
extends Node3D

@export var meshes:Array[PackedScene] = [];
@export var printMaterials:bool = false;

var dict = {};

func _ready():
	pass

func _process(delta):
	if printMaterials:
		print("");
		dict = {};
		printMaterials = false;
		PrintMaterialPtr(self);
		print("");
		for matName in dict.keys():
			var d = dict[matName];
			var mat = d["mat"];
			var str:String = ("material(%d, %d): " % [mat.get_reference_count(), d["count"]]);
			var meshes = d["path"];
			print(str, mat, "   mat path: ", mat.resource_path);
			#if meshes.size() == 1:
			#	print(str, mat, "   mat path: ", mat.resource_path, "    unique path: ", meshes[0].resource_path);
			#else:
			#	print(str, mat, "   mat path: ", mat.resource_path);
		print("");

func PrintMaterialPtr(node):
	if node is MeshInstance3D:
		for i in range(node.mesh.get_surface_count()):
			var mat:Material = node.mesh.surface_get_material(i);
			#print("material(", i, ", ", mat.get_reference_count(), "): ", mat, "    Mesh `", node.mesh.resource_path, "`");
			if !dict.has(mat):
				dict[("%s" % mat)] = {"mat": mat, "count": 1, "path":[node.mesh]};
			else:
				var str:String = ("%s" % mat);
				print("Mat str: `", str, "`");
				var d = dict[("%s" % mat)];
				d["count"] += 1;
				d["path"].append(node.mesh);
	else:
		for c in node.get_children(true):
			PrintMaterialPtr(c);
