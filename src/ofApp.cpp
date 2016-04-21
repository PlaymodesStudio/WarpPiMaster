#include "ofApp.h"

// IP SHIT

// https://www.google.com/url?q=https%3A%2F%2Fgithub.com%2FPlaymodesStudio%2FofxArtNet_PM%2Fblob%2Fmaster%2Fsrc%2FLocalAddressGrabber.h&sa=D&sntz=1&usg=AFQjCNE3-diuprU7GRR5xGUcSmyf5CTFWw

// https://www.google.com/url?q=https%3A%2F%2Fgithub.com%2FPlaymodesStudio%2FofxArtNet_PM%2Fblob%2Fmaster%2Fsrc%2FofxArtNet.cpp%23L18&sa=D&sntz=1&usg=AFQjCNGsGat2blUS6spgw7sJBUXdQZMU4Q

//

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
    // TCP
    ///////
    tcpPort = 11999;
    tcpLock.lock();
    if(true)
    {
        cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
        isTcpConnected = tcpServer.setup(tcpPort);
        if (isTcpConnected) cout << "TCP Server Setup. Port : " << tcpPort << endl;
        else cout << "TCP Server Setup. Port : " << tcpPort << " ERROR on setup !!" << endl;
        cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
    }
    tcpLock.unlock();

    
    
    // GUI MASTER
    ///////////////
    // as we moved from ofxDatGui to ofxDatGui_PM
    ofxDatGui::setAssetPath("");

    guiMaster = new ofxDatGui();
    
    guiMaster->addHeader("WARPI MASTER");
    
    
    string myIP = "127.127.127.127";
    guiMaster->addToggle("TCP Connected",isTcpConnected);
    guiMaster->addLabel(myIP);
    guiMaster->addTextInput("TCP Port")->setText(ofToString(tcpPort));
    guiMaster->setPosition(10, 10);
    guiMaster->addButton("Reset TCP Connection");
    
    guiMaster->onButtonEvent(this, &ofApp::onButtonEvent);

    // GUI SLAVE
    ///////////////
    
    // instantiate and position the gui //
    guiSlaves = new ofxDatGui();
    guiSlaves->setPosition(guiMaster->getWidth()+10+10, 10);

    // adding the optional header allows you to drag the gui around //
    guiSlaves->addHeader("SLAVES");

    //slavesListFolder = guiSlaves->addFolder("Slaves List", ofColor::red);
    
    guiSlaves->addButton("Test")->setStripe(ofColor(0,164,220), 5);
    guiSlaves->addButton("Debug")->setStripe(ofColor(0,164,220), 5);;
    guiSlaves->addButton("Reboot")->setStripe(ofColor(0,164,220), 5);;
    guiSlaves->addButton("Shutdown")->setStripe(ofColor(0,164,220), 5);;
    guiSlaves->addButton("Exit")->setStripe(ofColor(0,164,220), 5);;
    guiSlaves->addButton("Select All")->setStripe(ofColor(0,255,64), 5);
    guiSlaves->addButton("Select None")->setStripe(ofColor(0,255,64), 5);
    guiSlaves->addButton("PING ?")->setStripe(ofColor(255,255,0), 5);
    // add a folder to group a few components together //
    guiSlaves->addBreak();
    slavesListFolder = new ofxDatGuiFolder("Slaves List", ofColor::red);
    slavesListFolder->setStripe(ofColor(255,0,0), 5);
    
    if(false)
    {
        for(int i=0;i<6;i++)
        {
            // to slave info
            slaveInfo s;
            s.id = i;
            s.name = "Pi_" + ofToString(i);
            s.ip = "192.168.0." + ofToString(i+101);
    
            // to the gui
            ofxDatGuiToggle* tog = new ofxDatGuiToggle(ofToString(s.id) + " " +s.name + " " + s.ip,true);
            slavesListFolder->addToggle(tog->getLabel(),tog->getEnabled());
    
            // add toggle to slave info
            s.toggle = tog;
        }
        guiSlaves->addBreak();
        slavesListFolder->expand();
    }
    
    guiSlaves->addFolder(slavesListFolder);

    
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
void ofApp::update()
{
    // TCP
    tcpLock.lock();
    handleTcpIn();
    handleTcpOut();
    tcpLock.unlock();
}

//-------------------------------------------------------------------------------
void ofApp::draw()
{

}

