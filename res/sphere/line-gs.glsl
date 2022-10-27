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

	gsFragmentPosition = S;

	// Sets the fragment position
	gl_Position = gsFragmentPosition;
	
#ifdef RS_LINKEDLIST

	// Only used in the fragment shader
	gsPrev = vec4(prev,1);
	gsStart = vec4(start,1);
	gsEnd = vec4(end,1);
	gsNext = vec4(next,1);

#else
	gsStart = gl_in[0].gl_Position;
	gsEnd = gl_in[1].gl_Position;    
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

	// vec2 ndCoordinates = (a.xy-viewportSizeSize/2)/(viewportSizeSize/2);
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

void constructSegment(vec2 p0, vec2 p1, vec2 p2, vec2 p3){
	gsFragmentLineWidth = length(modelViewProjectionMatrix*vec4(0.0f, 0.0f, 0.0f, 1.0f) - modelViewProjectionMatrix*(vec4(lineWidth, 0.0f, 0.0f, 1.0f)));
		
	// consider the current aspect ratio to make sure all shalos have equal width
	float aspectRatio = viewportSize.x/viewportSize.y;
	gsFragmentLineWidth *= aspectRatio;	



    /* perform naive culling */
	
    vec2 area = viewportSize * 4;
    if( p1.x < -area.x || p1.x > area.x ) return;
    if( p1.y < -area.y || p1.y > area.y ) return;
    if( p2.x < -area.x || p2.x > area.x ) return;
    if( p2.y < -area.y || p2.y > area.y ) return;
	
    // determine the direction of each of the 3 segments (previous, current, next) 
    vec2 v0 = normalize( p1 - p0 );
    vec2 v1 = normalize( p2 - p1 );
    vec2 v2 = normalize( p3 - p2 );

    // determine the normal of each of the 3 segments (previous, current, next) 
    vec2 n0 = vec2( -v0.y, v0.x );
    vec2 n1 = vec2( -v1.y, v1.x );
    vec2 n2 = vec2( -v2.y, v2.x );

    // determine miter lines by averaging the normals of the 2 segments 
    vec2 miter_a = normalize( n0 + n1 );	// miter at start of current segment
    vec2 miter_b = normalize( n1 + n2 ); // miter at end of current segment

    // determine the length of the miter by projecting it onto normal and then inverse it 
    float an1 = dot(miter_a, n1);
    float bn1 = dot(miter_b, n2);
    if (an1==0) an1 = 1;
    if (bn1==0) bn1 = 1;
    float length_a = (lineWidth) / an1;
    float length_b = (lineWidth) / bn1;
	
	// prevent excessively long miters at sharp corners
	float miterLimit = 0.75;
    if( dot( v0, v1 ) < -miterLimit ) {
        miter_a = n1;
        length_a = lineWidth*0.5;

		vec4 c0, c1, c2;
        // close the gap 
        if( dot( v0, n1 ) > 0 ) {

            c0 = vec4( ( p1 + lineWidth*0.5 * n0 ), 0, 1);
			spawnPoint(c0, 0); 

            c1 = vec4( ( p1 + lineWidth*0.5 * n1 ), 0, 1);
            spawnPoint(c1, 0);

            c2 = vec4( p1, 0.0, 1.0 );
            spawnPoint(c2, 0);

            EndPrimitive();
        }
        else {
            c0 = vec4( ( p1 - lineWidth*0.5 * n1 ), 0, 1);
			spawnPoint(c0, 0); 

            c0 = vec4( ( p1 - lineWidth*0.5 * n0 ), 0, 1 );
			spawnPoint(c1, 0);

            c0 = vec4( p1, 0, 1);
			spawnPoint(c2, 0);

            EndPrimitive();
        }
    }
    if( dot( v1, v2 ) < -miterLimit ) {
        miter_b = n1;
        length_b = lineWidth*0.5;
    }
	
    // generate the triangle strip
	vec4 topLeft = vec4( ( p1 + length_a * miter_a ), 0, 1);
	vec4 bottomLeft = vec4( ( p1 - length_a * miter_a) , 0, 1);
	vec4 topRight = vec4( ( p2 + length_b * miter_b ), 0, 1);
	vec4 bottomRight = vec4( ( p2 - length_b * miter_b ) , 0, 1);

	spawnPoint(topLeft, 0);
	spawnPoint(bottomLeft, 0);
	spawnPoint(topRight, 1);
	spawnPoint(bottomRight, 1);
	EndPrimitive(); 

}

void main() {

	gsPrev = vec4(vsOut[0].prev, 0, 1);
	gsStart = gl_in[0].gl_Position;
	gsEnd =  gl_in[1].gl_Position;
	gsNext = vec4(vsOut[1].next, 0, 1);


	constructSegment(vsOut[0].prev, gl_in[0].gl_Position.xy, gl_in[1].gl_Position.xy, vsOut[1].next);
}