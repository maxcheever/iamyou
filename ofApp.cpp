#include "ofApp.h"
/*
 TODO:
    - make code somewhat readable
        helper functions for drawing lines, etc. in generateGCODE
    - sound 'i am' chant audio piece (repeated during drawing)
    - save arrays for compilation piece
        write cur array to file, make piece separate?
        depends on access to pis, if extra available can probably project in draw()?
*/

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0);
    
    ofTrueTypeFont::setGlobalDpi(96);
    font.load("monospace", 12);
    
       
    send.addListener(this,&ofApp::sendButtonPressed);
    
    gui.setup("", "settings.xml", 450, 200); // most of the time you don't need a name but don't forget to call setup
//    gui.add(name.set("enter your name here", "type here", "min", "max", 42,50));
//    gui.add(email.set("enter email address here", "type here", "min", "max", 42,50));
    gui.add(name.set("type name here", ""));
    gui.add(email.set("type email here", ""));
    gui.add(send.setup("<---- draw", 50, 50));
        
    bHide = false;
    gHide = true;
    drawn = false;
    
    display_result = "test";
//    ofDrawBitmapString(result, 100, 100);
//    font.drawString("this is a string hahhahahaha", 100, 100);
    
    camWidth = 640;
    camHeight = 480;
    videoGrabber.initGrabber(camWidth,camHeight);
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    videoGrabber.update();
    
//    if(videoGrabber.isFrameNew()){
//        makeNewFrame();
//    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    videoGrabber.draw(315,400, 480, 270);
    
    font.drawString(display_result, 10, 100);
        
    if(!gHide){
        gcode.draw();
        ofImage currentFrame = ofImage(videoGrabber.getPixels());
        currentFrame.save("frame.jpeg");
        display_result = templateMatching("../../../data/frame.jpeg");
        gHide = true;
    }
//    if(!bHide){
//        gui.draw();
//    }
//    if(!gHide && !drawn){
//        templateMatching("../../../data/IMG_9109.jpeg");
////        plot(); // call this from template matching
////        drawn = true;
//        gHide = true;
//    } else drawn = false;
    

}

// for button listener
void ofApp::sendButtonPressed(){
    gHide = false;
}

// template matching function for blocks (will take input image and make call to generateGCODE())
std::string ofApp::templateMatching(std::string inputImage) {
    
    // running from venv on my computer for now cause opencv wont install for some reason
    std::string command = "/opt/anaconda3/bin/conda run --no-capture-output -n graphics /opt/anaconda3/envs/graphics/bin/python3 ../../../data/template_match.py " + inputImage;
    std::array<char, 128> buffer;
    std::string result;
        
    // using popen to execute the command and capture the output
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        ofLogError() << "Failed to run Python script";
        return "";
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
     
    // trimming
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    std::cout << "closest match: " << result << endl;
    
    
    
    return generateGCODE(templates[result]);
}

