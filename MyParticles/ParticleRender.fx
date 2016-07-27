struct Particle
{
	float3 Position;
	float3 Velocity;
	float3 StartPosition;
};

StructuredBuffer<Particle> Particles : register(t0);
Texture2D<float> ParticleTexture : register(t1);
SamplerState ParticleSampler : register(s1);

cbuffer Params : register(b0)
{
	float4x4 View;
	float4x4 Projection;
};

struct VertexInput
{
	uint VertexID : SV_VertexID;
};

struct PixelInput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
	float3 PositionTWS : TEXCOORD1;
};

struct PixelOutput
{
	float4 Color : SV_TARGET0;
};

PixelInput TriangleVS(VertexInput input)
{
	PixelInput output = (PixelInput)0;

	Particle particle = Particles[input.VertexID];
	float4 worldPosition = float4(particle.Position, 1);
	//float4 viewPosition = mul(worldPosition, View);
	float4 viewPosition = mul(worldPosition, 1);

	output.Position = viewPosition;

	output.UV = 0;

	float speedLength = length(particle.Velocity);
	//float magnitude = saturate(length(worldPosition.xyz) * 0.1);

	//output.PositionTWS = float3(lerp(float3(0.1, 0.5, 1.0), float3(1.0, 0.5, 0.1), speedLength * 0.1));
	output.PositionTWS = float3(lerp(float3(0.1, 0.5, 1.0), float3(1.0, 0.5, 0.1),0));

	return output;
}

PixelInput _offsetNprojected(PixelInput data, float2 offset, float2 uv)
{
	data.Position.xy += offset;
	//data.Position = mul(data.Position, Projection);
	data.Position = mul(data.Position, 1);
	data.UV = uv;

	return data;
}

[maxvertexcount(4)]
void TriangleGS(point PixelInput input[1], inout TriangleStream<PixelInput> stream)
{
	PixelInput pointOut = input[0];

	float size = 0.001f;

	stream.Append(_offsetNprojected(pointOut, float2(-1, -1) * size, float2(0, 0)));
	stream.Append(_offsetNprojected(pointOut, float2(-1, 1) * size, float2(0, 1)));
	stream.Append(_offsetNprojected(pointOut, float2(1, -1) * size, float2(1, 0)));
	stream.Append(_offsetNprojected(pointOut, float2(1, 1) * size, float2(1, 1)));

	stream.RestartStrip();
}

PixelOutput TrianglePS(PixelInput input)
{
	PixelOutput output = (PixelOutput)0;
	//float particle = ParticleTexture.Sample(ParticleSampler, input.UV).x * 0.3;
	float3 speedColor = input.PositionTWS;
	output.Color.r = cos(speedColor.x);
	//output.Color.g = cos(particle);
	//output.Color = float4(1, 0, 0, 1);
	return output;


}

technique11 ParticleRender
{
	pass DefaultPass
	{
		
		SetVertexShader(CompileShader(vs_5_0, TriangleVS()));
		SetGeometryShader(CompileShader(gs_5_0, TriangleGS()));
		SetPixelShader(CompileShader(ps_5_0, TrianglePS()));
	}
}

