#pragma once

#include "ofMain.h"
#include "ofxGCode.hpp"
#include "ofxHersheyFont.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "jwsmtp.h"

class ofApp : public ofBaseApp{

    public:
        void setup() override;
        void update() override;
        void draw() override;
        std::string templateMatching(std::string inputImage);
        std::string generateGCODE(array<int,2> xy);
        void sendButtonPressed();
        std::string plot();
    
        ofxGCode gcode;
    
        bool bHide;
        bool gHide;
        bool drawn;
        unordered_map<string, array<int, 2>> templates = {
            {"I_PENT", {0,0}}, {"V_PENT", {1,0}}, {"Z_PENT", {0,1}}, {"P_PENT", {2,0}},
            {"T_PENT", {0,2}}, {"N_PENT", {3,0}}, {"X_PENT", {0,3}}, {"F_PENT", {4,0}},
            {"W_PENT", {0,4}}, {"Y_PENT", {5,0}}, {"L_PENT", {0,5}}, {"U_PENT", {6,0}}
        };
//        ofxTextField email;
        ofParameter<string> email;
        ofParameter<string> name;
//        ofxInputField<string> email;
//        ofxInputField<string> name;
        ofxButton send;

        ofxPanel gui;
    
    
        ofVideoGrabber videoGrabber;
        ofTexture texture;
        ofPixels dest;

        int camWidth;
        int camHeight;
    
        std::string display_result;
        ofTrueTypeFont font;
        
};
