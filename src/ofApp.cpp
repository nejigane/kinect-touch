#include "ofApp.h"
#include "cv.h"

//--------------------------------------------------------------
void ofApp::setup(){
    openni::Status rc = openni::OpenNI::initialize();
    if (rc != openni::STATUS_OK) {
        cout << "Failed to initialize OpenNI:" << openni::OpenNI::getExtendedError() << endl;
        return;
    }
    
    rc = device.open(openni::ANY_DEVICE);
    if (rc != openni::STATUS_OK) {
        cout << "Failed to open device:" << openni::OpenNI::getExtendedError() << endl;
        return;
    }

    colorStream.create(device, openni::SENSOR_COLOR);
    depthStream.create(device, openni::SENSOR_DEPTH);
    colorStream.start();
    depthStream.start();

    device.setDepthColorSyncEnabled(true);
    device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);

    backgroundDepthPixels.allocate(640, 480, 1);
    depthPixels.allocate(640, 480, 1);
    mode = SEARCH;
}

//--------------------------------------------------------------
void ofApp::update(){
    colorStream.readFrame(&colorFrame);
    depthStream.readFrame(&depthFrame);
    depthPixels.setFromPixels((unsigned short *)depthFrame.getData(),
                              depthFrame.getWidth(), depthFrame.getHeight(), 1);

    cv::Mat colorMat(480, 640, CV_8UC3, (void*)colorFrame.getData());
    cv::Mat grayscaleMat;
    cv::cvtColor(colorMat, grayscaleMat, CV_RGB2GRAY);

    cv::Mat binaryMat;
    //cv::threshold(grayscaleMat, binaryMat, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
    cv::threshold(grayscaleMat, binaryMat, 30, 255, CV_THRESH_BINARY);

    grayscaleImage.setFromPixels(binaryMat.ptr(), 640, 480);
    
    if (mode == SEARCH) {
        vector<vector<cv::Point> > contours;
        cv::findContours(binaryMat, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
        for (vector<vector<cv::Point> >::iterator it = contours.begin();
             it != contours.end(); it++) {
            vector<cv::Point> approx;
            cv::approxPolyDP(cv::Mat(*it), approx, 20.0, true);
            if (approx.size() == 4) {
                
                vector<ofPoint> newPoints;
                for (vector<cv::Point>::const_iterator pit = approx.begin(); pit != approx.end(); pit++) {
                    newPoints.push_back(ofPoint(pit->x, pit->y));
                    ofLog() << pit->x << ":" << pit->y;
                 }
                screenRectangle.clear();
                screenRectangle.addVertices(newPoints);
                screenRectangle.close();
                mode = CONFIRM;
                break;
            }
        }
    } else if (mode == DRAW) {
        unsigned char tmp[640 * 480];
        for (int y = 0; y < 480; y++) {
            for (int x = 0; x < 640; x++) {
                int index = y * 640 + x;
                int diff = backgroundDepthPixels[index] - depthPixels[index];
                tmp[index] = (diff > 20 && diff < 30) && screenRectangle.inside(x, y) ? 65535 : 0;
            }
        }
        ofxCvGrayscaleImage touchImage;
        touchImage.setFromPixels(tmp, 640, 480);

        ofxCvContourFinder contourFinder;
        contourFinder.findContours(touchImage, 100, 2500, 1, false);
        if (contourFinder.blobs.size() > 0) {
            ofPoint p = contourFinder.blobs[0].centroid;
            double m[] = { p.x, p.y, 1 };
            cv::Mat srcPoint(3, 1, CV_64FC1, m);
            cv::Mat touch = homography * srcPoint;
            touchPoint = ofPoint(touch.at<double>(0, 0) / touch.at<double>(2, 0),
                                 touch.at<double>(1, 0) / touch.at<double>(2, 0));
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    if (mode == SEARCH) {
        grayscaleImage.draw(0, 0);
    } else if (mode == CONFIRM) {
        grayscaleImage.draw(0, 0);
        ofSetColor(255, 0, 0);
        ofSetLineWidth(3);
        screenRectangle.draw();
        ofSetLineWidth(1);
        ofSetColor(255, 255, 255);
    } else if (mode == DRAW) {
        ofSetColor(0, 0, 0);
        ofCircle(touchPoint.x, touchPoint.y, 10);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if (mode == CONFIRM) {
        if (key == 13){
            mode = DRAW;
            backgroundDepthPixels = depthPixels;

            const vector<ofPoint>& vertices = screenRectangle.getVertices();
            double srcPoints[] = {
                vertices[0].x, vertices[0].y,
                vertices[1].x, vertices[1].y,
                vertices[2].x, vertices[2].y,
                vertices[3].x, vertices[3].y
            };
            cv::Mat srcMat(4, 2, CV_64FC1, srcPoints);
            
            double dstPoints[] = {
                0, 0,
                0, 480,
                640, 480,
                640, 0
            };
            cv::Mat dstMat(4, 2, CV_64FC1, dstPoints);
            
            homography = cv::findHomography(srcMat, dstMat);
            ofSetBackgroundAuto(false);
            ofBackground(255);
        } else {
            mode = SEARCH;
        }
    }
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
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
