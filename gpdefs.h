#ifndef GPDEFS_H
#define GPDEFS_H

#define GP_PORT                         30000

#define GP_MAGIC                        0xa3cf

#define INVERTER_CONTEXT                0x01
#define CAES_CONTEXT                    0x02
#define CONTROLLER_CONTEXT              0x00

//message type codes
//comms
#define MISDIRECTED_MESSAGE_CODE         0x0000
#define UNIMPLEMENTED_PROTOCOL_CODE      0x0001
#define UNIMPLEMENTED_FUNCTION_CODE      0x0002
#define GP_ACK_CODE                      0x0003
#define INVERTER_WAKEUP_CODE             0x0004

//primary commands
#define PHASE_ADJUST_CODE                0x1000
#define AMPLITUDE_ADJUST_CODE            0x1001
#define REAL_POWER_ADJUST_CODE           0x1002
#define REACTIVE_POWER_ADJUST_CODE       0x1003
#define FREQUENCY_ADJUST_CODE            0x1004

//admin commands
#define SET_MODE_GRID_FOLLOWING_CODE     0x2000
#define SET_MODE_STANDALONE_CODE         0x2001
#define DISCONNECT_CODE                  0x2002
#define CONNECT_CODE                     0x2003
#define LOCK_SCREEN_CODE                 0x2004
#define UNLOCK_SCREEN_CODE               0x2005
#define SCREEN1_CODE                     0x2006
#define SCREEN2_CODE                     0x2007
#define SCREEN3_CODE                     0x2008
#define SCREEN4_CODE                     0x2009

//requests
#define REQUEST_PHASE_CODE               0x3000
#define REQUEST_AMPLITUDE_CODE           0x3001
#define REQUEST_MODE_CODE                0x3002
#define REQUEST_PWM_PERIOD_CODE          0x3003
#define REQUEST_NSAMPLES_CODE            0x3004
#define REQUEST_OUTPUT_VOLTAGE_CODE      0x3005
#define REQUEST_OUTPUT_CURRENT_CODE      0x3006
#define REQUEST_REAL_POWER_CODE          0x3007
#define REQUEST_REACTIVE_POWER_CODE      0x3008
#define REQUEST_FREQUENCY_CODE           0x3009

//responses
#define RESPONSE_PHASE_CODE              0x4000
#define RESPONSE_AMPLITUDE_CODE          0x4001
#define RESPONSE_MODE_CODE               0x4002
#define RESPONSE_PWM_PERIOD_CODE         0x4003
#define RESPONSE_NSAMPLES_CODE           0x4004
#define RESPONSE_OUTPUT_VOLTAGE_CODE     0x4005
#define RESPONSE_OUTPUT_CURRENT_CODE     0x4006
#define RESPONSE_REAL_POWER_CODE         0x4007
#define RESPONSE_REACTIVE_POWER_CODE     0x4008
#define RESPONSE_FREQUENCY_CODE          0x4009




typedef struct msgtype_t{
    const char name[64];
    unsigned short int code;
} msgtype;

typedef struct gpheader_t{
    unsigned short int magic;
    unsigned char context;
    unsigned char msgcomponents;
    unsigned short int seqnum;
} gpheader;

typedef struct valpair_t{
    unsigned short int code;
    double value;
} valpair;

extern msgtype signallist[40];

extern msgtype commsignallist[5];
extern msgtype primarysignallist[5];
extern msgtype adminsignallist[10];
extern msgtype requestsignallist[10];
extern msgtype responsesignallist[10];

#endif
