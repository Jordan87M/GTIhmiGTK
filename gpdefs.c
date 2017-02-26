#include <stdio.h>

#include "gpdefs.h"
#include "logconf.h"


msgtype signallist[40];
msgtype commsignallist[5];
msgtype primarysignallist[5];
msgtype adminsignallist[10];
msgtype requestsignallist[10];
msgtype responsesignallist[10];


void setupsignals(void){
//form list of signals for display
//communications signals
    sprintf(commsignallist[0].name,"Misdirected Message");
    commsignallist[0].code = MISDIRECTED_MESSAGE_CODE;

    sprintf(commsignallist[1].name,"Unimplemented Protocol");
    commsignallist[1].code = UNIMPLEMENTED_PROTOCOL_CODE;

    sprintf(commsignallist[2].name,"Unimplemented Function");
    commsignallist[2].code = UNIMPLEMENTED_FUNCTION_CODE;

    sprintf(commsignallist[3].name,"GP_ACK");
    commsignallist[3].code = GP_ACK_CODE;

    sprintf(commsignallist[4].name,"Inverter Waking Up");
    commsignallist[4].code = INVERTER_WAKEUP_CODE;

    //primary signals
    sprintf(primarysignallist[0].name,"Set Phase");
    primarysignallist[0].code = PHASE_ADJUST_CODE;

    sprintf(primarysignallist[1].name,"Set Amplitude");
    primarysignallist[1].code = AMPLITUDE_ADJUST_CODE;

    sprintf(primarysignallist[2].name,"Set Real Power");
    primarysignallist[2].code = REAL_POWER_ADJUST_CODE;

    sprintf(primarysignallist[3].name,"Set Reactive Power");
    primarysignallist[3].code = REAL_POWER_ADJUST_CODE;

    sprintf(primarysignallist[4].name,"Set Frequency");
    primarysignallist[4].code = FREQUENCY_ADJUST_CODE;


    //administrative command signals
    sprintf(adminsignallist[0].name,"Select Grid Following");
    adminsignallist[0].code = SET_MODE_GRID_FOLLOWING_CODE;

    sprintf(adminsignallist[1].name,"Select Standalone");
    adminsignallist[1].code = SET_MODE_STANDALONE_CODE;

    sprintf(adminsignallist[2].name,"Disconnect");
    adminsignallist[2].code = DISCONNECT_CODE;

    sprintf(adminsignallist[3].name,"Connect");
    adminsignallist[3].code = CONNECT_CODE;

    sprintf(adminsignallist[4].name,"Lock Screen");
    adminsignallist[4].code = LOCK_SCREEN_CODE;

    sprintf(adminsignallist[5].name,"Unlock Screen");
    adminsignallist[5].code = UNLOCK_SCREEN_CODE;

    sprintf(adminsignallist[6].name,"Display Screen1");
    adminsignallist[6].code = SCREEN1_CODE;

    sprintf(adminsignallist[7].name,"Display Screen2");
    adminsignallist[7].code = SCREEN2_CODE;

    sprintf(adminsignallist[8].name,"Display Screen3");
    adminsignallist[8].code = SCREEN3_CODE;

    sprintf(adminsignallist[9].name,"Display Screen4");
    adminsignallist[9].code = SCREEN4_CODE;


    //requests
    sprintf(requestsignallist[0].name,"Request Phase");
    requestsignallist[0].code = REQUEST_PHASE_CODE;

    sprintf(requestsignallist[1].name,"Request Amplitude");
    requestsignallist[1].code = REQUEST_AMPLITUDE_CODE;

    sprintf(requestsignallist[2].name,"Request Current Mode");
    requestsignallist[2].code = REQUEST_MODE_CODE;

    sprintf(requestsignallist[3].name,"Request PWM Period");
    requestsignallist[3].code = REQUEST_PWM_PERIOD_CODE;

    sprintf(requestsignallist[4].name,"Request nsamples");
    requestsignallist[4].code = REQUEST_NSAMPLES_CODE;

    sprintf(requestsignallist[5].name,"Request Output Voltage");
    requestsignallist[5].code = REQUEST_OUTPUT_VOLTAGE_CODE;

    sprintf(requestsignallist[6].name,"Request Output Current");
    requestsignallist[6].code = REQUEST_OUTPUT_CURRENT_CODE;

    sprintf(requestsignallist[7].name,"Request Real Power");
    requestsignallist[7].code = REQUEST_REAL_POWER_CODE;

    sprintf(requestsignallist[8].name,"Request Reactive Power");
    requestsignallist[8].code = REQUEST_REACTIVE_POWER_CODE;

    sprintf(requestsignallist[9].name,"Request Frequency");
    requestsignallist[9].code = REQUEST_FREQUENCY_CODE;


    //response
    sprintf(responsesignallist[0].name,"Response: Phase");
    responsesignallist[0].code = RESPONSE_PHASE_CODE;

    sprintf(responsesignallist[1].name,"Response: Amplitude");
    responsesignallist[1].code = RESPONSE_AMPLITUDE_CODE;

    sprintf(responsesignallist[2].name,"Response: Current Mode");
    responsesignallist[2].code = RESPONSE_MODE_CODE;

    sprintf(responsesignallist[3].name,"Response: PWM Period");
    responsesignallist[3].code = RESPONSE_PWM_PERIOD_CODE;

    sprintf(responsesignallist[4].name,"Response: nsamples");
    responsesignallist[4].code = RESPONSE_NSAMPLES_CODE;

    sprintf(responsesignallist[5].name,"Response: Output Voltage");
    responsesignallist[5].code = RESPONSE_OUTPUT_VOLTAGE_CODE;

    sprintf(responsesignallist[6].name,"Response: Output Current");
    responsesignallist[6].code = RESPONSE_OUTPUT_CURRENT_CODE;

    sprintf(responsesignallist[7].name,"Response: Real Power");
    responsesignallist[7].code = RESPONSE_REAL_POWER_CODE;

    sprintf(responsesignallist[8].name,"Response: Reactive Power");
    responsesignallist[8].code = RESPONSE_REACTIVE_POWER_CODE;

    sprintf(responsesignallist[9].name,"Response: Frequency");
    responsesignallist[9].code = RESPONSE_FREQUENCY_CODE;
}



