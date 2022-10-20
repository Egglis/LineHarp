#version 450
#include "/defines.glsl"

/** The number of sides in the bounding polygon. Must be even. */
#define N 4

// Useless, but keeping it for now!
#ifdef RS_LINKEDLIST
layout(lines) in;
#else
layout(lines) in;
#endif

layout(triangle_strip, max_vertices = N) out;

in vsData {
    float pointImportance;
	vec3 prev;
	vec3 next;
	vec4 up;
	vec4 down;
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

uniform mat4 modelViewProjectionMatrix;

// Magic lens
uniform float lineWidth;
uniform float testSlider;
uniform vec2 lensPosition;
uniform float lensRadius;

uniform vec2 viewportSize;

uniform int numberOfTrajectories;
uniform int numberOfTimesteps;

void spawnPoint(vec4 S, vec3 prev, vec3 start, vec3 end, vec3 next){
	float aspectRatio = viewportSize.x/viewportSize.y;

	gsFragmentPosition = modelViewProjectionMatrix * S;

	// Sets the fragment position
	gl_Position = gsFragmentPosition;
	
#ifdef RS_LINKEDLIST

	// Only used in the fragment shader
	gsPrev = modelViewProjectionMatrix * vec4(prev,1);
	gsStart = modelViewProjectionMatrix * vec4(start,1);
	gsEnd = modelViewProjectionMatrix * vec4(end,1);
	gsNext = modelViewProjectionMatrix * vec4(next,1);

#else
	gsStart = modelViewProjectionMatrix * gl_in[0].gl_Position;
	gsEnd = modelViewProjectionMatrix * gl_in[1].gl_Position;    
#endif
	
	// compute line width in fragment coordinates
	gsFragmentLineWidth = length(modelViewProjectionMatrix*vec4(0.0f, 0.0f, 0.0f, 1.0f) - modelViewProjectionMatrix*(vec4(lineWidth, 0.0f, 0.0f, 1.0f)));
		
	// consider the current aspect ratio to make sure all halos have equal width
	gsFragmentLineWidth *= aspectRatio;	


	EmitVertex();
}


float lens(inout vec3 a, inout vec3 b) {
	float aspectRatio = viewportSize.x/viewportSize.y;
	vec4 Ma = modelViewProjectionMatrix*vec4(b,1);
	vec2 lPos = lensPosition;

	Ma.x *= aspectRatio;
	lPos.x *= aspectRatio;
	
	// Computes Distacne to vertex
	float dl = distance(lPos, Ma.xy);

	// vec2 ndCoordinates = (a.xy-viewportSize/2)/(viewportSize/2);
	// float pxlDistance = length((lensPosition-ndCoordinates) * vec2(aspectRatio, 1.0));


	if (dl <= lensRadius && dl > 0) {
		
		// Compute displacment direction
		vec2 dir = normalize(vec2(Ma.x - lPos.x, Ma.y - lPos.y));

		// Scaling is done with a cos function
		float zoomIntensity = 1;
		float x = dl/lensRadius;
		float scaling = zoomIntensity*cos(3.1415 + x*2*3.1415)+zoomIntensity;

		a += vec3(dir*scaling*testSlider,0);
		b += vec3(dir*scaling*testSlider,0);
		return lensRadius - dl;
	}

	return 0;


}


void main() {

	gsFragmentLineWidth = length(modelViewProjectionMatrix*vec4(0.0f, 0.0f, 0.0f, 1.0f) - modelViewProjectionMatrix*(vec4(lineWidth, 0.0f, 0.0f, 1.0f)));
		
	// consider the current aspect ratio to make sure all halos have equal width
	float aspectRatio = viewportSize.x/viewportSize.y;
	gsFragmentLineWidth *= aspectRatio;	



	gsPrev = modelViewProjectionMatrix * gl_in[0].gl_Position;
	gsStart = modelViewProjectionMatrix * gl_in[0].gl_Position;
	gsEnd = modelViewProjectionMatrix * gl_in[1].gl_Position;
	gsNext = modelViewProjectionMatrix * gl_in[1].gl_Position;


	gsFragmentDepth = 0.0f;
	gsFragmentLayerLuminance = vsOut[0].pointImportance;
	gsFragmentImportance = vsOut[0].pointImportance;;
	gsFragmentPosition = modelViewProjectionMatrix * vsOut[0].up;
			
	gl_Position =  gsFragmentPosition;
	EmitVertex();


	gsFragmentDepth = 0.0f;
	gsFragmentLayerLuminance = vsOut[0].pointImportance;
	gsFragmentImportance = vsOut[0].pointImportance;
	gsFragmentPosition = modelViewProjectionMatrix * vsOut[0].down;
	gl_Position =  gsFragmentPosition;
	EmitVertex();


	gsFragmentDepth = 0.0f;
	gsFragmentLayerLuminance = vsOut[1].pointImportance;
	gsFragmentImportance = vsOut[1].pointImportance;
	gsFragmentPosition = modelViewProjectionMatrix * vsOut[1].up;
	gl_Position =  gsFragmentPosition;
	EmitVertex();


	gsFragmentDepth = 0.0f;
	gsFragmentLayerLuminance = vsOut[1].pointImportance;
	gsFragmentImportance = vsOut[1].pointImportance;
	gsFragmentPosition = modelViewProjectionMatrix * vsOut[1].up;
	gl_Position =  gsFragmentPosition;
	EmitVertex();


	EndPrimitive();
}