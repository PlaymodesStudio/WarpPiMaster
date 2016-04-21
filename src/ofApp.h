#pragma once

#include "ofMain.h"
#include "ofxDatGui.h"
#include "ofxNetwork.h"

class ofApp : public ofBaseApp{

	public:
        void setup();
        void draw();
        void update();
        void exit();
    
        bool mFullscreen;
        void refreshWindow();
        void toggleFullscreen();

        void keyPressed(int key);
        void onButtonEvent(ofxDatGuiButtonEvent e);
        void onSliderEvent(ofxDatGuiSliderEvent e);
        void onTextInputEvent(ofxDatGuiTextInputEvent e);
        void on2dPadEvent(ofxDatGui2dPadEvent e);
        void onDropdownEvent(ofxDatGuiDropdownEvent e);
        void onColorPickerEvent(ofxDatGuiColorPickerEvent e);
        void onMatrixEvent(ofxDatGuiMatrixEvent e);
    
    // GUI
    ofxDatGui* guiSlaves;
    ofxDatGui* guiMaster;
    ofxDatGuiFolder* slavesListFolder;
    vector<ofxDatGuiToggle> slavesList;
    
    // TCP
    int                 tcpPort;
    bool                isTcpConnected;
    ofxTCPServer        tcpServer;
    ofMutex             tcpLock;
    void                handleTcpOut();
    void                sendTcpMessageToAll(string mess);
    void                handleTcpIn();

};
