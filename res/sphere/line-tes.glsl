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

patch in vec4 pp0;
patch in vec4 pp3;

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

	vec4 p0 = pp0;
	vec4 p1 = gl_in[0].gl_Position;
	vec4 p2 = gl_in[1].gl_Position;
	vec4 p3 = pp3;


	// Importance is interpolated
	vsOut.pointImportance = mix(tessOut[0].pointImportance, tessOut[0].pointImportance, u);

	// du: delimiter, t0, t1, t2 (previous, current, next) t-values 
	float du = 1.0f/(16.0f);
	float t0 = u-du;
	float t1 = u;
	float t2 = u+du;

	vec4 prev_pos, pos, next_pos;

	// Current Position
	pos = mix(p1, p2, t1);

	// Used to detect end points for diffirent next, and prev (TODO not working)
	if(u < 0){
		prev_pos = p1;
		disp(prev_pos, p0, p1);
	} else {
		prev_pos = mix(p1, p2, t0);
		disp(prev_pos, p1, p2);
	}

	if(u > 1){
		next_pos = p3;
		disp(next_pos, p2, p3);
	} else {
		next_pos = mix(p1, p2, t2);
		disp(next_pos, p1, p2);

	}


	disp(pos, p1, p2);

	vsOut.prev = prev_pos.xy;
	vsOut.next = next_pos.xy;

	gl_Position = pos;

}

void main(){

	defaultMode();
}