void makefullsignallist(void)
{
    int position = 0;
    if(sizeof(signallist) > sizeof(commsignallist))
    {
        memcpy(signallist, commsignallist, sizeof(commsignallist));
        position += (sizeof(commsignallist)/sizeof(commsignallist[0]));

        if((sizeof(signallist)/sizeof(signallist[0])) > (position + (sizeof(primarysignallist)/sizeof(primarysignallist[0]))))
        {
            memcpy(&signallist[position], primarysignallist,sizeof(primarysignallist));
            position += (sizeof(commsignallist)/sizeof(primarysignallist[0]));

            if((sizeof(signallist)/sizeof(signallist[0])) > (position + (sizeof(adminsignallist)/sizeof(adminsignallist[0]))))
            {
                memcpy(&signallist[position], adminsignallist, sizeof(adminsignallist));
                position += (sizeof(adminsignallist)/sizeof(adminsignallist[0]));

                if((sizeof(signallist)/sizeof(signallist[0])) > (position + (sizeof(requestsignallist)/sizeof(requestsignallist[0]))))
                {
                    memcpy(&signallist[position],requestsignallist,sizeof(requestsignallist));
                    position += sizeof(requestsignallist)/sizeof(requestsignallist[0]);

                    if((sizeof(signallist)/sizeof(signallist[0])) >= (position + (sizeof(responsesignallist)/sizeof(responsesignallist[0]))))
                    {
                        memcpy(&signallist[position],responsesignallist,sizeof(responsesignallist));
                    }
                    else
                    {
                        printf("signal list construction failure: responses too big (%d)", position);
                        return;
                    }
                }
                else
                {
                    printf("signal list construction failure: requests too big (%d)", position);
                    return;
                }
            }
            else
            {
                printf("signal list construction failure: admin too big (%d)", position);
                return;
            }
        }
        else
        {
            printf("signal list construction failure: primary too big (%d)", position);
            return;
        }
    }
    else
    {
        printf("signal list construction failure: comms too big (%d)", position);
        return;
    }

}

void printfullsignallist(FILE *fp)
{

    int i = 0;
    for(i = 0; i < sizeof(signallist)/sizeof(signallist[0]); i++)
    {
        logwriteln(fp, signallist[i].name);
    }
}
