#include <Arduino.h>
#include <ESPStepperMotorServer.h>

ESPStepperMotorServer *stepperMotorServer;

const char *wifiName= "<don't commit to github>"; // enter the SSID of the wifi network to connect to
const char *wifiSecret = "<don't commit to github>"; // enter the password of the the existing wifi network here

void setup() 
{
  Serial.begin(115200);
  stepperMotorServer = new ESPStepperMotorServer(ESPServerRestApiEnabled | ESPServerWebserverEnabled | ESPServerSerialEnabled);
  stepperMotorServer->setWifiCredentials(wifiName, wifiSecret);
  stepperMotorServer->setWifiMode(ESPServerWifiModeClient); //start the server as a wifi client (DHCP client of an existing wifi network)

  ESPStepperMotorServer_StepperConfiguration* config = 
    new ESPStepperMotorServer_StepperConfiguration(12, 14);

  config->setDisplayName("Motor1");
  stepperMotorServer->addOrUpdateStepper(config, 0);
  stepperMotorServer->start();
}

void loop() 
{
}