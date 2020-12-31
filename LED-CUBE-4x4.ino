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

const uint16_t mass[4][4]= {
  {256, 512, 1024, 2048},
  {4096, 8192, 16384, 32768},
  {1, 2, 4, 8},
  {16, 32, 64, 128},
};

void setup() {

  //2,3,4 порты настраиваем на вывод
  DDRD |= B11111100;
  PORTD |= B00000000; // 1

  //2,3,4 порты настраиваем на вывод
  DDRB |= B00011101;
  PORTB |= B00000000; // 1
}

void loop(){
  uint16_t i;
while (1) {
  for (uint8_t i = 0; i < 8; i++) {
    PrintYlitka();
    PrintKris();
    PrintMass();
    PrintSlice(0);
    PrintSlice(1);
    PrintVertical(0);
    PrintVertical(1);
    PrintGorizont(0);
    PrintGorizont(1);    
    PrintIncrement();
  }
}}
//End loop

void ShiftOut( uint8_t value ) {
    for (uint8_t i = 0; i < 8; i++) {
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
  delay(500);
}
 
void PrintIncrement() {
  //PORTD |= B00100000; // 1
  //PORTD &= ~B00100000;  // 0
  PORTB |= B00000001;

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
    delay(100);
  }
 delay(1000);
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
  delay(500);
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
  delay(500);
}

void PrintKris() {
  PORTD |= B11100000; //1
  //PORTD &= ~B00100000;  //0
  PORTB |= B00000001;

    uint8_t i = 0;
    uint16_t a[4] = {44234, 60136, 43754, 59534};

    while(1) {
     //digitalWrite(latch_pin, LOW); 4 port
      PORTD &= ~B00010000;
      ShiftOut(a[i]);
      ShiftOut(a[i] >> 8);
      //digitalWrite(latch_pin, HIGH);
      PORTD |= B00010000;

      PORTB &= ~B00000001;
      PORTB |= B00000001; // 1
      delay(500);
      i++;
      if (i > 3) break;
    }

  PORTD &= ~B11100000;  //0
  delay(500);
}

void PrintMass() {
  uint8_t i, y, z;

  for(z = 0; z < 4; z++) {
    if(z == 0) PORTB |= B00000001; // 1
    if(z == 1) PORTD |= B10000000; // 1
    if(z == 2) PORTD |= B01000000; // 1
    if(z == 3) PORTD |= B00100000; // 1
  for(i = 0; i < 4; i++) {
  for(y = 0; y < 4; y++) {    
    //digitalWrite(latch_pin, LOW); 4 port
    PORTD &= ~B00010000;
    ShiftOut(mass[i][y] >> 8);
    ShiftOut(mass[i][y]);
    //digitalWrite(latch_pin, HIGH);
    PORTD |= B00010000;
    delay(350);
  }}
    PORTD &= ~B11100000;
    PORTB &= ~B00000001;
  }
}

void PixelXY(uint8_t x, uint8_t y) {
    //digitalWrite(latch_pin, LOW); 4 port
    PORTD &= ~B00010000;
    ShiftOut( mass[x][y] >> 8 );
    ShiftOut( mass[x][y] );
    //digitalWrite(latch_pin, HIGH);
    PORTD |= B00010000;
  
    PORTB &= ~B00000001;
    PORTB |= B00000001; // 1
    delay(300);
}

void PrintYlitka() {
  PixelXY(0,0);
  PixelXY(0,1);
  PixelXY(0,2);
  PixelXY(0,3);
  PixelXY(1,3);
  PixelXY(2,3);
  PixelXY(3,3);
  PixelXY(3,2);
  PixelXY(3,1);
  PixelXY(3,0);
  PixelXY(2,0);
  PixelXY(1,0);
  PixelXY(1,1);
  PixelXY(1,2);
  PixelXY(2,2);
  PixelXY(2,1);
}
