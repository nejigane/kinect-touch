#include "ofMain.h"
#include "OpenNI.h"
#include "ofxOpenCv.h"

class ofApp : public ofBaseApp{
private:
    enum {
        SEARCH = 0,
        CONFIRM = 1,
        DRAW = 2
    } mode;
    
    
    openni::Device device;
    openni::VideoStream colorStream;
    openni::VideoStream depthStream;
    openni::VideoFrameRef colorFrame;
    openni::VideoFrameRef depthFrame;

    ofShortPixels depthPixels;
    ofShortPixels backgroundDepthPixels;
    ofPolyline screenRectangle;
    ofxCvGrayscaleImage grayscaleImage;
    ofPoint touchPoint;
    cv::Mat homography;
    
	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
};
