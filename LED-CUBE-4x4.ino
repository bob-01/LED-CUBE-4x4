// #define data_pin     2  // DS     14-Pin
// #define clk_pin      3  // SH_CP  11-Pin
// #define latch_pin    4  // ST_CP  12-Pin

const uint8_t NUM_SEG[] = {
  1,
  2,
  4
};

void setup(){

  //2,3,4 порты настраиваем на вывод
  DDRD |= B00011100;

  //2,3,4 порты настраиваем на вывод
  DDRB |= B00011100;
}

void loop(){
  uint16_t i;
while (1) {

  for (i = 0; i < 65535; i++) {
    i=65300;
    //digitalWrite(latch_pin, LOW); 4 port
    //Но если надо выставить 0 так, чтобы не прибить остальные биты в 0,
    // нужен оператор "И", обозначающийся &. Так же к нему понадобится оператор "НЕ" - обозначается ~.
    PORTD &= ~B00010000;
    
    // Отправляем байт на последнею микросхему в цепочке
    // инвертирование битов исключающим или ^
    ShiftOut( i >> 8 );
    // Отправляем байт на вторую микросхему в цепочке
    ShiftOut( i & 0xFF );

    //digitalWrite(latch_pin, HIGH);
    PORTD |= B00010000;
    delay(5);
 }
}
}
//End loop

void ShiftOut( uint8_t value ) {
    for (uint8_t i = 0; i < 8; i++) {
      //digitalWrite(data_pin,(value & (0x80 >> i)));  //MSB
      !!(value & (0x80 >> i)) == LOW ? PORTD |= B00000100 : PORTD &= ~B00000100;
      //digitalWrite(clk_pin, HIGH);
      PORTD |= B00001000;
      //digitalWrite(clk_pin, LOW);
      PORTD &= ~B00001000;
    }
}
// End ShiftOut



