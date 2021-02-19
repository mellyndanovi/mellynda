#include <ModbusMaster.h>
#include <Wire.h>
#define  BLYNK_PRINT Serial 
#include <WiFi.h>
#include <WiFiClient.h>
#include <RTClib.h>
#include <BlynkSimpleEsp32.h>
#include <SimpleModbusMaster.h>
#include <MySQL_Generic_WiFi.h>
#define MYSQL_DEBUG_PORT      Serial1
//#define _MYSQL_LOGLEVEL_     0
#define baud 9600
#define DE  22
#define RE  21

unsigned long HitunganMillis = 0;
unsigned long MillisSekarang = millis ();
const unsigned long nilai = 120000;
const unsigned long waktu = 360000;

char auth[]       = "Va8WuYGrbaq5SwHxSj_kJWbjrUta0KjG";//token blynk
char ssid[]       = "PORSCHE";
char pass[]       = "gakhafalll";
char user[]       = "arduino";
char password[]   = "arduino123";
char id_users[]   = "2"; //ngebaca id user yg di pakai oleh user login (id bukan status)

IPAddress server_addr(192,168,137,1); //ipserver (ip laptop karena server db nya di laptop)
uint16_t server_port = 3306;

char default_database[]   = "monitoring";
char default_table2[]     = "konsumsilistrik";
char date[20];
String INSERT_SQL;

MySQL_Connection MELL((Client *)&client);
MySQL_Query *menu;

unsigned token;
float volt;
float current;
float kwh;
float watt;
float kvar;
float kva;
float freq;
float PF;
ModbusMaster node;
BlynkTimer timer;
RTC_DS1307 rtc;

void preTransmission ()
{
  digitalWrite (DE, 1);
  digitalWrite (RE, 1);
}

void postTransmission ()
{
  digitalWrite (DE, 0);
  digitalWrite (RE, 0);
}

void setup ()
{
  Serial.begin(9600);
  //HitunganMillis = 0;
  //while (!Serial);
  pinMode (RE, OUTPUT);
  pinMode (DE, OUTPUT);
  digitalWrite (RE, 0);
  digitalWrite (DE, 0);
  Serial.begin (9600);
  node.begin (1, Serial);
  node.preTransmission (preTransmission);
  node.postTransmission (postTransmission);
  Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8080);
  timer.setInterval(1000L, ambil_data);
}

void ambil_data()
{
  uint8_t resultMain;
  
  resultMain = node.readHoldingRegisters (0x40000,13);
  if (resultMain == node.ku8MBSuccess)
  {
    Serial.println ("..");
    Serial.print ("Voltage = "); 
    volt = node.getResponseBuffer (0x02) / 100.0f;
    Serial.println (String (volt) + "V");
    Blynk.virtualWrite(V0, volt );
    delay (500);
    
    Serial.print ("Current = ");
    current = node.getResponseBuffer (0x03) / 100.0f;
    Serial.println (String (current) + "A");
    Blynk.virtualWrite(V1, current );
    delay (500);

    Serial.print ("Active Energy = ");
    kwh = node.getResponseBuffer (0x00) / 10.0f;
    Serial.println (String (kwh) + "kWh");
    Blynk.virtualWrite(V3, kwh );
    delay (500);
    
    Serial.print ("Active Power = ");
    watt = node.getResponseBuffer (0x05) / 1000.0f;
    Serial.println ( String (watt) + "kW");
    Blynk.virtualWrite(V2, watt );
    delay (500);
    
    Serial.print ("Reactive Power = ");
    kvar = node.getResponseBuffer (0x09) / 1000.0f;
    Serial.println (String (kvar) + "kvar");
    Blynk.virtualWrite(V5, kvar);
    delay (500);

    Serial.print ("Apparent Power = ");
    kva = node.getResponseBuffer (0x08) / 1000.0f;
    Serial.println (String (kva) + "kva");
    delay (500);
    
    Serial.print ("Frequency = ");
    freq = node.getResponseBuffer (0x0B) / 100.0f;
    Serial.println (String (freq) + "Hz");
    Blynk.virtualWrite(V4, freq);
    delay (500);
    
    Serial.print ("Power Factor = ");
    PF = node.getResponseBuffer (0x0C) / 1000.0f;
    Serial.println (PF);

    Serial.print ("Token = ");
    token= (500 - kwh);
    Serial.println (String(token) + "kWh");
    Blynk.virtualWrite(V7, token);
    delay (500); 
  }
  
    if (token <= 0)
    {
      Blynk.disconnect();
    }
  delay (1000);
}

