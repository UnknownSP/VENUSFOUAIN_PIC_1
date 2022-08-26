#include "main.h"
#include <xc.h>
#include <stdbool.h>

void init(void);
void SendToMotor(uint16_t speed,uint8_t stat);
void LED_Handler(int num, bool on);
int input(int num);

void main(void) {
    uint16_t speed = 0;
    int8_t receive_mode = 0;
    uint8_t com_flg = 0;

    uint8_t recent_mode = 0;
    uint8_t now_mode = 0;
    uint32_t mode_change_count = 0;
    bool mode_change_flag = false;
    bool test_flag = true;

    Game_mode game_mode = 0;

    int ball_in_count = 0;
    int sens_count[5] = {0};
    bool sensor_reactiong = false;

    int i;

    init();

    while(true){

        if(I2C_ReceiveCheck()){
            if (com_flg == 0) com_flg = 1;
            if((rcv_data[0] & 0b10000000) >> 7 == 1){
                game_mode = IN_GAME;
            }else{
                game_mode = STANDBY;
            }
            CLRWDT();
        }
        else if (com_flg == 0){
            CLRWDT();
        }

        if(!RA5){
            PWMSet(256,FORWARD_MODE);
            LED_1 = 1;
        }else if(!RA4){
            PWMSet(512,FORWARD_MODE);
        }else if(!RA3){
            PWMSet(768,FORWARD_MODE);
        }else if(!RA2){
            PWMSet(1023,FORWARD_MODE);
        }else{ 
            LED_1 = 0;
            PWMSet(0,FREE_MODE);   
        }
    }
}

void init(void){
    uint8_t addr = 0x16;

    // Set oscilation
    OSCCON = 0xF0; //PLL　Enable

    // Set pin mode
    ANSELA = 0x00;
    ANSELB = 0x00;

  
    TRISCbits.TRISC0 = 0;
    TRISCbits.TRISC1 = 0;
    TRISCbits.TRISC5 = 0;

    // Set watch dog
    WDTCON = 0x13;

    I2C_init(addr);//アドレス
    PWMInit();
}

int input(int num){
    switch(num){
        case 1:
            return SENS_1;
            break;
        case 2:
            return SENS_2;
            break;
        case 3:
            return SENS_3;
            break;
        case 4:
            return SENS_4;
            break;
        case 5:
            return SENS_5;
            break;
    }
}

void LED_Handler(int num, bool on){
    switch(num){
        case 1:
            if(on){
                LED_1 = 1;
                LED_2 = 0;
                LED_3 = 0;
            }
            break;
        case 2:
            if(on){
                LED_1 = 0;
                LED_2 = 1;
                LED_3 = 0;
            }
            break;
        case 3:
            if(on){
                LED_1 = 1;
                LED_2 = 1;
                LED_3 = 0;
            }
            break;
        case 4:
            if(on){
                LED_1 = 0;
                LED_2 = 0;
                LED_3 = 1;
            }
            break;
        case 5:
            if(on){
                LED_1 = 1;
                LED_2 = 0;
                LED_3 = 1;
            }
            break;
    }
    if(!on){
        LED_1 = 0;
        LED_2 = 0;
        LED_3 = 0;
    }
}

void interrupt  HAND(void){
    Slave_Interrupt();
}
