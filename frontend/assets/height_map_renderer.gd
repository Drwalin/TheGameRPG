@tool
extends MeshInstance3D

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
	var start = Time.get_ticks_usec();
	width = _width;
	depth = _depth;
	scaleSize = _scaleSize;
	combinedImages = Image.create(width, depth, false, Image.Format.FORMAT_RGBA8);
	var pixels:PackedByteArray;
	pixels.resize(width*depth*4);
	
	for i in range(0, width*depth):
		pixels[(i<<2)+0] = _heights[(i<<1)+0];
		pixels[(i<<2)+1] = _heights[(i<<1)+1];
		pixels[(i<<2)+2] = _materials[i];
		pixels[(i<<2)+3] = 0;
	combinedImages.set_data(width, depth, false, Image.Format.FORMAT_RGBA8, pixels);
	Rebuild();
	var end = Time.get_ticks_usec();
	var duration = (end - start) / 1000000.0;
	print("Generating: ", duration, " sec");

func Rebuild():
	print(width, " x ", depth)
	if (oldWidth != width || oldDepth != depth):
		var arrayMesh:ArrayMesh = ArrayMesh.new();
		var arrays = [];
		arrays.resize(Mesh.ARRAY_MAX);
		var verts:PackedVector2Array;
		verts.resize(width*depth*2*3);
		arrays[Mesh.ARRAY_VERTEX] = verts;
		arrayMesh.add_surface_from_arrays(
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
		#mesh.surface_set_material(0, self.material_override);
		var ext = Vector3(width*scaleSize.x+1,scaleSize.y*2+1,depth*scaleSize.z+1) + Vector3(100, 100, 100);
		arrayMesh.custom_aabb = AABB(-ext, ext*3);
		self.mesh = arrayMesh;
		combinedTexture = ImageTexture.create_from_image(combinedImages);
		oldWidth = width;
		oldDepth = depth;
		self.scale = Vector3(1,1,1);
	else:
		combinedTexture.update(combinedImages);
	
	var shadMat:ShaderMaterial  = ShaderMaterial.new();
	var s = load("res://assets/HeightMapRenderer.gdshader");
	shadMat.shader = s;
	shadMat.set_shader_parameter("terrainData", combinedTexture);
	shadMat.set_shader_parameter("size", Vector2i(width, depth));
	shadMat.set_shader_parameter("materialColorsRamp", 0);
	shadMat.set_shader_parameter("scaleSize", scaleSize);
	self.material_override = shadMat;
	
	var start = Time.get_ticks_usec();
	imdfskfdsaf = combinedTexture.get_image();
	var end = Time.get_ticks_usec();
	var duration = (end - start);
	print("Getting image: ", duration, " us");
var imdfskfdsaf;

func _process(_delta:float):
	counter = counter + 1;
	if (counter % 660 != 7):
		return;
	var w:int = width;
	var d:int = depth;
	var m:PackedByteArray;
	var h:PackedByteArray;
	h.resize(w*d*2);
	m.resize(w*d);
	for i in range(0, w*d):
		var v = (noise.get_noise_2d(i%w, i/w) + 0.5) / 1.0;
		h[i*2+0] = v*255.0;
		h[i*2+1] = v * 255.0 * 255.0 - int(v*255.0)*255.0;
		m[i] = noise.get_noise_3d(i%w, 123, i/w) * 1155.0;
	Update(h, m, w, d, Vector3(2, 2.0, 2));
