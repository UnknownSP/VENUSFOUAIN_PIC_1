#include "main.h"
#include <xc.h>
#include <stdbool.h>

void init(void);
void SendToMotor(uint16_t speed,uint8_t stat);

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
            receive_mode = rcv_data[0] & 0b00000011;
            speed = rcv_data[1] * 4;
            if((recent_mode == 1 && receive_mode == 2) || (recent_mode == 2 && receive_mode == 1)){
                mode_change_flag = true;
                mode_change_count = 0;
            }
            if(mode_change_flag){
                mode_change_count++;
                if(mode_change_count >= MODE_CHANGE_WAIT_COUNT){
                    mode_change_flag = false;
                    mode_change_count = 0;
                }
                now_mode = 0;
            }else{
                now_mode = receive_mode;
            }
            recent_mode = receive_mode;  
            
            if(((rcv_data[0] & 0b00000100)>>2) == 1){
                LOUNCH = 1;
            }else{
                LOUNCH = 0;
            }
            if(((rcv_data[0] & 0b00001000)>>3) == 1){
                BALL_SET = 1;
            }else{
                BALL_SET = 0;
            }
        }else{
            speed = 0;
            now_mode = 0;
            recent_mode = 0;
            mode_change_flag = false;
            mode_change_count = 0;
            LOUNCH = 0;
            BALL_SET = 0;
        }

        if(RIFT_IN == 1){
            snd_data[1] |= 0b00000001;
        }else{
            snd_data[1] &= 0b11111110;
        }
        if(RIFT_LOWER == 1){
            if(now_mode == BACK_MODE){
                speed = 0;
                now_mode = 0;
            }
            snd_data[1] |= 0b00000010;
        }else{
            snd_data[1] &= 0b11111101;
        }
        if(RIFT_UPPER == 1){
            if(now_mode == FORWARD_MODE){
                speed = 0;
                now_mode = 0;
            }
            snd_data[1] |= 0b00000100;
        }else{
            snd_data[1] &= 0b11111011;
        }
        if(LOUNCH_RAIL_S == 1){
            snd_data[1] |= 0b00001000;
        }else{
            snd_data[1] &= 0b11110111;
        }
        if(OUT_1_S == 0){
            snd_data[1] |= 0b00010000;
        }else{
            snd_data[1] &= 0b11101111;
        }
        if(OUT_2_S == 0){
            snd_data[1] |= 0b00100000;
        }else{
            snd_data[1] &= 0b11011111;
        }

        PWMSet(speed,now_mode);
    }
}

void init(void){
    uint8_t addr = 0x14;

    // Set oscilation
    OSCCON = 0xF0; //PLL　Enable

    // Set pin mode
    ANSELA = 0x00;
    ANSELB = 0x00;

    TRISB = 0x00;
    WPUB = 0x00;

    TRISAbits.TRISA3 = 1;
    TRISAbits.TRISA4 = 1;
    TRISAbits.TRISA5 = 1;
    TRISCbits.TRISC0 = 0;
    TRISCbits.TRISC1 = 0;
    TRISCbits.TRISC5 = 0;
  
    // Set watch dog
    WDTCON = 0x13;

    I2C_init(addr);//アドレス
    PWMInit();
}

void interrupt  HAND(void){
    Slave_Interrupt();
}
