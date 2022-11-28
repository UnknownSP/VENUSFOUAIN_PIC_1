#include "main.h"
#include <xc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

void init(void);
void SendToMotor(uint16_t speed,uint8_t stat);
void LED_Handler(int num, bool on);
int pos_input(int num);
int trapezoidControl(int now_speed, int target_speed);

void main(void) {
    uint16_t target_speed = 0;
    int input_speed = 0;
    static int now_speed = 0;
    uint16_t control_speed = 0;
    static int now_mode = 0;
    int control_mode = 0;
    int8_t receive_mode = 0;
    uint8_t com_flg = 0;

    uint8_t recent_mode = 0;
    uint8_t target_mode = 0;
    uint32_t mode_change_count = 0;
    bool mode_change_flag = false;
    bool test_flag = true;

    Game_mode game_mode = 0;

    int ball_in_count = 0;
    int sens_count[5] = {0};
    bool sensor_reactiong = false;

    static int pos_1_count = 0;
    static int pos_2_count = 0;
    static int pos_3_count = 0;
    static int pos_4_count = 0;
    static int pos_5_count = 0;
    static int ball_1_count = 0;
    static int ball_2_count = 0;
    static int ball_3_count = 0;
    static int ball_4_count = 0;
    const int on_count = 5;

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
            game_mode = STANDBY;
            CLRWDT();
        }

        if(game_mode == IN_GAME){
            receive_mode = (rcv_data[0] >> 2) & 0b00000011;
            target_speed = ((rcv_data[0] & 0b00000011) << 8 ) + rcv_data[1];
            target_mode = receive_mode;
            if(target_mode == 0 || target_mode == 3){
                input_speed = 0;
            }else if(target_mode == 1){
                input_speed = target_speed;
            }else if(target_mode == 2){
                input_speed = -target_speed;
            }

            if(target_mode == 0 || target_mode == 3){
                now_speed = 0;
            }else{
                now_speed = trapezoidControl(now_speed, input_speed);
            }
            if(now_speed > 0){
                control_speed = now_speed;
                now_mode = 1;
            }else if(now_speed < 0){
                control_speed = -now_speed;
                now_mode = 2;
            }else{
                control_speed = 0;
                if(target_mode == 0 || target_mode == 3){
                    now_mode = 0;
                }else{
                    now_mode = target_mode;
                }
            }
            control_mode = now_mode;
            if(control_mode == 0 || control_mode == 3){
                if(target_mode == 0){
                    PWMSet(0,0);
                    SW_RUN = 0;
                    SW_REV = 0;
                    SW_RUN = 0;
                    SW_REV = 0;
                }
            }else if(control_mode == 1){
                PWMSet(control_speed,1);
                SW_RUN = 1;
                SW_REV = 0;
                SW_RUN = 1;
                SW_REV = 0;
            }else if(control_mode == 2){
                PWMSet(control_speed,1);
                SW_RUN = 1;
                SW_REV = 1;
                SW_RUN = 1;
                SW_REV = 1;
            }

            //SW_RUN = (rcv_data[0] >> 4) & 0b00000001;
            //SW_REV = (rcv_data[0] >> 5) & 0b00000001;
            SW_ALM = (rcv_data[0] >> 6) & 0b00000001;
        }else{
            target_speed = 0;
            target_mode = 0;
            input_speed = 0;
            now_mode = 0;
            now_speed = 0;
            control_mode = 0;
            control_speed = 0;
            recent_mode = 0;
            mode_change_flag = false;
            mode_change_count = 0;
            SW_RUN = 0;
            SW_REV = 0;
            SW_ALM = 0;
            PWMSet(0,0);
        }

        uint8_t temp = snd_data[1] & 0b11110000;

        temp |= (control_mode & 0b00000011) << 2;
        temp |= ((control_speed >> 8) & 0b00000011 );

        snd_data[1] = temp;
        snd_data[2] =  (uint8_t)(control_speed & 0b0000000011111111);

        if(now_mode == 0){
            LED_3 = 0;
        }else {
            LED_3 = 1;
        }

        if(!BALL_SENS_1){
            ball_1_count++;
            if(ball_1_count > on_count){
                ball_1_count = on_count + 1;
                LED_1 = 1;
                snd_data[1] |= 0b00010000;
            }
        }else{
            ball_1_count = 0;
            snd_data[1] &= 0b11101111;
        }
        if(!BALL_SENS_2){
            ball_2_count++;
            if(ball_2_count > on_count){
                ball_2_count = on_count + 1;
                LED_1 = 1;
                snd_data[1] |= 0b00100000;
            }
        }else{
            ball_2_count = 0;
            snd_data[1] &= 0b11011111;
        }
        if(!BALL_SENS_3){
            ball_3_count++;
            if(ball_3_count > on_count){
                ball_3_count = on_count + 1;
                LED_1 = 1;
                snd_data[1] |= 0b01000000;
            }
        }else{
            ball_3_count = 0;
            snd_data[1] &= 0b10111111;
        }
        if(!BALL_SENS_4){
            ball_4_count++;
            if(ball_4_count > on_count){
                ball_4_count = on_count + 1;
                LED_1 = 1;
                snd_data[1] |= 0b10000000;
            }
        }else{
            ball_4_count = 0;
            snd_data[1] &= 0b01111111;
        }
        if(BALL_SENS_1 && BALL_SENS_2 && BALL_SENS_3 && BALL_SENS_4){
            LED_1 = 0;
        }

        if(POS_SENS_1){
            pos_1_count++;
            if(pos_1_count > on_count){
                pos_1_count = on_count + 1;
                LED_2 = 1;
                snd_data[0] |= 0b00000001;
            }
        }else{
            pos_1_count = 0;
            LED_2 = 0;
            snd_data[0] &= 0b11111110;
        }
        if(POS_SENS_2){
            pos_2_count++;
            if(pos_2_count > on_count){
                pos_2_count = on_count + 1;
                snd_data[0] |= 0b00000010;
            }
        }else{
            pos_2_count = 0;
            snd_data[0] &= 0b11111101;
        }
        if(POS_SENS_3){
            pos_3_count++;
            if(pos_3_count > on_count){
                pos_3_count = on_count + 1;
                snd_data[0] |= 0b00000100;
            }
        }else{
            pos_3_count = 0;
            snd_data[0] &= 0b11111011;
        }
        if(POS_SENS_4){
            pos_4_count++;
            if(pos_4_count > on_count){
                pos_4_count = on_count + 1;
                snd_data[0] |= 0b00001000;
            }
        }else{
            pos_4_count = 0;
            snd_data[0] &= 0b11110111;
        }
        if(POS_SENS_5){
            pos_5_count++;
            if(pos_5_count > on_count){
                pos_5_count = on_count + 1;
                snd_data[0] |= 0b00010000;
            }
        }else{
            pos_5_count = 0;
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

int trapezoidControl(int now_speed, int target_speed){
    //speed = -1023 〜 1023
    int speed_diff = target_speed - now_speed;
    int diff_abs = abs(speed_diff);
    int speed_coeff = 1;
    int return_speed = 0;
    static int accel_count = 0;
    accel_count += 1;
    if(speed_diff < 0){
        speed_coeff = -1;
    }
    if(diff_abs > M_ACCELRANGE){
        if(accel_count >= M_MAXACCEL_TIMING){
            accel_count = 0;
            return_speed = now_speed + speed_coeff * M_MAXACCEL;
        }else{
            return_speed = now_speed;
        }
    }else{
        return_speed = target_speed;
    }
    return return_speed;
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
