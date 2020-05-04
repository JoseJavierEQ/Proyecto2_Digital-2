//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo 8 bits
   Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
   Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
   Con ayuda de: José Guerra
   IE3027: Electrónica Digital 2 - 2019
*/
//***************************************************************************************************************************************
//                        NOTAS:
//***************************************************************************************************************************************
// Memoria utilizada 66%
// Ambos jugadores se mueven.
// Necesitamos que la musica siempre se reproduzca. 
// Pantalla de ganador.
// Posible pantalla de inicio. 
// Ver como poder mover los pines de la pantalla para mas analogicos libres. Porque solo un jugador pones bombas
// Cualquier bomba que mate a ambos jugadores.

//***************************************************************************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#define PART_TM4C123GH6PM
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

# include "pitches.h"

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"

#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1

int melody[] = {
  NOTE_FS5, NOTE_FS5, NOTE_D5, NOTE_B4, NOTE_B4, NOTE_E5,
  NOTE_E5, NOTE_E5, NOTE_GS5, NOTE_GS5, NOTE_A5, NOTE_B5,
  NOTE_A5, NOTE_A5, NOTE_A5, NOTE_E5, NOTE_D5, NOTE_FS5,
  NOTE_FS5, NOTE_FS5, NOTE_E5, NOTE_E5, NOTE_FS5, NOTE_E5
};

int durations[] = {
  8, 8, 8, 4, 4, 4,
  4, 5, 8, 8, 8, 8,
  8, 8, 8, 4, 4, 4,
  4, 5, 8, 8, 8, 8
};

int songLength = sizeof(melody) / sizeof(melody[0]);

int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};
const int joystickSel = 10;     // the number of the joystick select pin
const int joystickX = 6;       // the number of the joystick X-axis analog
const int joystickY =  5;     // the number of the joystick Y-axis analog
const int joystickSel1 = 28;     // the number of the joystick select pin
const int joystickX1 = 18;       // the number of the joystick X-axis analog
const int joystickY1 =  29;     // the number of the joystick Y-axis analog
const int buzzer = 40;
int joystickSelState = 0;      // variable for reading the joystick sel status
int joystickSelState1 = 0; 
int joystickXState, joystickYState, xJ, yJ, bx, by,joystickXState1, joystickYState1, xJ1, yJ1, bx2, by2;
int tempo = 144;
int b = 0;
int b5 = 0;
int tiempo = 300;
int x = 21;
int y = 33;
int x2 = 281;
int y2 = 192;
int v1 = 3;
int v2 = 3;
int anim = (x / 11) % 8;

//Bomba
int bx1,by1;
int bx3,by3;
int Explo = 0;
int Explo5 = 0;
int borrar = 0;
int borrar1 = 0;
int Bombaexplota= 0;
int Bombaexplota1= 0;
//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);

void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset);

// Añadido para el timer
void initTimer()
{
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);   // 32 bits Timer
  TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0Isr);    // Registering  isr
  ROM_TimerEnable(TIMER0_BASE, TIMER_A);
  ROM_IntEnable(INT_TIMER0A);
  ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

void Timer0Isr(void)
{
  ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);  // Clear the timer interrupt
  if (tiempo > 0) {
    // Bandera de musica 1 (normal)
    tiempo--;
    String text5 = String(tiempo);
    LCD_Print(text5, 70, 3, 1, 0x000000, 0x8cc6ff);
    // Vidas
    String text6 = String(v1);
    LCD_Print(text6, 190, 3, 1, 0x000000, 0x8cc6ff);
    String text7 = String(v2);
    LCD_Print(text7, 270, 3, 1, 0x000000, 0x8cc6ff);
  } else {
    tiempo = 000;
    String text5 = String(tiempo);
    LCD_Print(text5, 70, 3, 1, 0x000000, 0x8cc6ff);
    String text9 = "Sudden Death";
    LCD_Print(text9, 140, 3, 1, 0x000000, 0x8cc6ff);
    // Reduccion de vidas a 1 para sudden death
    v2 = 1;
    v1 = 1;
    String text7 = String(v2);
    LCD_Print(text7, 270, 3, 1, 0x000000, 0x8cc6ff);
  }
  // Aqui aumento el tic de cada
}

