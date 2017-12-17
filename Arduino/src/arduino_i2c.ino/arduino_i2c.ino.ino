#include <Wire.h>
#include <stdint.h>
#include <EEPROM.h>
#include <QueueArray.h>

#define CHANGE_INFLUENCE 0x3
#define CHANGE_REF 0x2
#define RASP_ADDR 0x48
//MODOS DE COMUNICAÃ‡ÃƒO
//R - partilhar referencias e duty cicle
//C - partilhar a sua linha da matriz dos Ks
//A - partilhar o seu address
//O DO STILAU Ãˆ o 0f
boolean done=false;

enum packet_ids{
  PACKET_NONE = 0,
  RASP_RQST_MIN = 0x40,
  RASP_RQST_ILU,
  RASP_RQST_DUTY_CICLE,
  RASP_RQST_LOWER_ILUMINANCE,
  RASP_RQST_ACC_ENERGY,
  RASP_RQST_ACC_CONFORT_ERR,
  RASP_RQST_ACC_CONFORT_VAR,
  RASP_RQST_POW_CONSUP,
  RASP_RQST_EXT_ILU,
  RASP_RQST_ILU_CTR,
  RASP_RQST_OCCUPANCY_ST,
  RASP_RQST_SET_NOT_OCCUP,
  RASP_RQST_SET_OCCUP,
  RASP_RQST_MAX,
  
  ARD_RQST_MIN = 0x80,
  ARD_RQST_REF,
  //ARDUINO_TO_ARDUINO REQUEST 3PACKETS
  ARD_RQST_MAX,

  ARD_RESP_MIN = 0xC0,
  ARD_RESP_ILU,
  ARD_RESP_DUTY_CICLE,
  ARD_RESP_LOWER_ILUMINANCE,
  ARD_RESP_ACC_ENERGY,
  ARD_RESP_ACC_CONFORT_ERR,
  ARD_RESP_ACC_CONFORT_VAR,
  ARD_RESP_POW_CONSUP,
  ARD_RESP_EXT_ILU,
  ARD_RESP_ILU_CTR,
  ARD_RESP_OCCUPANCY_ST,
  ARD_RESP_MAX
};

typedef struct send_single_float_{
  uint8_t src_address;
  uint8_t dest_address;
  uint8_t packet_id;
  float val;
} __attribute__((__packed__)) single_float_package;

typedef struct send_single_byte_{
  uint8_t src_address;
  uint8_t dest_address;
  uint8_t packet_id;
  uint8_t val;
}__attribute__((__packed__))  single_byte_package;

typedef struct packet_t_{
  uint8_t src_address;
  uint8_t dest_address;
  uint8_t packet_id;
} __attribute__((__packed__)) packet_t;

#define MAX_PACKET_SIZE sizeof(single_float_package)

int my_place; //indice do dispositivo no vector da rede.
int my_addr; // addr do dispositivo.
int net[7]={0,0,0,0,0,0,0};
packet_t* i2c_input_buffer, *i2c_output_buffer;
QueueArray<packet_t*> i2c_input_messages, i2c_output_messages;
int aux1=0;
  
int n_ard = 1;    //nÂº de arduinos na rede
int ref_table[1][1];
float influence[1][1];
volatile boolean r_flag=false;
bool occupancy_state = false;


int response_of(int id){
  switch(id){
    case RASP_RQST_ILU:               return ARD_RESP_ILU;
    case RASP_RQST_DUTY_CICLE:        return ARD_RESP_DUTY_CICLE;
    case RASP_RQST_LOWER_ILUMINANCE:  return ARD_RESP_LOWER_ILUMINANCE;
    case RASP_RQST_ACC_ENERGY:        return ARD_RESP_ACC_ENERGY;
    case RASP_RQST_ACC_CONFORT_ERR:   return ARD_RESP_ACC_CONFORT_ERR;
    case RASP_RQST_ACC_CONFORT_VAR:   return ARD_RESP_ACC_CONFORT_VAR;
    case RASP_RQST_POW_CONSUP:        return ARD_RESP_POW_CONSUP;
    case RASP_RQST_EXT_ILU:           return ARD_RESP_EXT_ILU;
    case RASP_RQST_ILU_CTR:           return ARD_RESP_ILU_CTR;
    case RASP_RQST_OCCUPANCY_ST:      return ARD_RESP_OCCUPANCY_ST;
    default:                          return PACKET_NONE;    
  }
}

int size_of_packet(packet_t * p){
  switch(p->packet_id){
    case ARD_RESP_ILU:
    case ARD_RESP_DUTY_CICLE:
    case ARD_RESP_LOWER_ILUMINANCE:
    case ARD_RESP_ACC_ENERGY:
    case ARD_RESP_ACC_CONFORT_ERR:
    case ARD_RESP_ACC_CONFORT_VAR:
    case ARD_RESP_POW_CONSUP:
    case ARD_RESP_EXT_ILU:
    case ARD_RESP_ILU_CTR:         
      return sizeof(single_float_package);
    case ARD_RESP_OCCUPANCY_ST:
      return sizeof(single_byte_package);
    default:
      return 0;
  }
}

