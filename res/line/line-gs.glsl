#version 450
#include "/defines.glsl"

/** The number of sides in the bounding polygon. Must be even. */
#define N 4

#define START 0
#define END 1
#define LEFT 1
#define RIGHT -1



// Useless, but keeping it for now!
#ifdef RS_LINKEDLIST
layout(lines) in;
#else
layout(lines) in;
#endif

layout(triangle_strip, max_vertices = N) out;

in vsData {
    float pointImportance;
	vec2 prev;
	vec2 next;
	vec4 left;
	vec4 right;
} vsOut[];



out vec4 gsFragmentPosition;
out float gsFragmentImportance;

flat out float gsFragmentLineWidth;
flat out float gsFragmentDepth;
flat out float gsFragmentLayerLuminance;

flat out vec4 gsPrev;
flat out vec4 gsStart;
flat out vec4 gsEnd;
flat out vec4 gsNext;

flat out vec2 segDir;
flat out float segDist;

flat in vec2[2] segDirPreDisplacement;
flat in float[2] segDistToMouse;

uniform mat4 modelViewProjectionMatrix;

// Magic lens
uniform float lineWidth;
uniform float testSlider;
uniform vec2 lensPosition;
uniform float lensRadius;

uniform vec2 viewportSize;

uniform int numberOfTrajectories;
uniform int numberOfTimesteps;

void spawnPoint(vec4 point, int index) {


	gsFragmentDepth = 0.0f;
	gsFragmentLayerLuminance = vsOut[index].pointImportance;
	gsFragmentImportance = vsOut[index].pointImportance;


	gsFragmentPosition = point;
	gl_Position = gsFragmentPosition;

	// compute line width in fragment coordinates
	gsFragmentLineWidth = length(modelViewProjectionMatrix*vec4(0.0f, 0.0f, 0.0f, 1.0f) - modelViewProjectionMatrix*(vec4(lineWidth, 0.0f, 0.0f, 1.0f)));
		
	// consider the current aspect ratio to make sure all halos have equal width
	float aspectRatio = viewportSize.x/viewportSize.y;
	gsFragmentLineWidth *= aspectRatio;	

	EmitVertex();

}

void main() {

	gsPrev = vec4(vsOut[0].prev, 0, 1);
	gsStart = gl_in[0].gl_Position;
	gsEnd =  gl_in[1].gl_Position;
	gsNext = vec4(vsOut[1].next, 0, 1);


	segDir = segDirPreDisplacement[0];
	segDist = segDistToMouse[0];

	spawnPoint(vsOut[0].left, 0);
	spawnPoint(vsOut[0].right, 0);


	spawnPoint(vsOut[1].left, 1);
	spawnPoint(vsOut[1].right, 1);

	// Debugging Tesselation control shader

	//spawnPoint(gl_in[1].gl_Position, 1);
	//spawnPoint(gl_in[1].gl_Position, 1);

	EndPrimitive();

}