#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	blur.setup(ofGetWidth(), ofGetHeight(), 7, 0.2, 3);
	largeCircleGlow.load("images/Translucent Hole Ellipse.png");
	ofBackground(0,0,0);
	useParallax = false;
}

//--------------------------------------------------------------
void ofApp::update(){

}

double eucDist(double x1, double y1, double x2, double y2) {
	return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

int sign(float x) {
	if (x > 0) return 1;
	if (x < 0) return -1;
	return 0;
}

float* parallaxDelta(float x, float y, float parallaxX, float parallaxY, float xCenter, float yCenter, float extent = 1.) {
	// In coordinates relative to some desired center... (in this app's case, the center of the screen)
	float adjMouseX = parallaxX - xCenter;
	float adjMouseY = parallaxY - yCenter;
	float adjPX = x - xCenter;
	float adjPY = y - yCenter;
	
	// Convert to polar coords
	float rMouse = sqrt(adjMouseX*adjMouseX + adjMouseY*adjMouseY);
	float thetaMouse = atan2(adjMouseY, adjMouseX);
	float rP = sqrt(adjPX*adjPX + adjPY*adjPY);
	float thetaP = atan2(adjPY, adjPX);
	
	// Project mouse/parallax vector onto point vector
	float rMouseOnP = (adjMouseX * adjPX + adjMouseY * adjPY) / rP;
	float percentR = rMouseOnP / (ofGetWidth() / 2.);
	// If vectors point in same direction, then pull inward; otherwise, push outward
	float rPParallaxDelta = rP * percentR * (abs(thetaMouse - thetaP) > 90 && abs(thetaMouse - thetaP) < 270 ? -1 : 1);
	
	float* results = (float*) malloc(sizeof(float) * 2);
	results[0] = rPParallaxDelta * cos(thetaP) * extent;
	results[1] = rPParallaxDelta * sin(thetaP) * extent;
	return results;
}

//--------------------------------------------------------------
void ofApp::draw(){
	// Cool thing from initial attempt
	// const int NUM_CIRCLES = 9;
	// const int NUM_POINTS = 100;
	// const int R_CIRCLE = 200;
	//
	// float xCenterMain = ofGetWindowWidth() / 2;
	// float yCenterMain = ofGetWindowHeight() / 2;
	// float rOrbit = ofMap(sin(ofGetElapsedTimef()), -1, 1, 0, ofGetWindowWidth() * 0.75);
	//
	// for (int i = 0; i < NUM_CIRCLES; i++) { // large circles
	// 	float xCenter = xCenterMain + rOrbit * cos(ofGetElapsedTimef() + i * PI * 2 / NUM_CIRCLES);
	// 	float yCenter = yCenterMain + rOrbit * sin(ofGetElapsedTimef() + i * PI * 2 / NUM_CIRCLES);
	//
	// 	for (int j = 0; j < NUM_POINTS; j++) {
	// 		ofDrawCircle(
	// 			xCenter + R_CIRCLE * cos(ofGetElapsedTimef() + j * PI * 2 / NUM_POINTS),
	// 			yCenter + R_CIRCLE * sin(ofGetElapsedTimef() + j * PI * 2 / NUM_POINTS),
	// 			R_CIRCLE
	// 		);
	// 	}
	// }
	// #endregion
	
	blur.begin();

	const int NUM_CIRCLES = 9;
	const int NUM_POINTS = 100;
	const int R_CIRCLE = 120;
	const float R_DOT = 2;

	ofSetBackgroundColor(0);
	ofSetColor(0);
	ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());

	float xCenterMain = ofGetWindowWidth() / 2;
	float yCenterMain = ofGetWindowHeight() / 2;
	float rOrbit = ofMap(sin(ofGetElapsedTimef() * 0.75), -1, 1, 0, ofGetWindowWidth() * 0.75 / 2);
	float maxDistFromCenter = eucDist(0, 0, xCenterMain, yCenterMain); // cartesian
	float minDistFromCenter = rOrbit - R_CIRCLE; // radial

	bool tempUseParallax = useParallax;
	
	auto drawDot = [tempUseParallax, xCenterMain, yCenterMain, maxDistFromCenter, minDistFromCenter, R_DOT] (float x, float y, float radius = 0, float parallaxExtent = 0., float r = ofRandom(0, 79), float g = ofRandom(148, 232), float b = ofRandom(148, 255), float forceAlpha = -1) {
		if (tempUseParallax) {
			float* parallaxResult = parallaxDelta(x, y, ofGetMouseX(), ofGetMouseY(), xCenterMain, yCenterMain, parallaxExtent);
			x += parallaxResult[0];
			y += parallaxResult[1];
			free(parallaxResult);
		}
		radius = radius == 0 ? R_DOT : radius;
		float distFromCenter = eucDist(x, y, xCenterMain, yCenterMain);
		float a = forceAlpha == -1 ? ofRandom(ofMap(distFromCenter, minDistFromCenter, maxDistFromCenter, 255, 0), ofMap(distFromCenter, minDistFromCenter, maxDistFromCenter, 255, 30)) : forceAlpha;
		ofSetColor(r, g, b, a);
		ofFill();
		ofDrawCircle(x, y, radius);
	};

	// Large circles converging
	for (int i = 0; i < NUM_CIRCLES; i++) {
		float mainAngle = ofGetElapsedTimef() + i * PI * 2 / NUM_CIRCLES;
		float xCenter = xCenterMain + rOrbit * cos(mainAngle);
		float yCenter = yCenterMain + rOrbit * sin(mainAngle);

		// Draw circle glow texture
		float xTopLeft = xCenter - R_CIRCLE;
		float yTopLeft = yCenter - R_CIRCLE;
		float glowOffset = R_CIRCLE * 39 / (422 / 2); // Numbers based on dimensions of illustration
		largeCircleGlow.draw(xTopLeft - glowOffset, yTopLeft - glowOffset, (R_CIRCLE + glowOffset) * 2, (R_CIRCLE + glowOffset) * 2);
		// FIXME: This won't draw for just one of the circles (seems to be idx 0), but I'm not sure why.

		for (int j = 0; j < NUM_POINTS; j++) {
			float angle = ofGetElapsedTimef() + j * PI * 2 / NUM_POINTS;

			float r = ofRandom(82, 98);
			float g = ofRandom(130, 190);
			float b = ofRandom(131, 199);

			float xPrev, yPrev;
			int lineLength = ofRandom(10);

			for (int k = 0; k < lineLength; k++) {
				float radius = R_CIRCLE + ofMap(k, 0, lineLength - 1, 0, 20);
				float radius_x = radius;
				float radius_y = radius;
				// Very slight oscillation of tails
				float angleOffset = ofMap(sin(ofGetElapsedTimef() * 0.25), -1, 1, 0, k * 0.005);
				float x = xCenter + radius_x * cos(angle + angleOffset);
				float y = yCenter + radius_y * sin(angle + angleOffset);
				
				drawDot(x, y, R_DOT, 1., r, g, b);

				if (k > 0) {
					ofSetColor(255, 255, 255, 0.25 * 255);
					ofSetLineWidth(1);
					ofDrawLine(xPrev, yPrev, x, y);
				}

				xPrev = x;
				yPrev = y;
			}
		}
	}

	// Swarm of dots could be seen as a series of circles with different orbit freqs
	float innerCircleRadius = rOrbit - R_CIRCLE;
	float minSwarmRadius = innerCircleRadius < 0 ? abs(innerCircleRadius) : 0;
	float maxSwarmRadius = ofGetWindowWidth() * 0.85 / 2;
	int numSwarmCircles = 200;
	float radiusIncrement = (maxSwarmRadius - minSwarmRadius) / numSwarmCircles;

	float t = ofGetElapsedTimef();

	for (int i = 0; i < numSwarmCircles; i++) {
		float radius = minSwarmRadius + i * radiusIncrement;
		int numDots = radius == 0 ? 1 : 50; // (int) floor(ofMap(i, 0, numSwarmCircles, 1, 100));
		for (int j = 0; j < numDots; j++) {
			float freq = ofMap(i, 0, numSwarmCircles, 0.1, 2) * ofMap(j, 0, numDots, 0.8, 1.2);
			float angleOffset = 2 * PI / numDots * j;
			float angle = freq * t + angleOffset; // Mul by f
			float x = xCenterMain + radius * cos(angle);
			float y = yCenterMain + radius * sin(angle);
			drawDot(x, y, R_DOT, 1.25);
			if (i < numSwarmCircles / 2) {
				drawDot(x, y, 1, 1.25, 255, 255, ofMap(i, 0, numSwarmCircles / 2, 1, 0.5) * 255, ofMap(i, 0, numSwarmCircles / 2, 0.5, 0.1) * 255);
			}
		}
	}

	blur.end();
	blur.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == OF_KEY_RETURN) {
		useParallax = !useParallax;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
