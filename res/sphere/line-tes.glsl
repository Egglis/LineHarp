#version 450
#include "/defines.glsl"
#include "/globals.glsl"


layout (isolines, equal_spacing) in;

in tessVsData {
	float pointImportance;
	float tDiff;
} tessOut[];

out vsData {
	float pointImportance;
	vec2 prev;
	vec2 next;

} vsOut;


uniform float xAxisScaling;
uniform float yAxisScaling;
uniform float testSlider;
uniform vec2 lensPosition;
uniform float lensRadius;
uniform vec2 viewportSize;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 inverseModelViewProjectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 inverseViewMatrix;

vec3 bezier(float u, vec3 p0, vec3 p1, vec3 p2, vec3 p3)
{
	float B0 = (1.-u)*(1.-u)*(1.-u);
	float B1 = 3.*u*(1.-u)*(1.-u);
	float B2 = 3.*u*u*(1.-u);
	float B3 = u*u*u;

	vec3 p = B0*p0 + B1*p1 + B2*p2 + B3*p3;
	return p;
} 

// Same solution as previously only now it effect all the "generated" vertexes as well
vec3 lens(vec3 p0, vec3 p3, float u){

	float aspectRatio = viewportSize.x/viewportSize.y;
	vec2 lPos = lensPosition;
	lPos.x *= aspectRatio;

	vec3 currPoint = mix(p0, p3, u);
	vec4 MPoint = modelViewProjectionMatrix*vec4(currPoint, 1);
	MPoint.x *= aspectRatio;

	float dl = distance(lPos, MPoint.xy);

	if (dl <= lensRadius) {
		vec2 dir = normalize(vec2(MPoint.x - lPos.x, MPoint.y - lPos.y));

		
		float zoomIntensity = 1;
		float x = dl/lensRadius;
		float scaling = zoomIntensity*cos(3.1415 + x*2*3.1415)+zoomIntensity;
		
		return currPoint+vec3(dir*scaling*testSlider, 0);

	}
	return mix(p0, p3, u);
}


vec3 bezierDisplacment(vec3 cp) {

	float aspectRatio = viewportSize.x/viewportSize.y;
	vec2 lPos = lensPosition;
	lPos.x *= aspectRatio;

	vec4 MPoint = modelViewProjectionMatrix*vec4(cp, 1);
	MPoint.x *= aspectRatio;

	float dl = distance(lPos, MPoint.xy);
	if(dl <= lensRadius) {
		vec2 dir = normalize(vec2(MPoint.x - lPos.x, MPoint.y - lPos.y));

		float zoomIntensity = 1;
		float x = dl/lensRadius;
		float scaling = zoomIntensity*cos(3.1415 + x*2*3.1415)+zoomIntensity;


		float pxlRadius = lensRadius*inverseModelViewProjectionMatrix[1].y;
		return vec3(dir, 0)*pxlRadius/2*scaling;
	}

	return vec3(0);
}


vec3 bezierLens(vec3 p0, vec3 p3, float u){
	
	vec3 cp0 = p0;
	vec3 cp3 = p3;
	vec3 cp1 = mix(p0, p3, 0.33);
	vec3 cp2 = mix(p0, p3, 0.66);


	cp0 += bezierDisplacment(cp0);
	cp3 += bezierDisplacment(cp3);

	cp1 += bezierDisplacment(cp1);
	cp2 += bezierDisplacment(cp2);



	return bezier(u, cp0, cp1, cp2, cp3);
}