void wifi ()
{
    if (WiFi.status() != WL_CONNECTED) 
    {
      Serial.print("Connecting Wifi: ");
      Serial.println(ssid);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, pass);
      while (WiFi.status() != WL_CONNECTED) 
      {
        Serial.print(".");
        delay(500);
      }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Connecting to SQL Server @ ");
    Serial.print(server_addr);
    Serial.println(String(", Port = ") + server_port);
    Serial.println(String("User = ") + user + ", PW = " + password + ", DB = " + default_database);
    }
}

void runInsert()
{
  while (token <= 0)
  {
    MELL.close();
    delay (1000);
  }

  MillisSekarang = millis ();
  if ((unsigned long) MillisSekarang - HitunganMillis >= nilai)
  {
  //insert data
    String INSERT_SQL = String("INSERT INTO ") + default_database + "." + default_table2 + " (id_users, Konsumsi_Listrik, Token, Arus, Tegangan, Daya_Aktif, Daya_Reaktif, Daya_Semu, Frekuensi, Faktor_Daya) VALUES ('" + id_users + "','" + kwh + "','" + token + "','" + current + "','" + volt + "','" + watt + "','" + kvar + "','" + kva + "','" + freq + "','" + PF +"')";                      
    MySQL_Query menu = MySQL_Query(&MELL);

    if (MELL.connected())
    {
      Serial.println(INSERT_SQL);
    
      if ( !menu.execute(INSERT_SQL.c_str()) )
          Serial.println("Insert error");
      else    
          Serial.println("Data Inserted.");
    }
    else
    {
      Serial.println("Disconnected from Server. Can't insert.");
    }
  HitunganMillis = millis ();
  }
}

void loop()
{
  MillisSekarang = millis ();
  if ((unsigned long) MillisSekarang - HitunganMillis >= waktu)
  {
  if (token >= 100 && token < 105)
  {
      Blynk.notify (String ("Hey! Be thrifty! Your token remaining ") + token + "kWh." + "\t" + "You're alredy spent" + "\n" + kwh + "kWh.");
  }
  else if(token >= 200 && token < 205)
  {
      Blynk.notify (String ("Hey! Be thrifty! Your token remaining ") + token + "kWh." + "\t" + "You're alredy spent" + "\n" + kwh + "kWh.");
  }
  else if(token >= 300 && token < 305)
  {
      Blynk.notify (String ("Hey! Be thrifty! Your token remaining ") + token + "kWh." + "\t" + "You're alredy spent" + "\n" + kwh + "kWh.");
  }
  else if(token >= 400 && token < 405)
  {
      Blynk.notify (String ("Hey! Be thrifty! Your token remaining ") + token + "kWh." + "\t" + "You're alredy spent" + "\n" + kwh + "kWh.");
  }
  else
  {
    Serial.println ("failed sending notification");
  }
  HitunganMillis = millis ();
  }
  
  ambil_data();
  wifi ();
  Blynk.run();
  timer.run();
  delay (1000);

 MillisSekarang = millis ();
 if ((unsigned long) MillisSekarang - HitunganMillis >= nilai)
 {
  Serial.println("Connecting...");
  if (MELL.connectNonBlocking(server_addr, server_port, user, password) != RESULT_FAIL)
  {
    runInsert();
    MELL.close();                   
  } else 
    {
      Serial.println("\nConnect failed. Trying again on next iteration.");
    }
  Serial.println("\nSleeping...");
  Serial.println("================================================");
  HitunganMillis = millis ();
 }
}
