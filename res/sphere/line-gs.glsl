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
} vsOut[];



out vec4 gsFragmentPosition;
out float gsFragmentImportance;

flat out float gsFragmentLineWidth;
flat out float gsFragmentDepth;
flat out float gsFragmentLayerLuminance;

#ifdef RS_LINKEDLIST
flat out vec4 gsPrev;
flat out vec4 gsStart;
flat out vec4 gsEnd;
flat out vec4 gsNext;
#else
flat out vec4 gsStart;
flat out vec4 gsEnd;
#endif

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


#ifdef RS_LINKEDLIST
	// author: Talis Tammearu
	// the implementation is taken from: https://blog.tammearu.eu/posts/gllines/


    vec3 prev = vsOut[0].prev;
    vec3 start = gl_in[0].gl_Position.xyz;
    vec3 end = gl_in[1].gl_Position.xyz;
    vec3 next = vsOut[1].next;

	// Displace (prev, start) and then (next, end)
	float startImportance = lens(prev, start)*3;
	float endImportance = lens(next, end)*3;

    vec3 lhs = cross(normalize(end-start), vec3(0.0, 0.0, -1.0));

    // is previous line segment a zero vector?
    bool colStart = length(start-prev) < 0.0001; // 0.0001 is arbitrary epsilon

    // is next line segment a zero vector?
    bool colEnd = length(end-next) < 0.0001;

    vec3 a = normalize(start-prev);
    vec3 b = normalize(start-end);
    vec3 c = (a+b)*0.5;

	// prevent problems with straight line-segmetns ------
	if(length(c) == 0) c = lhs;
	//----------------------------------------------------

    vec3 startLhs = normalize(c) * sign(dot(c, lhs));
    
	a = normalize(end-start);
    b = normalize(end-next);
    c = (a+b)*0.5;
    
	// prevent problems with straight line-segmetns ------
	if(length(c) == 0) c = lhs;
	//----------------------------------------------------

	vec3 endLhs = normalize(c) * sign(dot(c, lhs));

    if(colStart){
        startLhs = lhs;
	}

    if(colEnd){
        endLhs = lhs;
	}

    float startInvScale = dot(startLhs, lhs);
    float endInvScale = dot(endLhs, lhs);

    startLhs *= lineWidth*0.5;
	endLhs *= lineWidth*0.5;

	// create caps by adding an additional offset -------------------------------------

	// batchSize represents the total number of points -----------------
    // remove duplicates of start and endpoint (-2)
    // subtract 1 since we want the number of edges not points (-1)
    // subtract 1 since indexing starts with 0 and not 1 (-1)
    int numberOfLinePrimitives = numberOfTimesteps - 4; 
    //------------------------------------------------------------------

	vec3 startOffset = vec3(0);
	vec3 endOffset = vec3(0);

	if(gl_PrimitiveIDIn == 0){

		// this is the FIRST line-segment of the line-strip
		startOffset = normalize(start-end)*lineWidth;

	} else if (gl_PrimitiveIDIn == numberOfLinePrimitives){

		// this is the LAST line-segment of the line-strip
		endOffset = normalize(end-start)*lineWidth;
	}
	// --------------------------------------------------------------------------------
#else
	vec3 start = gl_in[0].gl_Position.xyz;
	vec3 end = gl_in[1].gl_Position.xyz;

	vec3 direction = normalize(end-start);
	direction = vec3(direction.y,-direction.x,direction.z);
	direction *= lineWidth;

	vec3 startOffset = normalize(start-end)*lineWidth;
	vec3 endOffset = normalize(end-start)*lineWidth;

	float timestepStepsize = 1.0f-(float(numberOfTimesteps-1) / (float(numberOfTimesteps)));
	float depthPerTimeRatio = timestepStepsize / float(numberOfTrajectories);

	float depthPerTrajectory = 1.0f / float(numberOfTrajectories);
	float depthPerSubsegment = depthPerTrajectory / float(numberOfTimesteps);
#endif

	// spawn impostor-points -----------------------------------------------------------------------------------------
	
#ifdef RS_LINKEDLIST
	gsFragmentDepth = 0.0f;
	gsFragmentLayerLuminance = vsOut[0].pointImportance;
	gsFragmentImportance = vsOut[0].pointImportance;

	spawnPoint(vec4(start+startOffset+startLhs/startInvScale, 1.0),prev,start,end,next);
#else
	spawnPoint(vec4(start+startOffset+direction, 1.0f), vec3(0), start, end, vec3(0));
#endif
	
	// spawn impostor-points -----------------------------------------------------------------------------------------

#ifdef RS_LINKEDLIST
	gsFragmentDepth = 0.0f;
	gsFragmentLayerLuminance = vsOut[0].pointImportance;
	gsFragmentImportance = vsOut[0].pointImportance;

	spawnPoint(vec4(start+startOffset-startLhs/startInvScale, 1.0),prev,start,end,next);
#else
	spawnPoint(vec4(start+startOffset-direction, 1.0f), vec3(0), start, end, vec3(0));
#endif

	// spawn impostor-points -----------------------------------------------------------------------------------------

#ifdef RS_LINKEDLIST
	gsFragmentDepth = 0.0f;
	gsFragmentLayerLuminance = vsOut[1].pointImportance;
	gsFragmentImportance = vsOut[1].pointImportance;

	spawnPoint(vec4(end+endOffset+endLhs/endInvScale, 1.0),prev,start,end,next);
#else
	spawnPoint(vec4(end+endOffset+direction, 1.0f), vec3(0), start, end, vec3(0));
#endif

	// spawn impostor-points -----------------------------------------------------------------------------------------

#ifdef RS_LINKEDLIST
	gsFragmentDepth = 0.0f;
	gsFragmentLayerLuminance = vsOut[1].pointImportance;
	gsFragmentImportance = vsOut[1].pointImportance;

	spawnPoint(vec4(end+endOffset-endLhs/endInvScale, 1.0),prev,start,end,next);
#else
	spawnPoint(vec4(end+endOffset-direction, 1.0f), vec3(0), start, end, vec3(0));
#endif

	//----------------------------------------------------------------------------------------------------------------


	EndPrimitive();
}