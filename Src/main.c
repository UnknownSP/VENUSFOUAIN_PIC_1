#include "main.h"
#include <xc.h>
#include <stdbool.h>

void init(void);
void SendToMotor(uint16_t speed,uint8_t stat);
void LED_Handler(int num, bool on);
int pos_input(int num);

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

        if(game_mode == IN_GAME){
            receive_mode = (rcv_data[0] >> 2) & 0b00000011;
            speed = ((rcv_data[0] & 0b00000011) << 8 ) + rcv_data[1];
            now_mode = receive_mode;

            SW_RUN = (rcv_data[0] >> 4) & 0b00000001;
            SW_REV = (rcv_data[0] >> 5) & 0b00000001;
            SW_ALM = (rcv_data[0] >> 6) & 0b00000001;
        }else{
            speed = 0;
            now_mode = 0;
            recent_mode = 0;
            mode_change_flag = false;
            mode_change_count = 0;
        }

        PWMSet(speed,now_mode);

        if(!BALL_SENS_1){
            LED_1 = 1;
            snd_data[1] |= 0b00000001;
        }else{
            snd_data[1] &= 0b11111110;
        }
        if(!BALL_SENS_2){
            LED_1 = 1;
            snd_data[1] |= 0b00000010;
        }else{
            snd_data[1] &= 0b11111101;
        }
        if(!BALL_SENS_3){
            LED_1 = 1;
            snd_data[1] |= 0b00000100;
        }else{
            snd_data[1] &= 0b11111011;
        }
        if(!BALL_SENS_4){
            LED_1 = 1;
            snd_data[1] |= 0b00001000;
        }else{
            snd_data[1] &= 0b11110111;
        }
        if(BALL_SENS_1 && BALL_SENS_2 && BALL_SENS_3 && BALL_SENS_4){
            LED_1 = 0;
        }

        if(!POS_SENS_1){
            LED_2 = 1;
            snd_data[0] |= 0b00000001;
        }else{
            LED_2 = 0;
            snd_data[0] &= 0b11111110;
        }
        if(!POS_SENS_2){
            snd_data[0] |= 0b00000010;
        }else{
            snd_data[0] &= 0b11111101;
        }
        if(!POS_SENS_3){
            snd_data[0] |= 0b00000100;
        }else{
            snd_data[0] &= 0b11111011;
        }
        if(!POS_SENS_4){
            snd_data[0] |= 0b00001000;
        }else{
            snd_data[0] &= 0b11110111;
        }
        if(!POS_SENS_5){
            snd_data[0] |= 0b00010000;
        }else{
            snd_data[0] &= 0b11101111;
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
    
    TRISBbits.TRISB5=0;
    TRISBbits.TRISB3=0;
    TRISBbits.TRISB0=0;

    TRISCbits.TRISC0 = 1;
    TRISCbits.TRISC1 = 1;
    TRISCbits.TRISC5 = 0;
    TRISCbits.TRISC6 = 0;
    TRISCbits.TRISC7 = 0;

    // Set watch dog
    WDTCON = 0x13;

    I2C_init(addr);//アドレス
    PWMInit();
}

int pos_input(int num){
    switch(num){
        case 1:
            return POS_SENS_1;
            break;
        case 2:
            return POS_SENS_2;
            break;
        case 3:
            return POS_SENS_3;
            break;
        case 4:
            return POS_SENS_4;
            break;
        case 5:
            return POS_SENS_5;
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
