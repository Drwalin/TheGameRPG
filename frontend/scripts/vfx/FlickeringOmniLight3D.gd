@tool
extends OmniLight3D

@export var movementAmplitude:float = 0.02;
@export var energyMin:float = 1;
@export var energyMax:float = 1.3;
@export var movementSpeed:float = 0.25;
@export var flickeringSpeed:float = 0.25;

var dstEnergy:float = 1;
var energyTime:float = 0;
var energyStartTime:float = 0;
var oldEnergy:float = 0;

var dstPosition:Vector3 = Vector3.ZERO;
var positionTime:float = 0;
var positionStartTime:float = 0;
var oldPosition:Vector3 = Vector3.ZERO;



func _process(delta):
	energyTime -= delta;
	positionTime -= delta;
	
	if energyTime <= 0:
		oldEnergy = light_energy;
		energyTime = randf_range(0.1, flickeringSpeed);
		energyStartTime = energyTime;
		dstEnergy = randf_range(energyMin, energyMax);
	
	var dE = (dstEnergy - oldEnergy)*(1-energyTime/energyStartTime);
	light_energy = oldEnergy + dE;
	
	
	if movementAmplitude > 0.0001:
		if positionTime <= 0:
			oldPosition = position;
			positionTime = randf_range(0.1, movementSpeed);
			positionStartTime = positionTime;
			dstPosition = Vector3(
				randf_range(-movementAmplitude, movementAmplitude),
				randf_range(-movementAmplitude, movementAmplitude),
				randf_range(-movementAmplitude, movementAmplitude));
		
		var dX = (dstPosition - oldPosition)*(1-positionTime/positionStartTime);
		position = oldPosition + dX;