void main(){
	// Move to globals.glsl or similar
	int num_points = 16;


uniform float testSlider;
uniform vec2 viewportSize;

// Magic lens
uniform float lineWidth;
uniform vec2 lensPosition;
uniform float lensRadius;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 inverseModelViewProjectionMatrix;


// Ease in quad from defines.glsl, used in disp()
float easeInOut(float t) {
	if(t < 0.5f){
		return 2*t*t; 
	}else{
		return -1+(4-2*t)*t;
	}
};


// Catmull rom, (TODO currently not used)
vec4 catmull(vec4 p0, vec4 p1, vec4 p2, vec4 p3, float u ){

	float alpha = 1.0;
    float tension = 0.0;
    
    float t01 = pow(distance(p0, p1), alpha);
	float t12 = pow(distance(p1, p2), alpha);
	float t23 = pow(distance(p2, p3), alpha);

	vec2 m1 = (1.0f - tension) *
    	(p2.xy - p1.xy + t12 * ((p1.xy - p0.xy) / t01 - (p2.xy - p0.xy) / (t01 + t12)));
	vec2 m2 = (1.0f - tension) *
    	(p2.xy - p1.xy + t12 * ((p3.xy - p2.xy) / t23 - (p3.xy - p1.xy) / (t12 + t23)));
    
	vec2 a = 2.0f * (p1.xy - p2.xy) + m1 + m2;
	vec2 b = -3.0f * (p1.xy - p2.xy)  - m1 - m1 - m2;
	vec2 c = m1;
	vec2 d = p1.xy;

	vec2 new_pos =  (a * u * u * u) + (b * u * u) + (c * u) + (d);

	return vec4(new_pos, 0, 1);

}

// Computes distance to lens and direction to lens (x, y, distance);
vec3 distanceToLens(vec4 point){
	float aspectRatio = viewportSize.x/viewportSize.y;

	vec2 lPos = lensPosition;

	point.x *= aspectRatio;
	lPos.x *= aspectRatio;

	float dl = distance(lPos, point.xy);
	vec2 dir = normalize(point.xy - lPos);

	return vec3(dir, dl);
}


// Find the closest point and returns which side it is on===
float leftOrRight(vec4 a, vec4 b){
	float du = 1.0f/gl_PatchVerticesIn;
	vec2 line = normalize(a.xy - b.xy);

	float minDistance = 10000000.0f; 
	float currentZ = 0.0f;

	for(float t = 0.0f; t < 1.0f; t+=du){
		vec2 currentPoint = mix(a.xy, b.xy, t);
		vec3 dl = distanceToLens(vec4(currentPoint, 0, 1));
		if (dl.z < minDistance) {
			currentZ = cross(vec3(dl.xy, 1), vec3(line, 1)).z;
			minDistance = dl.z;
		}

	}
	return currentZ;
}


// Displace a point by normal of the line it is from based on left or right of the line:
void disp(inout vec4 pos, vec4 a, vec4 b) {
	vec3 dl = distanceToLens(pos);

	float z = leftOrRight(a, b);

	if (dl.z <= lensRadius){
		float t = 1 - (dl.z/lensRadius);
		vec2 dir = dl.xy;
		vec2 line = normalize(a.xy - b.xy);

		vec2 n = vec2(-line.y, line.x);
		vec3 crossProduct = cross(vec3(dir, 1), vec3(line, 1));

		vec2 dispDir;
		if (z < 0){
			dispDir = n;
		} else if(z > 0)  {
			dispDir = -n;
		} else {
			dispDir = vec2(0,0);
		} 

		//pos += vec4(testSlider*dispDir*(easingFunction(t)/10), 0, 0);
		pos += vec4(testSlider*dispDir*(easeInOut(t)/10), 0, 0);
	}

}

/*
Loop every point along the line ........
Find the closest to the lens Position, and then use that as the left or right decider 
it will always be within the radius

*/


void defaultMode(){
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	
	vsOut.pointImportance = mix(tessOut[0].pointImportance, tessOut[1].pointImportance, u);

	// Initial start and end
	vec3 p0 = vec3(gl_in[0].gl_Position);
	vec3 p3 = vec3(gl_in[1].gl_Position);

	// Construct 2 extra control points|
	vec3 p1 = vec3(mix(p0.x, p3.x, 1/3), mix(p0.y, p3.y, 1/3), 0);
	vec3 p2 = vec3(mix(p0.x, p3.x, 2/3), mix(p0.y, p3.y, 2/3), 0);

	// Check for displacment

	// Normalized length between segments
	float du = 1/num_points;

	vec3 prev;
	vec3 next;
	
	/*
	// Bezier
	vsOut.prev = bezierLens(p0,p3,u-du);
	vsOut.next = bezierLens(p0,p3,u+du);

	gl_Position = vec4(bezierLens(p0, p3, u) , 1);
	*/
	
	
	vsOut.prev = lens(p0,p3,u-du);
	vsOut.next = lens(p0,p3,u+du);

	gl_Position = vec4(lens(p0, p3, u) , 1);
	
	
}