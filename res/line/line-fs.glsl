#version 450
#include "/defines.glsl"
#include "/globals.glsl"

layout (location = 0) out vec4 lineChartTexture;
layout(r32ui, binding = 0) uniform uimage2D offsetImage;

struct BufferEntry
{
	uint id;
	uint previous;

	float importance;
	vec4 color;
	vec2 dir;
	float dist;
};

layout(std430, binding = 1) buffer intersectionBuffer
{
	uint count;
	BufferEntry intersections[];
};

in vec4 gsFragmentPosition;
in float gsFragmentDepth;
in float gsFragmentLayerLuminance;
in float gsFragmentImportance;

flat in float gsFragmentLineWidth;


#ifdef RS_LINKEDLIST
flat in vec4 gsPrev;
flat in vec4 gsStart;
flat in vec4 gsEnd;
flat in vec4 gsNext;
#else
flat in vec4 gsStart;
flat in vec4 gsEnd;
#endif

flat in vec2 segDir;
flat in float segDist;

uniform vec2 viewportSize;

uniform vec3 lineColor;
uniform vec3 focusLineColor;
uniform vec3 haloColor;

uniform int focusLineID;
uniform int audioLineID;
uniform int trajectoryID;

uniform float lensRadius;
uniform vec2 lensPosition;
uniform vec2 delayedLensPosition;

uniform float brushingAngle;
uniform float testSlider;
uniform float similarity;
uniform float lineOsc;



// Very basic color map, only for Figures and not very dynamic
vec4 colors[8];
void initColors(){
    colors[0] = vec4(0.105882, 0.619608, 0.466667, 1.0);   // Dark2-1
    colors[1] = vec4(0.850980, 0.372549, 0.007843, 1.0);   // Dark2-2
    colors[2] = vec4(0.458824, 0.439216, 0.701961, 1.0);   // Dark2-3
    colors[3] = vec4(0.905882, 0.160784, 0.541176, 1.0);   // Dark2-4
    colors[4] = vec4(0.400000, 0.650980, 0.117647, 1.0);   // Dark2-5
    colors[5] = vec4(0.901961, 0.670588, 0.007843, 1.0);   // Dark2-6
    colors[6] = vec4(0.650980, 0.462745, 0.113725, 1.0);   // Dark2-7
    colors[7] = vec4(0.400000, 0.400000, 0.400000, 1.0);   // Dark2-8
}

vec3 indexToColor(int index) {
	return vec3(
	float(index & 0xff) / 255.0,
	float((index >> 8) & 0xff) / 255.0,
	float((index >> 16) & 0xff) / 255.0
);
}
float computeDistanceToLineSegment(vec2 v, vec2 w, vec2 p){

	// consider the current aspect ratio to make sure all lines have equal width
	float aspectRatio = viewportSize.x/viewportSize.y;
	v.x *= aspectRatio;
	w.x *= aspectRatio;
	p.x *= aspectRatio;

	// signed distance function -------------------------------------------------------------------------
	// from https://www.iquilezles.org/www/articles/distfunctions2d/distfunctions2d.htm

	vec2 pa = p-v, ba = w-v;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	return length( pa - ba*h );
	// --------------------------------------------------------------------------------------------------

}