//--------------------------------------------------------------
void ofApp::exit()
{
    cout << "Trying to close TCP Server on exit() !!" << endl;

    tcpLock.lock();
    
    if(tcpServer.disconnectAllClients())
    {
        if(tcpServer.close())
        {
            cout << "TCP Closed and All Clients Disconnected !! " << endl;
        }
        else
        {
            cout << "Couldn't Close TCP Connection" << endl;
        }
    }
    else
    {
        cout << "Couldn't Disconect All clients !! ERROR " << endl;
    }

    tcpLock.unlock();
    
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
    
    if(e.target->is("PING ?"))
    {
        string messageTcp = "all ping";
        sendTcpMessageToAll(messageTcp);
        cout << "Sending PING to ALL clients" << endl;
        
        //slavesListFolder->collapse();
        slavesListFolder->clear();
        
        guiSlaves->setPosition(guiSlaves->getPosition().x,guiSlaves->getPosition().y);

    }
    else if(e.target->is("Select All"))
    {
        int num = slavesListFolder->size();
        for(int i=0;i<num;i++)
        {
            ofxDatGuiToggle* t = slavesListFolder->getToggleAt(i);
            t->setEnabled(true);
        }
    }
    else if(e.target->is("Select None"))
    {
        int num = slavesListFolder->size();
        for(int i=0;i<num;i++)
        {
            ofxDatGuiToggle* t = slavesListFolder->getToggleAt(i);
            t->setEnabled(false);
        }
        
    }
    else if(e.target->is("Test"))
    {
        int num = slavesListFolder->size();
        for(int i=0;i<num;i++)
        {
            ofxDatGuiToggle* t = slavesListFolder->getToggleAt(i);
            string whichIdString = ofSplitString(t->getLabel()," ")[0];
            int whichId = ofToInt(whichIdString);
            string messageTcp = ofToString(whichId) + " test";
            if(t->getEnabled())
            {
                messageTcp=messageTcp + " 1";
            }
            else
            {
                messageTcp=messageTcp + " 0";
            }
            sendTcpMessageToAll(messageTcp);
        }

        
    }
    else if(e.target->is("Debug"))
    {
        int num = slavesListFolder->size();
        for(int i=0;i<num;i++)
        {
            ofxDatGuiToggle* t = slavesListFolder->getToggleAt(i);
            string whichIdString = ofSplitString(t->getLabel()," ")[0];
            int whichId = ofToInt(whichIdString);
            string messageTcp = ofToString(whichId) + " debug";
            if(t->getEnabled())
            {
                messageTcp=messageTcp + " 1";
            }
            else
            {
                messageTcp=messageTcp + " 0";
            }
            sendTcpMessageToAll(messageTcp);
        }
    }
    else if(e.target->is("Reboot"))
    {
        int num = slavesListFolder->size();
        for(int i=0;i<num;i++)
        {
            ofxDatGuiToggle* t = slavesListFolder->getToggleAt(i);
            string whichIdString = ofSplitString(t->getLabel()," ")[0];
            int whichId = ofToInt(whichIdString);
            string messageTcp = ofToString(whichId) + " reboot";
//            if(t->getEnabled())
//            {
//                messageTcp=messageTcp + " 1";
//            }
//            else
//            {
//                messageTcp=messageTcp + " 0";
//            }
            sendTcpMessageToAll(messageTcp);
        }
    }
    else if(e.target->is("Shutdown"))
    {
        int num = slavesListFolder->size();
        for(int i=0;i<num;i++)
        {
            ofxDatGuiToggle* t = slavesListFolder->getToggleAt(i);
            string whichIdString = ofSplitString(t->getLabel()," ")[0];
            int whichId = ofToInt(whichIdString);
            string messageTcp = ofToString(whichId) + " shutdown";
//            if(t->getEnabled())
//            {
//                messageTcp=messageTcp + " 1";
//            }
//            else
//            {
//                messageTcp=messageTcp + " 0";
//            }
            sendTcpMessageToAll(messageTcp);
        }

    }
    else if(e.target->is("Exit"))
    {
        int num = slavesListFolder->size();
        for(int i=0;i<num;i++)
        {
            ofxDatGuiToggle* t = slavesListFolder->getToggleAt(i);
            string whichIdString = ofSplitString(t->getLabel()," ")[0];
            int whichId = ofToInt(whichIdString);
            string messageTcp = ofToString(whichId) + " exit";
//            if(t->getEnabled())
//            {
//                messageTcp=messageTcp + " 1";
//            }
//            else
//            {
//                messageTcp=messageTcp + " 0";
//            }
            sendTcpMessageToAll(messageTcp);
        }

    }
    else if(e.target->is("Reset TCP Connection"))
    {
        tcpLock.lock();
        
        if(tcpServer.disconnectAllClients())
        {
            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
            cout << "TCP Server has disconnected from All Clients ..." << endl;
            if(tcpServer.close())
            {
                cout << "TCP Server has been closed for reset ..." << endl;
                // CREATE TCP SERVER AGAIN
                isTcpConnected = tcpServer.setup(tcpPort);
                if (isTcpConnected)
                {
                    cout << "TCP Server Setup. OK!! Port : " << tcpPort << endl;
                    isTcpConnected = true;
                    
                }
                else cout << "TCP Server reSetup FAIL!! . Port : " << tcpPort<< endl;
            }
            else
            {
                cout << "Couldn't Close TCP Connection" << endl;
            }
        }
        else
        {
            cout << "Couldn't Disconect All clients !! ERROR " << endl;
        }
        tcpLock.unlock();

        ofxDatGuiToggle* t = (ofxDatGuiToggle*)guiMaster->getToggle("TCP Connected");
        cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
        t->setEnabled(isTcpConnected);
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
//    if (!mFullscreen) {
//        ofSetWindowShape(1920, 1400);
//        ofSetWindowPosition((ofGetScreenWidth()/2)-(1920/2), 0);
//    }
}

//-------------------------------------------------------------------------------
// TCP
//-------------------------------------------------------------------------------
///--------------------------------------------------------------
void ofApp::handleTcpOut()
{
    
    if(!tcpServer.isConnected())
    {
        return;
    }
    
    
    int numMessages = 0;
    
    /*
    
    //any bangs that came our way this frame send them out too
    for(int i = 0; i < bangsReceived.size(); i++)
    {
        cout << "TCP FOUND BANGS! oscAddress : " << bangsReceived[i].getAddress() << " :: " << bangsReceived[i].getArgAsString(0) << endl;
        string addressWithoutSlash = bangsReceived[i].getAddress().substr(1,bangsReceived[i].getAddress().size()-1);
        // SPLIT STRING ?
        string buf; // Have a buffer string
        stringstream ss(addressWithoutSlash); // Insert the string into a stream
        vector<string> tokens; // Create vector to hold our words
        while (ss >> buf)
        {
            tokens.push_back(buf);
        }
        
        // send a TCP MESSAGE FOR EVERY ADDRESS ITEM
        for(int j=0;j<tokens.size();j++)
        {
            string messageTcp = tokens[j] + " " + bangsReceived[i].getArgAsString(0);
            sendTcpMessageToAll(messageTcp);
        }
        
    }
    */
}

//-------------------------------------------------------------------------------
void ofApp::handleTcpIn()
{
    if((!tcpServer.isConnected()))
    {
        return;
    }
    
    for(int i = 0; i < tcpServer.getLastID(); i++)
    {
        if( !tcpServer.isClientConnected(i) ) continue;
        
        string str = tcpServer.receive(i);
        
        if(str.length() > 0)
        {
            
            vector<string> tokens = ofSplitString(str, " ");
            
            if(tokens[0]=="pong")
            {
                //slavesListFolder->collapse();

                int theId = ofToInt(tokens[1]);
                cout << "Hi !! I got a PONG TCP message !! >> " << str <<" <<  from client : " << i << " with ID : " << theId << endl;

                // to slave info
                slaveInfo s;
                s.id = theId;
                s.name = "myName?";
                s.ip = "myIPAddress?";
                
                
                ofxDatGuiToggle* tog = slavesListFolder->addToggle(ofToString(s.id) + " " +s.name + " " + s.ip,true);
                tog->setStripe(ofColor(200,0,0), 5);
                tog->setBackgroundColor(ofColor(32));
                // stupid hack to
                guiSlaves->setPosition(guiSlaves->getPosition().x,guiSlaves->getPosition().y);

                s.toggle = tog;
                slavesListFolder->expand();
            }
        }
        
    }
}

//-------------------------------------------------------------------------------
void ofApp::sendTcpMessageToAll(string mess)
{
    /// SEND TO ALL TCP CLIENTS !!
    //for each client lets send them a message
    for(int j = 0; j < tcpServer.getLastID(); j++)
    {
        if( !tcpServer.isClientConnected(j) )
        {
            continue;
        }
        tcpServer.send(j,mess);
        cout << "TCP Send : " << mess << endl;
    }
}

