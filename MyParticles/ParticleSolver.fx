struct Particle
{
	float3 Position;
	float3 Color;
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
	float Sensivity;
	int PointersCount;
};
StructuredBuffer<Particle> ParticlesIN : register(t0);
StructuredBuffer<Pointer> Pointers : register(t1);
RWStructuredBuffer<Particle> ParticlesOUT : register(u0);

#define THREAD_GROUP_X 32
#define THREAD_GROUP_Y 24
#define THREAD_GROUP_TOTAL 768

[numthreads(1, 1, 1)]
void DefaultCS(uint3 DTid : SV_DispatchThreadID)
{
	uint index = DTid.x;
	[flatten]
	if (index >= MaxParticles)
		return;

	Particle particle = ParticlesIN[index];
	float3 p = particle.Position;
	float theta = 0;
	float distance = 0;
	p.x += (cos(theta)*distance + (particle.StartPosition.x - p.x)*0.05);
	p.y += (sin(theta)*distance + (particle.StartPosition.y - p.y)*0.05);
	for (int j = 0; j < PointersCount; j++) {
		theta = atan2(p.y - Pointers[j].y, p.x - Pointers[j].x);
		distance = Sensivity * 0.001 / sqrt((Pointers[j].x - p.x)*(Pointers[j].x - p.x)
			+ (Pointers[j].y - p.y)*(Pointers[j].y - p.y));
		p.x += (cos(theta)*distance + (particle.StartPosition.x - p.x)*0.05);
		p.y += (sin(theta)*distance + (particle.StartPosition.y - p.y)*0.05);
	}
	particle.Position = p;
	ParticlesOUT[index] = particle;
}

technique11 ParticleSolver {
	pass DefaultPass {
		SetComputeShader(CompileShader(cs_5_0, DefaultCS()));
	}
}
