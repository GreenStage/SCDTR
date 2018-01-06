#ifndef DEFS_HEADER
#define DEFS_HEADER

#define DEBUG 
#define ARRAYLENGTH(A) ( sizeof(A) / sizeof(A[0]) )
#define MAX_BYTE_ARR_LENGTH 60
#define MAX_DESKS 20
typedef struct packet_t_{
  uint8_t src_address;
  uint8_t dest_address;
  uint8_t packet_id;
} __attribute__((__packed__)) packet_t;

typedef struct ard_single_float_{
  uint8_t src_address;
  uint8_t dest_address;
  uint8_t packet_id;
  float val;
} __attribute__((__packed__)) single_float_packet;

typedef struct ard_single_byte_{
  uint8_t src_address;
  uint8_t dest_address;
  uint8_t packet_id;
  uint8_t val;
}__attribute__((__packed__))  single_byte_packet;

typedef struct _ard_multiple_float{
    uint8_t src_address;
    uint8_t dest_address;
    uint8_t packet_id;
    uint8_t n_data;
    float data[MAX_BYTE_ARR_LENGTH];
} __attribute__((__packed__)) multiple_float_packet;

typedef struct _ard_multiple_byte{
    uint8_t src_address;
    uint8_t dest_address;
    uint8_t packet_id;
    uint8_t n_data;
    uint8_t data[MAX_BYTE_ARR_LENGTH];
} __attribute__((__packed__)) multiple_byte_packet;

typedef struct _ard_multiple_2byte{
    uint8_t src_address;
    uint8_t dest_address;
    uint8_t packet_id;
    uint8_t n_data;
    uint16_t data[MAX_BYTE_ARR_LENGTH];
} __attribute__((__packed__)) multiple_2byte_packet;

typedef enum stream_state_t{
	NOT_STREAMING = 0x0,
	STREAMING_ILU = 0x1,
	STREAMING_DT = 0x2
} stream_state_;
#endif
