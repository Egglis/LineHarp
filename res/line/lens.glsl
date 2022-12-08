#include "/globals.glsl"

uniform vec2 lensPosition;
uniform float lensRadius;
uniform float lensDisp;
uniform float prevLensDisp;
uniform float lensDepthValue;
uniform vec2 delayedLensPosition;
uniform float time;

uniform int trajectoryID;

uniform float testTime;
uniform float foldTime;

uniform float delayedTValue;
uniform int focusLineID;
uniform float similarity;

struct Disp {
	vec2 dir;
	float weight;
};


// Returns distance to given lens position
float distanceToLens(vec4 point, vec2 lensPos) {
	float aspectRatio = viewportSize.x/viewportSize.y;

#ifdef LENS_DEPTH
	
	// Option: 1, Resets the lensDepth after animation is complete 
	float ldepth = mix(lensDepthValue, 1.3 , foldTime*1.3);
	
	
	vec3 lPos = vec3(lensPos, ldepth);

	point.x *= aspectRatio;
	lPos.x *= aspectRatio;

	float dist = point.z > ldepth ? distance(lPos.xy, point.xy) : distance(lPos, point.xyz);

#else
	vec2 lPos = lensPos;

	point.x *= aspectRatio;
	lPos.x *= aspectRatio;

	float dist = distance(lPos, point.xy);

#endif

	return dist;
}


// Displace a point
Disp disp(vec4 pos, vec2 lensPos) {

	float dist = distanceToLens(pos, lensPos);
	vec2 normDir = normalize(pos.xy - lensPos);

	float weight = 1.0 - smoothstep(0.0, lensRadius, dist);
	weight *= (lensRadius*viewportSize.y) * lensDisp;

	// Interpolate with a custome value 0-1 based on importance (pos.z) and the testTimer

	//pos.xy +=  weight * (normDir/viewportSize);	

	return Disp(normDir/viewportSize, weight);
}



vec4 lensDisplacment(vec4 pos, float vertexImportance) {
	
	// Interpolate between delayedLensPosition and lensPosition

	vec4 position = pos;
	position.z = vertexImportance;


	vec4 delayedPosition = position;
	Disp orgDisp = disp(position, lensPosition);
	Disp dlDisp = disp(delayedPosition, delayedLensPosition);

	
	#ifdef FOCUS_LINE
		orgDisp.weight *= 1.0 - similarity;
		dlDisp.weight *= 1.0 - similarity;
	#endif

	const float mixingFactor = 0.2;
	position.xy += orgDisp.dir * mix(mix(orgDisp.weight , dlDisp.weight, mixingFactor) , 0.0, delayedTValue);

	position.z = 0.0;
	return position;

}



vec4 displace(vec4 pos, float vertexImportance){


#ifdef LENS_FEATURE
	return lensDisplacment(pos, vertexImportance);
#endif

	return pos;
}


#define MAX_POINTS 64

// Lazy tesselation that returns how many points that are displaced < 64
int lazyTesselation(vec4 p1, vec4 p2) {
	int totalDisplacedPoints = 0;
	for(float t = 0.0; t <= 1.0; t += (1.0/float(MAX_POINTS) )) {
		vec4 currPoint = mix(p1, p2, t);
		Disp disp = disp(currPoint, lensPosition);
		if(disp.weight > 0.0) {
			return MAX_POINTS;
		}
	}

	// GPU already limits the amount of tesselated control points to 64 (Nividia 1070)
	//return max(2, min(totalDisplacedPoints, MAX_POINTS));
	return 2;
}

// Finds entry, exit of line thorugh circle, similar to ray marchiung?
vec2 lensIntersection(vec4 p1, vec4 p2){
	float entry;
	float exit;
	bool entrySet = false;
	for(float t = 0.0; t < 1.0; t += 1.0/MAX_POINTS){
		vec4 currentPoint = mix(p1, p2, t);
		float dist = distanceToLens(currentPoint, lensPosition);
		if(dist < lensRadius && !entrySet){
			entry = t;
			entrySet = true;
		}
		if(dist > lensRadius && entrySet){
			exit = t;
			return vec2(entry, exit);
		}
	}

	if(entrySet){
		return vec2(entry, 1.0);
	}
	return vec2(0.0, 1.0);
	
}

float rxEase2(float x, float k, float c)
{
    k = clamp(k, 0.0001, 10000.0); // clamp optional, if you know your k
    x = 0.5 - x; // re-center at 0
    float s = sign(x);
    x = clamp(abs(x) * 2.0, 0.0, 1.0);
    return c + 0.5 * s * x / (x * (k - 1.0) - k);
}


// Project linear t-values onto a stepping function defined by lens radius
float projection(float t, float entry, float exit){
	
	float v = (smoothstep(0.0, 0.0, t) * entry) + t*(exit-entry) + step(1.0, t);


	return clamp(v, 0.0, 1.0);

}

// Finds the correct T-Value when projected onto custom function
float calcT(vec4 p1, vec4 p2, float t){
	vec2 entryExit = lensIntersection(p1, p2);
	float value = projection(t, entryExit.x, entryExit.y);

	return value;
}


float compT(vec4 p1, vec4 p2, float t) {
	
	vec4 prevPos = vec4(-1);
	
	float maxWeight = 0.0;
	for(float x = 0.0; x <= 1.0; x += 1.0 / float(MAX_POINTS)){
		vec4 currPos = mix(p1, p2, x);
		Disp disp = disp(currPos, lensPosition);


		maxWeight = max(maxWeight, disp.weight);
	}

	return 0.5;
}


// Recalulates the T values used for interpolation 
vec3 calulateInterpolationValues(vec4 p1, vec4 p2, float t){
	
	float prev, curr, next;

	float du = 1.0/MAX_POINTS;

	int vertIndex = int(round(t*MAX_POINTS));

	float t0 = t-du;
	float t1 = t;
	float t2 = t+du;


	prev = calcT(p1, p2, t0);
	curr = calcT(p1, p2, t1);
	next = calcT(p1, p2, t2);

	return vec3(prev, curr, next);
}





