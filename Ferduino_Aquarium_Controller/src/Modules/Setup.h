#pragma once // Do not change this line!
// ------------------------setup------------------------------
void setup()
{
  #ifdef WATCHDOG // Do not change this line!
    wdt_disable();
  #endif // Do not change this line!

  #ifdef DEBUG           // Do not change this line
    Serial.begin(38400); // Inicia a comunicação com a  porta serial 0 para obter mensagens de depuração.
    Serial.flush();
    LOGLN();
  #endif // Do not change this line

  #if defined(STAMPS_EZO) || defined(STAMPS_V4X) // Do not change this line!
    Serial3.begin(38400); // Inicia a comunicação com a  porta serial 3 onde estão conectados os "stamps".
  #endif // Do not change this line!

  configPins();                        // Define a função dos pinos.

  #ifdef USE_TFT                       // Do not change this line
    myGLCD.InitLCD(LANDSCAPE);         // Orientação da imagem no LCD.
    clearScreen();                     // Limpa o LCD.

    myTouch.InitTouch(LANDSCAPE);      // Orientação do "touch screen".
    myTouch.setPrecision(PREC_MEDIUM); // Define a precisão do "touch screen".
  #endif // Do not change this line

  ReadDallasAddress ();
  sensors.begin();                                 // Inicia as leituras das sondas de temperatura.
  sensors.setResolution(sensor_agua, 10);          // Define a resolução em 10 bits.
  sensors.setResolution(sensor_dissipador, 10);    // Define a resolução em 10 bits.
  sensors.setResolution(sensor_ambiente, 10);      // Define a resolução em 10 bits.
  sensors.requestTemperatures();                   // Chamada para todos os sensores.
  #ifdef USE_FAHRENHEIT                            // Do not change this line
    tempC = (sensors.getTempF(sensor_agua));       // Lê a temperatura da água
    tempH = (sensors.getTempF(sensor_dissipador)); // Lê a temperatura do dissipador.
    tempA = (sensors.getTempF(sensor_ambiente));   // Lê a temperatura do ambiente.
  #else // Do not change this line
    tempC = (sensors.getTempC(sensor_agua));       // Lê a temperatura da água
    tempH = (sensors.getTempC(sensor_dissipador)); // Lê a temperatura do dissipador.
    tempA = (sensors.getTempC(sensor_ambiente));   // Lê a temperatura do ambiente.
  #endif // Do not change this line

  rtc.halt(false); // Inicia o funcionamento do RTC.

  byte k =  EEPROM.read(0);

  if (k != 222)    // Verifica se a EEPROM foi formatada por este programa
  {
    #ifdef USE_TFT // Do not change this line
      setFont(LARGE, 255, 0, 0, 0, 0, 0);

      strcpy_P(buffer, (char*)pgm_read_word_near(&(tabela_textos[225]))); // "FORMATANDO A EEPROM..."
      myGLCD.print(buffer, CENTER, 115);
    #endif // Do not change this line

    LOGLN(F("Formatting EEPROM..."));

    EEPROM.write(0, 222); // Indica que a EEPROM foi formatada por este programa

    for (int i = 1; i < 4096; i++)
    {
      EEPROM.write(i, 0); // Formata a eeprom
    }
  }

  k =  EEPROM.read(796);// Copia valores dos LEDs para a eeprom.

  if (k != 222)
  {
    SaveLEDToEEPROM();
  }

// Lê as variáveis guardadas na EEPROM.
  ReadFromEEPROM();
  lertpaEEPROM();
  lerPHAEEPROM();
  lerPHREEPROM();
  lerORPEEPROM();
  lerDENEEPROM();
  ler_dosadora_EEPROM();
  ler_luz_noturna_EEPROM();
  ler_timers_EEPROM();
  ler_coolersEEPROM();
  ler_tempPotEEPROM();
  ler_wave_EEPROM();
  ler_calib_dosadora_EEPROM();
  ler_predefinido_EEPROM();
  check_erro_tpa_EEPROM();
  ler_alimentador_EEPROM();
  ler_clima_EEPROM();

  k = EEPROM.read(840); // Ponteiro que indica que a configuração já foi alterada

  if (k != 66)
  {
    for (byte i = 0; i < 9; i++)
    {
      outlets[i] = 0;
    }
  }
  else
  {
    ler_outlets_EEPROM();
  }

  selecionar_SPI(SD_CARD);                        // Seleciona disposito SPI que será utilizado.
  while (!SD.begin(ChipSelect_SD, SD_CARD_SPEED)) // Inicia a comunicação com o cartão SD.
  {
    #ifdef USE_TFT                                // Do not change this line
      setFont(LARGE, 255, 0, 0, 0, 0, 0);
      myGLCD.print((char *)"PLEASE INSERT A SD CARD.", CENTER, 115);
    #endif // Do not change this line

    LOGLN(F("Please insert a SD CARD"));
  }

  t = rtc.getTime();                             // Atualiza as variáveis que usam o RTC.
  check_arquivo_temp_agua();                     // Corrige o log de temperatura
  check_arquivo_ph_agua();                       // Corrige o log de pH da água
  check_arquivo_ph_reator();                     // Corrige o log de pH do reator
  check_arquivo_orp();                           // Corrige o log de ORP
  check_arquivo_densidade();                     // Corrige o log de densidade

  #if defined(STAMPS_EZO) || defined(STAMPS_V4X) // Do not change this line!

    #ifdef USE_STAMP_FOR_CALCIUM_REACTOR         // Do not change this line
      iniciar_stamp_ph_reator();                 // Verifica o pH do reator de cálcio.
    #endif // Do not change this line

    #ifdef USE_STAMP_FOR_ORP // Do not change this line
      iniciar_stamp_orp();   // Verifica a ORP
    #endif // Do not change this line

    #ifndef TILT_HYDROMETER        // Do not change this line
      #ifdef USE_STAMP_FOR_DENSITY // Do not change this line
        iniciar_stamp_densidade(); // Verifica a densidade
      #endif // Do not change this line
    #endif // Do not change this line

    #ifdef USE_STAMP_FOR_TANK_PH  // Do not change this line
      iniciar_stamp_ph_aquario(); // Verifica o pH do aquário
    #endif // Do not change this line
  #endif // Do not change this line!

  #ifdef ETHERNET_SHIELD                        // Do not change this line!
    #ifndef USE_ESP8266                         // Do not change this line!
      start_ethernet();
      MQTT.setServer("www.ferduino.com", 1883); // Do NOT change this URL!
      MQTT.setCallback(requestAction);

      if (!MQTT.connected())
      {
        reconnect();
      }
    #else
      Serial1.begin(38400);

      ESP8266.wifiCb.attach(wifiCb);
      sincronizar();
    #endif // Do not change this line!
  #endif   // Do not change this line!

  #if defined(RFM12B_LED_CONTROL) || defined(RFM12B_RELAY_CONTROL) // Do not change this line!
    selecionar_SPI(RFM);                                           // Seleciona disposito SPI que será utilizado.
    radio.Initialize(MY_ID, FREQUENCY, NETWORK_ID);
    radio.Encrypt((byte*)KEY);

    #ifdef DEBUG // Do not change this line
      char buff[50];
      sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY == RF12_433MHZ ? 433 : FREQUENCY == RF12_868MHZ ? 868 : 915);
      LOGLN(buff);
    #endif // Do not change this line!
  #endif   // Do not change this line!

  #ifdef USE_TFT      // Do not change this line
    clearScreen();    // Limpa o LCD.
    mainScreen(true); // Exibe a tela inicial no LCD.
  #endif // Do not change this line

  #ifdef WATCHDOG // Do not change this line!
    wdt_enable(WDTO_8S);
  #endif // Do not change this line!

  randomSeed(millis()); // Referência para a função random().
  probabilidadeNuvem(); // Calcula a probabilidade de ocorrer nuvens.
}
// ***************************************************************************************************************************************
// ****************************** Fim do setup() *****************************************************************************************
// ***************************************************************************************************************************************
#ifdef ETHERNET_SHIELD // Do not change this line!
 #ifndef USE_ESP8266   // Do not change this line
    void start_ethernet()
    {
      selecionar_SPI(ETHER_CARD); // Seleciona disposito SPI que será utilizado.
      Ethernet.init(SelectSlave_ETH);

      #ifdef USE_DHCP // Do not change this line
        if(Ethernet.begin(mac) == 0)
        {
          LOGLN(F("Failed to configure Ethernet using DHCP"));

          if (Ethernet.hardwareStatus() == EthernetNoHardware)
          {
            LOGLN(F("Ethernet shield was not found."));
          }
          if (Ethernet.linkStatus() == LinkOFF)
          {
            LOGLN(F("Ethernet cable is not connected."));
          }
        }
      #else // Do not change this line
        Ethernet.begin(mac, ip, dnsServer, gateway, subnet);

        if (Ethernet.hardwareStatus() == EthernetNoHardware)
        {
          LOGLN(F("Ethernet shield was not found."));
        }
        if (Ethernet.linkStatus() == LinkOFF)
        {
          LOGLN(F("Ethernet cable is not connected."));
        }
      #endif // Do not change this line

      LOG(F("\nController IP: "));
      LOGLN(Ethernet.localIP());
    }
 #endif // Do not change this line!
