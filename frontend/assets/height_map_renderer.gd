@tool
extends Node3D

var counter:int = 0;

@export var width:int;
@export var depth:int;
var combinedImages:Image;
var combinedTexture: ImageTexture;
var oldWidth:int = 0;
var oldDepth:int = 0;
@export var noise:FastNoiseLite = FastNoiseLite.new();
var scaleSize:Vector3 = Vector3(1,1,1);

func Update(_heights:PackedByteArray, _materials:PackedByteArray, _width: int, _depth: int, _scaleSize:Vector3):
	width = _width;
	depth = _depth;
	scaleSize = _scaleSize;
	combinedImages = Image.create(width, depth, false, Image.Format.FORMAT_RGBA8);
	for i in range(0, width*depth):
		var r:float = _heights[i*2+0] / 255.0;
		var g:float = _heights[i*2+1] / 255.0;
		var b:float = _materials[i] / 255.0;
		combinedImages.set_pixel(i % width, i / width, Color(r, g, b, 1));
	Rebuild();

func Rebuild():
	var start = Time.get_ticks_usec();
	print(width, " x ", depth)
	if (oldWidth != width || oldDepth != depth):
		var mesh:ArrayMesh = ArrayMesh.new();
		var arrays = [];
		arrays.resize(Mesh.ARRAY_MAX);
		var verts:PackedVector2Array;
		verts.resize(width*depth*2*3);
		arrays[Mesh.ARRAY_VERTEX] = verts;
		mesh.add_surface_from_arrays(
					Mesh.PrimitiveType.PRIMITIVE_TRIANGLES,
					arrays,
					[],
					{},
					0
					| Mesh.ARRAY_FORMAT_VERTEX
					| Mesh.ARRAY_FLAG_USE_2D_VERTICES
					#| Mesh.ARRAY_FORMAT_NORMAL
					#| Mesh.ARRAY_FLAG_USES_EMPTY_VERTEX_ARRAY
					#| Mesh.ARRAY_FORMAT_TEX_UV
					);
		#mesh.surface_set_material(0, $MeshInstance3D.material_override);
		mesh.custom_aabb = AABB(Vector3(0,0,0), Vector3(width*scaleSize.x+1,scaleSize.y+1,depth*scaleSize.z+1));
		$MeshInstance3D.mesh = mesh;
		combinedTexture = ImageTexture.create_from_image(combinedImages);
		oldWidth = width;
		oldDepth = depth;
		$MeshInstance3D.position = -(Vector3(width,0,depth) * scaleSize) / 2.0;
		$MeshInstance3D.scale = Vector3(1,1,1);
	else:
		combinedTexture.update(combinedImages);
		
	var material = $MeshInstance3D.material_override;
	var shadMat:ShaderMaterial = material;
	shadMat.set_shader_parameter("terrainData", combinedTexture);
	shadMat.set_shader_parameter("size", Vector2i(width, depth));
	shadMat.set_shader_parameter("materialColorsRamp", 0);
	shadMat.set_shader_parameter("scaleSize", scaleSize);
	var end = Time.get_ticks_usec();
	var duration = (end - start) / 1000000.0;
	print("Generating: ", duration, " sec");
	

func _process(delta):
	counter = counter + 1;
	if (counter % 60 != 7):
		return;
	var w = 64;
	var d = 64;
	var m:PackedByteArray;
	var h:PackedByteArray;
	h.resize(w*d*2);
	m.resize(w*d);
	for i in range(0, w*d):
		var v = (noise.get_noise_2d(i%w, i/w) + 0.5) / 1.0;
		h[i*2+0] = v*255.0;
		h[i*2+1] = v * 255.0 * 255.0 - int(v*255.0)*255.0;
		m[i] = noise.get_noise_3d(i%w, 123, i/w) * 1155.0;
	Update(h, m, w, d, Vector3(2, 1.0, 2));
