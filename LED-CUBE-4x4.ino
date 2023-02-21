/*
  define data_pin     2  // DS     14-Pin PD2
  define clk_pin      3  // SH_CP  11-Pin PD3
  define latch_pin    4  // ST_CP  12-Pin PD4
  5 Pin  4 slice // PD5
  6 Pin  3 slice // PD6
  7 Pin  2 slice // PD7
  8 Pin  1 slice // PB0

  Побитово выводит цифру через 74HC595
  Но если надо выставить 0 так, чтобы не прибить остальные биты в 0,
  нужен оператор "И", обозначающийся &. Так же к нему понадобится оператор "НЕ" - обозначается ~.
  PORTD &= ~B01000000;
  Отправляем байт на последнею микросхему в цепочке
  инвертирование битов исключающим или ^
  ShuftOut( SEG[ solder_s[i] ] ^ 0xFF );
  Отправляем байт на вторую микросхему в цепочке
  ShuftOut( SEG[ hold_s[i] ] ^ 0xFF );
  Зажигает по очереди сегменты на обоих индикаторах
  ShuftOut(NUM_SEG[i]);
  digitalWrite(latch_pin, HIGH);
  PORTD |= B01000000;
*/
#define F_CPU           8000000
#define F_MCU           8000000

#define BUTTON          9
#define BUTTON_READ !bitRead(PINB, 1)

#ifndef F_CPU
  #define F_CPU 8e6
#endif

#include <avr/io.h>
#include <avr/interrupt.h>

int8_t BUTTON_FLAG = 0;
static int8_t cube_mode = 0;
uint8_t prev_mode = 0;
uint8_t press_count_time = 0;

uint8_t counter = 0;
uint8_t lev_ch1, lev_ch2, lev_ch3, lev_ch4;
uint8_t buf_lev_ch1, buf_lev_ch2, buf_lev_ch3, buf_lev_ch4;

void(* gotoLoop) (void) = *loop;

enum mode_t {
    KRIS,
    CUBE,
    CUBE_PWM,
    YLITKA,
    MASS,
    SLICE,
    INV_SLICE,
    VERTIKAL,
    INV_VERTIKAL,
    GORIZONT,
    INV_GORIZONT,
    INCREMENT
};

typedef enum
{
    NO_PRESS = 0x00,
    SINGLE_PRESS = 0x01,
    LONG_PRESS = 0x02,
    DOUBLE_PRESS = 0x03,
} eButtonEvent;

volatile uint8_t event = NO_PRESS;

const uint16_t mass[4][4]= {
  {256, 512, 1024, 2048},
  {4096, 8192, 16384, 32768},
  {1, 2, 4, 8},
  {16, 32, 64, 128},
};

ISR(TIMER1_COMPA_vect)  //обработчик прерывания по совпадению 1А
{
  TCNT1 = 0;            //обнуляем регистр TCNT1

  if(digitalRead(BUTTON) == 0 && BUTTON_FLAG == 0) {
    BUTTON_FLAG = 1;
  }

  if(digitalRead(BUTTON) == 0 && BUTTON_FLAG == 1) {
    press_count_time = press_count_time > 1000 ? 1000 : press_count_time + 1;
  }

  if(digitalRead(BUTTON) == 1 && BUTTON_FLAG == 1) {
    if (press_count_time > 75) {
      event = LONG_PRESS;
    }
    else if (press_count_time > 10) {
      event = SINGLE_PRESS;
    }

    if (event != NO_PRESS) {
    
      if (event == LONG_PRESS) {
        cube_mode--;
        cube_mode = (cube_mode < 0) ? INCREMENT : cube_mode;
      }
      else {
        cube_mode++;
        cube_mode = (cube_mode > INCREMENT) ? 0 : cube_mode;
      }
    }

    BUTTON_FLAG = 0;
    press_count_time = 0;

/*
    asm volatile (
      "pop r0\n"   // remove PC value from stack
      "pop r0\n"
      // load new PC into regs 30 and 31 (I can't remember how to reference an external C function address)
      "push r30\n"
      "push r31\n"
//      "reti \n"
    );

    return loop();
//    gotoLoop();
*/
  }
}