//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  pinMode(joystickSel, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  Serial.begin(9600);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  Serial.println("Inicio");
  LCD_Init();
  LCD_Clear(0x00);
  FillRect(0, 0, 319, 228, 0x8cc6ff);
  initTimer();
  //LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);

  //LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
  LCD_Bitmap(52, 69, 16, 16, pared);
  LCD_Bitmap(104, 69, 16, 16, pared);
  LCD_Bitmap(156, 69, 16, 16, pared);
  LCD_Bitmap(208, 69, 16, 16, pared);
  LCD_Bitmap(260, 69, 16, 16, pared);
  LCD_Bitmap(52, 122, 16, 16, pared);
  LCD_Bitmap(104, 122, 16, 16, pared);
  LCD_Bitmap(156, 122, 16, 16, pared);
  LCD_Bitmap(208, 122, 16, 16, pared);
  LCD_Bitmap(260, 122, 16, 16, pared);
  LCD_Bitmap(52, 175, 16, 16, pared);
  LCD_Bitmap(104, 175, 16, 16, pared);
  LCD_Bitmap(156, 175, 16, 16, pared);
  LCD_Bitmap(208, 175, 16, 16, pared);
  LCD_Bitmap(260, 175, 16, 16, pared);

  for (int x = 0; x < 319; x++) {
    LCD_Bitmap(x, 16, 16, 16, pared);
    LCD_Bitmap(x, 228, 16, 16, pared);
    x += 15;
  }
  for (int y = 16; y < 319; y++) {
    LCD_Bitmap(0, y, 16, 16, pared);
    LCD_Bitmap(312, y, 16, 16, pared);
    y += 15;
  }
  String text2 = "Tiempo:";
  LCD_Print(text2, 8, 3, 1, 0x000000, 0x8cc6ff);
  String text3 = "J1 x ";
  LCD_Print(text3, 140, 3, 1, 0x000000, 0x8cc6ff);
  String text4 = "J2 x ";
  LCD_Print(text4, 220, 3, 1, 0x000000, 0x8cc6ff);
  LCD_Bitmap(175, 5, 9, 9, corazon);
  LCD_Bitmap(255, 5, 9, 9, corazon);
  //Vidas. Debo cambiar los numeros a variables. Y ver lo que disminuyan.
  String text6 = String(v1);
  LCD_Print(text6, 190, 3, 1, 0x000000, 0x8cc6ff);
  String text7 = String(v2);
  LCD_Print(text7, 270, 3, 1, 0x000000, 0x8cc6ff);

}
//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {
  unsigned long ulPeriod;
  unsigned int Hz = 1;   // frequency in Hz
  ulPeriod = (SysCtlClockGet() / Hz) ;
  ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ulPeriod - 1);
  LCD_Sprite(x, y, 28, 36, bomberman1, 3, anim, 0, 0);
  LCD_Sprite(x2, y2, 28, 36, bomberman2, 3, anim, 1, 0);
  while (1) {
    for (int thisNote = 0; thisNote < songLength; thisNote++) {
//**************************************************************************************************
//                              MUSICA (HACER QUE SUENE SIEMPRE)
//*************************************************************************************************
      int duration = 1000 / durations[thisNote];
      tone(buzzer, melody[thisNote], duration);
      // pause between notes
      int pause = duration * 1.3;
      delay(pause);
      // stop the tone
      noTone(8);
//**************************************************************************************************
//                              INICIAR CONTROLES
//*************************************************************************************************
      joystickXState = analogRead(joystickX);
      if (joystickXState < 1800) {
        xJ = 2;
      } else if (joystickXState > 2100) {
        xJ = 1;
      } else {
        xJ = 0;
      }
      joystickYState = analogRead(joystickY);
      if (joystickYState < 1800) {
        yJ = 2;
      } else if (joystickYState > 2100) {
        yJ = 1;
      } else {
        yJ = 0;
      }
      joystickXState1 = analogRead(joystickX1);
      if (joystickXState1 < 1800) {
        xJ1 = 2;
      } else if (joystickXState1 > 2100) {
        xJ1 = 1;
      } else {
        xJ1 = 0;
      }
      joystickYState1 = analogRead(joystickY1);
      if (joystickYState1 < 1800) {
        yJ1 = 2;
      } else if (joystickYState1 > 2100) {
        yJ1 = 1;
      } else {
        yJ1 = 0;
      }
//**************************************************************************************************
//                                EXPLOSIONES J1
//*************************************************************************************************
      if (Explo != 0 ){
      Explo++;
      if (Explo > 4){
        bx1 = bx ;
        by1 = by;
        FillRect(bx1, by1, 26, 26, 0x8cc6ff);
        LCD_Sprite(bx1+3, by1+3, 20, 20, explosion, 3, 0, 1, 0); //centro
//**************************************************************************************************
//                           EXPLOSIONES ANTI PAREDES
//**************************************************************************************************
         if ((by1 > 42 && by1<228)&& ((bx1 >= 16 && bx1 <= 52-20)||(bx1 >= 68 && bx1 <= 104-20)||(bx1 >= 120 && bx1 <= 156-20)||(bx1 >= 172 && bx1 <= 208-20)||(bx1 >= 224 && bx1 <= 260-20)||(bx>276))){
           LCD_Sprite(bx1+3, by1-17, 20, 20, explosion, 3, 2, 1, 0); //arriba 2
           if ((by1+5>y && y> by1+5-73)&&(bx1-1 == x)){
              Bombaexplota = 2;
            }
           if ((by1+5>y2 && y2> by1+5-73)&&(bx1-1 == x2)){
              Bombaexplota = 3;
            }
           if (by1> 68 && by1 < 288){
              LCD_Sprite(bx1+3, by1-37, 20, 20, explosion, 3, 2, 1, 0); // arriba
              if ((by1+5>y && y> by1-93+5)&&(bx1-1 == x)){
              Bombaexplota = 2;
              }
              if ((by1+5>y2 && y2> by1-93+5)&&(bx1-1 == x2)){
                Bombaexplota = 3;
              }
           }
         }

         
         if ((bx1>16 && bx1< 273 )&& ((by1>=32 && by1<=69-20)||(by1>=85 && by1<=122-20)||(by1>=138 && by1<=175-20)||(by1>=191))){
            LCD_Sprite(bx1+23, by1+3, 20, 20, explosion, 3, 1, 1, 0); //derecha 2
            if ((bx1+43+1>x && x> bx1+1)&& (by1-5 == y)){
              Bombaexplota = 2;
            }
            if ((bx1+43+1>x2 && x2> bx1+1)&& (by1-5 == y2)){
              Bombaexplota = 3;
            }
            if (bx>16 && bx < 253){
              LCD_Sprite(bx1+43, by1+3, 20, 20, explosion, 3, 1, 1, 0); // derecha
              if ((bx1+63+1>x && x> bx1+1)&& (by1-5 == y)){
                Bombaexplota = 2;
              }
              if ((bx1+63+1>x2 && x2> bx1+1)&& (by1-5 == y2)){
                Bombaexplota = 3;
              }
            }
         }

         
        if((bx1 > 41 && bx1 <290)&&((by1>=32 && by1<=69-20)||(by1>=85 && by1<=122-20)||(by1>=138 && by1<=175-20)||(by1>=191))){
            LCD_Sprite(bx1-17, by1+3, 20, 20, explosion, 3, 1, 1, 0); // izquierda 2
            if ((bx1+1-65>=x && x>bx1+1)&& (by1-5 == y)){
              Bombaexplota = 2;
            }
            if ((bx1+1-65>=x2 && x2>bx1+1)&& (by1-5 == y2)){
              Bombaexplota = 3;
            }
            if (bx1 > 61 && bx1 < 290){
              LCD_Sprite(bx1-37, by1+3, 20, 20, explosion, 3, 1, 1, 0); // izquierda
              if ((bx1+1>x && x> bx1-85+1)&& (by1-5 == y)){
              Bombaexplota = 2;
              }
              if ((bx1+1>x2 && x2> bx1-85+1)&& (by1-5 == y2)){
              Bombaexplota = 3;
              }
            }
        }

        
        if ((by1>16 && by1<191)&&((bx1 >= 16 && bx1 <= 52-20)||(bx1 >= 68 && bx1 <= 104-20)||(bx1 >= 120 && bx1 <= 156-20)||(bx1 >= 172 && bx1 <= 208-20)||(bx1 >= 224 && bx1 <= 260-20)||(bx>276))){
            LCD_Sprite(bx1+3, by1+23, 20, 20, explosion, 3, 2, 1, 0); // abajo 2 
            if ((by1+5+59>y && y> by1+5)&&(bx1-1 == x)){
              Bombaexplota = 2;
            }
            if ((by1+5+59>y2 && y2> by1+5)&&(bx1-1 == x2)){
              Bombaexplota = 3;
            }
            if (by1>16 && by1<165){
              LCD_Sprite(bx1+3, by1+43, 20, 20, explosion, 3, 2, 1, 0); // abajo
              if ((by1+5+79>y && y> by1+5)&&(bx1-1 == x)){
              Bombaexplota = 2;
              }
              if ((by1+5+79>y2 && y2> by1+5)&&(bx1-1 == x2)){
              Bombaexplota = 3;
              }
            }
        }
         
         Explo = 0;
         borrar = 1;
      }
    }
//**************************************************************************************************
//                           BORRAR EXPLOSIONES
//**************************************************************************************************
    if (borrar != 0){
      borrar++;
      if (borrar > 2){
        FillRect(bx1+3, by1+3, 20, 20, 0x8cc6ff); // centro

        //arriba
        if ((by1 > 42 && by1<228)&& ((bx1 >= 16 && bx1 <= 52-20)||(bx1 >= 68 && bx1 <= 104-20)||(bx1 >= 120 && bx1 <= 156-20)||(bx1 >= 172 && bx1 <= 208-20)||(bx1 >= 224 && bx1 <= 260-20)||(bx>276))){
          FillRect(bx1+3, by1-17, 20, 20, 0x8cc6ff);
          if (by1> 68 && by1 < 288){
            FillRect(bx1+3, by1-37, 20, 20, 0x8cc6ff);
          }
        }
        
        //derecha
        if ((bx1>16 && bx1< 273 )&& ((by1>=32 && by1<=69-20)||(by1>=85 && by1<=122-20)||(by1>=138 && by1<=175-20)||(by1>=191))){
          FillRect(bx1+23, by1+3, 20, 20, 0x8cc6ff);
          if (bx>16 && bx<253){
            FillRect(bx1+43, by1+3, 20, 20, 0x8cc6ff);
          }
        }
        
        //izquierda
        if((bx1 > 58 && bx1 <290)&&((by1>=32 && by1<=69-20)||(by1>=85 && by1<=122-20)||(by1>=138 && by1<=175-20)||(by1>=191))){
          FillRect(bx1-17, by1+3, 20, 20, 0x8cc6ff);
          if (bx1 > 68 && bx1 < 290){
            FillRect(bx1-37, by1+3, 20, 20, 0x8cc6ff);
          }
        }
        
        //abajo
        if ((by1>16 && by1<191)&&((bx1 >= 16 && bx1 <= 52-20)||(bx1 >= 68 && bx1 <= 104-20)||(bx1 >= 120 && bx1 <= 156-20)||(bx1 >= 172 && bx1 <= 208-20)||(bx1 >= 224 && bx1 <= 260-20)||(bx>276))){
          FillRect(bx1+3, by1+23, 20, 20, 0x8cc6ff);
        if (by1>16 && by1<165){
            FillRect(bx1+3, by1+43, 20, 20, 0x8cc6ff);
          }
        }
        if (Bombaexplota == 2){
          FillRect(x, y, 28, 36, 0x8cc6ff);
          x = 21;
          y = 33;
          LCD_Sprite(x, y, 28, 36, bomberman1, 3, anim, 0, 0);
          v1 --;
          Bombaexplota = 0;
        }

        if (Bombaexplota == 3){
          FillRect(x2, y2, 28, 36, 0x8cc6ff);
          x2 = 281;
          y2 = 192;
          LCD_Sprite(x2, y2, 28, 36, bomberman2, 3, anim, 0, 0);
          v2 --;
          Bombaexplota = 0;
        }
        
        borrar = 0;
      }
    }

//**************************************************************************************************
//                                EXPLOSIONES J2
//*************************************************************************************************
      if (Explo5 != 0 ){
      Explo5++;
      if (Explo5 > 4){
        bx3 = bx2 ;
        by3 = by2;
        FillRect(bx3, by3, 26, 26, 0x8cc6ff);
        LCD_Sprite(bx3+3, by3+3, 20, 20, explosion, 3, 0, 1, 0); //centro
//**************************************************************************************************
//                           EXPLOSIONES ANTI PAREDES
//**************************************************************************************************
         if ((by3 > 42 && by3<228)&& ((bx3 >= 16 && bx3 <= 52-20)||(bx3 >= 68 && bx3 <= 104-20)||(bx3 >= 120 && bx3 <= 156-20)||(bx3 >= 172 && bx3 <= 208-20)||(bx3 >= 224 && bx3 <= 260-20)||(bx1>276))){
           LCD_Sprite(bx3+3, by3-17, 20, 20, explosion, 3, 2, 1, 0); //arriba 2
           if ((by3+5>y2 && y2> by3+5-73)&&(bx3-1 == x2)){
              Bombaexplota1 = 2;
            }
            if ((by3+5>y && y> by3+5-73)&&(bx3-1 == x)){
              Bombaexplota1 = 3;
            }
           if (by3> 68 && by3 < 288){
              LCD_Sprite(bx3+3, by3-37, 20, 20, explosion, 3, 2, 1, 0); // arriba
              if ((by3+5>y2 && y2> by3-93+5)&&(bx3-1 == x2)){
              Bombaexplota1 = 2;
              }
              if ((by3+5>y && y> by3-93+5)&&(bx3-1 == x)){
              Bombaexplota1 = 3;
              }
           }
         }

         
         if ((bx3>16 && bx3< 273 )&& ((by3>=32 && by3<=69-20)||(by3>=85 && by3<=122-20)||(by3>=138 && by3<=175-20)||(by3>=191))){
            LCD_Sprite(bx3+23, by3+3, 20, 20, explosion, 3, 1, 1, 0); //derecha 2
            if ((bx3+43+1>x2 && x2> bx3+1)&& (by3-5 == y2)){
              Bombaexplota1 = 2;
            }
            if ((bx3+43+1>x && x> bx3+1)&& (by3-5 == y)){
              Bombaexplota1 = 3;
            }
            if (bx2>16 && bx2 < 253){
              LCD_Sprite(bx3+43, by3+3, 20, 20, explosion, 3, 1, 1, 0); // derecha
              if ((bx3+63+1>x2 && x2> bx3+1)&& (by3-5 == y2)){
                Bombaexplota1 = 2;
              }
              if ((bx3+63+1>x && x> bx3+1)&& (by3-5 == y)){
                Bombaexplota1 = 3;
              }
            }
         }

         
        if((bx3 > 41 && bx3 <290)&&((by3>=32 && by3<=69-20)||(by3>=85 && by3<=122-20)||(by3>=138 && by3<=175-20)||(by3>=191))){
            LCD_Sprite(bx3-17, by3+3, 20, 20, explosion, 3, 1, 1, 0); // izquierda 2
            if ((bx3+1-65>=x2 && x2>bx3+1)&& (by3-5 == y2)){
              Bombaexplota1 = 2;
            }
            if ((bx3+1-65>=x && x>bx3+1)&& (by3-5 == y)){
              Bombaexplota1 = 3;
            }
            if (bx3 > 61 && bx3 < 290){
              LCD_Sprite(bx3-37, by3+3, 20, 20, explosion, 3, 1, 1, 0); // izquierda
              if ((bx3+1>x2 && x2> bx3-85+1)&& (by3-5 == y2)){
              Bombaexplota1 = 2;
              }
              if ((bx3+1>x && x> bx3-85+1)&& (by3-5 == y)){
              Bombaexplota1 = 3;
              }
            }
        }

        
        if ((by3>16 && by3<191)&&((bx3 >= 16 && bx3 <= 52-20)||(bx3 >= 68 && bx3 <= 104-20)||(bx3 >= 120 && bx3 <= 156-20)||(bx3 >= 172 && bx3 <= 208-20)||(bx3 >= 224 && bx3 <= 260-20)||(bx2>276))){
            LCD_Sprite(bx3+3, by3+23, 20, 20, explosion, 3, 2, 1, 0); // abajo 2 
            if ((by3+5+59>y2 && y2> by3+5)&&(bx3-1 == x2)){
              Bombaexplota1 = 2;
            }
            if ((by3+5+59>y && y> by3+5)&&(bx3-1 == x)){
              Bombaexplota1 = 3;
            }
            if (by3>16 && by3<165){
              LCD_Sprite(bx3+3, by3+43, 20, 20, explosion, 3, 2, 1, 0); // abajo
              if ((by3+5+79>y2 && y2> by3+5)&&(bx3-1 == x2)){
              Bombaexplota1 = 2;
              }
              if ((by3+5+79>y && y> by3+5)&&(bx3-1 == x)){
              Bombaexplota1 = 3;
              }
            }
        }
         
         Explo5 = 0;
         borrar1 = 1;
      }
    }
//**************************************************************************************************
//                           BORRAR EXPLOSIONES
//**************************************************************************************************
    if (borrar1 != 0){
      borrar1++;
      if (borrar1 > 2){
        FillRect(bx3+3, by3+3, 20, 20, 0x8cc6ff); // centro

        //arriba
        if ((by3 > 42 && by3<228)&& ((bx3 >= 16 && bx3 <= 52-20)||(bx3 >= 68 && bx3 <= 104-20)||(bx3 >= 120 && bx3 <= 156-20)||(bx3 >= 172 && bx3 <= 208-20)||(bx3 >= 224 && bx3 <= 260-20)||(bx2>276))){
          FillRect(bx3+3, by3-17, 20, 20, 0x8cc6ff);
          if (by3> 68 && by3 < 288){
            FillRect(bx3+3, by3-37, 20, 20, 0x8cc6ff);
          }
        }
        
        //derecha
        if ((bx3>16 && bx3< 273 )&& ((by3>=32 && by3<=69-20)||(by3>=85 && by3<=122-20)||(by3>=138 && by3<=175-20)||(by3>=191))){
          FillRect(bx3+23, by3+3, 20, 20, 0x8cc6ff);
          if (bx2>16 && bx2<253){
            FillRect(bx3+43, by3+3, 20, 20, 0x8cc6ff);
          }
        }
        
        //izquierda
        if((bx3 > 58 && bx3 <290)&&((by3>=32 && by3<=69-20)||(by3>=85 && by3<=122-20)||(by3>=138 && by3<=175-20)||(by3>=191))){
          FillRect(bx3-17, by3+3, 20, 20, 0x8cc6ff);
          if (bx3 > 68 && bx3 < 290){
            FillRect(bx3-37, by3+3, 20, 20, 0x8cc6ff);
          }
        }
        
        //abajo
        if ((by3>16 && by3<191)&&((bx3 >= 16 && bx3 <= 52-20)||(bx3 >= 68 && bx3 <= 104-20)||(bx3 >= 120 && bx3 <= 156-20)||(bx3 >= 172 && bx3 <= 208-20)||(bx3 >= 224 && bx3 <= 260-20)||(bx2>276))){
          FillRect(bx3+3, by3+23, 20, 20, 0x8cc6ff);
        if (by3>16 && by3<165){
            FillRect(bx3+3, by3+43, 20, 20, 0x8cc6ff);
          }
        }
        if (Bombaexplota1 == 2){
          FillRect(x2, y2, 28, 36, 0x8cc6ff);
          x2 = 281;
          y2 = 192;
          LCD_Sprite(x2, y2, 28, 36, bomberman2, 3, anim, 0, 0);
          v2 --;
          Bombaexplota1 = 0;
        }
        if (Bombaexplota1 == 3){
          FillRect(x, y, 28, 36, 0x8cc6ff);
          x = 21;
          y = 33;
          LCD_Sprite(x, y, 28, 36, bomberman1, 3, anim, 0, 0);
          v1 --;
          Bombaexplota1 = 0;
        }
        
        borrar1 = 0;
      }
    }

//**************************************************************************************************
//                                  MOVIMIENTO J1
//*************************************************************************************************
      // Derecha
      while ((x < 309 - 28) && (xJ == 1) && ((y <= 33) || (y >= 85)) && ((y <= 87) || (y >= 137)) && ((y <= 139) || (y >= 189))) {
        delay(10);
        int anim = (x / 11) % 8;
        LCD_Sprite(x, y, 28, 36, bomberman1, 3, anim, 0, 0);
        V_line( x - 1, y, 36, 0x8cc6ff);
        x++;
        joystickXState = analogRead(joystickX);
        if (joystickXState < 1800) {
          xJ = 2;
        } else if (joystickXState > 2100) {
          xJ = 1;
        } else {
          xJ = 0;
        }
      }

      // Izquierda
      while ((x > 20) && (xJ == 2) && ((y <= 33) || (y >= 85)) && ((y <= 87) || (y >= 137)) && ((y <= 139) || (y >= 189))) {
        delay(10);
        int anim = (x / 11) % 8;
        LCD_Sprite(x, y, 28, 36, bomberman1, 3, anim, 1, 0);
        V_line( x + 28, y, 36, 0x8cc6ff);
        x--;
        joystickXState = analogRead(joystickX);
        if (joystickXState < 1800) {
          xJ = 2;
        } else if (joystickXState > 2100) {
          xJ = 1;
        } else {
          xJ = 0;
        }
      }

      // Abajo
      while ((y < 227 - 36) && (yJ == 1) && ((x <= 32) || (x >= 70)) && ((x <= 74) || (x >= 122)) && ((x <= 126) || (x >= 174)) && ((x <= 178) || (x >= 226)) && ((x <= 230) || (x >= 278))) {
        delay(10);
        int anim = (y / 11) % 8;
        LCD_Sprite(x, y, 28, 36, bomberman1, 3, anim, 0, 0);
        H_line( x, y - 1, 28, 0x8cc6ff);
        y++;
        joystickYState = analogRead(joystickY);
        if (joystickYState < 1800) {
          yJ = 2;
        } else if (joystickYState > 2100) {
          yJ = 1;
        } else {
          yJ = 0;
        }
      }

      // Arriba
      while ((y > 33) && (yJ == 2) && ((x <= 32) || (x >= 70)) && ((x <= 74) || (x >= 122)) && ((x <= 126) || (x >= 174)) && ((x <= 178) || (x >= 226)) && ((x <= 230) || (x >= 278))) {
        delay(10);
        int anim = (y / 11) % 8;
        LCD_Sprite(x, y, 28, 36, bomberman1, 3, anim, 1, 0);
        H_line( x, y + 36, 28, 0x8cc6ff);
        y--;
        joystickYState = analogRead(joystickY);
        if (joystickYState < 1800) {
          yJ = 2;
        } else if (joystickYState > 2100) {
          yJ = 1;
        } else {
          yJ = 0;
        }
      }
      // Bomba
      if ((b == 1) && ((bx + 26 <= x) || (bx - 26 >= x) || (by + 26 <= y) || (by - 26 >= y))) {
        LCD_Sprite(bx, by, 26, 26, bomba, 3, anim, 1, 0);
        b = 0;
        //v1--; Activar para disminuir vida al poner bomba. SOLO prueba si funciona peor no va aqui.
      }
      joystickSelState = digitalRead(joystickSel);
      if (joystickSelState == LOW) {
        bx = x + 1;
        by = y + 5;
        b = 1;
        Explo = 1;
      }
      
//**************************************************************************************************
//                                  MOVIMIENTO J2
//*************************************************************************************************
      // Derecha
      while ((x2 < 309 - 28) && (xJ1 == 1) && ((y2 <= 33) || (y2 >= 85)) && ((y2 <= 87) || (y2 >= 137)) && ((y2 <= 139) || (y2 >= 189))) {
        delay(10);
        int anim = (x2 / 11) % 8;
        LCD_Sprite(x2, y2, 28, 36, bomberman2, 3, anim, 0, 0);
        V_line( x2 - 1, y2, 36, 0x8cc6ff);
        x2++;
        joystickXState1 = analogRead(joystickX1);
        if (joystickXState1 < 1800) {
          xJ1 = 2;
        } else if (joystickXState1 > 2100) {
          xJ1 = 1;
        } else {
          xJ1 = 0;
        }
      }

      // Izquierda
      while ((x2 > 20) && (xJ1 == 2) && ((y2 <= 33) || (y2 >= 85)) && ((y2 <= 87) || (y2 >= 137)) && ((y2 <= 139) || (y2 >= 189))) {
        delay(10);
        int anim = (x2 / 11) % 8;
        LCD_Sprite(x2, y2, 28, 36, bomberman2, 3, anim, 1, 0);
        V_line( x2 + 28, y2, 36, 0x8cc6ff);
        x2--;
        joystickXState1 = analogRead(joystickX1);
        if (joystickXState1 < 1800) {
          xJ1 = 2;
        } else if (joystickXState1 > 2100) {
          xJ1 = 1;
        } else {
          xJ1 = 0;
        }
      }

      // Abajo
      while ((y2 < 227 - 36) && (yJ1 == 1) && ((x2 <= 32) || (x2 >= 70)) && ((x2 <= 74) || (x2 >= 122)) && ((x2 <= 126) || (x2 >= 174)) && ((x2 <= 178) || (x2 >= 226)) && ((x2 <= 230) || (x2 >= 278))) {
        delay(10);
        int anim = (y2 / 11) % 8;
        LCD_Sprite(x2, y2, 28, 36, bomberman2, 3, anim, 0, 0);
        H_line( x2, y2 - 1, 28, 0x8cc6ff);
        y2++;
        joystickYState1 = analogRead(joystickY1);
        if (joystickYState1 < 1800) {
          yJ1 = 2;
        } else if (joystickYState1 > 2100) {
          yJ1 = 1;
        } else {
          yJ1 = 0;
        }
      }

      // Arriba
      while ((y2 > 33) && (yJ1 == 2) && ((x2 <= 32) || (x2 >= 70)) && ((x2 <= 74) || (x2 >= 122)) && ((x2 <= 126) || (x2 >= 174)) && ((x2 <= 178) || (x2 >= 226)) && ((x2 <= 230) || (x2 >= 278))) {
        delay(10);
        int anim = (y2 / 11) % 8;
        LCD_Sprite(x2, y2, 28, 36, bomberman2, 3, anim, 1, 0);
        H_line( x2, y2 + 36, 28, 0x8cc6ff);
        y2--;
        joystickYState1 = analogRead(joystickY1);
        if (joystickYState1 < 1800) {
          yJ1 = 2;
        } else if (joystickYState1 > 2100) {
          yJ1 = 1;
        } else {
          yJ1 = 0;
        }
      }
      // Bomba
      if ((b5 == 1) && ((bx2 + 26 <= x2) || (bx2 - 26 >= x2) || (by2 + 26 <= y2) || (by2 - 26 >= y2))) {
        LCD_Sprite(bx2, by2, 26, 26, bomba, 3, anim, 1, 0);
        b5 = 0;
      }
      joystickSelState = digitalRead(joystickSel);
      if (joystickSelState == LOW) {
        bx2 = x2 + 1;
        by2 = y2 + 5;
        b5 = 1;
        Explo5 = 1;
      }

    }
  }
}
//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER)
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40 | 0x80 | 0x20 | 0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
  //  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);
  LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c) {
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
    }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y + h, w, c);
  V_line(x  , y  , h, c);
  V_line(x + w, y  , h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y + i, w, c);
  }
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background)
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;

  if (fontSize == 1) {
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if (fontSize == 2) {
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }

  char charInput ;
  int cLength = text.length();
  Serial.println(cLength, DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength + 1];
  text.toCharArray(char_array, cLength + 1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1) {
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2) {
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + width;
  y2 = y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k + 1]);
      //LCD_DATA(bitmap[k]);
      k = k + 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 =   x + width;
  y2 =    y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  int k = 0;
  int ancho = ((width * columns));
  if (flip) {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width - 1 - offset) * 2;
      k = k + width * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k - 2;
      }
    }
  } else {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width + 1 + offset) * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k + 2;
      }
    }


  }
  digitalWrite(LCD_CS, HIGH);
}
