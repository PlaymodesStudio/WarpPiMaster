#include "ofApp.h"


//-------------------------------------------------------------------------------
string ofApp::getIP(string device)
{
    vector<string> list = LocalAddressGrabber :: availableList();
    return LocalAddressGrabber :: getIpAddress(device);
}


//-------------------------------------------------------------------------------
vector<string> ofApp::getDevicesIPs()
{
    vector<string> v = LocalAddressGrabber :: availableList();
    cout << "....................................." << endl;
    cout << " ...... NETWORK DEVICES AND IPs" << endl;
    for(int i=0;i<v.size();i++)
    {
        networkIPs.push_back(LocalAddressGrabber :: getIpAddress(v[i]));
        cout << " ... " << v[i] << " : " << networkIPs[i] << endl;
    }
    cout << "....................................." << endl;
    return v;
}

//-------------------------------------------------------------------------------
vector<string> ofApp::buildDevicesIPsString()
{
    vector<string> ndi;
    for(int i=0;i<networkDevices.size();i++)
    {
        ndi.push_back(networkDevices[i] + " : " + networkIPs[i] );
    }
    return ndi;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

void ofApp::setup()
{
    ofSetBackgroundColor(48, 48, 48);
    deviceSelected = -1;
    networkDevices       = getDevicesIPs();
    networkDevicesAndIPs = buildDevicesIPsString();
    timeLastConnection = ofGetElapsedTimef();
    
    /// READ CONF
    readConfig();
    
    // DAT GUI
    // as we moved from ofxDatGui to ofxDatGui_PM
    ofxDatGui::setAssetPath("");

    // GUI MASTER
    setupGuiMaster();
    
    // TCP
    ///////
    setupTCPConnection(confTCPPort);
    
    // GUI SLAVES
    setupGuiSlaves();
    
    // GUI SCREEN
    setupGuiScreen();

    // GUI VIDEO
    setupGuiVideo();

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
    
    if(!tcpServer.isConnected() && (ofGetElapsedTimef()-timeLastConnection>10.0))
    {
        ofxDatGuiTextInput* i = (ofxDatGuiTextInput*) guiMaster->getTextInput("TCP Port");
        setupTCPConnection(ofToInt(i->getText()));
        timeLastConnection=ofGetElapsedTimef();
        
    }
    
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
int ofApp::getIdFromSlave(int i)
{
    ofxDatGuiToggle* t = slavesListFolder->getToggleAt(i);
    string whichIdString = ofSplitString(t->getLabel()," ")[0];

    return (ofToInt(whichIdString));
}

//-------------------------------------------------------------------------------
void ofApp::onButtonEvent(ofxDatGuiButtonEvent e)
{
//    cout << "onButtonEvent: " << e.target->getLabel() << " " << e.target->getEnabled() << endl;
    
    if(e.target->is("Ping all ¿?"))
    {
        if(tcpServer.isConnected())
        {
            string messageTcp = "all ping";
            sendTcpMessageToAll(messageTcp);
            cout << "Sending PING to ALL clients" << endl;
            
            slavesListFolder->clear();
            slavesListFolder->expand();
            
            guiSlaves->setPosition(guiSlaves->getPosition().x,guiSlaves->getPosition().y);
        }
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
            string messageTcp = ofToString(getIdFromSlave(i)) + " test";
            
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
            string messageTcp = ofToString(getIdFromSlave(i)) + " debug";

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
            string messageTcp = ofToString(getIdFromSlave(i)) + " reboot";

            sendTcpMessageToAll(messageTcp);
        }
    }
    else if(e.target->is("Shutdown"))
    {
        int num = slavesListFolder->size();
        for(int i=0;i<num;i++)
        {
            string messageTcp = ofToString(getIdFromSlave(i)) + " shutdown";
            
            sendTcpMessageToAll(messageTcp);
        }

    }
    else if(e.target->is("Exit"))
    {
        int num = slavesListFolder->size();
        for(int i=0;i<num;i++)
        {
            string messageTcp = ofToString(getIdFromSlave(i)) + " exit";
            
            sendTcpMessageToAll(messageTcp);
        }

    }
    else if(e.target->is("useFbo"))
    {
        int num = slavesListFolder->size();
        for(int i=0;i<num;i++)
        {
            ofxDatGuiToggle* t = slavesListFolder->getToggleAt(i);
            string whichIdString = ofSplitString(t->getLabel()," ")[0];
            int whichId = ofToInt(whichIdString);
            string messageTcp = ofToString(whichId) + " fbo";
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
    else if(e.target->is("doHomography"))
    {
        int num = slavesListFolder->size();
        for(int i=0;i<num;i++)
        {
            ofxDatGuiToggle* t = slavesListFolder->getToggleAt(i);
            string whichIdString = ofSplitString(t->getLabel()," ")[0];
            int whichId = ofToInt(whichIdString);
            string messageTcp = ofToString(whichId) + " homography";
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
    else if(e.target->is("Play"))
    {
        ofxDatGuiToggle* t = slavesListFolder->getToggleAt(0);
        string whichIdString = ofSplitString(t->getLabel()," ")[0];
        int whichId = ofToInt(whichIdString);
        
        //sendTcpMessageToAll(ofToString(whichId) + " load Timecoded_Big_bunny_1.mov 2");
        sendTcpMessageToAll(ofToString(whichId) + " loadImage test/testImage3.jpg 5");
    }
    else if(e.target->is("Play"))
    {
        ofxDatGuiToggle* t = slavesListFolder->getToggleAt(0);
        string whichIdString = ofSplitString(t->getLabel()," ")[0];
        int whichId = ofToInt(whichIdString);

        //sendTcpMessageToAll(ofToString(whichId) + " load Timecoded_Big_bunny_1.mov 2");
    }
    else if(e.target->is("Save Config"))
    {
        saveConfig();
    }
    
    /// GUI MAIN
    /////////////
    
    else if(e.target->is("Reset TCP Connection"))
    {
        ofxDatGuiTextInput* i = (ofxDatGuiTextInput*) guiMaster->getTextInput("TCP Port");
        resetTCPConnection(ofToInt(i->getText()));
    }
    

    /// GUI SCREEN
    ///////////////
    
    else if(e.target->is("Use FBO ?"))
    {
        cout << "Do something when : --use FBO-- is pressed !! TO DO !! " << endl;
    }
    else if(e.target->is("Use Homography ?"))
    {
        cout << "Do something when : --use Homography-- is pressed !! TO DO !! " << endl;
    }
    else if(e.target->is("Edit Quad"))
    {
        cout << "Do something when : --edit Quad-- is pressed !! TO DO !! " << endl;
    }
    else if(e.target->is("Next Corner"))
    {
        cout << "Do something when : --next Corner-- is pressed !! TO DO !! " << endl;
    }
    else if(e.target->is("Previous Corner"))
    {
        cout << "Do something when : --Previous Corner-- is pressed !! TO DO !! " << endl;
    }
    else if(e.target->is("Reset Quad"))
    {
        cout << "Do something when : --Reset Quad-- is pressed !! TO DO !! " << endl;
    }
    else if(e.target->is("Save Quad"))
    {
        cout << "Do something when : --Save Quad-- is pressed !! TO DO !! " << endl;
    }

    
    /// GUI SCREEN
    ///////////////
    
    else if(e.target->is("Play Video"))
    {
        cout << "Do something when : --Play Video-- is pressed !! TO DO !! " << endl;
    }
    else if(e.target->is("Stop Video"))
    {
        cout << "Do something when : --Stop Video-- is pressed !! TO DO !! " << endl;
    }
    else if(e.target->is("Pause Video"))
    {
        cout << "Do something when : --Pause Video-- is pressed !! TO DO !! " << endl;
    }
    else if(e.target->is("Restart Video"))
    {
        cout << "Do something when : --Restart Video-- is pressed !! TO DO !! " << endl;
    }



}

//-------------------------------------------------------------------------------
void ofApp::onImageButtonEvent(ofxDatGuiButtonEvent e)
{
    cout << "onImageButtonEvent: " << e.target->getLabel() << " " << e.target->getEnabled() << endl;
    
    if(e.target->is("play"))
    {
        
    }
    else if(e.target->is("pause"))
    {
        
    }
    else if(e.target->is("stop"))
    {
        ofxDatGuiToggle* t = slavesListFolder->getToggleAt(0);
        string whichIdString = ofSplitString(t->getLabel()," ")[0];
        int whichId = ofToInt(whichIdString);
        sendTcpMessageToAll(ofToString(whichId) + " loadImage noimage 2");
    }
    else if(e.target->is("load image 1"))
    {
        ofxDatGuiToggle* t = slavesListFolder->getToggleAt(0);
        string whichIdString = ofSplitString(t->getLabel()," ")[0];
        int whichId = ofToInt(whichIdString);
        sendTcpMessageToAll(ofToString(whichId) + " loadImage test/testImage1.jpg 5");
    }
    else if(e.target->is("load image 2"))
    {
        ofxDatGuiToggle* t = slavesListFolder->getToggleAt(0);
        string whichIdString = ofSplitString(t->getLabel()," ")[0];
        int whichId = ofToInt(whichIdString);
        sendTcpMessageToAll(ofToString(whichId) + " loadImage test/testImage2.jpg 2");
    }
    else if(e.target->is("load folder"))
    {
        ofxDatGuiToggle* t = slavesListFolder->getToggleAt(0);
        string whichIdString = ofSplitString(t->getLabel()," ")[0];
        int whichId = ofToInt(whichIdString);
        sendTcpMessageToAll(ofToString(whichId) + " loadFolder test 2");
    }

}

//-------------------------------------------------------------------------------
void ofApp::onTextInputEvent(ofxDatGuiTextInputEvent e)
{
    //    cout << "onTextInputEvent: " << e.target->getLabel() << " " << e.target->getText() << endl;
    if(e.target->is("TCP Port"))
    {
        ofxDatGuiTextInput* i = (ofxDatGuiTextInput*) guiMaster->getTextInput("TCP Port");
        resetTCPConnection(ofToInt(i->getText()));
    }
    else if(e.target->is("Load Video"))
    {
        cout << "Do something when : --Load Video-- is pressed !! TO DO !! " << endl;
    }



}

//-------------------------------------------------------------------------------
void ofApp::on2dPadEvent(ofxDatGui2dPadEvent e)
{
    //    cout << "on2dPadEvent: " << e.target->getLabel() << " " << e.x << ":" << e.y << endl;
}

//-------------------------------------------------------------------------------
void ofApp::onDropdownEvent(ofxDatGuiDropdownEvent e)
{
    deviceSelected=e.target->getSelectedIndex();
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
                if(tokens.size()>=2)
                {
                    s.name = tokens[2];
                    s.ip = tcpServer.getClientIP(i);
                }
                else
                {
                    s.name = "defaultName";
                    s.ip = tcpServer.getClientIP(i);
                }
                
                
                ofxDatGuiToggle* tog = slavesListFolder->addToggle(ofToString(s.id) + " " +s.name + " " + s.ip,true);
                tog->setStripe(ofColor(0,128,255), 5);
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
    if(tcpServer.isConnected())
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
}

//--------------------------------------------------------------
void ofApp::saveConfig()
{
    cout << "-----------------------------" << endl;
    cout << "--- Saving to XML Config ---" << endl;
    
    ofxXmlSettings configXML;
    configXML.load("./app/config.xml");
    configXML.pushTag("config");
    
    // Network Device
    if(deviceSelected!=-1)
    {
        configXML.setValue("networkDevice",networkDevices[deviceSelected]);
    }
    else
    {
        configXML.setValue("networkDevice","?");
    }
    cout << "--- Network Device : " << deviceSelected <<" :: "<< networkDevices[deviceSelected] << endl;
    
    // TCP Port
    ofxDatGuiTextInput* tcpP = guiMaster->getTextInput("TCP Port");
    configXML.setValue("TCPPort",tcpP->getText());
    cout << "--- TCP Port : " << tcpP->getText() << endl;
    
    cout << "-----------------------------" << endl;
    configXML.save("./app/config.xml");
    
}
//--------------------------------------------------------------
void ofApp::readConfig()
{
    ofxXmlSettings configXML;
    configXML.load("./app/config.xml");
    configXML.pushTag("config");
    
    /// NETWORK DEVICE
    confNetworkDevice = configXML.getValue("networkDevice","error");
    
    /// TCP SETUP
    confTCPPort = configXML.getValue("TCPPort",11999);
    
    /// add to LOG
    //ofLog(OF_LOG_NOTICE) << "ofApp :: readConfig :: networkDevice " << confNetworkDevice << " :: TCP Port " << confTCPPort << endl;
    cout << "ofApp :: readConfig :: networkDevice " << confNetworkDevice << " :: TCP Port " << confTCPPort << endl;
    
}

//--------------------------------------------------------------
void ofApp::setupTCPConnection(int _port)
{
    tcpLock.lock();

    ofxDatGuiToggle* t = (ofxDatGuiToggle*)guiMaster->getToggle("TCP Connected");
    ofxDatGuiButton* r = (ofxDatGuiButton*)guiMaster->getButton("Reset TCP Connection");
    ofxDatGuiTextInput* i = (ofxDatGuiTextInput*)guiMaster->getTextInput("TCP Port");
    
    if(true)
    {
        cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
        isTcpConnected = tcpServer.setup(_port);

        if (isTcpConnected)
        {
            cout << "TCP Server Setup. OK!! Port : " << _port << endl;
            isTcpConnected = true;
            t->setStripe(ofColor(0,225,0), 5);
            r->setStripe(ofColor(0,255,0), 5);
            i->setStripe(ofColor(0,255,0), 5);

        }
        else
        {
            cout << "TCP Server reSetup FAIL!! . Port : " << _port<< endl;
            isTcpConnected = false;
            t->setStripe(ofColor(255,0,0), 5);
            r->setStripe(ofColor(255,0,0), 5);
            i->setStripe(ofColor(255,0,0), 5);

        }
        t->setEnabled(isTcpConnected);
        cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
    }
    tcpLock.unlock();
    
    
  
}

//--------------------------------------------------------------
void ofApp::resetTCPConnection(int _port)
{
    tcpLock.lock();
    
    string messageTcp = "all close";
    sendTcpMessageToAll(messageTcp);
    sleep(1);
    if(tcpServer.disconnectAllClients())
    {
        cout << "oooooooooooooooooooooooooooooooooooooooooooooooo" << endl;
        cout << "oo TCP Server has disconnected from All Clients ..." << endl;
        if(tcpServer.close())
        {
            tcpLock.unlock();
            cout << "oo TCP Server has been closed for reset ..." << endl;
            setupTCPConnection(_port);
            tcpLock.lock();
            
        }
        else
        {
            cout << "oo Couldn't Close TCP Connection" << endl;
        }
    }
    else
    {
        cout << "oo Couldn't Disconect All clients !! ERROR " << endl;
    }
    tcpLock.unlock();
}


//--------------------------------------------------------------
void ofApp::setupGuiSlaves()
{
    // GUI SLAVE
    ///////////////
    
    // instantiate and position the gui //
    guiSlaves = new ofxDatGui();
    guiSlaves->setPosition(guiMaster->getWidth()+10+10, 10);
    
    // adding the optional header allows you to drag the gui around //
    guiSlaves->addHeader("SLAVES")->setBackgroundColor(ofColor(127));
    
    //slavesListFolder = guiSlaves->addFolder("Slaves List", ofColor::red);
    
    ofColor c = ofColor(255,255,255);
    guiSlaves->addButton("Test")->setStripe(c, 5);
    guiSlaves->addButton("Debug")->setStripe(c, 5);
    guiSlaves->addButton("Reboot")->setStripe(c, 5);
    guiSlaves->addButton("Shutdown")->setStripe(c, 5);
    guiSlaves->addButton("Exit")->setStripe(c, 5);
    guiSlaves->addButton("Ping all ¿?")->setStripe(c, 5);
    
    {
        // add a folder to group a few components together //
        guiSlaves->addBreak();
        ofColor cl = ofColor(0,128,255);
        slavesListFolder = guiSlaves->addFolder("Slaves List", cl);
        slavesListFolder->setStripe(cl, 5);
        
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
                slavesListFolder->addToggle(tog->getLabel(),tog->getEnabled())->setStripe(cl, 5);
                
                // add toggle to slave info
                s.toggle = tog;
            }
            guiSlaves->addBreak();
            slavesListFolder->expand();
        }
        
    }
    
    //    guiSlaves->addFolder(slavesListFolder);
    
    guiSlaves->addBreak();
    guiSlaves->addButton("Select All")->setStripe(ofColor(255,128,0), 5);
    guiSlaves->addButton("Select None")->setStripe(ofColor(255,128,0), 5);
    
    
    // once the gui has been assembled, register callbacks to listen for component specific events //
    guiSlaves->onButtonEvent(this, &ofApp::onButtonEvent);
    guiSlaves->onSliderEvent(this, &ofApp::onSliderEvent);
    guiSlaves->onTextInputEvent(this, &ofApp::onTextInputEvent);
    guiSlaves->on2dPadEvent(this, &ofApp::on2dPadEvent);
    guiSlaves->onDropdownEvent(this, &ofApp::onDropdownEvent);
    guiSlaves->onColorPickerEvent(this, &ofApp::onColorPickerEvent);
    guiSlaves->onMatrixEvent(this, &ofApp::onMatrixEvent);
    
}
//--------------------------------------------------------------
void ofApp::setupGuiMaster()
{
    // GUI MASTER
    ///////////////
    guiMaster = new ofxDatGui();
    
    guiMaster->addHeader("WARPI MASTER")->setBackgroundColor(ofColor(127));
    guiMaster->addToggle("TCP Connected",isTcpConnected);
    ofxDatGuiTextInput *tcpP = guiMaster->addTextInput("TCP Port");
    tcpP->setText(ofToString(confTCPPort));
    tcpP->setStripe(ofColor(255,128,0), 5);
    guiMaster->addButton("Reset TCP Connection");
    
    guiMaster->addDropdown("NETWORK",networkDevicesAndIPs)->setStripe(ofColor(0,128,255), 5);;
    
    // set the dropdown to the device coming from config.xml if exists ... NETWORK instead
    ofxDatGuiDropdown* dd = (ofxDatGuiDropdown*) guiMaster->getDropdown("NETWORK");
    dd->setStripe(ofColor(255,255,255),5);
    int whichOne=-1;
    for(int i=0;i<networkDevices.size();i++)
    {
        if(networkDevices[i]==confNetworkDevice)
        {
            // this is the device we got saved in config.xml
            whichOne=i;
        }
    }
    if(whichOne>-1)
    {
        dd->select(whichOne);
        deviceSelected=whichOne;
    }
    guiMaster->addButton("Save Config")->setStripe(ofColor(255,255,255),5);
    
    guiMaster->setPosition(10, 10);
    
    guiMaster->onButtonEvent(this, &ofApp::onButtonEvent);
    guiMaster->onDropdownEvent(this, &ofApp::onDropdownEvent);
    guiMaster->onTextInputEvent(this, &ofApp::onTextInputEvent);
    
}
//--------------------------------------------------------------
void ofApp::setupGuiScreen()
{
    // GUI SLAVE
    ///////////////
    
    // instantiate and position the gui //
    guiScreen = new ofxDatGui();
    guiScreen->setPosition(guiSlaves->getPosition().x + guiSlaves->getWidth() +10, 10);
    
    // adding the optional header allows you to drag the gui around //
    guiScreen->addHeader("SCREEN")->setBackgroundColor(ofColor(127));
    
    //slavesListFolder = guiSlaves->addFolder("Slaves List", ofColor::red);
    
    ofColor c = ofColor(255,255,255);
    guiScreen->addToggle("Use FBO ?")->setStripe(c, 5);
    guiScreen->addToggle("Use Homography ?")->setStripe(c, 5);
    guiScreen->addBreak();
    guiScreen->addToggle("Edit Quad")->setStripe(c, 5);
    guiScreen->addButton("Next Corner")->setStripe(c, 5);
    guiScreen->addButton("Previous Corner")->setStripe(c, 5);
    guiScreen->addLabel("Use arrow keys to move corners.")->setStripe(ofColor(128), 5);;
    guiScreen->addBreak();
    guiScreen->addButton("Reset Quad")->setStripe(c, 5);
    guiScreen->addButton("Save Quad")->setStripe(c, 5);
    
    
    // once the gui has been assembled, register callbacks to listen for component specific events //
    guiScreen->onButtonEvent(this, &ofApp::onButtonEvent);
//    guiSlaves->onSliderEvent(this, &ofApp::onSliderEvent);
//    guiSlaves->onTextInputEvent(this, &ofApp::onTextInputEvent);
//    guiSlaves->on2dPadEvent(this, &ofApp::on2dPadEvent);
//    guiSlaves->onDropdownEvent(this, &ofApp::onDropdownEvent);
//    guiSlaves->onColorPickerEvent(this, &ofApp::onColorPickerEvent);
//    guiSlaves->onMatrixEvent(this, &ofApp::onMatrixEvent);

    
}
//--------------------------------------------------------------
void ofApp::setupGuiVideo()
{
    // GUI SLAVE
    ///////////////
    
    // instantiate and position the gui //
    guiVideo = new ofxDatGui();
    guiVideo->setPosition(guiScreen->getPosition().x + guiScreen->getWidth() +10, 10);
    
    // adding the optional header allows you to drag the gui around //
    guiVideo->addHeader("VIDEO")->setBackgroundColor(ofColor(127));
    
    //slavesListFolder = guiSlaves->addFolder("Slaves List", ofColor::red);
    
    ofColor c = ofColor(255,255,255);
    guiVideo->addButton("Play Video")->setStripe(c, 5);
    guiVideo->addButton("Stop Video")->setStripe(c, 5);
    guiVideo->addButton("Pause Video")->setStripe(c, 5);
    guiVideo->addButton("Restart Video")->setStripe(c, 5);
    guiVideo->addTextInput("Load Video","test.mov")->setStripe(c, 5);
    
    guiVideo->addBreak();
    
    // once the gui has been assembled, register callbacks to listen for component specific events //
    guiVideo->onButtonEvent(this, &ofApp::onButtonEvent);
    guiVideo->onTextInputEvent(this, &ofApp::onTextInputEvent);
    //    guiSlaves->onSliderEvent(this, &ofApp::onSliderEvent);
    //    guiSlaves->on2dPadEvent(this, &ofApp::on2dPadEvent);
    //    guiSlaves->onDropdownEvent(this, &ofApp::onDropdownEvent);
    //    guiSlaves->onColorPickerEvent(this, &ofApp::onColorPickerEvent);
    //    guiSlaves->onMatrixEvent(this, &ofApp::onMatrixEvent);
    
    
}

