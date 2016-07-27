struct Particle
{
	float3 Position;
	float3 Velocity;
	float3 StartPosition;
};

struct Pointer {
	float x;
	float y;
};

cbuffer Handler : register(b0)
{
	int GroupDim;
	uint MaxParticles;
	float DeltaTime;
	float3 Attractor;

	float Sensivity;
	float2 Pointers[20];
	int PointersCount;
};

RWStructuredBuffer<Particle> Particles : register(u0);


#define THREAD_GROUP_X 32
#define THREAD_GROUP_Y 24
#define THREAD_GROUP_TOTAL 768


float3 _calculate(float3 anchor, float3 position)
{
	float3 direction = anchor - position;
	float distance = length(direction);
	direction /= distance;

	return direction * max(0.01, (1 / (distance*distance)));
}

[numthreads(THREAD_GROUP_X, THREAD_GROUP_Y, 1)]
void DefaultCS(uint3 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
	uint index = groupID.x * THREAD_GROUP_TOTAL + groupID.y * GroupDim * THREAD_GROUP_TOTAL + groupIndex;

	[flatten]
	if (index >= MaxParticles)
		return;

	Particle particle = Particles[index];

	float theta = 0;
	float distance = 0;
	float _maxDist = 0;
	float _maxTheta = 0;

	float sum = 0;
	float thetaSum = 0;

	for (int i = 0; i < PointersCount; i++) {

		float2 _mouse = Pointers[i];

		_mouse.x -= 0.5;
		_mouse.y += 0.5;

		_mouse.y *= -0.7;

		_mouse.x *= 350;
		_mouse.y *= 170;

		theta = atan2(particle.Position.y - _mouse.y, particle.Position.x - _mouse.x);
		distance = Sensivity;
		distance = distance * 10 / sqrt((_mouse.x - particle.Position.x)*(_mouse.x - particle.Position.x) + (_mouse.y - particle.Position.y)*(_mouse.y - particle.Position.y));

		//sum += distance;
		//thetaSum += theta;

		//if (Sensivity < 0)
		//{
		//	if (-distance > _maxDist)
		//	{
		//		_maxDist = -distance;
		//		_maxTheta = theta;
		//	}
		//	theta = _maxTheta;
		//	distance = -_maxDist;
		//}
		//else
		//{
		//	if (distance > _maxDist)
		//	{
		//		_maxDist = distance;
		//		_maxTheta = theta;
		//	}
		//	theta = _maxTheta;
		//	distance = _maxDist;
		//}

		/*particle.Position.x += (cos(theta)*distance + (particle.StartPosition.x - particle.Position.x)*0.05);
		particle.Position.y += (sin(theta)*distance + (particle.StartPosition.y - particle.Position.y)*0.05);*/
	}

	/*particle.Position.x += (cos(thetaSum)*sum + (particle.StartPosition.x - particle.Position.x)*0.05);
	particle.Position.y += (sin(thetaSum)*sum + (particle.StartPosition.y - particle.Position.y)*0.05);*/

	particle.Position.x += (cos(theta)*distance + (particle.StartPosition.x - particle.Position.x)*0.05);
	particle.Position.y += (sin(theta)*distance + (particle.StartPosition.y - particle.Position.y)*0.05);

	particle.Position.x -= 0.01;
	
	Particles[index] = particle;
}

//technique ParticleSolver
//{
//	pass DefaultPass
//	{
//
//		Profile = 10.0;
//		ComputeShader = DefaultCS;
//
//	}
//}


technique11 ParticleSolver {
	pass DefaultPass {
		
		SetComputeShader(CompileShader(cs_5_0, DefaultCS()));
	}
}
