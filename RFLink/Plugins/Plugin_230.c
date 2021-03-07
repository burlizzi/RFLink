


#ifdef PLUGIN_230
void sendMsg(char * pbuffer);



#include "fauxmoESP.h"
#include <map>
#include "7_Utils.h"
#include <FS.h>





#define PLUGIN_DESC_230 "Alexa"
#include <EEPROM.h>
#define MAX_STORAGE INPUT_COMMAND_SIZE




std::map<unsigned int,std::string> devices;


//pippoc pippo;

fauxmoESP fauxmoEsp;

boolean CopySerial(const char *src);

std::string lastCommand;
boolean ReadQueue()
{
    if (lastCommand.size())
    {
        CopySerial(lastCommand.c_str());
        lastCommand="";
        return true;
    }
    return false;  
}


void callback(unsigned char device_id, const char * device_name, bool state, unsigned char value) {
    DEBUG_MSG_FAUXMO("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
    if (devices.find(device_id)!=devices.end())
    {
        lastCommand=devices[device_id]+(state?"on":"off");
    }
    else
    {
        DEBUG_MSG_FAUXMO(device_name);
        DEBUG_MSG_FAUXMO(" not found");
    } 
}

void Plugin_230_setup()
{
  fauxmoEsp.createServer(false); // not needed, this is the default value
  fauxmoEsp.setPort(80); // This is required for gen3 devices
  fauxmoEsp.enable(true);
    // Custom entry point (not required by the library, here just as an example)

  fauxmoEsp.onSetState(callback);

}


boolean Plugin_230(byte function, char *string)
{
    if (function==0)//init
        Plugin_230_setup();
    
    return false;
}


boolean PluginTX_230(byte function, char *string)
{
    // 10;alexa;light;10:intertechno:0F00FFFF0F:FF:F0:;light 1;10:intertechno:0F00FFFF0F:FF:F0:;
    const char * command;
    const char * name;
    if (function==0)
    {

        if (SPIFFS.exists("init.alx")) {                            // If the file exists
            File file = SPIFFS.open("init.alx", "r");                 // Open it
            auto init=file.readString();
            file.close();
            DEBUG_MSG_FAUXMO("found init string ");
            memset(InputBuffer_Serial,0,sizeof(InputBuffer_Serial));
            strncpy(InputBuffer_Serial,init.c_str(),sizeof(InputBuffer_Serial));
            for (auto &c:InputBuffer_Serial) //here be dragons
                if (!isprint(c))
                    c=0;
            DEBUG_MSG_FAUXMO(InputBuffer_Serial);
        }
        else
            DEBUG_MSG_FAUXMO("invalid EEPROM");
        return true;
        
    }
    DEBUG_MSG_FAUXMO("always");
    retrieve_Init();
    if (!retrieve_Name("10"))
      return false;
    DEBUG_MSG_FAUXMO("10 passed");
    if (!retrieve_Name("alexa"))
      return false;
    DEBUG_MSG_FAUXMO("alexxa, yes");

    if (function==1)
    {
        File file = SPIFFS.open("init.alx", "w");                 // Open it
        file.write((const char*)string);
        file.close();
    }
    DEBUG_MSG_FAUXMO("so far so good");
    while(retrieve_String(name) && retrieve_String(command))
    {
        DEBUG_MSG_FAUXMO(name);
        DEBUG_MSG_FAUXMO(command);
        auto id=fauxmoEsp.addDevice(name);
        std::string cmd(command);
        for (auto&c:cmd)
            if (c==':') 
                c=';';
        devices[id]=cmd;
        DEBUG_MSG_FAUXMO("command for ");
        DEBUG_MSG_FAUXMO(name);
        DEBUG_MSG_FAUXMO(" is ");

        DEBUG_MSG_FAUXMO(cmd.c_str());
    }

     
    return true;
}
#endif