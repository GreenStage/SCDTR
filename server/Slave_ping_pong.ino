#include <Wire.h>

//MODOS DE COMUNICAÇÃO
//R - partilhar referencias e duty cicle
//C - partilhar a sua linha da matriz dos Ks
//A - partilhar o seu address



typedef struct _message{
  char mode;
  int address,value1,value2;
} message;

const int my_addr=45; // addr do dispositivo.


char out_buff[40], buff[40];
int net[10]={0,0,0,0,0,0,0};
message auxr;
int aux1=0;

volatile boolean r_flag=false;
boolean done=false;

//preenche a estrutura para envio
message construct_out_message(char mode, int addr, float value1, float value2){
  sprintf(out_buff,"%c%d,%d,%d.",mode, addr, value1*100, value2*100);
  Serial.println("out_buff construido");
  Serial.println(out_buff);
}

//envia mensagem
void forward_message2(){
  Wire.beginTransmission(0);
  Wire.write(out_buff);
  for(int i=0;i<7;i++){
    if(!net[i] && done==false){
      net[i]=my_addr;
      i=8;
      done=true;
    }
  }
  Wire.endTransmission();
  Serial.println("Mensagem address");
}

void forward_message(byte who){
  Wire.beginTransmission(who);
  Wire.write(out_buff);
  Wire.endTransmission();
  Serial.println("Mensagem enviada");
}
//filtra a mensagem recebida e coloca os valores nas variaveis
void filter_message(){
    switch(auxr.mode){
      case 'c':
        influence[auxr.address][0]=auxr.value1/100;
        influence[auxr.address][1]=auxr.value2/100;
        break;

      case 'r':
        ref_table[auxr.address][0]=auxr.value1/100;
        ref_table[auxr.address][1]=auxr.value2/100;
        break;
      case 'a':
        Serial.println("Entrei aqui");
        for(int i=0;i<7;i++){
          if(net[i]==0){
            net[i]=auxr.address;
            i=8;
          }
        }
        break;
      case 'A':
        net[auxr.value1/100]=auxr.address;
        break;
      default:
        Serial.println("UNKNOWN MESSAGE FORMAT");
        Serial.println(auxr.mode);
        Serial.println(auxr.address);
        Serial.println(auxr.value1);
        Serial.println(auxr.value2);
        break;
    }
}

void share_addr(){
  construct_out_message('a', my_addr, 0, 0);
  forward_message2();

}

//espera pela primeira comunicação
void wait_4_init(){
  while(r_flag==false){
  }
  r_flag=false;
}

void receive_event(int num_bytes){
  if(Wire.readBytes(buff,num_bytes)!=0){
    r_flag=true;
    Serial.println(buff);
    sscanf(buff,"%c%d,%d,%d.", &auxr.mode,&auxr.address,&auxr.value1,&auxr.value2);
    message_recv();
    filter_message();
  }


}

//share de addr of all
void broadcast_net(){
  int i=0;
  while(net[i]!=0 && i<7){
    construct_out_message('A', net[i] , i, 0);
    forward_message(0);
    i++;
  }
  delay(2000);
}

//o master inicia o setup da rede partilhando o seu address
void init_command(){
  while(Serial.available()==0){}
  aux1=Serial.available();
  Serial.println(aux1);
  if(aux1!=0){
    Serial.readBytes(out_buff, aux1);
    Serial.println("escrevi setup");
    share_addr();
  }

}

//imprime todo o conteudo da mensagem
void message_recv(){
  Serial.println("mode");
  Serial.println(auxr.mode);
  Serial.println("address");
  Serial.println(auxr.address);
  Serial.println("value1");
  Serial.println(auxr.value1);
  Serial.println("value2");
  Serial.println(auxr.value2);
}

//descobre o indice da net onde está o seu address
int find_my_place(){
  for(int i=0; i<7 && net[i]!=0;i++){
      if(net[i]==my_addr){
        my_place=i;
        return my_place;
      }
  }
}

void setup(){
  Serial.begin(9600);
  Wire.begin(my_addr);     //  join as a master/slave
  Wire.onReceive(receive_event);
  wait_4_init();
  share_addr();

  delay(2000);
  find_my_place();
  Serial.println("O meu lugar na rede é:");
  Serial.println(my_place);
  Serial.println("");
}

void loop(){





}
