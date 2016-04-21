#include "ofApp.h"

//// http://stackoverflow.com/questions/7072989/iphone-ipad-how-to-get-my-ip-address-programmatically
//string ofxBonjourIp::GetMyIPAddress()
//{
//    struct ifaddrs *interfaces = NULL;
//    struct ifaddrs *temp_addr = NULL;
//    string networkAddress = "";
//    string cellAddress = "";
//    
//    // retrieve the current interfaces - returns 0 on success
//    if(!getifaddrs(&interfaces)) {
//        // Loop through linked list of interfaces
//        temp_addr = interfaces;
//        while(temp_addr != NULL) {
//            sa_family_t sa_type = temp_addr->ifa_addr->sa_family;
//            if(sa_type == AF_INET || sa_type == AF_INET6) {
//                string name = temp_addr->ifa_name; //en0
//                string addr = inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr); // pdp_ip0
//                
//                // ignore localhost "lo0" addresses 127.0.0.1, and "0.0.0.0"
//                //if(!ofIsStringInString(name, "lo") && addr != "0.0.0.0") {
//                if(addr != "127.0.0.1" && addr != "0.0.0.0") {
//                    
//                    // can assume here it's "en0" or "en3" or "wlan0" or "pdp_ip0" (cell address)
//                    // may need to add in a check to match the name (used to be matched to "en0")
//                    ofLog() << "interface name / ip address: " << name << " / " << addr;
//                    if(name == "pdp_ip0") {
//                        // Interface is the cell connection on the iPhone
//                        cellAddress = addr;
//                    } else {
//                        // if(name == "en0") - ignoring the name as this can be different
//                        networkAddress = addr;
//                    }
//                }
//                
//            }
//            temp_addr = temp_addr->ifa_next;
//        }
//        // Free memory
//        freeifaddrs(interfaces);
//    }
//    
//    // will return 0.0.0.0 of it hasn't found address
//    string address = (networkAddress != "") ? networkAddress : cellAddress;
//    return (address != "") ? address : "0.0.0.0";
//}

void ofApp::setup()
{
    // GUI MASTER
    ///////////////

    guiMaster = new ofxDatGui();
    guiMaster->addHeader("WARPI MASTER");
    string myIP = "127.127.127.127";
    guiMaster->addLabel(myIP);
    guiMaster->setPosition(10, 10);

    // GUI SLAVE
    ///////////////
    // instantiate and position the gui //
    guiSlaves = new ofxDatGui();
    guiSlaves->setPosition(guiMaster->getWidth()+10+10, 10);

    // adding the optional header allows you to drag the gui around //
    guiSlaves->addHeader("SLAVES");
    guiSlaves->addButton("Ping Slaves");
    
    // add a folder to group a few components together //
    guiSlaves->addBreak();
    slavesListFolder = guiSlaves->addFolder("Slaves List", ofColor::red);
    
    for(int i=0;i<6;i++)
    {
        ofxDatGuiToggle* t = new ofxDatGuiToggle("ID_" +ofToString(i),true);
        slavesListFolder->addToggle(t->getLabel(),t->getEnabled());
        slavesList.push_back(*t);
    }
    guiSlaves->addBreak();
    slavesListFolder->expand();

    guiSlaves->addButton("Select All");
    guiSlaves->addButton("Select None");
    guiSlaves->addButton("Test");
    guiSlaves->addButton("Reboot");
    guiSlaves->addButton("Shutdown");
    guiSlaves->addButton("Exit");
    
    // once the gui has been assembled, register callbacks to listen for component specific events //
   guiSlaves->onButtonEvent(this, &ofApp::onButtonEvent);
   guiSlaves->onSliderEvent(this, &ofApp::onSliderEvent);
   guiSlaves->onTextInputEvent(this, &ofApp::onTextInputEvent);
   guiSlaves->on2dPadEvent(this, &ofApp::on2dPadEvent);
   guiSlaves->onDropdownEvent(this, &ofApp::onDropdownEvent);
   guiSlaves->onColorPickerEvent(this, &ofApp::onColorPickerEvent);
   guiSlaves->onMatrixEvent(this, &ofApp::onMatrixEvent);
    
    // launch the app //
    mFullscreen = false;
    refreshWindow();
}

//-------------------------------------------------------------------------------
void ofApp::onSliderEvent(ofxDatGuiSliderEvent e)
{
    cout << "onSliderEvent: " << e.target->getLabel() << " "; e.target->printValue();
    //if (e.target->is("datgui opacity"))guiSlaves->setOpacity(e.scale);
}

//-------------------------------------------------------------------------------
void ofApp::onButtonEvent(ofxDatGuiButtonEvent e)
{
    cout << "onButtonEvent: " << e.target->getLabel() << " " << e.target->getEnabled() << endl;
    
    if(e.target->is("Ping Slaves"))
    {
        
    }
    else if(e.target->is("Select All"))
    {
        int num = slavesListFolder->size();
        for(int i=0;i<num;i++)
        {
            //ofxDatGuiFolder slavesListFolder->getInstance()
        }
    }
    else if(e.target->is("Select None"))
    {
        
    }
    else if(e.target->is("Test"))
    {
        
    }
    else if(e.target->is("Reboot"))
    {
        
    }
    else if(e.target->is("Shutdown"))
    {
        
    }
    else if(e.target->is("Exit"))
    {
        
    }
}

//-------------------------------------------------------------------------------
void ofApp::onTextInputEvent(ofxDatGuiTextInputEvent e)
{
//    cout << "onTextInputEvent: " << e.target->getLabel() << " " << e.target->getText() << endl;
}

//-------------------------------------------------------------------------------
void ofApp::on2dPadEvent(ofxDatGui2dPadEvent e)
{
//    cout << "on2dPadEvent: " << e.target->getLabel() << " " << e.x << ":" << e.y << endl;
}

//-------------------------------------------------------------------------------
void ofApp::onDropdownEvent(ofxDatGuiDropdownEvent e)
{
//    cout << "onDropdownEvent: " << e.target->getLabel() << " Selected" << endl;
}

//-------------------------------------------------------------------------------
void ofApp::onColorPickerEvent(ofxDatGuiColorPickerEvent e)
{
//    cout << "onColorPickerEvent: " << e.target->getLabel() << " " << e.target->getColor() << endl;
//    ofSetBackgroundColor(e.color);
}

//-------------------------------------------------------------------------------
void ofApp::onMatrixEvent(ofxDatGuiMatrixEvent e)
{
//    cout << "onMatrixEvent " << e.child << " : " << e.enabled << endl;
//    cout << "onMatrixEvent " << e.target->getLabel() << " : " << e.target->getSelected().size() << endl;
}

//-------------------------------------------------------------------------------
void ofApp::draw() { }
//-------------------------------------------------------------------------------
void ofApp::update() { }

//-------------------------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    if (key == 'f') {
        toggleFullscreen();
    }
}

//-------------------------------------------------------------------------------
void ofApp::toggleFullscreen()
{
    mFullscreen = !mFullscreen;
    refreshWindow();
}

//-------------------------------------------------------------------------------
void ofApp::refreshWindow()
{
    ofSetFullscreen(mFullscreen);
    if (!mFullscreen) {
        ofSetWindowShape(1920, 1400);
        ofSetWindowPosition((ofGetScreenWidth()/2)-(1920/2), 0);
    }
}

