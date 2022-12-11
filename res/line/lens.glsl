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

	float ldepth = mix(lensDepthValue, 1.3 , easeInOutElastic(foldTime*1.3));

	
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
	weight = mix(weight, 1.0 , easeInOutElastic(foldTime));

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
			return 3;
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
			exit = t - 1.0/MAX_POINTS;
			return vec2(entry, exit);
		}
	}

	if(entrySet){
		return vec2(entry, 1.0);
	}
	return vec2(0.0, 1.0);
	
}


// Project linear t-values onto a stepping function defined by lens radius
float projection(float t, float entry, float exit){
	float v = (smoothstep(0.0, 0.0, t) * entry) + t*(exit-entry) + step(1.0, t);
	return clamp(v, 0.0, 1.0);

}

// Finds the correct T-Value when projected onto custom function
float calcT(vec4 p1, vec4 p2, float t){

	// Entry, Exit
	vec2 entryExit = lensIntersection(p1, p2);
	float value = projection(t, entryExit.x, entryExit.y);

	return value;
}


float closestPoint(vec2 p0, vec2 p1, vec2 lPos){
	if (p0.x == p1.x) return 1.0;
	if (p0.y == p1.y) return 0.0;

	float m1 = (p1.y-p0.y)/(p1.x-p0.x);
	float m2 = -1.0/m1;
	float x = ((m1 * p0.x) - (m2 * lPos.x) + lPos.y - p0.y) / (m1-m2);
	float y = m2 * (x - lPos.x) + lPos.y;
	float t = abs(distance(p0, vec2(x,y)) / distance(p0, p1));
	if(disp(vec4(x,y,1,1), lPos).weight > 0.0){
		return t;
	}
	return -1.0;

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


float getTValue(vec2 p1, vec2 p2, float t){
	float mid_t = closestPoint(p1.xy, p2.xy, lensPosition);
	if(mid_t < 0.0){
		return t;
	}

	// T is to the left of the middel???
	if(t < mid_t){
		return t + abs(mid_t - t);
	}
	return t;
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





