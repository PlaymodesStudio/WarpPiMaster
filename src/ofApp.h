#pragma once

#include "ofMain.h"
#include "ofxDatGui.h"
#include "ofxNetwork.h"
#include "LocalAddressGrabber.h"
#include "ofxXmlSettings.h"

typedef struct
{
    int                 id;
    string              name;
    string              ip;
    ofxDatGuiToggle*    toggle;
} slaveInfo;


class ofApp : public ofBaseApp
{
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
    ofxDatGui*              guiSlaves;
    ofxDatGui*              guiMaster;
    ofxDatGuiFolder*        slavesListFolder;
    
    // TCP
    void                setupTCPConnection(int port);
    void                resetTCPConnection(int port);
    int                 tcpPort;
    bool                isTcpConnected;
    ofxTCPServer        tcpServer;
    ofMutex             tcpLock;
    void                handleTcpOut();
    void                sendTcpMessageToAll(string mess);
    void                handleTcpIn();
    float               timeLastConnection;

    // IP
    vector<string>      getDevicesIPs();
    vector<string>      buildDevicesIPsString();
    vector<string>      networkDevices;
    vector<string>      networkIPs;
    vector<string>      networkDevicesAndIPs;
    string              getIP(string device);
    int                 deviceSelected;
    
    /// CONFIG
    void                        readConfig();
    void                        saveConfig();
    string                      confNetworkDevice;
    int                         confTCPPort;

};
