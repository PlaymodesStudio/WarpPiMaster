#include "ofApp.h"

# pragma mark ---------- Setup and loop ----------

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
    
    // GUI IMAGE
    setupGuiImage();
    
    // GUI DMX
    setupGuiDmx();
    
    // GUI ARTNET
    setupGuiArtnet();
    
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
    
    // if we got too much time without connection ...
    if(!tcpServer.isConnected() && (ofGetElapsedTimef()-timeLastConnection>35.0))
    {
        ofxDatGuiTextInput* i = (ofxDatGuiTextInput*) guiMaster->getTextInput("TCP Port");
        // reset TCP connection
        resetTCPConnection(ofToInt(i->getText()));
        
        timeLastConnection=ofGetElapsedTimef();
        
        // send ping to all
        sendTCPPingAll();
        
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

# pragma mark ---------- Auxiliari methods ----------
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

//-------------------------------------------------------------------------------
int ofApp::getIdFromSlave(int i)
{
    ofxDatGuiToggle* t = slavesListFolder->getToggleAt(i);
    string whichIdString = ofSplitString(t->getLabel()," ")[0];
    
    return (ofToInt(whichIdString));
}

# pragma mark ---------- Button Events ----------
//-------------------------------------------------------------------------------
void ofApp::onMasterButtonEvent(ofxDatGuiButtonEvent e)
{
    if(e.target->is("Reset TCP Connection"))
    {
        ofxDatGuiTextInput* i = (ofxDatGuiTextInput*) guiMaster->getTextInput("TCP Port");
        resetTCPConnection(ofToInt(i->getText()));
    }
}

//-------------------------------------------------------------------------------
void ofApp::onSlavesButtonEvent(ofxDatGuiButtonEvent e)
{
    if(e.target->is("Ping all ¿?"))
    {
        sendTCPPingAll();
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
        string toggleState = ((e.target->getEnabled()) ? "1" : "0");
        sendMessageToSlavesFolder("test " + toggleState);
    }
    else if(e.target->is("Debug"))
    {
        string toggleState = ((e.target->getEnabled()) ? "1" : "0");
        sendMessageToSlavesFolder("debug " + toggleState);
    }
    else if(e.target->is("Reboot"))
    {
        sendMessageToSlavesFolder("reboot");
    }
    else if(e.target->is("Shutdown"))
    {
        sendMessageToSlavesFolder("shutdown");
    }
    else if(e.target->is("Exit"))
    {
        sendMessageToSlavesFolder("exit");
    }
    else if(e.target->is("Save Config"))
    {
        saveConfig();
    }
}

//-------------------------------------------------------------------------------
void ofApp::onScreenButtonEvent(ofxDatGuiButtonEvent e)
{
    if(e.target->is("Use FBO ?"))
    {
        string toggleState = ((e.target->getEnabled()) ? "1" : "0");
        sendMessageToSlavesFolder("fbo " + toggleState);
    }
    else if(e.target->is("Use Homography ?"))
    {
        string toggleState = ((e.target->getEnabled()) ? "1" : "0");
        sendMessageToSlavesFolder("homography " + toggleState);
    }
    else if(e.target->is("Edit Quad"))
    {
        string toggleState = ((e.target->getEnabled()) ? "1" : "0");
        sendMessageToSlavesFolder("editQuad " + toggleState);
        
        ofColor c;
        //color change
        if(toggleState == "1")
            c = ofColor::darkGreen;
        else
            c = ofColor::darkRed;
        
        guiScreen->getToggle("Edit Quad")->setStripeColor(c);
        guiScreen->getButton("Next Corner")->setStripeColor(c);
        guiScreen->getButton("Previous Corner")->setStripeColor(c);
        guiScreen->getButton("Reset Quad")->setStripeColor(c);
        guiScreen->getButton("Save Quad")->setStripeColor(c);
        
    }
    
    if(guiScreen->getToggle("Edit Quad")->getEnabled()){
        if(e.target->is("Next Corner"))
        {
            sendMessageToSlavesFolder("nextQuadPoint");
        }
        else if(e.target->is("Previous Corner"))
        {
            sendMessageToSlavesFolder("preQuadPoint");
        }
        else if(e.target->is("Reset Quad"))
        {
            sendMessageToSlavesFolder("resetQuad");
        }
        else if(e.target->is("Save Quad"))
        {
            sendMessageToSlavesFolder("saveQuad");
        }
    }
}

//-------------------------------------------------------------------------------
void ofApp::onVideoButtonEvent(ofxDatGuiButtonEvent e)
{
    if(e.target->is("Play Video"))
    {
        sendMessageToSlavesFolder("playVideo " + ofToString(guiVideo->getSlider("Fade Time")->getValue()));
    }
    else if(e.target->is("Stop Video"))
    {
        sendMessageToSlavesFolder("stopVideo " + ofToString(guiVideo->getSlider("Fade Time")->getValue()));
    }
    else if(e.target->is("Pause Video"))
    {
        sendMessageToSlavesFolder("pauseVideo " + ofToString(guiVideo->getSlider("Fade Time")->getValue()));
    }
    else if(e.target->is("Restart Video"))
    {
        sendMessageToSlavesFolder("restartVideo " + ofToString(guiVideo->getSlider("Fade Time")->getValue()));
    }
}

//-------------------------------------------------------------------------------
void ofApp::onImageButtonEvent(ofxDatGuiButtonEvent e)
{
    cout << "onImageButtonEvent: " << e.target->getLabel() << " " << e.target->getEnabled() << endl;
    
    if(e.target->is("play"))
    {
        sendMessageToSlavesFolder("playImage " + ofToString(guiImage->getSlider("Fade Time")->getValue()));
    }
    else if(e.target->is("stop"))
    {
        sendMessageToSlavesFolder("stopImage " + ofToString(guiImage->getSlider("Fade Time")->getValue()));
    }
}

//-------------------------------------------------------------------------------
void ofApp::onDmxButtonEvent(ofxDatGuiButtonEvent e)
{
    cout << "onDmxButtonEvent: " << e.target->getLabel() << " " << e.target->getEnabled() << endl;
    
    if(e.target->is("All chan Test"))
    {
        if(e.target->getEnabled())
            sendMessageToSlavesFolder("setAllDMXCh 255 0");
        else
            sendMessageToSlavesFolder("setAllDMXXh 0 0");
    }
    else if(e.target->is("stop"))
    {
        sendMessageToSlavesFolder("stopImage " + ofToString(guiImage->getSlider("Fade Time")->getValue()));
    }
}

//-------------------------------------------------------------------------------
void ofApp::onArtnetButtonEvent(ofxDatGuiButtonEvent e)
{
    if(e.target->is("Play Artnet Video"))
    {
        sendMessageToSlavesFolder("playArtnet " + ofToString(0));
    }
    else if(e.target->is("Stop Artnet Video"))
    {
        sendMessageToSlavesFolder("stopArtnet " + ofToString(0));
    }
    else if(e.target->is("Pause Artnet Video"))
    {
        sendMessageToSlavesFolder("pauseArtnet " + ofToString(0));
    }
    else if(e.target->is("Restart Artnet Video"))
    {
        sendMessageToSlavesFolder("restartArtnet " + ofToString(0));
    }
}

# pragma mark ---------- Other GUI events ----------

//-------------------------------------------------------------------------------
void ofApp::onTextInputEvent(ofxDatGuiTextInputEvent e)
{
    if(e.target->is("TCP Port"))
    {
        ofxDatGuiTextInput* i = (ofxDatGuiTextInput*) guiMaster->getTextInput("TCP Port");
        resetTCPConnection(ofToInt(i->getText()));
    }
    else if(e.target->is("Load Video"))
    {
        sendMessageToSlavesFolder("loadVideo " + e.target->getText() + " " + ofToString(guiVideo->getSlider("Fade Time")->getValue()));
    }
    else if(e.target->is("Load Artnet Video"))
    {
        sendMessageToSlavesFolder("loadArtnet " + e.target->getText() + " " + ofToString(0));
    }
    else if(e.target->is("Load Image"))
    {
        sendMessageToSlavesFolder("loadImage " + e.target->getText() + " " + ofToString(guiImage->getSlider("Fade Time")->getValue()));
    }
    else if(e.target->is("Load Folder"))
    {
        sendMessageToSlavesFolder("loadFolder " + e.target->getText() + " " + ofToString(guiImage->getSlider("Fade Time")->getValue())+ " " + ofToString(guiImage->getSlider("SlideShow Time")->getValue()));
    }
}

//-------------------------------------------------------------------------------
void ofApp::onDropdownEvent(ofxDatGuiDropdownEvent e)
{
    deviceSelected=e.target->getSelectedIndex();
}

void ofApp::onSliderEvent(ofxDatGuiSliderEvent e)
{
    if(e.target->is("First DMX Channel"))
    {
        guiDmx->getSlider("ch "+ofToString(lastDmxChannel+3))->setLabel("ch "+ofToString(e.value+3));
        guiDmx->getSlider("ch "+ofToString(lastDmxChannel+3))->setName("ch "+ofToString(e.value+3));

        guiDmx->getSlider("ch "+ofToString(lastDmxChannel+2))->setLabel("ch "+ofToString(e.value+2));
        guiDmx->getSlider("ch "+ofToString(lastDmxChannel+2))->setName("ch "+ofToString(e.value+2));
        
        guiDmx->getSlider("ch "+ofToString(lastDmxChannel+1))->setLabel("ch "+ofToString(e.value+1));
        guiDmx->getSlider("ch "+ofToString(lastDmxChannel+1))->setName("ch "+ofToString(e.value+1));
        
        guiDmx->getSlider("ch "+ofToString(lastDmxChannel))->setLabel("ch "+ofToString(e.value));
        guiDmx->getSlider("ch "+ofToString(lastDmxChannel))->setName("ch "+ofToString(e.value));
        
        lastDmxChannel = e.value;
    }
    else
    {
        int chToSend = ofToInt(e.target->getName().substr(3,e.target->getName().size()-1));
        int valueToSend = e.value;
        cout<<"Send value "<<e.value<<" to dmx Chanel "<<chToSend<<endl;
        sendMessageToSlavesFolder("setDMXCh " + ofToString(chToSend) + " " + ofToString(e.value)+ " 0");
    }
}

#pragma mark ---------- System events ----------
//-------------------------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    if (key == 'f') {
        //toggleFullscreen();
    }
    
    if(guiScreen->getToggle("Edit Quad")->getEnabled()){
        int quadStep = 1;
        if(ofGetKeyPressed(OF_KEY_SHIFT)) quadStep = 5;
        /// QUAD WARPING
        ////////////////
        if (key == OF_KEY_LEFT)
        {
            sendMessageToSlavesFolder("movePointLeft " + ofToString(quadStep));
        }
        else if (key == OF_KEY_RIGHT)
        {
            sendMessageToSlavesFolder("movePointRight " + ofToString(quadStep));
        }
        else if (key == OF_KEY_UP)
        {
            sendMessageToSlavesFolder("movePointUp " + ofToString(quadStep));
        }
        else if (key == OF_KEY_DOWN)
        {
            sendMessageToSlavesFolder("movePointDown " + ofToString(quadStep));
        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{
    if(guiMaster->hitTest(ofPoint(x, y)) && !guiMaster->getFocused()) guiMaster->focus();
    if(guiSlaves->hitTest(ofPoint(x, y)) && !guiSlaves->getFocused()) guiSlaves->focus();
    if(guiScreen->hitTest(ofPoint(x, y)) && !guiScreen->getFocused()) guiScreen->focus();
    if(guiVideo->hitTest(ofPoint(x, y)) && !guiVideo->getFocused()) guiVideo->focus();
    if(guiImage->hitTest(ofPoint(x, y)) && !guiImage->getFocused()) guiImage->focus();
    if(guiDmx->hitTest(ofPoint(x, y)) && !guiDmx->getFocused()) guiDmx->focus();
    if(guiArtnet->hitTest(ofPoint(x, y)) && !guiArtnet->getFocused()) guiArtnet->focus();
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

# pragma mark ---------- TCP ----------

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
            
            cout << "Received TCP message : " << str << endl;
            
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
                
                
                ofxDatGuiToggle* tog = slavesListFolder->addToggle(ofToString(i) + " " + ofToString(s.id) + " " +s.name + " " + s.ip,true);
                tog->setStripe(ofColor(0,128,255), 5);
                tog->setBackgroundColor(ofColor(32));
                // stupid hack to
                guiSlaves->setPosition(guiSlaves->getPosition().x,guiSlaves->getPosition().y);
                
                s.toggle = tog;
                slavesListFolder->expand();
            }
            if(tokens[0]=="awake")
            {
                cout << "Received : awake . So sending ping to all !! " << endl;
                sendTCPPingAll();
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
void ofApp::sendTcpMessageToSlave(string mess, int pos)
{
    if(tcpServer.isConnected())
    {
        if( tcpServer.isClientConnected(pos))
        {
            tcpServer.send(pos,mess);
            cout << "TCP Send : " << mess << endl;
        }
    }
}

//--------------------------------------------------------------
void ofApp::sendMessageToSlavesFolder(string m)
{
    int num = slavesListFolder->size();
    for(int i=0;i<num;i++)
    {
        ofxDatGuiToggle* t = slavesListFolder->getToggleAt(i);
        if(t->getEnabled()){
            string messageTcp = "all " + m;
            sendTcpMessageToSlave(messageTcp, getIdFromSlave(i));
        }
    }
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
void ofApp::sendTCPPingAll()
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

#pragma mark ---------- setup GUIS ----------
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
    guiSlaves->addToggle("Test")->setStripe(c, 5);
    guiSlaves->addToggle("Debug")->setStripe(c, 5);
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
    guiSlaves->onButtonEvent(this, &ofApp::onSlavesButtonEvent);
    
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
    
    guiMaster->onButtonEvent(this, &ofApp::onMasterButtonEvent);
    guiMaster->onDropdownEvent(this, &ofApp::onDropdownEvent);
    guiMaster->onTextInputEvent(this, &ofApp::onTextInputEvent);
    
}
//--------------------------------------------------------------
void ofApp::setupGuiScreen()
{
    // instantiate and position the gui //
    guiScreen = new ofxDatGui();
//    guiScreen->setPosition(guiSlaves->getPosition().x + guiSlaves->getWidth() +10, 10);
    guiScreen->setPosition(guiMaster->getPosition().x, guiMaster->getPosition().y + guiMaster->getHeight()*2);
    
    // adding the optional header allows you to drag the gui around //
    guiScreen->addHeader("SCREEN")->setBackgroundColor(ofColor(127));
    
    //slavesListFolder = guiSlaves->addFolder("Slaves List", ofColor::red);
    
    ofColor c = ofColor(255,255,255);
    guiScreen->addToggle("Use FBO ?")->setStripe(c, 5);
    guiScreen->addToggle("Use Homography ?")->setStripe(c, 5);
    guiScreen->addBreak();
    guiScreen->addToggle("Edit Quad")->setStripe(ofColor::darkRed, 5);
    guiScreen->addButton("Next Corner")->setStripe(ofColor::darkRed, 5);
    guiScreen->addButton("Previous Corner")->setStripe(ofColor::darkRed, 5);
    guiScreen->addButton("Reset Quad")->setStripe(ofColor::darkRed, 5);
    guiScreen->addButton("Save Quad")->setStripe(ofColor::darkRed, 5);
    guiScreen->addLabel("Use arrow keys to move corners.")->setStripe(ofColor(128), 5);;
    
    
    // once the gui has been assembled, register callbacks to listen for component specific events //
    guiScreen->onButtonEvent(this, &ofApp::onScreenButtonEvent);
    
}
//--------------------------------------------------------------
void ofApp::setupGuiVideo()
{
    // instantiate and position the gui //
    guiVideo = new ofxDatGui();
    guiVideo->setPosition(guiScreen->getPosition().x + guiScreen->getWidth() +10, guiScreen->getPosition().y);
    
    // adding the optional header allows you to drag the gui around //
    guiVideo->addHeader("VIDEO")->setBackgroundColor(ofColor(127));
    
    ofColor c = ofColor(255,255,255);
    guiVideo->addButton("Play Video")->setStripe(c, 5);
    guiVideo->addButton("Stop Video")->setStripe(c, 5);
    guiVideo->addButton("Pause Video")->setStripe(c, 5);
    guiVideo->addButton("Restart Video")->setStripe(c, 5);
    guiVideo->addTextInput("Load Video","test.mov")->setStripe(c, 5);
    guiVideo->addSlider("Fade Time", 0, 10);
    
    guiVideo->addBreak();
    
    // once the gui has been assembled, register callbacks to listen for component specific events //
    guiVideo->onButtonEvent(this, &ofApp::onVideoButtonEvent);
    guiVideo->onTextInputEvent(this, &ofApp::onTextInputEvent);
    
}

void ofApp::setupGuiImage()
{
    // GUI IMAGE
    //////////////
    // instantiate and position the gui //
    guiImage = new ofxDatGui();
    guiImage->setPosition(guiVideo->getPosition().x, guiVideo->getPosition().y+guiVideo->getHeight()+10);
    guiImage->setWidth(guiVideo->getWidth());
    
    // adding the optional header allows you to drag the gui around //
    guiImage->addHeader("Image Controls")->setBackgroundColor(ofColor(127));
    
    ofColor c = ofColor(ofColor::blueViolet);
    guiImage->addButton("Play")->setStripe(c, 5);
    guiImage->addButton("Stop")->setStripe(c, 5);
    guiImage->addTextInput("Load Image", "/test/testImage1.jpg")->setStripe(c, 5);
    guiImage->addTextInput("Load Folder", "test")->setStripe(c, 5);
    guiImage->addSlider("Fade Time", 0, 10);
    guiImage->addSlider("SlideShow Time", 0, 10);
    
    guiImage->onButtonEvent(this, &ofApp::onImageButtonEvent);
    guiImage->onTextInputEvent(this, &ofApp::onTextInputEvent);
}

void ofApp::setupGuiDmx()
{
    // GUI DMX
    //////////////
    // instantiate and position the gui //
    guiDmx = new ofxDatGui();
    guiDmx->setPosition(guiSlaves->getPosition().x + guiSlaves->getWidth() +10, 10);
    //guiDmx->setWidth(guiVideo->getWidth());
    
    // adding the optional header allows you to drag the gui around //
    guiDmx->addHeader("DMX controls")->setBackgroundColor(ofColor(127));
    
    ofColor c = ofColor(ofColor::greenYellow);
    guiDmx->addLabel("Test");
//    guiDmx->addButton("Play")->setStripe(c, 5);
//    guiDmx->addButton("Stop")->setStripe(c, 5);
//    guiDmx->addTextInput("Load Image", "/test/testImage1.jpg")->setStripe(c, 5);
//    guiDmx->addTextInput("Load Folder", "test")->setStripe(c, 5);
    auto slider = guiDmx->addSlider("First DMX Channel", 0, 512, 0);
    slider -> setStripe(c, 5);
    slider -> setPrecision(0);
    lastDmxChannel = 0;
    slider = guiDmx->addSlider("ch 0", 0, 256, 0);
    slider -> setStripe(c, 5);
    slider -> setPrecision(0);
    slider = guiDmx->addSlider("ch 1", 0, 256, 0);
    slider -> setStripe(c, 5);
    slider -> setPrecision(0);
    slider = guiDmx->addSlider("ch 2", 0, 256, 0);
    slider -> setStripe(c, 5);
    slider -> setPrecision(0);
    slider = guiDmx->addSlider("ch 3", 0, 256, 0);
    slider -> setStripe(c, 5);
    slider -> setPrecision(0);
    guiDmx->addBreak();
    guiDmx->addToggle("All Chan Test")->setStripe(c, 5);
    
    
    guiDmx->onButtonEvent(this, &ofApp::onDmxButtonEvent);
    guiDmx->onSliderEvent(this, &ofApp::onSliderEvent);
}

void ofApp::setupGuiArtnet()
{
    // instantiate and position the gui //
    guiArtnet = new ofxDatGui();
    guiArtnet->setPosition(guiVideo->getPosition().x + guiVideo->getWidth() +10, guiVideo->getPosition().y);
    
    // adding the optional header allows you to drag the gui around //
    guiArtnet->addHeader("ArtNet")->setBackgroundColor(ofColor(127));
    
    ofColor c = ofColor(255,255,255);
    guiArtnet->addButton("Play Artnet Video")->setStripe(c, 5);
    guiArtnet->addButton("Stop Artnet Video")->setStripe(c, 5);
    guiArtnet->addButton("Pause Artnet Video")->setStripe(c, 5);
    guiArtnet->addButton("Restart Artnet Video")->setStripe(c, 5);
    guiArtnet->addTextInput("Load Artnet Video","test.mov")->setStripe(c, 5);
    
    
    
    // once the gui has been assembled, register callbacks to listen for component specific events //
    guiArtnet->onButtonEvent(this, &ofApp::onArtnetButtonEvent);
    guiArtnet->onTextInputEvent(this, &ofApp::onTextInputEvent);
}