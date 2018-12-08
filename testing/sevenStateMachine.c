unsigned char determineData(int v){
  switch(v){
    case 1: {
      return 0b10000000;  //Decimal Point
    }
    case 2: {
      return 0b01111110;  //0
    }
    case 3: {
      return 0b00110000;  //1
    }
    case 4: {
      return 0b01101101;  //2
    }
    case 5: {
      return 0b01111001;  //3
    }
    case 6: {
      return 0b00110011;  //4
    }
    case 7: {
      return 0b01011011;  //5
    }
    case 8: {
      return 0b01011111;  //6
    }
    case 9: {
      return 0b01110000;  //7
    }
    case 10: {
      return 0b01111111;  //8
    }
    case 11: {
      return 0b01111011;  //9
    }
    case 12: {
      return 0b00000001;  //-
    }
    case 13: {
      return 0b00001001;  //=
    }
    case 14: {
      return 0b01110111;  //A
    }
    case 15: {
      return 0b00011111;  //b
    }
    case 16: {
      return 0b01001110;  //C
    }
    case 17: {
      return 0b00111101;  //d
    }
    case 18: {
      return 0b01001111;  //E
    }
    case 19: {
      return 0b01000111;  //F
    }
    case 20: {
      return 0b11000111;  //F.
    }
    case 21: {
      return 0b01011111;  //G
    }
    case 22: {
      return 0b00110111;  //H
    }
    case 23: {
      return 0b10110111;  //H.
    }
    case 24: {
      return 0b00010111;  //h
    }
    case 25: {
      return 0b00000110;  //I
    }
    case 26: {
      return 0b00001110;  //L
    }
    case 27: {
      return 0b01110110;  //upside down U
    }
    case 28: {
      return 0b00011101;  //o
    }
    case 29: {
      return 0b00010101;  //n
    }
    case 30: {
      return 0b01100111;  //P
    }
    case 31: {
      return 0b01011011;  //S or 5
    }
    case 32: {
      return 0b00001111;  //t
    }
    case 33: {
      return 0b00000101;  //r
    }
    case 34: {
      return 0b10000101;  //r.
    }
    case 35: {
      return 0b00111110;  //U
    }
    case 36: {
      return 0b00011100;  //u
    }
    case 37: {
      return 0b00111011;  //y
    }
    case 38: {
      return 0b01101101;  //Z or 2
    }
    case 39: {
      return 0b01100011;  //�
    }
  }