int ard_parse_request(packet_t * rqst, packet_t* response){
  /*TODO*/
  return 0;  
}

int rasp_parse_request(packet_t * rqst, packet_t* response){
  int send_bytes = 0;
  static single_float_package * float_response = (single_float_package*) response;
  static single_byte_package * byte_response = (single_byte_package*) response;
  
  response->dest_address = rqst->src_address;
  response->src_address = my_addr;
  response->packet_id = response_of(rqst->packet_id);
  
  /*SET PACKET_TYPE */
  switch(rqst->packet_id){
    case RASP_RQST_ILU:
      send_bytes = sizeof(single_float_package);
      float_response->val = 200.32;
      break;
    case RASP_RQST_DUTY_CICLE:
      send_bytes = sizeof(single_float_package);
      float_response->val = 150.432;
      break;
    case RASP_RQST_LOWER_ILUMINANCE:
      send_bytes = sizeof(single_float_package);
      float_response->val = 34.4324;
      break;
    case RASP_RQST_ACC_ENERGY:
      send_bytes = sizeof(single_float_package);
      float_response->val = 33.22;
      break;
    case RASP_RQST_ACC_CONFORT_ERR:
      send_bytes = sizeof(single_float_package);
      float_response->val = 99.22;
      break;
    case RASP_RQST_ACC_CONFORT_VAR:
      send_bytes = sizeof(single_float_package);
      float_response->val = 22.22;
      break;
    case RASP_RQST_POW_CONSUP:
      send_bytes = sizeof(single_float_package);
      float_response->val = 15.22;
      break;
    case RASP_RQST_EXT_ILU:
      send_bytes = sizeof(single_float_package);
      float_response->val = 44.22;
      break;
    case RASP_RQST_ILU_CTR:
      send_bytes = sizeof(single_float_package);
      float_response->val = 11.11;
      break;
    case RASP_RQST_SET_OCCUP:
      occupancy_state = true;
      break;
    case RASP_RQST_SET_NOT_OCCUP:
      occupancy_state = false;
      break;
    case RASP_RQST_OCCUPANCY_ST:
      send_bytes = sizeof(single_byte_package);
      byte_response->val = 11.11;
      break;
    default:
      break;
  }

  return send_bytes;
}

void receive_from_i2c(int num_bytes){
  int rBytes = 0,send_bytes;
  int error = 0;

  for(rBytes = 0; Wire.available(); rBytes++){
    ((byte*) (i2c_input_buffer) )[rBytes] = Wire.read();
  }

  Serial.println("Packet id:");
  Serial.println(i2c_input_buffer->packet_id);
  Serial.println("respond to: ");  
  Serial.println(i2c_input_buffer->src_address);

  switch(i2c_input_buffer->packet_id & 0xC0){
    case 0x40:
      send_bytes = rasp_parse_request(i2c_input_buffer,i2c_output_buffer);
      break;
    case 0x80:
      //Todo: Arduino sent me a request, deal with it
      send_bytes = ard_parse_request(i2c_input_buffer,i2c_output_buffer);
      break;
    case 0xC0:
      //TODO: Arduino sent me a response, deal with it
      break;
    default:
      return;
  }
  if(send_bytes){
    packet_t * output_message = (packet_t*) malloc(send_bytes);
    memcpy(output_message,i2c_output_buffer,send_bytes);
    Serial.println("Sending to:");
    Serial.println(i2c_output_buffer->src_address);
    Serial.println(i2c_output_buffer->dest_address);
    Serial.println(i2c_output_buffer->packet_id);
    Serial.println(send_bytes);
    i2c_output_messages.push(output_message);
  } 
  
}

void setup(){
  my_addr = EEPROM.read(0);
  i2c_input_buffer = ( packet_t *) malloc(MAX_PACKET_SIZE);
  i2c_output_buffer = ( packet_t *) malloc(MAX_PACKET_SIZE);
  Serial.begin(9600);
  Wire.begin(my_addr);
  Serial.println("Joined i2c");
  Serial.println(my_addr);
  Wire.onReceive(receive_from_i2c);
  while(!Serial);
  
}

void loop(){
  static packet_t* send_packet;
  if(!i2c_output_messages.isEmpty()){
      send_packet = i2c_output_messages.pop();
      Wire.beginTransmission(send_packet->dest_address);
      Wire.write((byte[])send_packet,size_of_packet(send_packet));
      Wire.endTransmission();
      free(send_packet);
  }
}