void initTimer1() {
  TCCR1A = 0;           // настройка таймера 1, канала А
  TCCR1B = 0;

// TCCR1B |= (1<<CS11);          // CLK/8
// TCCR1B |= (1<<CS10)|(1<<CS11); // CLK/64
// TCCR1B |= (1<<CS12);          // CLK/256
// TCCR1B |= (1<<CS10)|(1<<CS12); // CLK/1024
	TCCR1B |= (1 << CS12);        // CLK/256
	TCCR1B |= (1 << WGM12);     // CTC

  OCR1A = 100;                // количество отсчетов
  TIMSK |= (1 << OCIE1A);     // включить прерывание по совпадению таймера
  sei();                      // разрешаем прерывания (запрещаем: cli(); )
}

void setup() {
  //2,3,4 порты настраиваем на вывод
  DDRD |= B11111100;
  PORTD |= B00000000; // 1


  //2,3,4 порты настраиваем на вывод
  DDRB |= B00011101;
  PORTB |= B00000000; // 1

  pinMode(1, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  initTimer1();
}

void loop() {
  clear();

  if (prev_mode != cube_mode) {
    prev_mode = cube_mode;
  }

  switch(cube_mode) {
    case KRIS:
      PrintKris();
      break;
    case CUBE:
      PrintCude(0);
      break;
    case CUBE_PWM:
      PrintCude(1);
      break;
    case YLITKA:
      PrintYlitka();
      break;
    case MASS:
      PrintMass();
      break;
    case SLICE:
      PrintSlice(0);
      break;
      case INV_SLICE:
      PrintSlice(1);
      break;
    case VERTIKAL:
      PrintVertical(0);
      break;
    case INV_VERTIKAL:
      PrintVertical(1);
      break;
    case GORIZONT:
      PrintGorizont(0);
      break;
    case INV_GORIZONT:
      PrintGorizont(1);
      break;
    case INCREMENT:
      PrintIncrement();
      break;
  }
}
//End loop

void ShiftOut( uint8_t value ) {
  for (uint8_t i = 0; i < 8; i++) {
    if (prev_mode != cube_mode) {
      return;
    }

    //digitalWrite(data_pin,(value & (0x80 >> i)));  //MSB
    !!(value & (0x80 >> i)) == LOW ? PORTD &= ~B00000100 : PORTD |= B00000100;
    //digitalWrite(clk_pin, HIGH);
    PORTD |= B00001000;
    //digitalWrite(clk_pin, LOW);
    PORTD &= ~B00001000;
  }
}
// End ShiftOut

void PrintSlice(uint8_t r) {
  uint8_t i;
  
  r ? i = 3 : i = 0;
  const uint8_t a[3] = {128, 64, 32};

  //digitalWrite(latch_pin, LOW); 4 port
  PORTD &= ~B00010000;
  ShiftOut(255);
  ShiftOut(255);
  //digitalWrite(latch_pin, HIGH);
  PORTD |= B00010000;

  while(1) {
    if (prev_mode != cube_mode) {
      break;
    }

    if (i == 0) {
      PORTB &= ~1;
      PORTB |= B00000001; // 1
    } else {
      PORTD |= a[i-1]; // 1
    }

    delay(500);

    PORTD &= ~B11100000; //0
    PORTB &= ~B00000001; //0
    
    if (r) {
      if (i == 0) break;
    } else {
      if (i == 3) break;
    }
    r ? i-- : i++;
  }

  PORTD &= ~B11100000;
  PORTB &= ~B00000001;
}
 
void PrintIncrement() {
  PORTB |= B00000001;
  PORTD &= ~B11100000;  // 0

  for (uint16_t i = 0; i < 65535; i++) {
    //digitalWrite(latch_pin, LOW); 4 port
    //Но если надо выставить 0 так, чтобы не прибить остальные биты в 0,
    // нужен оператор "И", обозначающийся &. Так же к нему понадобится оператор "НЕ" - обозначается ~.
    PORTD &= ~B00010000;
    
    // Отправляем байт на последнею микросхему в цепочке
    // инвертирование битов исключающим или ^
    ShiftOut(i);
    ShiftOut(i >> 8);
    // Отправляем байт на вторую микросхему в цепочке

    //digitalWrite(latch_pin, HIGH);
    PORTD |= B00010000;

    if(cube_mode != prev_mode) {
      break;
    }

    delay(250);
  }
}

void PrintGorizont(uint8_t r) {
  uint16_t i;
  r ? i = 61440 : i = 15;
  
  while(1) {
    //digitalWrite(latch_pin, LOW); 4 port
    PORTD &= ~B00010000;
    ShiftOut(i);
    ShiftOut( i >> 8 );
    //digitalWrite(latch_pin, HIGH);
    PORTD |= B00010000;

    PORTD |= B11100000; // 1
    PORTB |= B00000001; // 1

    if (prev_mode != cube_mode) {
      break;
    }

    delay(500);

    if (r) {
      if (i == 15) break;
    } else {
      if (i == 61440) break;
    }
    r ? i = i/16 : i *= 16;
  }
  
  PORTD &= ~B11100000;
  PORTB &= ~B00000001;
}

void PrintVertical(uint8_t r) {
    uint16_t i;
    r ? i = 34952 : i = 4369;
  
      while(1) {
        //digitalWrite(latch_pin, LOW); 4 port
        PORTD &= ~B00010000;
        ShiftOut(i);
        ShiftOut(i >> 8);
        //digitalWrite(latch_pin, HIGH);
        PORTD |= B00010000;


        PORTD |= B11100000; // 1
        PORTB |= B00000001; // 1

        if (prev_mode != cube_mode) {
          break;
        }

        delay(500);
     
        if (r) {
          if (i == 4369) break;
        } else {
          if (i == 34952) break;
        }
        r ? i = i/2 : i *= 2;
      }
   
  PORTD &= ~B11100000;
  PORTB &= ~B00000001;
}

void PrintKris() {
  //PORTD |= B11100000; //1
  PORTD |= B00100000;  //0
  PORTB |= B00000001;

    uint8_t i = 0;
    uint16_t a[8] = { 44234, 61064, 43754, 59534, 58436, 43754, 44714, 19178 };

    while(1) {
      //digitalWrite(latch_pin, LOW); 4 port
      PORTD &= ~B00010000;
      ShiftOut(a[i]);
      ShiftOut(a[i] >> 8);
      //digitalWrite(latch_pin, HIGH);
      PORTD |= B00010000;

      PORTB &= ~B00000001;
      PORTB |= B00000001; // 1

      if (prev_mode != cube_mode) {
        break;
      }

      delay(300);
      i++;
      if (i > 8) break;
    }

  PORTD &= ~B11100000;  //0
}

void PrintMass() {
  PORTD &= ~B11100000;
  PORTB &= ~B00000001;

  for(uint8_t z = 0; z < 4; z++) {
    if (prev_mode != cube_mode) {
      break;
    }

    if(z == 0) PORTB |= B00000001; // 1
    if(z == 1) PORTD |= B10000000; // 1
    if(z == 2) PORTD |= B01000000; // 1
    if(z == 3) PORTD |= B00100000; // 1

    for(uint8_t i = 0; i < 4; i++) {
      for(uint8_t y = 0; y < 4; y++) {
        //digitalWrite(latch_pin, LOW); 4 port
        PORTD &= ~B00010000;
        ShiftOut(mass[i][y] >> 8);
        ShiftOut(mass[i][y]);
        //digitalWrite(latch_pin, HIGH);
        PORTD |= B00010000;

        if (prev_mode != cube_mode) {
          break;
        }

        delay(350);
      }

      if (prev_mode != cube_mode) {
        break;
      }
    }

    PORTD &= ~B11100000;
    PORTB &= ~B00000001;
  }
}

void PixelXY(uint8_t x, uint8_t y) {
    if (prev_mode != cube_mode) {
      return;
    }

    //digitalWrite(latch_pin, LOW); 4 port
    PORTD &= ~B00010000;
    ShiftOut( mass[x][y] >> 8 );
    ShiftOut( mass[x][y] );
    //digitalWrite(latch_pin, HIGH);

    if (prev_mode != cube_mode) {
      return;
    }

    PORTD |= B00010000;
  
    PORTB &= ~B00000001;
    PORTB |= B00000001; // 1
}

void PrintYlitka() {
  PORTD |= B11100000; //1
  PORTB |= B00000001;

  for(uint8_t i = 0; i < 4; i++) {
    PixelXY(0, i);

    if (prev_mode != cube_mode) {
      break;
    }

    delay(300);
  }

  if (prev_mode != cube_mode) {
    return;
  }

  for(uint8_t i = 1; i < 4; i++) {
    PixelXY(i, 3);

    if (prev_mode != cube_mode) {
      break;
    }

    delay(300);
  }

  if (prev_mode != cube_mode) {
    return;
  }

  PixelXY(3,2);
  if (prev_mode != cube_mode) {
    return;
  }  
  delay(300);

  PixelXY(3,1);
  if (prev_mode != cube_mode) {
    return;
  }  
  delay(300);

  PixelXY(3,0);
  if (prev_mode != cube_mode) {
    return;
  }  
  delay(300);

  PixelXY(2,0);
  if (prev_mode != cube_mode) {
    return;
  }  
  delay(300);

  PixelXY(1,0);
  if (prev_mode != cube_mode) {
    return;
  }  
  delay(300);

  PixelXY(1,1);
  if (prev_mode != cube_mode) {
    return;
  }  
  delay(300);

  PixelXY(1,2);
  if (prev_mode != cube_mode) {
    return;
  }  
  delay(300);

  PixelXY(2,2);
  if (prev_mode != cube_mode) {
    return;
  }  
  delay(300);
  
  PixelXY(2,1);
  if (prev_mode != cube_mode) {
    return;
  }  
  delay(300);
}

void clear() {
    PORTD &= ~B00010000;
    ShiftOut(0);
    ShiftOut(0);
    PORTD |= B00010000;
}

void PrintCude (uint8_t mode) {
  PORTD &= ~B00010000;
  ShiftOut(255);
  ShiftOut(255);
  PORTD |= B00010000;

  if (mode) {
    uint16_t _delay;
    uint8_t spine = 0;
    int8_t pwm = 3;
    int8_t max_pwm = 127;

    while (1)       //бесконечная шарманка
    {
      if (prev_mode != cube_mode) {
        break;
      }

      PORTB |= B00000001; // 1
      PORTD |= B11100000; // 1
      delay_nop(pwm);
      PORTD &= ~B11100000;
      PORTB &= ~B00000001;
      delay_nop(max_pwm - pwm);

      _delay++;

      if(_delay > max_pwm * 3) {
        _delay = 0;
        spine ? pwm-- : pwm++;
      }

      if(pwm >= max_pwm) {
        spine = 1;
      }

      if(pwm <= 0) {
        spine = 0;
      }
    }
  }
  else {
    PORTB |= B00000001; // 1
    PORTD |= B11100000; // 1
  }

  if (prev_mode != cube_mode) {
    return;
  }

  delay(300);
}

void delay_nop(uint16_t i) {
  while (i > 0)
  {
    asm volatile("nop");
    i--;
  }
} 