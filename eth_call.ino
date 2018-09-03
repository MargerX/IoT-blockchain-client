#include <WiFi.h>
#include <Web3.h>
#include <Contract.h>

#define ENV_SSID     "Krakow"
#define ENV_WIFI_KEY "8.warszawa.8"

const string CONTRACT_ADDRESS = "0xdeb1331a4b43839ad427d81ebee5ce915171e925";
const char* ACCOUNT_ADDRESS = "0xdce18b3f54c1ef557d48b98d3521aeda35ccc848";
const string INFURA_HOST = "ropsten.infura.io";
const string INFURA_PATH = "/YOUR_PATH";
string result = "";

int relayPin = 5;
int buzzerPin = 4;
int lastState = 0;
long previousMillis = 0;
long interval = 20000;


Web3 web3(&INFURA_HOST, &INFURA_PATH);
WiFiServer server(80);
Contract contract(&web3, &CONTRACT_ADDRESS);

void eth_call();

void setup() {
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW);
    pinMode(buzzerPin, OUTPUT);
    Serial.begin(115200);

    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    WiFi.begin(ENV_SSID, ENV_WIFI_KEY);

    // attempt to connect to Wifi network:
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        // wait 1 second for re-trying
        delay(1000);
    }

    Serial.println("Connected");
    server.begin();
}

void loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis > interval)
    { 
      eth_call();
      previousMillis = currentMillis;   
    }
    wifi_server();
}

void wifi_server()
{
   WiFiClient client = server.available();
    if (client) {
      while (client.connected()) {
      Serial.println(" ## web client conneted.");
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println("Connection: close");
      client.println("");
      client.println("Contract address: ");
      client.print(CONTRACT_ADDRESS.c_str());
      client.println("<br>Contract value: ");
      client.print(result.c_str());
      client.stop();
    }}
}

void eth_call() {  
    Serial.println(WiFi.localIP());
    strcpy(contract.options.from, ACCOUNT_ADDRESS);
    strcpy(contract.options.gasPrice,"2000000000000");
    contract.options.gas = 5000000;
    string func = "isActive()";
    string param = contract.SetupContractData(&func);
    result = contract.Call(&param);
    char value = result[result.length()-3];
    Serial.println(value);
    Serial.println(lastState);
    if(value == '1' && lastState == 0)
    {
      digitalWrite(relayPin, HIGH);
      for(int i = 0; i < 800; i++)
      {
        if ( i % 2 == 0) digitalWrite(buzzerPin, HIGH);
        else digitalWrite(buzzerPin, LOW);
        delay(2);
      }
      lastState = 1;
      Serial.println("Switch ON");
    }
    else if (value == '0' && lastState == 1)
    {
      digitalWrite(relayPin, LOW);
      lastState = 0;
      Serial.println("Switch OFF");
    }
}
