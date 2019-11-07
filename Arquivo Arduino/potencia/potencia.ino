#include "EmonLib.h"                  
EnergyMonitor emon1;                  
#include <SPI.h>         
#include <Ethernet.h>
#include <EthernetUdp.h>     

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 2, 177);
unsigned int localPort = 8888;      
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; 
char  ReplyBuffer[] = "COMANDO N EXECUTADO ERRO DE SINTAXE";    

EthernetUDP Udp;

int CharToInt(char vet[]);
boolean Valida(char vet[],int pino);
void comando(char vet[],boolean ok,int pin);
long getDecimal(float val);
int valorAnalog;

float corrente=0;             
double potencia=0;            
double kwh_consumido=0; 
double tensao=127; //Mudar caso necessário
double valor_conta=0; 
unsigned long tempo=0; 
float ruido = 0.08;
int i = 0;
int x = 0;
float tempo_medicao;

void setup(){ 
  Ethernet.begin(mac,ip);
  Udp.begin(localPort);  
  Serial.begin(9600); 
  emon1.current(A1,  57); //57 - 18.40  Função  de  calibração  da  corrente  recebida.  Current: input pin, calibration.
  Serial.println("\n\n--------------SISTEMA DE MONITORAMENTO DE CONSUMO--------------\n\n"); 
}
 
void loop (){
  String cmd;
  int packetSize = Udp.parsePacket();
  if(packetSize) {
    Serial.println(" ");
    Serial.println(" ");
    Serial.print("TAMANHO DO PACOTE RECEBIDO: ");
    Serial.println(packetSize);
    Serial.print("IP DE ORIGEM DA MENSAGEM: ");
    IPAddress remote = Udp.remoteIP();
    for (int i =0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", PORTA: ");
    Serial.println(Udp.remotePort()); 
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.print("MENSAGEM RECEBIDA: ");
    Serial.println(packetBuffer);
    if ((packetBuffer[0] == 'D') || (packetBuffer[0] == 'A')) { 
      cmd = (packetBuffer[0]);
      cmd = cmd + (packetBuffer[1]);      
      Serial.print("PINO:");
      int pinn = getPino(packetBuffer);
      Serial.println(pinn);   
      boolean validado = validaComando(packetBuffer, pinn); 
      Serial.print("COMANDO VALIDADO? (BOLEANO): ");
      Serial.println(validado);
      comando(validado, pinn, cmd, valorAnalog);
    }else if ((packetBuffer[0] == 'T') && (packetBuffer[1] == 'C')){
      Serial.print("Resposta: TCOK ");
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write("TCOK");
    }     
    Udp.endPacket();
  }
  delay(10);

  /* valor consumido!
  corrente = emon1.Irms; 
  
  double Irms = emon1.calcIrms(1480);  
  potencia = corrente*127.2;
  kwh_consumido = (kwh_consumido+(potencia/3600000));
  tempo = millis();
  tempo = tempo/1000;
  tempo_medicao=tempo*0.000277777778;
  i++;
  valor_conta = (kwh_consumido*0.39642);
  
  if(i==60){
    valor_conta = (kwh_consumido*0.39642);
    Serial.print(corrente);
    Serial.print(" AMPERES      ");
    Serial.print(potencia);
    Serial.print(" WATTS ");
    Serial.print(kwh_consumido);
    Serial.print(" Kw/h    ");
    Serial.print("R$ ");
    Serial.print(valor_conta);
    Serial.print("        ");
    Serial.print(tempo_medicao);
    Serial.println(" Horas");
    i=0;
    valor_conta = 0;
  }
  
  delay(1000);*/
}

int getPino(char vet[]){
  int und;
  und = int(vet[3]);
  und-=48;
  int dez;
  dez = int(vet[2]);
  dez-=48;
  dez=dez*10;
  int aux;
  aux = dez+und;
  return aux;
}

boolean validaComando(char vet[],int pino){
  boolean retorno;
  retorno = false;
  if ((toUpperCase(vet[0]) == 'D') && (toUpperCase(vet[1]) == 'W')) {
    if ((toUpperCase(vet[4]) == 'H') || (toUpperCase(vet[4]) == 'L')){
      if (vet[4] == 'H') { 
        valorAnalog = 1;
      } else if (vet[4] == 'L') {
        valorAnalog = 0;
      }
      retorno = true;
    }
  } else if (toUpperCase(vet[0]) == 'A') {
    if (((pino>=0)&&(pino<=5)) || ((pino>=0)&&(pino<=5))) {
      if (toUpperCase(vet[1]) == 'R'){
        retorno = true;
      } else if (toUpperCase(vet[1]) == 'W') {
        int centena,dezena,unidade;
        centena = (int(vet[4])-48)*100;
        dezena  = (int(vet[5])-48)*10;
        unidade = (int(vet[6])-48);
        valorAnalog = centena+dezena+unidade;        
        if ((valorAnalog>=0)&&(valorAnalog<=255)){
          retorno = true;
        }
      }
    }
  } else if ((toUpperCase(vet[0]) == 'D') && (toUpperCase(vet[1]) == 'R')) {
    if (((pino>=0)&&(pino<=13)) || ((pino>=22)&&(pino<=53))) {
        retorno = true;
    }
  } 
  return (retorno);
}    

void comando(boolean validado, int pin, String comando, int valor){
  if(validado){
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    comando.toUpperCase();
    if (comando == "AR") { 
      pinMode(pin,INPUT);
      int saida;

      emon1.calcVI(20,100); 
      corrente = emon1.Irms;
      
      Serial.println("COMANDOS EXECUTADOS");
      Serial.print("pinMode(");
      Serial.print(pin);
      Serial.println(", INPUT);");
      Serial.print("digitalRead(");
      Serial.print(pin);
      Serial.println(");");
      Serial.println("Leitura de entrada digital");
      Serial.print("Resposta do Arduino:");
      Serial.println(corrente); 
      int cor = corrente*100;
      char b[5];
      String calculoValor;
      calculoValor = String(int(cor))+"."+String(getDecimal(cor));
      calculoValor.toCharArray(b,5);
      Udp.write(b);
    } else if (comando == "DR") {
      float readAnalog;
      float valor;
      readAnalog = analogRead(pin);
      Serial.println("COMANDOS EXECUTADOS");
      Serial.print("AnalogRead(");
      Serial.print(pin);
      Serial.println(");"); 
      valor = readAnalog;
      Serial.println(valor);
      Serial.println("Leitura de Entrada analogica");
      Serial.print("Resposta do Arduino: ");
      Serial.println(valor);
      char b[5];
      String calculoValor;
      calculoValor = String(int(valor))+"."+String(getDecimal(valor));
      calculoValor.toCharArray(b,5);
      Udp.write(b);
      
    }
  }else{
    Serial.println("Resposta do Arduino: ERRO");
    Udp.write("ERRO");
  } 
}  

long getDecimal(float val) {
  int intPart = int(val);
  long decPart = 1000*(val-intPart); 
  if(decPart>0)  
    return(decPart);    
  else if(decPart<0) 
    return((-1)*decPart);
  else 
    return(00);          
}