#endif  // Do not change this line!

void configPins()
{
// Define a função dos pinos.
  pinMode(alarmPin, OUTPUT);           // Pino 0;
  pinMode(desativarFanPin, OUTPUT);    // Pino 1;

  pinMode (ChipSelect_SD, OUTPUT);     // Pino 4, 5 ou 53;

  pinMode(ledPinUV, OUTPUT);           // Pino 8;
  pinMode(ledPinBlue, OUTPUT);         // Pino 9;
  pinMode(ledPinWhite, OUTPUT);        // Pino 10;
  pinMode(ledPinRoyBlue, OUTPUT);      // Pino 11;
  pinMode(ledPinRed, OUTPUT);          // Pino 12;
  pinMode(fanPin, OUTPUT);             // Pino 13;
  pinMode(multiplexadorS0Pin, OUTPUT); // Pino 16;
  pinMode(multiplexadorS1Pin, OUTPUT); // Pino 17;

  pinMode(aquecedorPin, OUTPUT);       // Pino 42;
  pinMode(chillerPin, OUTPUT);         // Pino 43;
  pinMode(ledPinMoon, OUTPUT);         // Pino 44;
  pinMode(wavemaker1, OUTPUT);         // Pino 45;
  pinMode(wavemaker2, OUTPUT);         // Pino 46;

  pinMode(SelectSlave_ETH, OUTPUT);    // Pino 53;

  pinMode(sensor1, INPUT);             // Pino A0;
  pinMode(sensor2, INPUT);             // Pino A1;
  pinMode(sensor3, INPUT);             // Pino A2;
  pinMode(sensor4, INPUT);             // Pino A3;
  pinMode(sensor5, INPUT);             // Pino A4;
  pinMode(sensor6, INPUT);             // Pino A5;

  pinMode(dosadora1, OUTPUT);          // Pino A9;
  pinMode(dosadora2, OUTPUT);          // Pino A10;
  pinMode(dosadora3, OUTPUT);          // Pino A11;
  pinMode(dosadora4, OUTPUT);          // Pino A12;
  pinMode(dosadora5, OUTPUT);          // Pino A13;
  pinMode(dosadora6, OUTPUT);          // Pino A14;
  pinMode(ChipSelect_RFM, OUTPUT);     // Pino A15;

  PCF8575.begin();     // Inicia a comunicação com o PCF8575
  for (int i = 0; i < 16; i++)
  {
    PCF8575.pinMode(i, OUTPUT);
    PCF8575.digitalWrite(i, LOW);
  }
}