void main()
{	
	initColors();
	const float lineWidth = 0.25;//0.125;

#ifdef RS_LINKEDLIST
	// compute the distances of current framgent to all the line segments
	float dPrev = computeDistanceToLineSegment(gsPrev.xy, gsStart.xy, gsFragmentPosition.xy) - lineWidth*gsFragmentLineWidth;
	float dCurr = computeDistanceToLineSegment(gsStart.xy, gsEnd.xy, gsFragmentPosition.xy) - lineWidth*gsFragmentLineWidth;
	float dNext = computeDistanceToLineSegment(gsEnd.xy, gsNext.xy, gsFragmentPosition.xy) - lineWidth*gsFragmentLineWidth;

	float distanceToLine = min(dPrev,min(dCurr,dNext));
#else
	// compute the distances of current framgent to the line segments
	float distanceToCurrent = computeDistanceToLineSegment(gsStart.xy, gsEnd.xy, gsFragmentPosition.xy);

	// normalize it using the line width in projection space
	float distanceToLine = distanceToCurrent / gsFragmentLineWidth;
#endif
	
	vec3 currentLineColor = lineColor;

	// Very basic cluster coloring, only for showcasing and Figures, not very dynamic
	//vec3 currentLineColor = colors[int(gsFragmentImportance*8)].xyz;
	
#ifdef RANDOM_LINE_COLORS
	currentLineColor = getRandomizedColors(trajectoryID);
#endif

#ifdef FOCUS_LINE
	currentLineColor = mix(currentLineColor, focusLineColor, similarity);

#endif

#ifdef AUDIO_GUIDE
	if(lineOsc != 0) {
		currentLineColor =  mix(lineColor, vec3(0, 0, 1), abs(lineOsc)*100);
	}
#endif

#ifdef DEPTH_LUMINANCE_COLOR
	if(focusLineID != trajectoryID){
		currentLineColor.rgb *= (0.5+0.5*gsFragmentImportance);
	} 
	else if(audioLineID != trajectoryID){
		currentLineColor.rgb *= (0.5+0.5*gsFragmentImportance);
	}
#endif

	float opacityMultiplyer = 1.0f;
#ifdef IMPORTANCE_AS_OPACITY
	opacityMultiplyer = gsFragmentImportance;
#endif

	float currentImportance = easingFunction(gsFragmentImportance);
	float aspectRatio = viewportSize.x/viewportSize.y;
	vec2 ndCoordinates = (gl_FragCoord.xy-viewportSize/2)/(viewportSize/2);
	float pxlDistance = length((lensPosition-ndCoordinates) * vec2(aspectRatio, 1.0));

#ifdef LENS_FEATURE

	// compute minimum distance to the center of the lens
	float dLPrev = computeDistanceToLineSegment(gsPrev.xy, gsStart.xy, lensPosition.xy);
	float dLCurr = computeDistanceToLineSegment(gsStart.xy, gsEnd.xy, lensPosition.xy);
	float dLNext = computeDistanceToLineSegment(gsEnd.xy, gsNext.xy, lensPosition.xy);
	float distanceToLens = min(dLPrev,min(dLCurr,dLNext));

	const float epsilon = 0.0025f;

	if(distanceToLens <= epsilon && pxlDistance <= lensRadius){
		
		float scaling = clamp(1.0f-pow(length(lensPosition-delayedLensPosition),0.025f), 0.0f, 1.0f);
		currentImportance += (2.0f-pxlDistance/lensRadius)*scaling;

		//Debug - Output computed importance as color -------------------------
		//lineChartTexture.rgb = vec3(1,0,0)*(1.0f-pxlDistance/lensRadius)*scaling;
		//---------------------------------------------------------------------

		
	}


#endif

#ifdef ANGULAR_BRUSHING

	if(pxlDistance <= lensRadius){

		vec2 currentEdge = normalize(vec2(gsEnd.xy-gsStart.xy));
		float currentAngle = degrees(acos(dot(currentEdge,vec2(1,0))));

		const float angleEpsilon = 2.5f;

		if(currentAngle >= abs(brushingAngle)-angleEpsilon && currentAngle <= abs(brushingAngle)+angleEpsilon){

			if(sign(currentEdge.y) == sign(brushingAngle)){
			
				vec2 midPoint = vec2((gsStart.x+gsEnd.x)/2.0f,(gsStart.y+gsEnd.y)/2.0f); 
				float t = 1.0f-length(vec2(gsFragmentPosition.xy)-midPoint)/(length(vec2(gsStart.xy-gsEnd.xy))/2);

				// easeOutQuint - https://easings.net/en#easeOutQuint
				currentImportance += 2.0f*(1 - pow(1-t, 5));
			}
		}
	}

	//Debug - Output computed importance as color -------------------------
	//if(currentAngle >= abs(brushingAngle)-angleEpsilon && currentAngle <= abs(brushingAngle)+angleEpsilon){
	//	if(sign(currentEdge.y) == sign(brushingAngle)){
	//		lineChartTexture.rgb = vec3(1,0,0);
	//	}else{
	//		lineChartTexture.rgb = vec3(0,1,0);
	//	}
	//}else{
	//	lineChartTexture.rgb = vec3(0,1,0);
	//}
	//---------------------------------------------------------------------
#endif


	float dn = -distanceToLine/(lineWidth*gsFragmentLineWidth);
	float w = sqrt(dFdx(dn) * dFdx(dn) + dFdy(dn) * dFdy(dn));

	if(dn < -0.25f)
		discard;
		
	dn = min(1.0,dn);

	vec3 finalColor = vec3(0.0,0.0,0.0);

	const float outlineWidth = 0.25;
	const vec3 outlineColor = currentLineColor*0.25;
	
	float lineOutline = smoothstep(0.0-w,0.0+w,dn);
	finalColor = mix(finalColor,outlineColor,lineOutline);

	float lineFill = smoothstep(0.0-w+outlineWidth,0.0+w+outlineWidth,dn);
	finalColor = mix(finalColor,currentLineColor,lineFill);
	
	float opacity = lineOutline;
	lineChartTexture.rgb = finalColor*opacity;
	//lineChartTexture.rgb = vec3(1,0,0)*(1.0f-pxlDistance/lensRadius);
	lineChartTexture.a = opacity;


#ifdef LINE_HALOS
	float depthOutline = smoothstep(0.0,0.0+w,dn);
	gl_FragDepth = (1.0-currentImportance*depthOutline);
#else
	gl_FragDepth = 1.0f;
#endif

#ifdef RS_LINKEDLIST
	BufferEntry entry;

	uint index = atomicAdd(count, 1);
	uint prev = imageAtomicExchange(offsetImage, ivec2(gl_FragCoord.xy), index);

	entry.id = trajectoryID;
	entry.previous = prev;
	entry.importance = currentImportance;
	entry.dir = segDir;
	entry.dist = segDist;

	if(focusLineID == trajectoryID){
		// make sure line in focus is fully opaque
		entry.color = lineChartTexture;
	}else if (audioLineID == trajectoryID) {
		entry.color = lineChartTexture;
	}else{
		// use importance for pre-multiplied alpha
		entry.color = lineChartTexture*opacityMultiplyer;
	}
	intersections[index] = entry;

#endif

}