// generate GCODE for pen plotter drawing
std::string ofApp::generateGCODE(array<int,2> xy){
    
    /*/////////////////////////////// SETUP STARTS HERE ///////////////////////////////*/
    
    gcode.setup();  // no arguments means 100px per inch
    
    /*
     some initial drawing settings for plotter movement visualization,
     you can just turn these off if you just wanna see the lines to be plotted.
     this is only for testing purposes, as I will not be showing the drawing onscreen
     (it will be plotted)
    */
//    gcode.show_do_not_reverse = true;
//    gcode.show_transit_lines = true;
//    gcode.show_path_with_color = true;
    
    // allows for drawing from inside->out on edges so pen does not catch on edge of paper
    bool set_do_not_reverse_on_edges = true;
    if (set_do_not_reverse_on_edges){
        ofRectangle safe_zone;
        safe_zone.x = 20;
        safe_zone.y = 20;
        safe_zone.width = ofGetWidth() - 40;
        safe_zone.height = ofGetHeight() - 40;
        gcode.set_outwards_only_bounds(safe_zone);
    }
    
    // for text
    ofxHersheyFont hershey;
    float textStartY = ofGetHeight()/2;
    float textStartX = 200;
    float textScale = 4;
    float textRotation = 3*PI/2;
    
    // for drawing
    float rowHeight = 60;
    int rowLength = 13;
    int numRows = 5;

    /*/////////////////////////////// DRAWING STARTS HERE ///////////////////////////////*/
    
    // draw text
    hershey.draw("i am", textStartX, textStartY, textScale, true, textRotation, &gcode);

    // testing code for marking drawing space below text
//    gcode.rect(textStartX+10, 10, ofGetWidth()-textStartX-20, ofGetHeight()-20);
    
    // initialize number of lines and connected lines in each row upfront
    int connectedFromPrev[numRows];
    int linesInRow[numRows];
    int y = xy[0];
    int z = xy[1];
    for (int i=0; i<numRows; i++){
        
        connectedFromPrev[i] = static_cast<int>(round(ofRandom(z, rowLength-y)));
        linesInRow[i] = static_cast<int>(round(ofRandom(connectedFromPrev[i], rowLength-y)));
        
        if (y < z){
            y++;
        }
        if (z > 0){
            z--;
        }
        
    }
    
    // fill and draw first two rows
    int *prev = new int[rowLength];
    int *cur = new int[rowLength];
    int count = rand() % linesInRow[0];
    int m = count;
    
    int count2 = linesInRow[1];
    int matching = connectedFromPrev[1];
    int m2 = count2-matching;
    
    float startX, curX = textStartX + 200;
    int startY, curY = ofGetHeight() - 400;
    
    for (int i = 0; i < rowLength; ++i) {
        
        // fill prev randomly
        if ((rowLength - i) != 0 && rand() % (rowLength-i) < m){
            prev[i] = 1;
            
            float thickness = ofRandom(1,15); // Random thickness between 1 and 5 lines
            float spacing = 1; // Spacing between parallel lines (adjust as needed)
            float offset;
            
            for (int j = 0; j < thickness; ++j){
                offset = j * spacing;
                if (j%2==0){
                    gcode.line(curX, curY+offset*0.5, curX+rowHeight, curY+offset*0.5);
                } else {
                    gcode.line(curX, curY+offset*0.5, curX+rowHeight, curY+offset*0.5);
                }
            }
            
//            gcode.line(curX, curY, curX+rowHeight, curY);
            --m;
        } else prev[i] = 0;
        curX += rowHeight;
        
        // filling matching in cur from left and then rest on right for testing purposes - should change this
        if (prev[i] == 1 && matching > 0) {
            cur[i] = 1;
            
            float thickness = ofRandom(1,1); // Random thickness between 1 and 5 lines
            float spacing = 1; // Spacing between parallel lines (adjust as needed)
            float offset;
            
            for (int j = 0; j < thickness; ++j){
                offset = j * spacing;
                gcode.line(curX, curY+offset, curX+rowHeight, curY+offset);
            }
            
//            gcode.line(curX, curY, curX+rowHeight, curY);
            matching--;
            count2--;
        } else {
            // fill non-matching cur
            if ((rowLength - i) != 0 && rand() % (rowLength-i) < m2 && cur[i] == 0){
                cur[i] = 1;
                
                
                float thickness = ofRandom(1,15); // Random thickness between 1 and 5 lines
                float spacing = 1; // Spacing between parallel lines (adjust as needed)
                float offset;
                
                for (int j = 0; j < thickness; ++j){
                    offset = j * spacing;
                    if (j%2==0){
                        gcode.line(curX, curY+offset*0.5, curX+rowHeight, curY+offset*0.5);
                    } else {
                        gcode.line(curX, curY+offset*0.5, curX+rowHeight, curY+offset*0.5);
                    }
                }
                
//                gcode.line(curX, curY, curX+rowHeight, curY);
                if (prev[i] == 0) {
                    int space = 1;
                    int stop = rand() % 10;
                    while (space < stop){
                        if (i-space > 0 && prev[i-space] == 1){
                            
                            float thickness = ofRandom(1,1); // Random thickness between 1 and 5 lines
                            float spacing = 1; // Spacing between parallel lines (adjust as needed)
                            float offset;
                            
                            for (int j = 0; j < thickness; ++j){
                                offset = j * spacing;
                                gcode.line(curX-offset, curY, curX-offset, curY+(space*20));
                            }
                            
                            
//                            gcode.line(curX, curY, curX, curY+(space*20));
                        } else if (i+space < rowLength && prev[i+space == 1]) {
                            
                            float thickness = ofRandom(1,1); // Random thickness between 1 and 5 lines
                            float spacing = 1; // Spacing between parallel lines (adjust as needed)
                            float offset;
                            
                            for (int j = 0; j < thickness; ++j){
                                offset = j * spacing;
                                gcode.line(curX-offset, curY, curX-offset, curY-(space*20));
                            }
//
//                            gcode.line(curX, curY, curX, curY-(space*20));
                        }
                        space++;
                    }
                }
                --m2;
            } else {
                cur[i] = 0;
            }
        }
        curY -= 20;
        curX -= rowHeight;
        
    }
    
    
    // fill and draw rest of rows
    for (int i = 2; i < numRows; i++){
        // prev = cur;
        for (int j = 0; j < rowLength; j++){
            prev[j] = cur[j];
        }
        
        count = linesInRow[i];
        matching = connectedFromPrev[i];
        m = count - matching;
        
        curY = ofGetHeight()-200;
        curX += rowHeight;
        // filling matching from left and then rest on right for testing purposes - should change this
        for (int j = 0; j < rowLength; j++) {
            if (prev[j] == 1 && matching > 0) {
                cur[j] = 1;
                
                float thickness = ofRandom(1,15); // Random thickness between 1 and 5 lines
                float spacing = 1; // Spacing between parallel lines (adjust as needed)
                float offset;
                
                for (int k = 0; k < thickness; ++k){
                    offset = k * spacing;
                    if (k%2==0){
                        gcode.line(curX, curY+offset*0.5, curX+rowHeight, curY+offset*0.5);
                    } else {
                        gcode.line(curX, curY+offset*0.5, curX+rowHeight, curY+offset*0.5);
                    }
                }
                
//
//                gcode.line(curX, curY, curX+rowHeight, curY);
                matching--;
                count--;
            } else {
                if ((rowLength - j) != 0 && rand() % (rowLength-j) < m && cur[j] == 0){
                    cur[j] = 1;
                    
                    float thickness = ofRandom(1,15); // Random thickness between 1 and 5 lines
                    float spacing = 1; // Spacing between parallel lines (adjust as needed)
                    float offset;
                    
                    for (int k = 0; k < thickness; ++k){
                        offset = k * spacing;
                        if (k%2==0){
                            gcode.line(curX, curY+offset*0.5, curX+rowHeight, curY+offset*0.5);
                        } else {
                            gcode.line(curX, curY+offset*0.5, curX+rowHeight, curY+offset*0.5);
                        }
                    }
                    
//                    gcode.line(curX, curY, curX+rowHeight, curY);
                    if (prev[j] == 0) {
                        int space = 1;
                        int stop = rand() % 10;
                        while (space < stop){
                            if (j-space > 0 && prev[j-space] == 1){
                                
                                float thickness = ofRandom(1,1); // Random thickness between 1 and 5 lines
                                float spacing = 1; // Spacing between parallel lines (adjust as needed)
                                float offset;
                                
                                for (int k = 0; k < thickness; ++k){
                                    offset = k * spacing;
                                    gcode.line(curX-offset, curY, curX-offset, curY+(space*20));
                                }
                                
//                                gcode.line(curX, curY, curX, curY+(space*20));
                            } else if (j+space < rowLength && prev[j+space == 1]) {
                                
                                float thickness = ofRandom(1,1); // Random thickness between 1 and 5 lines
                                float spacing = 1; // Spacing between parallel lines (adjust as needed)
                                float offset;
                                
                                for (int k = 0; k < thickness; ++k){
                                    offset = k * spacing;
                                    gcode.line(curX-offset, curY, curX-offset, curY-(space*20));
                                }
                                
//                                gcode.line(curX, curY, curX, curY-(space*20));
                            }
                            space++;
                        }
                    }
                    --m;
                } else {
                    cur[j] = 0;
                }
            }
            curY -= 20;
        }
        
    }

    //sort() will do its best to reduce the travel time to draw your drawing
    //this will maintain all of the lines, but will change the order that they are drawn in
    //it will almost always speed up the plot, but if you have a specific order you want to draw your lines in you may want to skip this step
    //for drawings with thousands and thousands of lines, this may take a few seconds so you might not want to do this or save while you're still working out the design
    gcode.sort();

    //save will take your drawing and convert it to G-Code, saving it in the bin/data folder
    //You can use a site like https://ncviewer.com/ to test the output
    gcode.save("i-am-iter-1.nc");
    
    return plot();

}

std::string ofApp::plot(){
    std::string filename = "../../../data/axidraw_gcode_reader/reader.py ../../../data/i-am-iter-1.nc";
    std::string command = "/opt/anaconda3/bin/python3 ";
    command += filename;
    system(command.c_str());
    
    std::stringbuf sbuf( std::ios::out ) ; // create a stringbuf
    auto oldbuf = std::cout.rdbuf( std::addressof(sbuf) ) ; // associate the stringbuf with std::cout
    
    std::cout << "certificate of authenticity for " << name << "'s \"I AM (YOU)\", 2025 sent to " << email << endl;
    
    std::cout.rdbuf( oldbuf ) ; // restore cout's original buffer

    std::string output = sbuf.str() ; // get a copy of the underlying string
    
    
    name.set("type name here", "");
    email.set("type email here", "");
    
    return output;
}
