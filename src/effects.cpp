/*
Copyright © 2020 Dmytro Korniienko (kDn)
JeeUI2 lib used under MIT License Copyright (c) 2019 Marsel Akhkamov

    This file is part of FireLamp_JeeUI.

    FireLamp_JeeUI is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FireLamp_JeeUI is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FireLamp_JeeUI.  If not, see <https://www.gnu.org/licenses/>.

  (Этот файл — часть FireLamp_JeeUI.

   FireLamp_JeeUI - свободная программа: вы можете перераспространять ее и/или
   изменять ее на условиях Стандартной общественной лицензии GNU в том виде,
   в каком она была опубликована Фондом свободного программного обеспечения;
   либо версии 3 лицензии, либо (по вашему выбору) любой более поздней
   версии.

   FireLamp_JeeUI распространяется в надежде, что она будет полезной,
   но БЕЗО ВСЯКИХ ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА
   или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ. Подробнее см. в Стандартной
   общественной лицензии GNU.

   Вы должны были получить копию Стандартной общественной лицензии GNU
   вместе с этой программой. Если это не так, см.
   <https://www.gnu.org/licenses/>.)
*/

#include "main.h"
#include "patterns.h"


// непустой дефолтный деструктор (если понадобится)
// EffectCalc::~EffectCalc(){LOG(println, "Effect object destroyed");}

// ------------- Эффект "Конфетти" --------------
bool EffectSparcles::run(CRGB *ledarr, EffectWorker *opt){
  if (dryrun(3.0))
    return false;
  return sparklesRoutine(*&ledarr, &*opt);
}

String EffectSparcles::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) eff = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectSparcles::sparklesRoutine(CRGB *leds, EffectWorker *param)
{

#ifdef MIC_EFFECTS
  uint8_t mic = myLamp.getMicMapMaxPeak();
  uint8_t mic_f = map(myLamp.getMicMapFreq(), LOW_FREQ_MAP_VAL, HI_FREQ_MAP_VAL, 0, 255);
  if (isMicOn() && eff > 5)
    fadeToBlackBy(leds, NUM_LEDS, 255 - mic);
  EffectMath::fader(isMicOn() ? map(scale, 1, 255, 100, 1) : map(scale, 1, 255, 50, 1));
#else
  EffectMath::fader(map(scale, 1, 255, 1, 50));
#endif

  CHSV currentHSV;

  for (uint8_t i = 0; i < (uint8_t)round(2.5 * (speed / 255.0) + 1); i++) {
    uint8_t x = random8(0U, WIDTH);
    uint8_t y = random8(0U, HEIGHT);
#ifdef MIC_EFFECTS
    switch (eff)
  {
  case 1 :
  case 6 :
    mic_f = myLamp.getMicMapFreq();
    break;
  case 2 :
  case 7 :
    mic_f = map(myLamp.getMicMapFreq(), LOW_FREQ_MAP_VAL, HI_FREQ_MAP_VAL, 0, 255);
    break;
  case 3 :
  case 8 :
    mic_f = sin8(myLamp.getMicMapFreq());
    break;
  case 4 :
  case 9 :
    mic_f = cos8(myLamp.getMicMapFreq());
    break;
  case 5 :
  case 10 :
    mic_f = random8(255U);
    break;
  }

#endif
    if (EffectMath::getPixColorXY(x, y) == 0U) {
#ifdef MIC_EFFECTS
      if (isMicOn()) {
        currentHSV = CHSV(mic_f, 255U - myLamp.getMicMapMaxPeak()/3, constrain(mic * 1.25f, 48, 255));
      }
      else
        currentHSV = CHSV(random8(1U, 255U), random8(192U, 255U), random8(192U, 255U));
#else
        currentHSV = CHSV(random8(1U, 255U), random8(192U, 255U), random8(192U, 255U));
#endif
      EffectMath::setLed(myLamp.getPixelNumber(x, y), currentHSV);
    }
  }
  return true;
}

// ------ Эффект "Белая Лампа"
// ------------- белый свет (светится горизонтальная полоса по центру лампы; масштаб - высота центральной горизонтальной полосы; скорость - регулировка от холодного к тёплому; яркость - общая яркость) -------------
bool EffectWhiteColorStripe::run(CRGB *ledarr, EffectWorker *opt){

  return whiteColorStripeRoutine(*&ledarr, &*opt);
}

bool EffectWhiteColorStripe::whiteColorStripeRoutine(CRGB *leds, EffectWorker *param)
{
#ifdef MIC_EFFECTS
  byte _scale = isMicOn() ? myLamp.getMicMapMaxPeak() : scale;
#else
  byte _scale = scale;
#endif
    if(_scale <= 127){
        uint8_t centerY = max((uint8_t)round(HEIGHT / 2.0F) - 1, 0);
        uint8_t bottomOffset = (uint8_t)(!(HEIGHT & 1) && (HEIGHT > 1));                      // если высота матрицы чётная, линий с максимальной яркостью две, а линии с минимальной яркостью снизу будут смещены на один ряд
        //Serial.printf_P(PSTR("%d %d\n"), centerY, bottomOffset);
        for (int16_t y = centerY; y >= 0; y--)
        {
          int br = BRIGHTNESS-(13*(WIDTH-y)); if((br-(255-myLamp.getLampBrightness()))<0) br=0;

          CRGB color = CHSV(
            45U,                                                                              // определяем тон
            map(speed, 0U, 255U, 0U, 170U),                            // определяем насыщенность
            y == centerY                                                                                                    // определяем яркость
              ? BRIGHTNESS                                                                                                  // для центральной горизонтальной полосы (или двух) яркость всегда равна BRIGHTNESS
              : ((_scale) / 143.0F) > ((centerY + 1.0F) - (y + 1.0F)) / (centerY + 1.0F) ? BRIGHTNESS : br);  // для остальных горизонтальных полос яркость равна либо BRIGHTNESS, либо вычисляется по br

          for (int16_t x = 0U; x < (int16_t)WIDTH; x++)
          {
            EffectMath::drawPixelXY(x, y, color);                                                         // при чётной высоте матрицы максимально яркими отрисуются 2 центральных горизонтальных полосы
            EffectMath::drawPixelXY(x, max((uint8_t)(HEIGHT - 1U) - (y + 1U) + bottomOffset, 0U), color); // при нечётной - одна, но дважды
          }
        }
    } else {
        uint8_t centerX = max((uint8_t)round(WIDTH / 2.0F) - 1, 0);
        uint8_t leftOffset = (uint8_t)(!(WIDTH & 1) && (WIDTH > 1));                      // если ширина матрицы чётная, линий с максимальной яркостью две, а линии с минимальной яркостью слева будут смещены на один ряд
        //Serial.printf_P(PSTR("%d %d\n"), centerX, leftOffset);
        for (int16_t y = 0U; y < (int16_t)HEIGHT; y++)
        {
          for (int16_t x = centerX; x >= 0; x--)
          {
            int br = BRIGHTNESS-(13*(WIDTH-x)); if((br-(255-myLamp.getLampBrightness()))<0) br=0;

            CRGB color = CHSV(
              45U,                                                        // определяем тон
              map(speed, 0U, 255U, 0U, 170U),                            // определяем насыщенность
              x == centerX                                                                                                    // определяем яркость
                ? BRIGHTNESS                                                                                                  // для центральной вертикальной полосы (или двух) яркость всегда равна BRIGHTNESS
                : ((255-_scale) / 143.0F) > ((centerX + 1.0F) - (x + 1.0F)) / (centerX + 1.0F) ? BRIGHTNESS : br);  // для остальных вертикальных полос яркость равна либо BRIGHTNESS, либо вычисляется по br

            EffectMath::drawPixelXY(x, y, color);                                                      // при чётной ширине матрицы максимально яркими отрисуются 2 центральных вертикальных полосы
            EffectMath::drawPixelXY(max((uint8_t)(WIDTH - 1U) - (x + 1U) + leftOffset, 0U), y, color); // при нечётной - одна, но дважды
          }
        }
    }
  return true;
}

// ========== Эффект "Эффектопад"
// совместное творчество юзеров форума https://community.alexgyver.ru/
void EffectEverythingFall::load(){
    palettesload();    // подгружаем дефолтные палитры
}

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKINGNEW 80U // 50 // 30 // 120 // 90 // 60
bool EffectEverythingFall::run(CRGB *ledarr, EffectWorker *opt){
  if (dryrun(4.0))
    return false;

  uint8_t coolingnew = map (scale, 1, 255, 93, 10);

  for (uint8_t x = 0; x < WIDTH; x++) {
    // Step 1.  Cool down every cell a little
    for (uint8_t i = 0; i < HEIGHT; i++) {
      heat[x][i] = qsub8(heat[x][i], random(0, coolingnew));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (unsigned int k = HEIGHT - 1; k >= 2; k--) {
      heat[x][k] = (heat[x][k - 1] + heat[x][k - 2] + heat[x][k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < SPARKINGNEW) {
      int y = random(2);
      heat[x][y] = qadd8(heat[x][y], random(160, 255));
    }

    // Step 4.  Map from heat cells to LED colors
    for (uint8_t j = 0; j < HEIGHT; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8(heat[x][(uint8_t)j], 240);
      nblend(myLamp.getUnsafeLedsArray()[myLamp.getPixelNumber(x, (HEIGHT - 1) - j)], ColorFromPalette(*curPalette, colorindex /*, heat[x][j]*/), 50);

    }
  }
  return true;
}

// --------------------------- эффект пульс ----------------------
// Stefan Petrick's PULSE Effect mod by PalPalych for GyverLamp
bool EffectPulse::run(CRGB *ledarr, EffectWorker *opt){
  if (dryrun(3.0))
    return false;
  return pulseRoutine(*&ledarr, &*opt);
}

bool EffectPulse::pulseRoutine(CRGB *leds, EffectWorker *param) {

  CRGBPalette16 palette;
  CRGB _pulse_color;
  uint8_t _pulse_delta = 0;

  palette = RainbowColors_p;
  uint8_t _scale = scale;
#ifdef MIC_EFFECTS
  #define FADE 255U - (isMicOn() ? myLamp.getMicMapMaxPeak()*2 : 248U) // (isMicOn() ? 300U - myLamp.getMicMapMaxPeak() : 5U)
  #define BLUR (isMicOn() ? myLamp.getMicMapMaxPeak()/3 : 10U) //(isMicOn() ? map(myLamp.getMicMapMaxPeak(), 1, 255, 1, 30) : 10U)
#else
  #define FADE 7U
  #define BLUR 10U
#endif

  //EffectMath::dimAll(248U); // если эффект устанавливается с другими эффектами от Stefan Petrick, тогда  процедура должна называться dimAll (без двоечки)
  fadeToBlackBy(leds, NUM_LEDS, FADE);
  if (pulse_step <= currentRadius) {
    for (uint8_t i = 0; i < pulse_step; i++ ) {
      uint8_t _dark = qmul8( 2U, cos8 (128U / (pulse_step + 1U) * (i + 1U))) ;
      if (_scale == 1) {            // 1 - случайные диски
        _pulse_hue = pulse_hue;
        _pulse_color = CHSV(_pulse_hue, 255U, _dark);

      } else if (_scale <= 17) {    // 2...17 - перелив цвета дисков
        _pulse_delta = (17U - _scale) ;
        _pulse_color = CHSV(_pulse_hueall, 255U, _dark);

      } else if (_scale <= 33) {    // 18...33 - выбор цвета дисков
        _pulse_hue = (_scale - 18U) * 16U ;
        _pulse_color = CHSV(_pulse_hue, 255U, _dark);

      } else if (_scale <= 50) {    // 34...50 - дискоцветы
        _pulse_hue += (_scale - 33U) * 5U ;
        _pulse_color = CHSV(_pulse_hue, 255U, _dark);

      } else if (_scale <= 67) {    // 51...67 - пузыри цветы
        uint8_t _sat =  qsub8( 255U, cos8 (128U / (pulse_step + 1U) * (i + 1U))) ;
         _pulse_hue += (68U - _scale) * 7U ;
        _pulse_color = CHSV(_pulse_hue, _sat, _dark);

      } else if (_scale < 83) {     // 68...83 - выбор цвета пузырей
        uint8_t _sat =  qsub8( 255U, cos8 (128U / (pulse_step + 1U) * (i + 1U))) ;
        _pulse_hue = (_scale - 68U) * 16U ;
        _pulse_color = CHSV(_pulse_hue, _sat, _dark);

      } else if (_scale < 100) {    // 84...99 - перелив цвета пузырей
        uint8_t _sat =  qsub8( 255U, cos8 (128U / (pulse_step + 1U) * (i + 1U))) ;
        _pulse_delta = (_scale - 85U)  ;
        _pulse_color = CHSV(_pulse_hueall, _sat, _dark);

      } else { // 100 - случайные пузыри
        uint8_t _sat =  qsub8( 255U, cos8 (128U / (pulse_step + 1U) * (i + 1U))) ;
        _pulse_hue = pulse_hue;
        _pulse_color = CHSV(_pulse_hue, _sat, _dark);
      }
      EffectMath::drawCircle(centerX, centerY, i, _pulse_color);
    }
  } else {
    centerX = random8(WIDTH - 5U) + 3U;
    centerY = random8(HEIGHT - 5U) + 3U;
    _pulse_hueall += _pulse_delta;
    pulse_hue = random8(0U, 255U);
    currentRadius = random8(3U, 9U);
    pulse_step = 0;
  }
  pulse_step++;
  EffectMath::blur2d(BLUR);
  return true;
}

// радуги 2D
// ------------- радуга вертикальная/горизонтальная ----------------
//uint8_t hue;
bool EffectRainbow::rainbowHorVertRoutine(bool isVertical)
{
  for (uint8_t i = 0U; i < (isVertical?WIDTH:HEIGHT); i++)
  {
#ifdef MIC_EFFECTS
    uint8_t micPeak = myLamp.getMicMapMaxPeak();
    CHSV thisColor = CHSV((uint8_t)(hue + i * (micPeak > map(speed, 1, 255, 100, 10) and isMicOn() ? (scale - micPeak) : scale) % 170), 255, 255); // 1/3 без центральной между 1...255, т.е.: 1...84, 170...255
#else
    CHSV thisColor = CHSV((uint8_t)(hue + i * scale%170), 255, 255);
#endif
    for (uint8_t j = 0U; j < (isVertical?HEIGHT:WIDTH); j++)
    {
      EffectMath::drawPixelXY((isVertical?i:j), (isVertical?j:i), thisColor);
    }
  }
  return true;
}

// ------------- радуга диагональная -------------
bool EffectRainbow::run(CRGB *ledarr, EffectWorker *opt){
  return rainbowDiagonalRoutine(*&ledarr, &*opt);
}

bool EffectRainbow::rainbowDiagonalRoutine(CRGB *leds, EffectWorker *param)
{
  // коэф. влияния замаплен на скорость, 4 ползунок нафиг не нужен
  hue += (6.0 * (speed / 255.0) + 0.05 ); // скорость смещения цвета зависит от кривизны наклна линии, коэф. 6.0 и 0.05

  if(scale<85){
    rainbowHorVertRoutine(false);
    return true;
  } else if (scale>170){
    rainbowHorVertRoutine(true);
    return true;
  }

  for (uint8_t i = 0U; i < WIDTH; i++)
  {
    for (uint8_t j = 0U; j < HEIGHT; j++)
    {
      float twirlFactor = EffectMath::fmap((float)scale, 85, 170, 8.3, 24);      // на сколько оборотов будет закручена матрица, [0..3]
#ifdef MIC_EFFECTS
      twirlFactor *= myLamp.getMicMapMaxPeak() > map(speed, 1, 255, 80, 10) and isMicOn() ? 1.5f * ((float)myLamp.getMicMapFreq() / 255.0f) : 1.0f;
#endif
      CRGB thisColor = CHSV((uint8_t)(hue + ((float)WIDTH / (float)HEIGHT * i + j * twirlFactor) * ((float)255 / (float)myLamp.getmaxDim())), 255, 255);
      EffectMath::drawPixelXY(i, j, thisColor);
    }
  }
  return true;
}


// ------------- цвета -----------------
void EffectColors::load(){
    EffectMath::fillAll(CHSV(scale, 255U, 55U));
}

bool EffectColors::run(CRGB *ledarr, EffectWorker *opt){
  return colorsRoutine(*&ledarr, &*opt);
}

void EffectColors::setscl(const byte _scl){
  EffectCalc::setscl(_scl);
  modeColor = scale;
}

String EffectColors::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) mode = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectColors::colorsRoutine(CRGB *leds, EffectWorker *param)
{
  static unsigned int step = 0; // доп. задержка
  unsigned int delay = (speed==1)?4294967294:255-speed+1; // на скорости 1 будет очень долгое ожидание)))

  ihue = (speed==1)?scale:ihue;
  step=(step+1)%(delay+1);

  if(step!=delay) {

#ifdef MIC_EFFECTS
  uint16_t mmf = myLamp.getMicMapFreq();
  uint16_t mmp = myLamp.getMicMapMaxPeak();

#if defined(LAMP_DEBUG) && defined(MIC_EFFECTS)
EVERY_N_SECONDS(1){
  LOG(printf_P,PSTR("MF: %5.2f MMF: %d MMP: %d scale %d speed: %d\n"), myLamp.getMicFreq(), mmf, mmp, scale, speed);
}
#endif
  if(isMicOn()){
    // включен микрофон
    if(scale>=127){
      uint8_t pos = (round(3.0*(mmf+(25.0*speed/255.0))/255.0))*HEIGHT/8; // двигаем частоты по диапазону в зависимости от скорости и делим на 4 части 0...3
      for(uint8_t y=pos;y<pos+HEIGHT/8;y++){
        for(uint8_t x=0; x<WIDTH; x++){
          EffectMath::setLed(myLamp.getPixelNumber(x, y), CHSV(mmf / 1.5, 255U, constrain(mmp * (2.0 * (scale >> 1) / 127.0 + 0.33), 1, 255)));
          EffectMath::setLed(myLamp.getPixelNumber(x, HEIGHT - 1 - y), CHSV(mmf / 1.5, 255U, constrain(mmp * (2.0 * (scale >> 1) / 127.0 + 0.33), 1, 255)));
        }
      }
      EffectMath::dimAll(254); // плавно гасим
    } else {
      if(mmp>scale) // если амплитуда превышает масштаб
        EffectMath::fillAll(CHSV(constrain(mmf*(2.0*speed/255.0),1,255), 255U, constrain(mmp*(2.0*scale/127.0+1.5),1,255))); // превышает минимаьный уровень громкости, значит выводим текущую частоту
      else
        EffectMath::dimAll(252); // плавно гасим
    }
  } else {
    // выключен микрофон
    switch(mode){
      case 1:
        if(!step){
          if(!modeColor){
            modeColor = scale;
            EffectMath::fillAll(CHSV(modeColor, 255U, 255U));
          }
          else {
            modeColor = 0;
            FastLED.clear();
          }          
        }
        break;
      case 2:
        EffectMath::fillAll(CHSV(ihue, 255U, 255U));
        break;
      case 3:
        if(!step){
          if(scale<=127)
            modeColor = ~modeColor;
          else
            modeColor = modeColor ? 0 : map(scale,128,255,1,255);
          EffectMath::fillAll(CHSV(modeColor, 255U, 255U));
        }
        break;
      default:
        EffectMath::fillAll(CHSV(ihue, 255U, 255U));      
    }
  }
#else
  EffectMath::fillAll(CHSV(ihue, 255U, 255U));
#endif
  } else {
    ihue += scale; // смещаемся на следущий
  }
  return true;
}

// ------------- Эффект "New Матрица" ---------------
String EffectMatrix::setDynCtrl(UIControl*_val)
{
  if(_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) hue = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) gluk = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)

  if (hue == 1) {
    randColor = true;
    white = false;
  } else if (hue == 255) {
    white = true;
    randColor = false;
  } else {
    randColor = false;
    white = false;
  }
  return String();
}

bool EffectMatrix::run(CRGB *ledarr, EffectWorker *opt){
  //fpsmeter();
  return matrixRoutine(*&ledarr, &*opt);
}

void EffectMatrix::load(){
  randomSeed(analogRead(millis()));
  for (uint8_t i = 0U; i < LIGHTERS_AM; i++)
  {
    lightersPos[0U][i] = random(0, WIDTH);
    lightersPos[1U][i] = EffectMath::randomf(HEIGHT - HEIGHT /2, HEIGHT);
    lightersSpeed[0U][i] = 1;
    lightersSpeed[1U][i] = (float)random(10, 20) / 10.0f;
    lightersColor[i] = hue;
    light[i] = random(196,255);
  }
}

bool EffectMatrix::matrixRoutine(CRGB *leds, EffectWorker *param)
{
  float speedfactor = EffectMath::fmap((float)speed, 1., 255., 0.06, 0.4);
  EffectMath::dimAll(map(speed, 1, 255, 252, 240));
  
  CHSV color;

  for (uint8_t i = 0U; i < map(_scale, 1, 32, 1, LIGHTERS_AM); i++)
  {
    lightersPos[1U][i] -= lightersSpeed[1U][i]*speedfactor;

    if (white) {
      color = rgb2hsv_approximate(CRGB::Gray);
      color.val = light[i];
    } else if (randColor) {
      EVERY_N_MILLIS(600 / speedfactor) {
        hue = random(1, 250);
      }
      color = CHSV(hue, 255, light[i]);
    } else {
      color = CHSV(hue, 255, light[i]);
    }


    EffectMath::drawPixelXYF_Y(lightersPos[0U][i], lightersPos[1U][i], color);

    count += speedfactor;

    if (gluk > 1 and (uint8_t)count%2 == 0) 
      if (random8() < gluk) {
        lightersPos[0U][i] = lightersPos[0U][i] + random(-1, 2);
        light[i] = random(196,255);
      }

    if(lightersPos[1U][i] < -1) {
      lightersPos[0U][i] = random(0, WIDTH);
      lightersPos[1U][i] = EffectMath::randomf(HEIGHT - HEIGHT /2, HEIGHT);
      lightersSpeed[1U][i] = EffectMath::randomf(1.5, 2.5); 
      light[i] = random(127U, 255U);
      lightersColor[i] = hue;
    }
  }

  return true;
}

// ------------- звездопад/метель -------------
bool EffectStarFall::run(CRGB *ledarr, EffectWorker *opt){
  return snowStormStarfallRoutine(*&ledarr, &*opt);
}

void EffectStarFall::load(){
  randomSeed(millis());
  for (uint8_t i = 0U; i < LIGHTERS_AM; i++)
  {
    lightersPos[0U][i] = random(-(int)WIDTH, WIDTH);
    lightersPos[1U][i] = random(HEIGHT - 1, HEIGHT + 4);
    lightersSpeed[0U][i] = EffectMath::randomf(-1, 1);  // X
    lightersSpeed[1U][i] = EffectMath::randomf(1, 2);   // Y
    lightersColor[i] = random(0U, 255U);
    light[i] = 255;
  }
}

String EffectStarFall::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) effId = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) isNew = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectStarFall::snowStormStarfallRoutine(CRGB *leds, EffectWorker *param)
{
  float speedfactor = EffectMath::fmap(speed, 1, 255, 0.25, .5);
  EffectMath::dimAll(255 - (effId == 2 ? 70 : 60) * speedfactor);
  CHSV color;
  for (uint8_t i = 0U; i < map(_scale, 1, 10, LIGHTERS_AM/8, LIGHTERS_AM); i++) // LIGHTERS_AM
  {
    //color = CHSV((effId > 1 ? lightersColor[i] : 255), (effId > 1 ? light[i] : 0), (effId > 1 ? 255 : light[i]));
    switch (effId)
    {
    case 1:
      color = CHSV(127, 0, light[i]);
      break;
    case 2:
      if (light[i] > 10) { byte tmp = light[i] - 10 * speedfactor; color = CHSV(lightersColor[i], 255 - light[i], tmp); light[i]=tmp; }
      else color = rgb2hsv_approximate( CRGB::Black);
      break;
    case 3:
      color = CHSV(lightersColor[i], 255, light[i]);
      break;
    default:
      break;
    }

    if (isNew) {
      lightersPos[0U][i] -= lightersSpeed[0U][effId == 1 ? 0 : i] * speedfactor;
      lightersPos[1U][i] -= 1 * speedfactor;
    } else {
      lightersPos[0U][i] += lightersSpeed[0U][i] * (speedfactor / 2);
      lightersPos[1U][i] -= lightersSpeed[1U][0] * (speedfactor / 2);
    }

   // if (isNew){
      EffectMath::drawPixelXYF(lightersPos[0U][i], lightersPos[1U][i], color, 0);
   /* }
    else {
      EffectMath::drawPixelXY((uint8_t)lightersPos[0U][i], (uint8_t)lightersPos[1U][i], color);
    }*/

    if(lightersPos[1U][i] < -1) {
      if (isNew) {
        lightersPos[0U][i] = random(-(int)WIDTH, WIDTH);
        lightersPos[1U][i] = effId > 1 ? random(HEIGHT / 2, HEIGHT + 4) : random(HEIGHT - 1, HEIGHT + 4);
        lightersSpeed[0U][i] = EffectMath::randomf(-1, 1);  // X
        lightersSpeed[1U][i] = EffectMath::randomf(1, 2);   // Y
      } else {
        lightersPos[0U][i] = (float)random(-(WIDTH * 10 - 2), (WIDTH * 10 - 2)) / 10.0f;
        lightersPos[1U][i] = random(HEIGHT, HEIGHT + 4);
        lightersSpeed[0U][i] = (float)random(15, 25) / 10.0f;   // X
        lightersSpeed[1U][i] = lightersSpeed[0U][i]; //(float)random(15, 25) / 10.0f;   // Y
      }
      lightersColor[i] = random(0U, 255U);
      light[i] = random(127U, 255U);
    }
  }
  return true;
}

// ------------- светлячки --------------
//#define LIGHTERS_AM           (100U)
void EffectLighters::load(){
  randomSeed(millis());
  for (uint8_t i = 0U; i < LIGHTERS_AM; i++)
  {
    lightersIdx=0;
    lightersPos[0U][i] = random(0, WIDTH);
    lightersPos[1U][i] = random(0, HEIGHT);
    lightersSpeed[0U][i] = (float)random(-200, 200) / 10.0f;
    lightersSpeed[1U][i] = (float)random(-200, 200) / 10.0f;
    lightersColor[i] = random(0U, 255U);
    light[i] = random(1U, 3U)*127U;
  }
}

bool EffectLighters::run(CRGB *ledarr, EffectWorker *opt){
  return lightersRoutine(*&ledarr, &*opt);
}

String EffectLighters::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) cnt = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) subPix = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectLighters::lightersRoutine(CRGB *leds, EffectWorker *param)
{
  float speedfactor = (float)speed / 4096.0f + 0.001f;

 // EffectMath::blur2d(speed/10);
  //EffectMath::dimAll(50 + speed/10);
  memset8( leds, 0, NUM_LEDS * 3);

  EVERY_N_MILLIS(333)
  {
    lightersIdx = (lightersIdx+1)%constrain(cnt,1,LIGHTERS_AM);
    lightersSpeed[0U][lightersIdx] += random(-10, 10);
    lightersSpeed[1U][lightersIdx] += random(-10, 10);
    lightersSpeed[0U][lightersIdx] = fmod(lightersSpeed[0U][lightersIdx], 21);
    lightersSpeed[1U][lightersIdx] = fmod(lightersSpeed[1U][lightersIdx], 21);
    light[lightersIdx] = random(255U-(cnt*8),255U);
    if(!random(cnt+3))
      light[lightersIdx] = 127;
  }

  for (uint8_t i = 0U; i < constrain(cnt,1,LIGHTERS_AM); i++) // масштабируем на LIGHTERS_AM, чтобы не было выхода за диапазон
  {
    // EVERY_N_SECONDS(1)
    // {
    //   LOG.printf_P("S0:%d S1:%d P0:%3.2f P1:%3.2f, cnt:%3.2f\n", lightersSpeed[0U][i], lightersSpeed[1U][i],lightersPos[0U][i],lightersPos[1U][i],speedfactor);
    // }
    lightersPos[0U][i] += lightersSpeed[0U][i]*speedfactor;
    lightersPos[1U][i] += lightersSpeed[1U][i]*speedfactor;

    if (lightersPos[0U][i] < 0) lightersPos[0U][i] = (float)(WIDTH - 1);
    if (lightersPos[0U][i] >= (float)WIDTH) lightersPos[0U][i] = 0.0f;

    if (lightersPos[1U][i] <= 0.0f)
    {
      lightersPos[1U][i] = 0.0f;
      lightersSpeed[1U][i] = -lightersSpeed[1U][i];
      lightersSpeed[0U][i] = -lightersSpeed[0U][i];
    }
    if (lightersPos[1U][i] >= (int32_t)(HEIGHT - 1))
    {
      lightersPos[1U][i] = (HEIGHT - 1U);
      lightersSpeed[1U][i] = -lightersSpeed[1U][i];
      lightersSpeed[0U][i] = -lightersSpeed[0U][i];
    }

    if (subPix)
      EffectMath::drawPixelXYF(lightersPos[0U][i], lightersPos[1U][i], CHSV(lightersColor[i], 255U-(i*2), light[i]), 0);
    else
      EffectMath::drawPixelXY((uint8_t)lightersPos[0U][i], (uint8_t)lightersPos[1U][i], CHSV(lightersColor[i], 255U-(i*2), light[i]));
  }
  return true;
}

// ------------- светлячки со шлейфом -------------
String EffectLighterTracers::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) cnt = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectLighterTracers::load(){
  for (uint8_t j = 0U; j < _AMOUNT; j++)
  {
    int8_t sign;
    // забиваем случайными данными
    coord[j][0U] = (float)WIDTH / 2.0f;
    random(0, 2) ? sign = 1 : sign = -1;
    vector[j][0U] = ((float)random(40, 150) / 10.0f) * sign;
    coord[j][1U] = (float)HEIGHT / 2;
    random(0, 2) ? sign = 1 : sign = -1;
    vector[j][1U] = ((float)random(40, 150) / 10.0f) * sign;
    light[j] = 127;
    //ballColors[j] = random(0, 9) * 28;
  }
}


bool EffectLighterTracers::run(CRGB *ledarr, EffectWorker *opt){

  return lighterTracersRoutine(*&ledarr, &*opt);
}

bool EffectLighterTracers::lighterTracersRoutine(CRGB *leds, EffectWorker *param)
{
  //float speedfactor = speed/2048.0+0.01;
  float speedfactor = EffectMath::fmap(speed, 1, 255, 0.01, .1);

  fadeToBlackBy(leds, NUM_LEDS, map(speed, 1, 255, 6, 55)); // размер шлейфа должен сохранять размер, не зависимо от скорости

  // движение шариков
  uint8_t maxBalls = cnt;
  for (uint8_t j = 0U; j < maxBalls; j++)
  {
    ballColors[j] = (maxBalls-j) * _AMOUNT + j;

    // движение шариков
    for (uint8_t i = 0U; i < 2U; i++)
    {
      coord[j][i] += vector[j][i] * speedfactor;
      if (coord[j][i] < 0)
      {
        coord[j][i] = 0.0f;
        vector[j][i] = -vector[j][i];
      }
    }

    if ((uint16_t)coord[j][0U] > (WIDTH - 1))
    {
      coord[j][0U] = (WIDTH - 1);
      vector[j][0U] = -vector[j][0U];
    }
    if ((uint16_t)coord[j][1U] > (HEIGHT - 1))
    {
      coord[j][1U] = (HEIGHT - 1);
      vector[j][1U] = -vector[j][1U];
    }
    EVERY_N_MILLIS(random16(256, 1024)) {
      if (light[j] == 127)
        light[j] = 255;
      else light[j] = 127;
    }
    EffectMath::drawPixelXYF(coord[j][0U], coord[j][1U], CHSV(ballColors[j], 200U, 255U));
  }
  blur2d(leds, WIDTH, HEIGHT, 5);
  return true;
}

// ------------- пейнтбол -------------
bool EffectLightBalls::run(CRGB *ledarr, EffectWorker *opt){
  return lightBallsRoutine(*&ledarr, &*opt);
}

#define BORDERTHICKNESS       (1U)                          // глубина бордюра для размытия яркой частицы: 0U - без границы (резкие края); 1U - 1 пиксель (среднее размытие) ; 2U - 2 пикселя (глубокое размытие)
bool EffectLightBalls::lightBallsRoutine(CRGB *leds, EffectWorker *param)
{
  const uint8_t paintWidth = WIDTH - BORDERTHICKNESS * 2;
  const uint8_t paintHeight = HEIGHT - BORDERTHICKNESS * 2;

  // Apply some blurring to whatever's already on the matrix
  // Note that we never actually clear the matrix, we just constantly
  // blur it repeatedly.  Since the blurring is 'lossy', there's
  // an automatic trend toward black -- by design.
  uint8_t blurAmount = dim8_raw(beatsin8(3,64,100));
  blur2d(leds, WIDTH, HEIGHT, blurAmount);

  float speedScale = (((float)speed)/255.0)+0.1;

  // Use two out-of-sync sine waves
  uint16_t  i = beatsin16( 79*speedScale, 0, 255); //91
  uint16_t  j = beatsin16( 67*speedScale, 0, 255); //109
  uint16_t  k = beatsin16( 53*speedScale, 0, 255); //73
  uint16_t  m = beatsin16( 97*speedScale, 0, 255); //123

  // The color of each point shifts over time, each at a different speed.
  uint16_t ms = millis() / (scale/16 + 1);
  leds[myLamp.getPixelNumber( highByte(i * paintWidth) + BORDERTHICKNESS, highByte(j * paintHeight) + BORDERTHICKNESS)] += CHSV( ms / 29, 200U, 255U);
  leds[myLamp.getPixelNumber( highByte(j * paintWidth) + BORDERTHICKNESS, highByte(k * paintHeight) + BORDERTHICKNESS)] += CHSV( ms / 41, 200U, 255U);
  leds[myLamp.getPixelNumber( highByte(k * paintWidth) + BORDERTHICKNESS, highByte(m * paintHeight) + BORDERTHICKNESS)] += CHSV( ms / 37, 200U, 255U);
  leds[myLamp.getPixelNumber( highByte(m * paintWidth) + BORDERTHICKNESS, highByte(i * paintHeight) + BORDERTHICKNESS)] += CHSV( ms / 53, 200U, 255U);

  return true;
}

// ------------- эффект "блуждающий кубик" -------------
void EffectBall::load(){
  //LOG(printf_P, PSTR("getCtrlVal(5)=%s, %d\n"), getCtrlVal(5).c_str(), getCtrlVal(5).toInt()); /// провека чтения отсутствующего контрола

  if (scale <= 85)
    ballSize = map(scale, 1, 85, 1U, max((uint8_t)min(WIDTH,HEIGHT) / 3, 1));
  else if (scale > 85 and scale <= 170)
    ballSize = map(scale, 170, 86, 1U, max((uint8_t)min(WIDTH,HEIGHT) / 3, 1));
  else
    ballSize = map(scale, 171, 255, 1U, max((uint8_t)min(WIDTH,HEIGHT) / 3, 1));

  for (uint8_t i = 0U; i < 2U; i++)
  {
    coordB[i] = i? float(WIDTH - ballSize) / 2 : float(HEIGHT - ballSize) / 2;
    vectorB[i] = EffectMath::randomf(0, 24.) - 12.0;
    ballColor = CHSV(random(1, 250), random(200, 255), 255);
  }
}

bool EffectBall::run(CRGB *ledarr, EffectWorker *opt){
  return ballRoutine(*&ledarr, &*opt);
}

bool EffectBall::ballRoutine(CRGB *leds, EffectWorker *param)
{
  if (scale <= 85)
    ballSize = map(scale, 1, 85, 1U, max((uint8_t)min(WIDTH,HEIGHT) / 3, 1));
  else if (scale > 85 and scale <= 170)
    ballSize = map(scale, 170, 86, 1U, max((uint8_t)min(WIDTH,HEIGHT) / 3, 1));
  else
    ballSize = map(scale, 171, 255, 1U, max((uint8_t)min(WIDTH,HEIGHT) / 3, 1));


// каждые 5 секунд коррекция направления
  EVERY_N_SECONDS(5){
    //LOG(println,ballSize);
    for (uint8_t i = 0U; i < 2U; i++)
    {
      if(fabs(vectorB[i]) < 12)
        vectorB[i] += EffectMath::randomf(0, 8.) - 4.0;
      else if (vectorB[i] > 12)
        vectorB[i] -= EffectMath::randomf(1, 6);
      else
        vectorB[i] += EffectMath::randomf(1, 6);
      if(!(uint8_t)vectorB[i])
        vectorB[i] += vectorB[i] > 0 ? 0.25 : -0.25;

    }
  }

  for (uint8_t i = 0U; i < 2U; i++)
  {
    coordB[i] += vectorB[i] * EffectMath::fmap((float)speed, 1., 255., 0.02, 0.15);
    if ((int8_t)coordB[i] < 0)
    {
      coordB[i] = 0;
      vectorB[i] = -vectorB[i];
      if (RANDOM_COLOR) ballColor = CHSV(random(1, 250), random(200, 255), 255);
    }
  }
  if ((int8_t)coordB[0U] > (int8_t)(WIDTH - ballSize))
  {
    coordB[0U] = (WIDTH - ballSize);
    vectorB[0U] = -vectorB[0U];
    if (RANDOM_COLOR) ballColor = CHSV(random(1, 250), random(200, 255), 255);
  }
  if ((int8_t)coordB[1U] > (int8_t)(HEIGHT - ballSize))
  {
    coordB[1U] = (HEIGHT - ballSize);
    vectorB[1U] = -vectorB[1U];
    if (RANDOM_COLOR) ballColor = CHSV(random(1, 250), random(200, 255), 255);
  }

  if (scale <= 85)  // при масштабе до 85 выводим кубик без шлейфа
    memset8( leds, 0, NUM_LEDS * 3);
  else if (scale > 85 and scale <= 170)
    fadeToBlackBy(leds, NUM_LEDS, 255 - map(speed, 1, 255, 245, 200)); // выводим кубик со шлейфом, длинна которого зависит от скорости.
  else
    fadeToBlackBy(leds, NUM_LEDS, 255 - map(speed, 1, 255, 253, 248)); // выводим кубик с длинным шлейфом, длинна которого зависит от скорости.

  for (uint8_t i = 0; i < ballSize; i++)
  {
    for (uint8_t j = 0; j < ballSize; j++)
    {
      EffectMath::drawPixelXYF(coordB[0U] + (float)i, coordB[1U] + (float)j, ballColor, 0);
    }
  }
  //fpsmeter();
  return true;
}

// ----------- Эффекты "Лава, Зебра, etc"
void Effect3DNoise::fillNoiseLED()
{
  uint8_t dataSmoothing = 0;
  if (speed < 50)
  {
    dataSmoothing = 200 - (speed * 4);
  }
  for (uint8_t i = 0; i < myLamp.getminDim()*2; i++)
  {
    int32_t ioffset = _scale * i;
    for (uint8_t j = 0; j < myLamp.getmaxDim(); j++)
    {
      int32_t joffset = _scale * j;

      uint8_t data = inoise8(x + ioffset, y + joffset, z);

      data = qsub8(data, 16);
      data = qadd8(data, scale8(data, 39));

      if (dataSmoothing)
      {
        uint8_t olddata = noise[i][j];
        uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( data, 256 - dataSmoothing);
        data = newdata;
      }

      noise[i][j] = data;
    }
  }
  z += _speed;

  // apply slow drift to X and Y, just for visual variation.
  x += _speed * 0.125; // 1/8
  y -= _speed * 0.0625; // 1/16

  for (uint8_t i = 0; i < WIDTH; i++)
  {
    for (uint8_t j = 0; j < HEIGHT; j++)
    {
      uint8_t index = noise[j%(myLamp.getminDim()*2)][i];
      uint8_t bri =   noise[i%(myLamp.getminDim()*2)][j];
      // if this palette is a 'loop', add a slowly-changing base value
      if ( colorLoop)
      {
        index += ihue;
      }
      // brighten up, as the color palette itself often contains the
      // light/dark dynamic range desired
      if ( bri > 127 && colorLoop)
      {
        bri = 255;
      }
      else
      {
        bri = dim8_raw( bri * 2);
      }
      CRGB color = ColorFromPalette( *curPalette, index, bri);

      EffectMath::drawPixelXY(i, j, color);                             //leds[getPixelNumber(i, j)] = color;
    }
  }
  ihue += 1;
}

void Effect3DNoise::fillnoise8()
{
  for (uint8_t i = 0; i < myLamp.getminDim()*2; i++)
  {
    int32_t ioffset = _scale * i;
    for (uint8_t j = 0; j < myLamp.getmaxDim(); j++)
    {
      int32_t joffset = _scale * j;
      noise[i][j] = inoise8(x + ioffset, y + joffset, z);
    }
  }
  z += _speed;
}

void Effect3DNoise::load(){
  palettesload();
  fillnoise8();
}

String Effect3DNoise::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3 && _val->getVal().toInt()==0 && !isRandDemo())
    curPalette = &ZeebraColors_p;
  else if(_val->getId()==4) colorLoop = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  fillnoise8();
  return String();
}

bool Effect3DNoise::run(CRGB *ledarr, EffectWorker *opt){
  #ifdef MIC_EFFECTS
    uint8_t mmf = isMicOn() ? myLamp.getMicMapFreq() : 0;
    uint8_t mmp = isMicOn() ? myLamp.getMicMapMaxPeak() : 0;
    _scale = (NOISE_SCALE_AMP*scale/255.0+NOISE_SCALE_ADD)*(mmf>0?(1.5*mmf/255.0):1);
    _speed = NOISE_SCALE_AMP*speed/512.0*(mmf<LOW_FREQ_MAP_VAL && mmp>MIN_PEAK_LEVEL?10:2.5*mmp/255.0+1);
  #else
    _scale = NOISE_SCALE_AMP*scale/255.0+NOISE_SCALE_ADD;
    _speed = NOISE_SCALE_AMP*speed/512.0;
  #endif

  fillNoiseLED();
  return true;
}

//----- Эффект "Прыгающие Мячики"
//  BouncingBalls2014 is a program that lets you animate an LED strip
//  to look like a group of bouncing balls
//  Daniel Wilson, 2014
//  https://github.com/githubcdr/Arduino/blob/master/bouncingballs/bouncingballs.ino
//  With BIG thanks to the FastLED community!
//  адаптация от SottNick
// перевод на субпиксельную графику kostyamat
bool EffectBBalls::run(CRGB *ledarr, EffectWorker *opt){
  return bBallsRoutine(ledarr, opt);
}

// void EffectBBalls::setscl(const byte _scl){
//   EffectCalc::setscl(_scl); // перегрузка для масштаба
//   regen();
// }

void EffectBBalls::regen(){
  FastLED.clear();
  randomSeed(millis());
  if (_scale <= 16) {
    bballsNUM_BALLS =  map(_scale, 1, 16, 1, bballsMaxNUM_BALLS);
  } else {
    bballsNUM_BALLS =  map(_scale, 32, 17, 1, bballsMaxNUM_BALLS);
  }
  for (int i = 0 ; i < bballsNUM_BALLS ; i++) {          // Initialize variables
    bballsCOLOR[i] = random(0, 255);
    bballsBri[i] = 156;
    bballsX[i] = random(0, WIDTH-1);
    bballsBri[i] = halo ? 200 : (bballsX[i - 1] == bballsX[i] ? bballsBri[i-1] + 32 : 156);
    bballsTLast[i] = millis();
    bballsPos[i] = 0.0f;                                 // Balls start on the ground
    bballsVImpact[i] = bballsVImpact0 + EffectMath::randomf( - 2., 2.);                   // And "pop" up at vImpact0
    bballsCOR[i] = 0.90f - float(i) / pow(bballsNUM_BALLS, 2);
    bballsShift[i] = false;
  }
}

String EffectBBalls::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) { _scale = EffectCalc::setDynCtrl(_val).toInt(); regen(); }
  else if(_val->getId()==4) halo = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectBBalls::load(){
  regen();
}

bool EffectBBalls::bBallsRoutine(CRGB *leds, EffectWorker *param)
{
  fadeToBlackBy(leds, NUM_LEDS, _scale <= 16 ? 255 : 50);

  for (int i = 0 ; i < bballsNUM_BALLS ; i++) {
    bballsTCycle =  millis() - bballsTLast[i] ;     // Calculate the time since the last time the ball was on the ground

    // A little kinematics equation calculates positon as a function of time, acceleration (gravity) and intial velocity
    bballsHi = 0.55 * bballsGRAVITY * pow( bballsTCycle / (float)(1550 - speed * 3) , 2.0 ) + bballsVImpact[i] * bballsTCycle / (float)(1550 - speed * 3);

    if ( bballsHi < 0 ) {
      bballsTLast[i] = millis();
      bballsHi = 0.0f;                            // If the ball crossed the threshold of the "ground," put it back on the ground
      bballsVImpact[i] = bballsCOR[i] * bballsVImpact[i] ;   // and recalculate its new upward velocity as it's old velocity * COR


      //if ( bballsVImpact[i] < 0.01 ) bballsVImpact[i] = bballsVImpact0;  // If the ball is barely moving, "pop" it back up at vImpact0
      if ( bballsVImpact[i] < 0.1 ) // сделал, чтобы мячики меняли свою прыгучесть и положение каждый цикл
      {
        bballsCOR[i] = 0.90 - (EffectMath::randomf(0., 9.)) / pow(EffectMath::randomf(4., 9.), 2.); // сделал, чтобы мячики меняли свою прыгучесть каждый цикл
        bballsShift[i] = bballsCOR[i] >= 0.85;                             // если мячик максимальной прыгучести, то разрешаем ему сдвинуться
        bballsVImpact[i] = bballsVImpact0;
      }
    }
    bballsPos[i] = bballsHi * (float)(HEIGHT - 1) / bballsH0;       // Map "h" to a "pos" integer index position on the LED strip
    if (bballsShift[i] > 0.0f && bballsPos[i] >= (float)HEIGHT - 1.5) {                  // если мячик получил право, то пускай сдвинется на максимальной высоте 1 раз
      bballsShift[i] = 0.0f;
      if (bballsCOLOR[i] % 2 == 0) {                                       // чётные налево, нечётные направо
        if (bballsX[i] <= 0) bballsX[i] = (WIDTH - 1U);
        else bballsX[i] -= 1;
      } else {
        if (bballsX[i] >= (int8_t)(WIDTH - 1U)) bballsX[i] = 0;
        else bballsX[i] += 1;
      }
    }

    //if (bballsPos[i] < HEIGHT - 1)
      if (halo){ // если ореол включен
        EffectMath::drawCircleF(bballsX[i], bballsPos[i] + 2.75, 3., CHSV(bballsCOLOR[i], 225, bballsBri[i]));
      } else {
        // попытка создать объем с помощью яркости. Идея в том, что шарик на переднем фоне должен быть ярче, чем другой,
        // который движится в том же Х. И каждый следующий ярче предыдущего.
        bballsBri[i] = (bballsX[i - 1] == bballsX[i] ? bballsBri[i-1] + 32 : 156);
        EffectMath::drawPixelXYF_Y(bballsX[i], bballsPos[i], CHSV(bballsCOLOR[i], 255, bballsBri[i]), 5);
      }
  }
  return true;
}

// ***** SINUSOID3 / СИНУСОИД3 *****
// v1.7.0 - Updating for GuverLamp v1.7 by PalPalych 12.03.2020
/*
  Sinusoid3 by Stefan Petrick (mod by Palpalych for GyverLamp 27/02/2020)
  read more about the concept: https://www.youtube.com/watch?v=mubH-w_gwdA
*/
bool EffectSinusoid3::run(CRGB *ledarr, EffectWorker *opt){
  return sinusoid3Routine(*&ledarr, &*opt);
}

bool EffectSinusoid3::sinusoid3Routine(CRGB *leds, EffectWorker *param)
{
  const uint8_t semiHeightMajor =  HEIGHT / 2 + (HEIGHT % 2);
  const uint8_t semiWidthMajor =  WIDTH / 2  + (WIDTH % 2) ;
  float e_s3_speed = 0.004 * speed + 0.015; // speed of the movement along the Lissajous curves
  float e_s3_size = EffectMath::fmap(scale,1,255,3,9); // amplitude of the curves

  float time_shift = millis()&0xFFFFF; // на больших значениях будет страннео поведение, поэтому уменьшаем точность, хоть и будет иногда срыв картинки, но в 18 минут, так что - хрен с ним

  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      CRGB color;
      uint8_t _scale = map8(255-scale,50,150);

      float cx = (y - semiHeightMajor) + float(e_s3_size * (sin16 (e_s3_speed * 98.301 * time_shift)))/32767.0;  // the 8 centers the middle on a 16x16
      float cy = (x - semiWidthMajor) + float(e_s3_size * (cos16 (e_s3_speed * 72.0874 * time_shift)))/32767.0;
      int8_t v = 127 * (1 + sin16 ( 127*_scale*EffectMath::sqrt ( (((float)cx*cx) + ((float)cy*cy)) ) )/32767.0);
      color.r = ~v;

      cx = (y - semiHeightMajor) + float(e_s3_size * (sin16 (e_s3_speed * 68.8107 * time_shift)))/32767.0;
      cy = (x - semiWidthMajor) + float(e_s3_size * (cos16 (e_s3_speed * 65.534 * time_shift)))/32767.0;
      v = 127 * (1 + sin16 ( 127*_scale*EffectMath::sqrt ( (((float)cx*cx) + ((float)cy*cy)) ) )/32767.0);
      color.g = ~v;

      cx = (y - semiHeightMajor) + float(e_s3_size * (sin16 (e_s3_speed * 134.3447 * time_shift)))/32767.0;
      cy = (x - semiWidthMajor) + float(e_s3_size * (cos16 (e_s3_speed * 170.3884 * time_shift)))/32767.0;
      v = 127 * (1 + sin16 ( 127*_scale*EffectMath::sqrt ( (((float)cx*cx) + ((float)cy*cy)) ) )/32767.0);
      color.b = ~v;

      EffectMath::drawPixelXY(x, y, color);
    }
  }
  return true;
}

/*
  Metaballs proof of concept by Stefan Petrick (mod by Palpalych for GyverLamp 27/02/2020)
  ...very rough 8bit math here...
  read more about the concept of isosurfaces and metaballs:
  https://www.gamedev.net/articles/programming/graphics/exploring-metaballs-and-isosurfaces-in-2d-r2556
*/

// ***** METABALLS / МЕТАШАРИКИ *****
// v1.7.0 - Updating for GuverLamp v1.7 by PalPalych 12.03.2020
bool EffectMetaBalls::run(CRGB *ledarr, EffectWorker *opt){
  return metaBallsRoutine(*&ledarr, &*opt);
}

bool EffectMetaBalls::metaBallsRoutine(CRGB *leds, EffectWorker *param)
{
  float _speed = speed/127.0;

  // get some 2 random moving points
  uint8_t x2 = inoise8(millis() * _speed, 25355, 685 ) / WIDTH;
  uint8_t y2 = inoise8(millis() * _speed, 355, 11685 ) / HEIGHT;

  uint8_t x3 = inoise8(millis() * _speed, 55355, 6685 ) / WIDTH;
  uint8_t y3 = inoise8(millis() * _speed, 25355, 22685 ) / HEIGHT;

  // and one Lissajou function
  uint8_t x1 = beatsin8(23 * _speed, 0, 15);
  uint8_t y1 = beatsin8(28 * _speed, 0, 15);

  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {

      // calculate distances of the 3 points from actual pixel
      // and add them together with weightening
      uint8_t  dx =  fabs(x - x1);
      uint8_t  dy =  fabs(y - y1);
      uint8_t dist = 2 * EffectMath::sqrt((dx * dx) + (dy * dy));

      dx =  fabs(x - x2);
      dy =  fabs(y - y2);
      dist += EffectMath::sqrt((dx * dx) + (dy * dy));

      dx =  fabs(x - x3);
      dy =  fabs(y - y3);
      dist += EffectMath::sqrt((dx * dx) + (dy * dy));

      // inverse result
      byte color = scale*4 / (dist==0?1:dist);

      // map color between thresholds
      if (color > 0 and color < 60) {
        EffectMath::drawPixelXY(x, y, CHSV(color * 9, 255, 255));
      } else {
        EffectMath::drawPixelXY(x, y, CHSV(0, 255, 255));
      }
      // show the 3 points, too
      EffectMath::drawPixelXY(x1, y1, CRGB(255, 255, 255));
      EffectMath::drawPixelXY(x2, y2, CRGB(255, 255, 255));
      EffectMath::drawPixelXY(x3, y3, CRGB(255, 255, 255));
    }
  }
  return true;
}

// ***** Эффект "Спираль"     ****
/*
 * Aurora: https://github.com/pixelmatix/aurora
 * https://github.com/pixelmatix/aurora/blob/sm3.0-64x64/PatternSpiro.h
 * Copyright (c) 2014 Jason Coon
 * Неполная адаптация SottNick
 */

void EffectSpiro::load(){
  palettesload();    // подгружаем дефолтные палитры
}

bool EffectSpiro::run(CRGB *ledarr, EffectWorker *opt){
  return spiroRoutine(*&ledarr, &*opt);
}

bool EffectSpiro::spiroRoutine(CRGB *leds, EffectWorker *param)
{

  // страхуемся от креша
  if (curPalette == nullptr) {
    return false;
  }

  bool change = false;
  float speed_factor = (float)speed/127.0 + 1;
  uint8_t spirooffset = 256 / spirocount;

  //EffectMath::dimAll(254U - palettescale);
  //EffectMath::dimAll(250-speed_factor*7);
  uint8_t dim = beatsin8(16 / speed_factor, 5, 10);
  EffectMath::dimAll(254 - dim);

  for (int i = 0; i < spirocount; i++) {
    uint8_t  x = EffectMath::mapsincos8(MAP_SIN, spirotheta1 + i * spirooffset, spirominx, spiromaxx);
    uint8_t  y = EffectMath::mapsincos8(MAP_COS, spirotheta1 + i * spirooffset, spirominy, spiromaxy);
    uint8_t x2 = EffectMath::mapsincos8(MAP_SIN, spirotheta2 + i * spirooffset, x - spiroradiusx, x + spiroradiusx);
    uint8_t y2 = EffectMath::mapsincos8(MAP_COS, spirotheta2 + i * spirooffset, y - spiroradiusy, y + spiroradiusy);
    CRGB color = ColorFromPalette(*curPalette, (spirohueoffset + i * spirooffset), 128U);
    if (myLamp.getPixelNumber(x2, y2) < NUM_LEDS){
      CRGB tmpColor = EffectMath::getPixColorXY(x2, y2);
      tmpColor += color;
      EffectMath::setLed(myLamp.getPixelNumber(x2, y2), tmpColor);
      //EffectMath::setLed(myLamp.getPixelNumber(WIDTH - x2, HEIGHT - y2), (tmpColor += color)); // строка бредовая, поскольку число спиралей выбирается вовсе не здесь и ломается логика эффекта
    }

    if(x2 == spirocenterX && y2 == spirocenterY) change = true; // в центре могут находится некоторое время
  }

  spirotheta2 += 2.*speed_factor;
  spirotheta1 += 1.*speed_factor;
  spirohueoffset += 1.*speed_factor;

  if (change && !spirohandledChange) { // меняем кол-во спиралей
    spirohandledChange = true;

    if (spirocount >= WIDTH || spirocount == 1)
      spiroincrement = !spiroincrement;

    if (spiroincrement) {
      if(spirocount >= 4)
        spirocount *= 2;
      else
        spirocount += 1;
    } else {
      if(spirocount > 4)
        spirocount /= 2;
      else
          spirocount -= 1;
    }

    spirooffset = 256 / spirocount;
  }

  // сброс спустя время, чтобы счетчик спиралей не менялся скачками
  if(spirohandledChange){
    if(internalCnt==25) { // спустя 25 кадров
      spirohandledChange = false;
      internalCnt=0;
    } else {
      internalCnt++;
    }
  }

  EffectMath::blur2d(25);
  return true;
}

// ***** RAINBOW COMET / РАДУЖНАЯ КОМЕТА *****
// ***** Парящий огонь, Кровавые Небеса, Радужный Змей и т.п.
// базис (c) Stefan Petrick
void EffectComet::drawFillRect2_fast(int8_t x1, int8_t y1, int8_t x2, int8_t y2, CRGB color)
{ // Fine if: 0<x1<x2<WIDTH && 0<y1<y2<HEIGHT
  for (int8_t xP = x1; xP <= x2; xP++)
  {
    for (int8_t yP = y1; yP <= y2; yP++)
    {
      myLamp.getUnsafeLedsArray()[myLamp.getPixelNumber(xP, yP)] += color;
    }
  }
}

void EffectComet::FillNoise(int8_t layer) {
  for (uint8_t i = 0; i < WIDTH; i++) {
    int32_t ioffset = e_scaleX[layer] * (i - e_centerX);
    for (uint8_t j = 0; j < HEIGHT; j++) {
      int32_t joffset = e_scaleY[layer] * (j - e_centerY);
      int8_t data = inoise16(e_x[layer] + ioffset, e_y[layer] + joffset, e_z[layer]) >> 8;
      int8_t olddata = noise3d[layer][i][j];
      int8_t newdata = scale8( olddata, eNs_noisesmooth ) + scale8( data, 255 - eNs_noisesmooth );
      data = newdata;
      noise3d[layer][i][j] = data;
    }
  }
}

void EffectComet::load() {
  eNs_noisesmooth = 200;
  for (uint8_t i = 0; i < NUM_LAYERS; i++) {
    e_x[i] = random16();
    e_y[i] = random16();
    e_z[i] = random16();
    e_x[i] = 6000;
    e_y[i] = 6000;
  }
}

String EffectComet::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) colorId = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) smooth = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==6) blur = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectComet::run(CRGB *ledarr, EffectWorker *opt){
  speedy = map(speed, 1, 255, 20, 255);
  speedFactor = EffectMath::fmap(speed, 1., 255., 0.1, 1.0);
  effId = _scale;
  switch (effId)
  {
  case 1 :
    return rainbowCometRoutine(*&ledarr, &*opt);
    break;
  case 2 :
    return rainbowComet3Routine(*&ledarr, &*opt);
    break;
  case 3 :
    return firelineRoutine(*&ledarr, &*opt);
    break;
  case 4 :
    return fractfireRoutine(*&ledarr, &*opt);
    break;
  case 5 :
    return flsnakeRoutine(*&ledarr, &*opt);
    break;
  case 6 :
    return smokeRoutine(*&ledarr, &*opt);
    break;
  default:
    return false;
  }
}

bool EffectComet::smokeRoutine(CRGB *leds, EffectWorker *param) {
  speedFactor = EffectMath::fmap(speed, 1., 255., 0.1, .5);
  if(isDebug()){
    FastLED.clear(); // для отладки чистим матрицу, чтобы показать перемещение точек
  }
  count ++;
  if (colorId == 1) {
    if (count%2 == 0) hue ++;
  }
  else hue = colorId;

  CRGB color;//, color2;
  hsv2rgb_spectrum(CHSV(hue, (colorId == 255) ? 0 : beatsin8(speed, 220, 255, 0, 180), beatsin8(speed / 2, 64, 255)), color);


  spiral += 3. * speedFactor ;
  if (random8(WIDTH) != 0U) // встречная спираль движется не всегда синхронно основной
    spiral2 += 3. * speedFactor ;

  for (float i = 0; i < HEIGHT; i+= 0.5) {
    float n = (float)quadwave8(i * 4. + spiral) / (256. / (float)WIDTH + 1.0);
    float n2 = (float)quadwave8(i * 5. + beatsin8((smooth*3) * speedFactor)) / (256. / (float)WIDTH + 1.0);

    EffectMath::drawPixelXYF(n, (float)HEIGHT - 1.0 - i, color, 0);
    EffectMath::drawPixelXYF((float)WIDTH - 1.0 - n2, (float)HEIGHT - 1.0 - i, color, 0);

  }
  
  // скорость движения по массиву noise
  if(!isDebug()){
    e_x[0] += 1000 * speedFactor; //1000;
    e_y[0] += 1000 * speedFactor; //1000;
    e_z[0] += 1000 * speedFactor; //1000;
    e_scaleX[0] = 2000 * (blur/5);//12000;
    e_scaleY[0] = 1333 * smooth;

    FillNoise(0);

    EffectMath::MoveFractionalNoise(MOVE_X, noise3d, WIDTH / (getCtrlVal(3).toInt() + 2));//4
    EffectMath::MoveFractionalNoise(MOVE_Y, noise3d, HEIGHT / 8, 0.33);//4

    EffectMath::blur2d(64); // без размытия как-то пиксельно, наверное...  
  }
  return true;
}

bool EffectComet::firelineRoutine(CRGB *leds, EffectWorker *param) {
  //EffectMath::blur2d(15); // нужно ли размытие?
  if(!isDebug()) 
    fadeToBlackBy(leds, NUM_LEDS, map(blur, 1, 64, 20, 5)); // нужны ли эти фейдеры тут? хз...
  else FastLED.clear();

  count ++;
  if (colorId == 1) {
    if (count%2 == 0) hue ++;
  }
  else hue = colorId;

  for (uint8_t i = 1; i < WIDTH; i += 2) {
    leds[myLamp.getPixelNumber( i, e_centerY)] += CHSV(hue + i * 2 , colorId == 255 ? 64 : 255, 255);
  }
  // Noise
  float beat2 = (10.0 -  (float)beatsin88(3 * speedy, 10, 20)) / 10.;
  e_x[0] += 12 * speedy; // 3000;
  e_y[0] += 12 * speedy; // 3000;
  e_z[0] += 12 * speedy; // 3000;
  e_scaleX[0] = 1333 * smooth; // 8000
  e_scaleY[0] = 1333 * smooth; // 8000;
  count ++;

  FillNoise(0);

  EffectMath::MoveFractionalNoise(MOVE_Y, noise3d, 3);
  EffectMath::MoveFractionalNoise(MOVE_X, noise3d, 3, beat2);
  return true;
}

bool EffectComet::fractfireRoutine(CRGB *leds, EffectWorker *param) {

  if(!isDebug()) fadeToBlackBy(leds, NUM_LEDS, map(blur, 1, 64, 20, 5)); // нужны ли эти фейдеры тут? хз...
  else FastLED.clear();

  float beat = (float)beatsin88(5 * speedy, 50, 100) / 100 ;
  count ++;
  if (colorId == 1) {
    if (count%2 == 0) hue ++;
  }
  else hue = colorId;

  for (uint8_t i = 1; i < WIDTH; i += 2) {
    leds[myLamp.getPixelNumber(i, HEIGHT - 1)] += CHSV(hue + i * 2, colorId == 255 ? 64 : 255, 255);
  }
  // Noise
  e_y[0] += 12 * speedy; // 3000;
  e_z[0] += 12 * speedy; // 3000;
  e_scaleX[0] = 1333 * smooth; // 8000;
  e_scaleY[0] = 1333 * smooth; // 8000;
  FillNoise(0);

  EffectMath::MoveFractionalNoise(MOVE_Y, noise3d, 2, beat);
  EffectMath::MoveFractionalNoise(MOVE_X, noise3d, 3);
  EffectMath::blur2d(32); // нужно ли размытие?
  return true;
}

bool EffectComet::flsnakeRoutine(CRGB *leds, EffectWorker *param) {
  if(!isDebug()) EffectMath::dimAll(blur); 
  else FastLED.clear();
  
  count ++;
  if (colorId == 1 or colorId == 255) {
    if (count%2 == 0) hue ++;
  }
  else hue = colorId;

  for (uint8_t y = 2; y < HEIGHT; y += 5) {
    for (uint8_t x = 2; x < WIDTH; x += 5) {
      leds[myLamp.getPixelNumber(x, y)]  += CHSV(x * y + hue, colorId == 255 ? 64 : 255, 255);
      leds[myLamp.getPixelNumber(x + 1, y)] += CHSV((x + 4) * y + hue, colorId == 255 ? 64 : 255, 255);
      leds[myLamp.getPixelNumber(x, y + 1)] += CHSV(x * (y + 4) + hue, colorId == 255 ? 64 : 255, 255);
      leds[myLamp.getPixelNumber(x + 1, y + 1)] += CHSV((x + 4) * (y + 4) + hue, colorId == 255 ? 64 : 255, 255);
    }
  }
  // Noise
  e_x[0] += 12 * speedy; // 3000;
  e_y[0] += 12 * speedy; // 3000;
  e_z[0] += 12 * speedy; // 3000;
  e_scaleX[0] = 1333 * smooth; //8000;
  e_scaleY[0] = 1333 * smooth; //8000;
  FillNoise(0);

  EffectMath::MoveFractionalNoise(MOVE_X, noise3d, 5);
  EffectMath::MoveFractionalNoise(MOVE_Y, noise3d, 5);
  return true;
}

bool EffectComet::rainbowCometRoutine(CRGB *leds, EffectWorker *param)
{ // Rainbow Comet by PalPalych
/*
  Follow the Rainbow Comet Efect by PalPalych
  Speed = tail dispersing
  Scale = 0 - Random 3d color
          1...127 - time depending color
          128...254 - selected color
          255 - white
*/

  EffectMath::blur2d(e_com_BLUR);    // < -- размытие хвоста
  if (blur < 64) fadeToBlackBy(leds, NUM_LEDS, map(blur, 1, 64, 32, 0));

  if(isDebug()){
    FastLED.clear(); // для отладки чистим матрицу, чтобы показать перемещение точек
  }
  CRGB _eNs_color;
  if (colorId == 255) {
    _eNs_color= CRGB::White;
  } else if (colorId == 1) {
    _eNs_color = CHSV(noise3d[0][0][0] * e_com_3DCOLORSPEED , 255, 255);
  } else if (colorId >1 && colorId < 128) {
    _eNs_color = CHSV(millis() / ((uint16_t)colorId + 1U) * 4 + 10, 255, 255);
  } else {
    _eNs_color = CHSV((colorId - 128) * 2, 255, 255);
  }

  drawFillRect2_fast(e_centerX, e_centerY, e_centerX + 1, e_centerY + 1, _eNs_color);

  if(!isDebug()){
    // Noise
    e_x[0] += 12 * speedy; // 2000;
    e_y[0] += 12 * speedy; // 2000;
    e_z[0] += 12 * speedy; // 2000;
    e_scaleX[0] = 667 * smooth; //4000;
    e_scaleY[0] = 667 * smooth; //4000;
    FillNoise(0);
    EffectMath::MoveFractionalNoise(MOVE_X, noise3d, WIDTH / 3U);
    EffectMath::MoveFractionalNoise(MOVE_Y, noise3d, HEIGHT / 3U, 0.5);
  }
  return true;
}

bool EffectComet::rainbowComet3Routine(CRGB *leds, EffectWorker *param)
{ 
  count++;
  EffectMath::blur2d(e_com_BLUR);    // < -- размытие хвоста
  if (blur < 64) fadeToBlackBy(leds, NUM_LEDS, map(blur, 1, 64, 32, 0));

  if (count%2 == 0) hue++;

  if(isDebug()){
    FastLED.clear(); // для отладки чистим матрицу, чтобы показать перемещение точек
  }

  CHSV color;
  color = rgb2hsv_approximate(CRGB::Green);
  if (colorId == 1) color.hue += hue;
  else if (colorId == 255) color.sat = 64;
  else color.hue += colorId;
  EffectMath::drawPixelXYF((float)WIDTH / 2, (float)HEIGHT / 2, color, 0); // зеленый стоит по центру

  color = rgb2hsv_approximate(CRGB::Red);
  if (colorId == 1) color.hue += hue;
  else if (colorId == 255) color.sat = 64;
  else color.hue += colorId;
  float xx = 2. + (float)sin8( millis() / (10. / speedFactor)) / 22.;
  float yy = 2. + (float)cos8( millis() / (9. / speedFactor)) / 22.;
  EffectMath::drawPixelXYF(xx, yy, color, 0);

  xx = 4. + (float)sin8( millis() / (10. / speedFactor)) / 32.;
  yy = 4. + (float)sin8( millis() / (7. / speedFactor)) / 32.;
  color = rgb2hsv_approximate(CRGB::Blue);
  if (colorId == 1) color.hue += hue;
  else if (colorId == 255) color.sat = 64;
  else color.hue += colorId;
  EffectMath::drawPixelXYF(xx, yy, color, 0);

  if(!isDebug()){
    e_x[0] += 3000 * speedFactor;
    e_y[0] += 3000 * speedFactor;
    e_z[0] += 3000 * speedFactor;
    e_scaleX[0] = 667 * smooth; // 4000;
    e_scaleY[0] = 667 * smooth; // 4000;
    FillNoise(0);
    EffectMath::MoveFractionalNoise(MOVE_X, noise3d, WIDTH / 6);
    EffectMath::MoveFractionalNoise(MOVE_Y, noise3d, HEIGHT / 6, 0.33);
  }

  return true;
}

// ============= ЭФФЕКТ ПРИЗМАТА ===============
// Prismata Loading Animation
void EffectPrismata::load(){
  palettesload();
}

bool EffectPrismata::run(CRGB *leds, EffectWorker *opt) {
  EVERY_N_MILLIS(100) {
    spirohueoffset += 1;
  }

  fadeToBlackBy(leds, NUM_LEDS, map(fadelvl, 1, 255, 130, 2)); // делаем шлейф

  for (byte x = 0; x < WIDTH; x++) {
      float y = (float)beatsin16((uint8_t)x + 1 * (float)speed / 2.0f, 0, (HEIGHT-1)* 10) / 10.0f;
      EffectMath::drawPixelXYF_Y(x, y, ColorFromPalette(*curPalette, ((uint16_t)x + spirohueoffset) * 4));
    }
  return true;
}

String EffectPrismata::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) fadelvl = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

// ============= ЭФФЕКТ СТАЯ ===============
// Адаптация от (c) SottNick
void EffectFlock::load(){
  palettesload();    // подгружаем дефолтные палитры
  speedfactor = (float)speed / 196.0;
  for (uint8_t i = 0; i < AVAILABLE_BOID_COUNT; i++)
  {
    boids[i] = Boid(random8(0,WIDTH), random(0, HEIGHT));
    boids[i].maxspeed = 0.380 * speedfactor + 0.380 / 2;
    boids[i].maxforce = 0.015 * speedfactor + 0.015 / 2;
  }
  predator = Boid(random8(0,WIDTH), random(0, HEIGHT) );
  predator.maxspeed = 0.385 * speedfactor + 0.385 / 2;
  predator.maxforce = 0.020 * speedfactor + 0.020 / 2;
  predator.neighbordist = 8.0;
  predator.desiredseparation = 0.0;
}

String EffectFlock::setDynCtrl(UIControl*_val) {
  if(_val->getId()==4) predatorPresent = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectFlock::setspd(const byte _spd)
{
  EffectCalc::setspd(_spd);
  speedfactor = (float)speed / 196.0;
  for (uint8_t i = 0; i < AVAILABLE_BOID_COUNT; i++)
    {
      //boids[i] = Boid(15, 15);
      boids[i].maxspeed = 0.380 * speedfactor + 0.380 / 2;
      boids[i].maxforce = 0.015 * speedfactor + 0.015 / 2;
    }
  if (predatorPresent)
    {
      predator.maxspeed = 0.385 * speedfactor + 0.385 / 2;
      predator.maxforce = 0.020 * speedfactor + 0.020 / 2;
      //predator.neighbordist = 8.0;
      //predator.desiredseparation = 0.0;
    }
}

bool EffectFlock::run(CRGB *ledarr, EffectWorker *opt){
  if (curPalette == nullptr) {
    return false;
  }
  return flockRoutine(*&ledarr, &*opt);
}

bool EffectFlock::flockRoutine(CRGB *leds, EffectWorker *param) {
  hueoffset += (speedfactor/5.0+0.1);

  fadeToBlackBy(leds, NUM_LEDS, map(speed, 1, 255, 220, 10));
  // субпиксельным эффектам блюр, что мёртвому припарка. Они сами неплохо блюрят, и функция с ними не работает толком
  //EffectMath::blur2d(15);
  //EffectMath::dimAll(map(speed, 1, 255, 1, 220));
  bool applyWind = random(0, 255) > 240;
  if (applyWind) {
    wind.x = Boid::randomf() * .015 * speedfactor + .015 / 2;
    wind.y = Boid::randomf() * .015 * speedfactor + .015 / 2;
  }

  CRGB color = ColorFromPalette(*curPalette, hueoffset, 170);
  for (uint8_t i = 0; i < AVAILABLE_BOID_COUNT; i++) {

    Boid * boid = &boids[i];
    if (predatorPresent) {
          // flee from predator
          boid->repelForce(predator.location, 8);
        }
    boid->run(boids, AVAILABLE_BOID_COUNT);
    boid->wrapAroundBorders();
    PVector location = boid->location;
    EffectMath::drawPixelXYF(location.x, location.y, color);
    if (applyWind) {
          boid->applyForce(wind);
          applyWind = false;
        }
  }
  if (predatorPresent) {
    predator.run(boids, AVAILABLE_BOID_COUNT);
    predator.wrapAroundBorders();
    color = ColorFromPalette(*curPalette, hueoffset + 128, 255);
    PVector location = predator.location;
    EffectMath::drawPixelXYF(location.x, location.y, color);
  }

  return true;
}

// ============= SWIRL /  ВОДОВОРОТ ===============
// https://gist.github.com/kriegsman/5adca44e14ad025e6d3b
// Copyright (c) 2014 Mark Kriegsman
void EffectSwirl::load(){
  palettesload();    // подгружаем дефолтные палитры
}

bool EffectSwirl::run(CRGB *ledarr, EffectWorker *opt){
  return swirlRoutine(*&ledarr, &*opt);
}

#define e_swi_BORDER (1U)  // размытие экрана за активный кадр
bool EffectSwirl::swirlRoutine(CRGB *leds, EffectWorker *param)
{
  if (curPalette == nullptr) {
    return false;
  }

  // Apply some blurring to whatever's already on the matrix
  // Note that we never actually clear the matrix, we just constantly
  // blur it repeatedly.  Since the blurring is 'lossy', there's
  // an automatic trend toward black -- by design.
#if (WIDTH < 25)
  byte blurAmount = beatsin8(2, 10, 180);
  EffectMath::blur2d(blurAmount);
#else
  // Never mind, on my 64x96 array, the dots are just too small
   EffectMath::blur2d(172);
#endif

  // Use two out-of-sync sine waves
  uint8_t i = beatsin8(27 * ((float)speed / 100.0) + 5, e_swi_BORDER, HEIGHT - e_swi_BORDER); // borderWidth
  uint8_t j = beatsin8(41 * ((float)speed / 100.0) + 5, e_swi_BORDER, WIDTH - e_swi_BORDER);
  // Also calculate some reflections
  uint8_t ni = (WIDTH - 1U)-i;
  uint8_t nj = (WIDTH - 1U)-j;

  // The color of each point shifts over time, each at a different speed.
  uint16_t ms = millis();
  EffectMath::setLed(myLamp.getPixelNumber(i, j),CRGB(EffectMath::getPixColorXY(i, j)) + ColorFromPalette(*curPalette, ms / 11));
  EffectMath::setLed(myLamp.getPixelNumber(j, i),CRGB(EffectMath::getPixColorXY(j, i)) + ColorFromPalette(*curPalette, ms / 13));
  EffectMath::setLed(myLamp.getPixelNumber(ni, nj),CRGB(EffectMath::getPixColorXY(ni, nj)) + ColorFromPalette(*curPalette, ms / 17));
  EffectMath::setLed(myLamp.getPixelNumber(nj, ni),CRGB(EffectMath::getPixColorXY(nj, ni)) + ColorFromPalette(*curPalette, ms / 29));
  EffectMath::setLed(myLamp.getPixelNumber(i, nj),CRGB(EffectMath::getPixColorXY(i, nj)) + ColorFromPalette(*curPalette, ms / 37));
  EffectMath::setLed(myLamp.getPixelNumber(ni, j),CRGB(EffectMath::getPixColorXY(ni, j)) + ColorFromPalette(*curPalette, ms / 41));

  return true;
}

// ============= DRIFT / ДРИФТ ===============
// v1.0 - Updating for GuverLamp v1.7 by SottNick 12.04.2020
// v1.1 - +dither, +phase shifting by PalPalych 12.04.2020
// https://github.com/pixelmatix/aurora/blob/master/PatternIncrementalDrift.h

void EffectDrift::load(){
  palettesload();    // подгружаем дефолтные палитры
}

String EffectDrift::setDynCtrl(UIControl*_val){
  if(_val->getId()==4) driftType = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectDrift::run(CRGB *ledarr, EffectWorker *opt){
  //EffectMath::blur2d(beatsin8(3U, 5, 10 + scale*3));
  //EffectMath::dimAll(beatsin8(2U, 246, 252));

  if (driftType == 1 or driftType == 2)
    FastLED.clear();
  else
    fadeToBlackBy(ledarr, NUM_LEDS, beatsin88(350. * EffectMath::fmap((float)speed, 1., 255., 1., 5.), 256, 4096) / 256);

// есть разница когда коэффициент как переключатель 2-20, и дробный, так скорость можно крутить плавнее
// Особенно, если коеффициент участвует в умножении
  _dri_speed = EffectMath::fmap(speed, 1., 255., 2., 20.);
  _dri_delta = beatsin8(1U);
  //EVERY_N_MILLIS(13){
    dri_phase++;    // 13 ms это примерно каждый кадр и есть
  //}

  switch (driftType)
  {
  case 1:
  case 3:
    return incrementalDriftRoutine(*&ledarr, &*opt);
    break;
  case 2:
  case 4:
    return incrementalDriftRoutine2(*&ledarr, &*opt);
    break;
  default:
    return false;
  }
}

bool EffectDrift::incrementalDriftRoutine(CRGB *leds, EffectWorker *param)
{
  if (curPalette == nullptr) {
    return false;
  }

  for (uint8_t i = 1; i < WIDTH / 2U; i++) // возможно, стоит здесь использовать const MINLENGTH
  {
    int8_t x = beatsin8((float)(CENTER_max - i) * _dri_speed, WIDTH / 2U - 1 - i, WIDTH / 2U - 1 + 1U + i, 0, 64U + dri_phase); // используем константы центра матрицы из эффекта Кометы
    int8_t y = beatsin8((float)(CENTER_max - i) * _dri_speed, WIDTH / 2U - 1 - i, WIDTH / 2U - 1 + 1U + i, 0, dri_phase);       // используем константы центра матрицы из эффекта Кометы
    //EffectMath::drawPixelXY(x, y, ColorFromPalette(*curPalette, (i - 1U) * WIDTH_steps + _dri_delta) ); // используем массив палитр из других эффектов выше
    EffectMath::wu_pixel(x*256, y*256, ColorFromPalette(*curPalette, (i - 1U) * WIDTH_steps + _dri_delta));

  }
  EffectMath::blur2d(beatsin8(3U, 5, 100));
  return true;
}

// ============= DRIFT 2 / ДРИФТ 2 ===============
// v1.0 - Updating for GuverLamp v1.7 by SottNick 12.04.2020
// v1.1 - +dither, +phase shifting by PalPalych 12.04.2020
// https://github.com/pixelmatix/aurora/blob/master/PatternIncrementalDrift2.h
bool EffectDrift::incrementalDriftRoutine2(CRGB *leds, EffectWorker *param)
{
  if (curPalette == nullptr) {
    return false;
  }

  for (uint8_t i = 0; i < WIDTH; i++)
  {
    int8_t x = 0;
    int8_t y = 0;
    CRGB color;
    if (i < WIDTH / 2U)
    {
      x = beatsin8((i + 1) * _dri_speed, i + 1U, WIDTH - 1 - i, 0, 64U + dri_phase);
      y = beatsin8((i + 1) * _dri_speed, i + 1U, HEIGHT - 1 - i, 0, dri_phase);
      color = ColorFromPalette(*curPalette, i * WIDTH_steps * 2U + _dri_delta);
    }
    else
    {
      x = beatsin8((WIDTH - i) * _dri_speed, WIDTH - 1 - i, i + 1U, 0, dri_phase);
      y = beatsin8((HEIGHT - i) * _dri_speed, HEIGHT - 1 - i, i + 1U, 0, 64U + dri_phase);
      color = ColorFromPalette(*curPalette, ~(i * WIDTH_steps + _dri_delta)); //i * WIDTH_steps * 2U + _dri_delta
    }
    EffectMath::drawPixelXYF(x, y, color);
  }
  EffectMath::blur2d(beatsin8(3U, 5, 100));
  return true;
}

// Частотный (спектральный) анализатор
#ifdef MIC_EFFECTS
void EffectFreq::load()
{
  palettesload();    // подгружаем дефолтные палитры
  memset(peakX,0,sizeof(peakX));
  memset(x,0,sizeof(x));
}

String EffectFreq::setDynCtrl(UIControl*_val){
  if(_val->getId()==4) type = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectFreq::run(CRGB *ledarr, EffectWorker *opt){
  myLamp.setMicAnalyseDivider(0); // отключить авто-работу микрофона, т.к. тут все анализируется отдельно, т.е. не нужно выполнять одну и ту же работу дважды
  if (dryrun(3.0))
    return false;
  return freqAnalyseRoutine(*&ledarr, &*opt);
}

bool EffectFreq::freqAnalyseRoutine(CRGB *leds, EffectWorker *param)
{
  if(isMicOn())
  { // вот этот блок медленный, особенно нагружающим будет вызов заполенния массива
    MICWORKER *mw = new MICWORKER(myLamp.getMicScale(),myLamp.getMicNoise());

    if(mw!=nullptr){
      samp_freq = mw->process(myLamp.getMicNoiseRdcLevel()); // частота семплирования
      last_min_peak = mw->getMinPeak();
      last_max_peak = mw->getMaxPeak()*2;

      EVERY_N_MILLIS(MIC_POLLRATE){
        maxVal=mw->fillSizeScaledArray(x,WIDTH/freqDiv); // массив должен передаваться на 1 ед. большего размера, т.е. для 16 полос его размер 17!!!
      }
      samp_freq = samp_freq; last_min_peak=last_min_peak; last_freq=last_freq; // давим варнинги
    }
    delete mw;
  } else {
    EVERY_N_MILLIS(random(50,300)){
      last_max_peak=random(0,128);
      maxVal=random(0,last_max_peak)/10.0;
      for(uint16_t i=0; i<(sizeof(x)/sizeof(float))-1U;i++){
        x[i] = random(0,128)/100.0;
      }
      x[sizeof(x)/sizeof(float)-1] = random(60,20000);
    }
  }

  float _scale = (maxVal==0? 0 : last_max_peak/maxVal);
  _scale = _scale * ((scale%128)/127.0);
  freqDiv = 2U-scale/128; //1...2

// #ifdef LAMP_DEBUG
// EVERY_N_SECONDS(1){
//   for(uint8_t i=0; i<WIDTH/freqDiv; i++)
//     LOG(printf_P,PSTR("%5.2f "),x[i]);
//   LOG(printf_P,PSTR("F: %8.2f SC: %5.2f\n"),x[WIDTH/freqDiv], scale);
// }
// #endif

  TProgmemRGBPalette16 const *_curPalette;
  float _ptPallete; // сколько пунктов приходится на одну палитру; 255.1 - диапазон ползунка, не включая 255, т.к. растягиваем только нужное :)
  uint8_t _pos; // позиция в массиве указателей паллитр
  uint8_t _curVal; // curVal == либо var как есть, либо getScale
  if(paletteIdx){
    _ptPallete = ptPallete;
    _pos = palettepos;
    _curVal = paletteIdx;
  } else {
    _ptPallete = ptPallete/2.0;
    _pos = (uint8_t)((float)(scale%128)/_ptPallete);
    _curVal = scale%128;
  }

  // EVERY_N_SECONDS(1){
  //   LOG(printf_P,PSTR("palettepos: %d\n"),_pos);
  // }

  _curPalette = palettes.at(_pos); // выбираем из доп. регулятора

  for(uint8_t xpos=0; xpos<WIDTH/freqDiv; xpos++){
    for(uint8_t ypos=0; ypos<HEIGHT; ypos++){
      uint32_t color = (x[xpos]*_scale*(1.0/(ypos+1)))>5?255:0;
      if(color==255 && peakX[1][xpos] < ypos){
        peakX[1][xpos]=ypos;
        peakX[0][xpos]=10;
      }
      if(ypos>(1.5*HEIGHT/2.0)){
        color=color<<16;
      } else if(ypos>(HEIGHT/2.0)){
        color=(color<<8)|(color<<16);
      } else {
        //color=color<<8;
        if(color){
          CRGB tColor;
          if(!(_curVal%(uint8_t)(_ptPallete*(_pos+1)))) // для крайней точки рандом, иначе возьмем по индексу/2
            tColor = ColorFromPalette(*_curPalette,random8(15)); // sizeof(TProgmemRGBPalette16)/sizeof(uint32_t)
          else
            tColor = ColorFromPalette(*_curPalette,constrain(ypos,0,15)); // sizeof(TProgmemRGBPalette16)/sizeof(uint32_t)
          color=((unsigned long)tColor.r<<16)+((unsigned long)tColor.g<<8)+(unsigned long)tColor.b; // извлекаем и конвертируем цвет :)
        }
      }
      if(!(type>=2)){
        color = 0;
      }
        EffectMath::setLed(myLamp.getPixelNumber(freqDiv*xpos,ypos), color);
        if(freqDiv>1)
          EffectMath::setLed(myLamp.getPixelNumber(freqDiv*xpos+1,ypos), color);
      //}
    }
  }


  bool isfall=false;
  EVERY_N_MILLISECONDS(70){
    isfall = true;
  }

  for (size_t i = 0; i < WIDTH/freqDiv; i++)
  {
    uint32_t color = 255;
    int8_t &ypos=peakX[1][i];
    if(peakX[0][i])
      peakX[0][i]--;
    if(isfall && ypos>0 && !peakX[0][i]) ypos--;

    if(ypos>(1.5*HEIGHT/2.0)){
      color=color<<16;
    } else if(ypos>(HEIGHT/2.0)){
      color=(color<<8)|(color<<16);
    } else {
      //color=color<<8;
      if(color){
          CRGB tColor;
          if(!(_curVal%(uint8_t)(_ptPallete*(_pos+1)))) // для крайней точки рандом, иначе возьмем по индексу/2
            tColor = ColorFromPalette(*_curPalette,random8(15)); // sizeof(TProgmemRGBPalette16)/sizeof(uint32_t)
          else
            tColor = ColorFromPalette(*_curPalette,constrain(ypos,0,15)); // sizeof(TProgmemRGBPalette16)/sizeof(uint32_t)
          color=((unsigned long)tColor.r<<16)+((unsigned long)tColor.g<<8)+(unsigned long)tColor.b; // извлекаем и конвертируем цвет :)
      }
    }
    if(type<=2){
      EffectMath::setLed(myLamp.getPixelNumber(freqDiv*i,ypos), color);
      if(freqDiv>1)
        EffectMath::setLed(myLamp.getPixelNumber(freqDiv*i+1,ypos), color);
    }
  }
  return true;
}
#endif

// ------------------------------ ЭФФЕКТ МЕРЦАНИЕ ----------------------
// (c) SottNick
#define TWINKLES_SPEEDS 4     // всего 4 варианта скоростей мерцания
#define TWINKLES_MULTIPLIER 24 // слишком медленно, если на самой медленной просто по единичке добавлять

void EffectTwinkles::load(){
  palettesload();    // подгружаем дефолтные палитры
  setup();
}

void EffectTwinkles::setup()
{
  //tnum = scale; // получим внутренний коэф., ptPallete-palettescale == от меньшего к большему, palettescale - от большего к меньшему
  for (uint32_t idx = 0; idx < NUM_LEDS; idx++) {
    if (random8(tnum) == 0) {                                // чем ниже tnum, тем чаще будут заполняться элементы лампы
      ledsbuff[idx].r = random8();                           // оттенок пикселя
      ledsbuff[idx].g = random8(1, TWINKLES_SPEEDS * 2 + 1); // скорость и направление (нарастает 1-4 или угасает 5-8)
      ledsbuff[idx].b = random8();                           // яркость
    }
    else
      ledsbuff[idx] = 0; // всё выкл
  }
}

void EffectTwinkles::setscl(const byte _scl)
{
  EffectCalc::setscl(_scl); // дернем базовый, где будет пересчет палитры
  tnum = map(scale, 1, 255, 63, 1); // получим внутренний коэф., ptPallete-palettescale == от меньшего к большему, palettescale - от большего к меньшему
  setup();
}

bool EffectTwinkles::run(CRGB *ledarr, EffectWorker *opt){
  return twinklesRoutine(*&ledarr, &*opt);
}

bool EffectTwinkles::twinklesRoutine(CRGB *leds, EffectWorker *param)
{
  if (curPalette == nullptr) {
    return false;
  }

  byte speedfactor = (float)TWINKLES_MULTIPLIER * (float)speed / 380.0;

  for (uint32_t idx = 0; idx < NUM_LEDS; idx++)
  {
    if (ledsbuff[idx].b == 0)
    {
      if (random8(tnum) == 0 && thue > 0)
      {                                                    // если пиксель ещё не горит, зажигаем каждый ХЗй
        ledsbuff[idx].r = random8();                       // оттенок пикселя
        ledsbuff[idx].g = random8(1, TWINKLES_SPEEDS + 1); // скорость и направление (нарастает 1-4, но не угасает 5-8)
        ledsbuff[idx].b = ledsbuff[idx].g;                 // яркость
        thue--;                                            // уменьшаем количество погасших пикселей
      }
    }
    else if (ledsbuff[idx].g <= TWINKLES_SPEEDS)
    { // если нарастание яркости
      if (ledsbuff[idx].b > 255U - ledsbuff[idx].g - speedfactor)
      { // если досигнут максимум
        ledsbuff[idx].b = 255U;
        ledsbuff[idx].g = ledsbuff[idx].g + TWINKLES_SPEEDS;
      }
      else
        ledsbuff[idx].b = ledsbuff[idx].b + ledsbuff[idx].g + speedfactor;
    }
    else
    { // если угасание яркости
      if (ledsbuff[idx].b <= ledsbuff[idx].g - TWINKLES_SPEEDS + speedfactor)
      {                      // если досигнут минимум
        ledsbuff[idx].b = 0; // всё выкл
        thue++;              // считаем количество погасших пикселей
      }
      else
        ledsbuff[idx].b = ledsbuff[idx].b - ledsbuff[idx].g + TWINKLES_SPEEDS - speedfactor;
    }
    if (ledsbuff[idx].b == 0)
      EffectMath::setLed(idx, 0U);
    else
      EffectMath::setLed(idx, ColorFromPalette(*curPalette, ledsbuff[idx].r, ledsbuff[idx].b));
    }
  EffectMath::blur2d(32); // так они не только разгороються, но и раздуваються. Красивше :)
  return true;
}

// ============= RADAR / РАДАР ===============
void EffectRadar::load(){
  palettesload();    // подгружаем дефолтные палитры
}

bool EffectRadar::run(CRGB *ledarr, EffectWorker *opt){
  return radarRoutine(*&ledarr, &*opt);
}

String EffectRadar::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) subPix = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectRadar::radarRoutine(CRGB *leds, EffectWorker *param)
{
  if (curPalette == nullptr)
  {
    return false;
  }
  if (subPix)
  {
    fadeToBlackBy(leds, NUM_LEDS, 5 + 20 * (float)speed / 255);
    for (float offset = 0.0f; offset < (float)(WIDTH - 1) / 2U; offset += 0.25f)
    {
      float x = (float)EffectMath::mapsincos8(MAP_COS, eff_theta, offset * 10, (WIDTH - 1U) * 10 - offset * 10) / 10.0f;
      float y = (float)EffectMath::mapsincos8(MAP_SIN, eff_theta, offset * 10, (WIDTH - 1U) * 10 - offset * 10) / 10.0f;
      CRGB color = ColorFromPalette(*curPalette, hue, 255 / random8(1, 16));
      EffectMath::drawPixelXYF(x, y, color);
      //hue = beatsin8(100);
    }
  }
  else
  {
    uint8_t _scale = palettescale; // диапазоны внутри палитры, влияют на степень размытия хвоста
    EffectMath::blur2d(beatsin8(5U, 3U, 10U));
    EffectMath::dimAll(255U - (0 + _scale * 1.5));

    for (uint8_t offset = 0U; offset < WIDTH / 2U - 1U; offset++)
    {
      EffectMath::setLed(myLamp.getPixelNumber(EffectMath::mapsincos8(MAP_COS, eff_theta, offset, (WIDTH - 1U) - offset),
                                           EffectMath::mapsincos8(MAP_SIN, eff_theta, offset, (WIDTH - 1U) - offset)),
                                           ColorFromPalette(*curPalette, 255U - (offset * 16U + eff_offset)));
    }
  }
  EVERY_N_MILLIS(EFFECTS_RUN_TIMER)
  {
    eff_theta += 5.5 * (speed / 255.0) + 1;
    eff_offset += 3.5 * ((255 - speed) / 255.0) + 1;
    if (subPix) {
    //  hue = random8();
    hue = millis() / 16;// eff_offset;
    }
  }
  return true;
}

// ============= WAVES /  ВОЛНЫ ===============
// Prismata Loading Animation
// v1.0 - Updating for GuverLamp v1.7 by SottNick 11.04.2020
// https://github.com/pixelmatix/aurora/blob/master/PatternWave.h
// Адаптация от (c) SottNick
void EffectWaves::load(){
  palettesload();    // подгружаем дефолтные палитры
}

bool EffectWaves::run(CRGB *ledarr, EffectWorker *opt){
  //fpsmeter();
  return wavesRoutine(*&ledarr, &*opt);
}

String EffectWaves::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectWaves::wavesRoutine(CRGB *leds, EffectWorker *param) {
  if (curPalette == nullptr) {
    return false;
  }
  float speedFactor = EffectMath::fmap(speed, 1, 255, 0.25, 1);
  EffectMath::dimAll(255 - 10 * speedFactor); // димирование зависит от скорости, чем быстрее - тем больше димировать
  EffectMath::blur2d(20); // @Palpalych советует делать размытие. вот в этом эффекте его явно не хватает... (есть сабпиксель, он сам размывает)
  
  float n = 0;
  for (float i = 0.0; i < (_scale <=4 ? WIDTH : HEIGHT); i+= 0.5) {
    n = (float)quadwave8(i * 4 + waveTheta) / (256.0 / ((float)(_scale <=4 ? WIDTH : HEIGHT) -1));
    switch (_scale) {
      case 1: // одна волна горизонтально, справа на лево 
        EffectMath::drawPixelXYF(i, n, ColorFromPalette(*curPalette, whue + i));
        break;
      case 2: // две волны горизонтально, справа на лево
        EffectMath::drawPixelXYF(i, n, ColorFromPalette(*curPalette, whue + i));
        EffectMath::drawPixelXYF(i, (float)HEIGHT - 1.0 - n, ColorFromPalette(*curPalette, whue + i));
        break;
      case 3: // одна волна горизонтально, слева на право 
        EffectMath::drawPixelXYF((float)WIDTH - 1.0 - i, n, ColorFromPalette(*curPalette, whue + i));
        break;
      case 4: // две волны горизонтально, слева на право
        EffectMath::drawPixelXYF((float)WIDTH - 1.0 - i, n, ColorFromPalette(*curPalette, whue + i));
        EffectMath::drawPixelXYF((float)WIDTH - 1.0 - i, (float)HEIGHT - 1.0 - n, ColorFromPalette(*curPalette, whue + i));
        break;
      case 5: // одна волна вертликально, сверху вниз
        EffectMath::drawPixelXYF(n, i, ColorFromPalette(*curPalette, whue + i));
        break;
      case 6: // две волны вертликально, сверху вниз
        EffectMath::drawPixelXYF(n, i, ColorFromPalette(*curPalette, whue + i));
        EffectMath::drawPixelXYF((float)WIDTH - 1.0 - n, i, ColorFromPalette(*curPalette, whue + i));
      break;
      case 7: // одна волна верликально, снизу вверх
        EffectMath::drawPixelXYF(n, (float)HEIGHT - 1.0 - i, ColorFromPalette(*curPalette, whue + i));
        break;
      case 8: // две волны верликально, снизу вверх
        EffectMath::drawPixelXYF(n, (float)HEIGHT - 1.0 - i, ColorFromPalette(*curPalette, whue + i));
        EffectMath::drawPixelXYF((float)WIDTH - 1.0 - n, (float)HEIGHT - 1.0 - i, ColorFromPalette(*curPalette, whue + i));
      break;
    }
  }
  waveTheta += 5.0 * speedFactor;
  whue += 2.5 * speedFactor;

  return true;
}

// ============= FIRE 2012 /  ОГОНЬ 2012 ===============
// based on FastLED example Fire2012WithPalette: https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino
// v1.0 - Updating for GuverLamp v1.7 by SottNick 17.04.2020
/*
 * Эффект "Огонь 2012"
 */
void EffectFire2012::load(){
  // собираем свой набор палитр для эффекта
  palettes.reserve(NUMPALETTES);
  palettes.push_back(&PotassiumFireColors_p);
  palettes.push_back(&SodiumFireColors_p);
  palettes.push_back(&LithiumFireColors_p);
  palettes.push_back(&RubidiumFireColors_p);
  palettes.push_back(&NormalFire_p);
  palettes.push_back(&HeatColors2_p);
  palettes.push_back(&WoodFireColors_p);
  palettes.push_back(&CopperFireColors_p);
  palettes.push_back(&AlcoholFireColors_p);
  palettes.push_back(&WhiteBlackColors_p);

  usepalettes = true; // активируем "переключатель" палитр
  scale2pallete();    // выбираем палитру согласно "шкале"

  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(millis());
}

String EffectFire2012::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectFire2012::run(CRGB *ledarr, EffectWorker *opt){
  if (dryrun(4.0))
    return false;
#ifdef MIC_EFFECTS
  cooling = isMicOn() ? 255 - myLamp.getMicMapMaxPeak() : 130;
#endif
  return fire2012Routine(*&ledarr, &*opt);
}

bool EffectFire2012::fire2012Routine(CRGB *leds, EffectWorker *opt)
{
  if (curPalette == nullptr) {
    return false;
  }
  sparking = 64 + _scale;

#if HEIGHT / 6 > 6
  #define FIRE_BASE 6
#else
  #define FIRE_BASE HEIGHT / 6 + 1
#endif
  //fadeToBlackBy(leds, NUM_LEDS, 5);
  // Loop for each column individually
  for (uint8_t x = 0; x < WIDTH; x++)
  {
    // Step 1.  Cool down every cell a little
    for (uint8_t i = 0; i < HEIGHT; i++)
    {
      noise3d[0][x][i] = qsub8(noise3d[0][x][i], random(0, ((cooling * 10) / HEIGHT) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (uint8_t k = HEIGHT; k > 1; k--)
    {
      noise3d[0][x][wrapY(k)] = (noise3d[0][x][k - 1] + noise3d[0][x][wrapY(k - 2)] + noise3d[0][x][wrapY(k - 2)]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random(255) < sparking)
    {
      int j = random(FIRE_BASE);
      noise3d[0][x][j] = qadd8(noise3d[0][x][j], random(96, 255)); // 196, 255
    }

    // Step 4.  Map from heat cells to LED colors
    for (uint8_t y = 0; y < HEIGHT; y++)
    {
      // Blend new data with previous frame. Average data between neighbouring pixels
      //if(curPalette!=palettes.at(0)){
        //nblend(myLamp.getUnsafeLedsArray()[myLamp.getPixelNumber(x, y)], ColorFromPalette(HeatColors2_p, ((noise3d[0][x][y] * 0.7) + (noise3d[0][wrapX(x + 1)][y] * 0.3))), fireSmoothing);
     // }
      nblend(myLamp.getUnsafeLedsArray()[myLamp.getPixelNumber(x, y)], ColorFromPalette(*curPalette, ((noise3d[0][x][y] * 0.7) + (noise3d[0][wrapX(x + 1)][y] * 0.3))), fireSmoothing);
    }
  }
  return true;
}

// ============= FIRE 2018 /  ОГОНЬ 2018 ===============
// v1.0 - Updating for GuverLamp v1.7 by SottNick 17.04.2020
// https://gist.github.com/StefanPetrick/819e873492f344ebebac5bcd2fdd8aa8
// https://gist.github.com/StefanPetrick/1ba4584e534ba99ca259c1103754e4c5
bool EffectFire2018::run(CRGB *ledarr, EffectWorker *opt){
  return fire2018Routine(*&ledarr, &*opt);
}

String EffectFire2018::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) isLinSpeed = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectFire2018::fire2018Routine(CRGB *leds, EffectWorker *param)
{
  // some changing values
  uint16_t ctrl1 = inoise16(11 * millis(), 0, 0);
  uint16_t ctrl2 = inoise16(13 * millis(), 100000, 100000);
  uint16_t ctrl = ((ctrl1 + ctrl2) / 2);

  // parameters for the heatmap
#ifndef MIC_EFFECTS
  uint16_t _speed = isLinSpeed ? map(speed, 1, 255, 1, 255) : beatsin88(map(speed, 1, 255, 80, 200), 5, map(speed, 1, 255, 10, 255));     // speed пересекается с переменной в родительском классе
#else
  byte mic_p = myLamp.getMicMapMaxPeak();
  uint16_t _speed = isMicOn() ? (mic_p > map(speed, 1, 255, 225, 20) ? mic_p : 20) : (isLinSpeed ? map(speed, 1, 255, 20, 100) : beatsin88(map(speed, 1, 255, 80, 200), 5, map(speed, 1, 255, 10, 255)));     // speed пересекается с переменной в родительском классе
#endif

  // EVERY_N_SECONDS(3){
  //   LOG(printf_P, PSTR("%d %d\n"), _speed, ctrl);
  // }

  noise32_x[0] = 3 * ctrl * _speed;
  noise32_y[0] = 20 * millis() * _speed;
  noise32_z[0] = 5 * millis() * _speed;
  scale32_x[0] = ctrl1 / 2;
  scale32_y[0] = ctrl2 / 2;

  //calculate the noise data
  uint8_t layer = 0;

  for (uint8_t i = 0; i < WIDTH; i++)
  {
    uint32_t ioffset = scale32_x[layer] * (i - CentreX);
    for (uint8_t j = 0; j < HEIGHT; j++)
    {
      uint32_t joffset = scale32_y[layer] * (j - CentreY);
      uint16_t data = ((inoise16(noise32_x[layer] + ioffset, noise32_y[layer] + joffset, noise32_z[layer])) + 1);
      noise3dx[layer][i][j] = data >> 8;
    }
  }

  // parameters for te brightness mask
  _speed = _speed - _speed/4;
  noise32_x[1] = 3 * ctrl * _speed;
  noise32_y[1] = 20 * millis() * _speed;
  noise32_z[1] = 5 * millis() * _speed;
  scale32_x[1] = ctrl1 / 2;
  scale32_y[1] = ctrl2 / 2;

  //calculate the noise data
  layer = 1;
  for (uint8_t i = 0; i < WIDTH; i++)
  {
    uint32_t ioffset = scale32_x[layer] * (i - CentreX);
    for (uint8_t j = 0; j < HEIGHT; j++)
    {
      uint32_t joffset = scale32_y[layer] * (j - CentreY);
      uint16_t data = ((inoise16(noise32_x[layer] + ioffset, noise32_y[layer] + joffset, noise32_z[layer])) + 1);
      noise3dx[layer][i][j] = data >> 8;
    }
  }

  // draw lowest line - seed the fire
  for (uint8_t x = 0; x < WIDTH; x++)
  {
    fire18heat[myLamp.getPixelNumber(x, HEIGHT - 1)] = noise3dx[0][WIDTH - 1 - x][CentreY - 1]; // хз, почему взято с середины. вожможно, нужно просто с 7 строки вне зависимости от высоты матрицы
  }

  //copy everything one line up
  for (uint8_t y = 0; y < HEIGHT - 1; y++)
  {
    for (uint8_t x = 0; x < WIDTH; x++)
    {
      fire18heat[myLamp.getPixelNumber(x, y)] = fire18heat[myLamp.getPixelNumber(x, y + 1)];
    }
  }

  //dim
  for (uint8_t y = 0; y < HEIGHT - 1; y++)
  {
    for (uint8_t x = 0; x < WIDTH; x++)
    {
      uint8_t dim = noise3dx[0][x][y];
      // high value = high flames
      //dim = dim / 1.7 * constrain(0.05*myLamp.effects.getBrightness()+0.01,0.01,1.0); //точно нужен прямой доступ?
      dim = dim / 1.7 * constrain(0.05*brightness+0.01,0.01,1.0);
      dim = 255 - dim;
      fire18heat[myLamp.getPixelNumber(x, y)] = scale8(fire18heat[myLamp.getPixelNumber(x, y)], dim);
    }
  }

  for (uint8_t y = 0; y < HEIGHT; y++)
  {
    for (uint8_t x = 0; x < WIDTH; x++)
    {
      // map the colors based on heatmap
      //EffectMath::setLed(myLamp.getPixelNumber(x, HEIGHT - 1 - y), CRGB( fire18heat[myLamp.getPixelNumber(x, y)], 1 , 0));
      //EffectMath::setLed(myLamp.getPixelNumber(x, HEIGHT - 1 - y), CRGB( fire18heat[myLamp.getPixelNumber(x, y)], fire18heat[myLamp.getPixelNumber(x, y)] * 0.153, 0));
      //EffectMath::setLed(myLamp.getPixelNumber(x, HEIGHT - 1 - y), CHSV(Scale, 255-fire18heat[myLamp.getPixelNumber(x, y)], constrain(fire18heat[myLamp.getPixelNumber(x, y)]*10,1,255)));
      //nblend(myLamp.getUnsafeLedsArray()[myLamp.getPixelNumber(x, HEIGHT - 1 - y)], CRGB( fire18heat[myLamp.getPixelNumber(x, y)], fire18heat[myLamp.getPixelNumber(x, y)] * 0.153, 0), 200);
      CRGB color = CRGB(fire18heat[myLamp.getPixelNumber(x, y)], (float)fire18heat[myLamp.getPixelNumber(x, y)] * (scale/5.0) * 0.01, 0); color*=2.5;
      EffectMath::setLed(myLamp.getPixelNumber(x, HEIGHT - 1 - y), color);

      // dim the result based on 2nd noise layer
      myLamp.getUnsafeLedsArray()[myLamp.getPixelNumber(x, HEIGHT - 1 - y)].nscale8(noise3dx[1][x][y]);
    }
  }
  return true;
}

// ------------------------------ ЭФФЕКТ КОЛЬЦА / КОДОВЫЙ ЗАМОК ----------------------
// (c) SottNick
// из-за повторного использоваия переменных от других эффектов теперь в этом коде невозможно что-то понять.
// поэтому для понимания придётся сперва заменить названия переменных на человеческие. но всё равно это песец, конечно.

//uint8_t ringColor[HEIGHT]; // начальный оттенок каждого кольца (оттенка из палитры) 0-255
//uint8_t huePos[HEIGHT]; // местоположение начального оттенка кольца 0-WIDTH-1
//uint8_t shiftHueDir[HEIGHT]; // 4 бита на ringHueShift, 4 на ringHueShift2
////ringHueShift[ringsCount]; // шаг градиета оттенка внутри кольца -8 - +8 случайное число
////ringHueShift2[ringsCount]; // обычная скорость переливания оттенка всего кольца -8 - +8 случайное число
//uint8_t currentRing; // кольцо, которое в настоящий момент нужно провернуть
//uint8_t stepCount; // оставшееся количество шагов, на которое нужно провернуть активное кольцо - случайное от WIDTH/5 до WIDTH-3
bool EffectRingsLock::run(CRGB *ledarr, EffectWorker *opt){
  if (dryrun(3.0))
    return false;
  return ringsRoutine(*&ledarr, &*opt);
}

void EffectRingsLock::load(){
  palettesload();
}

String EffectRingsLock::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) { ringWidth = EffectCalc::setDynCtrl(_val).toInt(); ringsSet(); }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

// Установка параметров колец
void EffectRingsLock::ringsSet(){
  if (curPalette == nullptr) {
    return;
  }
  //FastLED.clear();

  ringNb = (float)HEIGHT / ringWidth + ((HEIGHT % ringWidth == 0U) ? 0U : 1U)%HEIGHT; // количество колец
  upRingHue = ringWidth - (ringWidth * ringNb - HEIGHT) / 2U; // толщина верхнего кольца. может быть меньше нижнего
  downRingHue = HEIGHT - upRingHue - (ringNb - 2U) * ringWidth; // толщина нижнего кольца = всё оставшееся

  for (uint8_t i = 0; i < ringNb; i++)
  {
    if (!i) ringColor[i] = 0; //random(255U - WIDTH / 4U);
    ringColor[i] = ringColor[i - 1] + 64; // начальный оттенок кольца из палитры 0-255 за минусом длины кольца, делённой пополам
    shiftHueDir[i] = random8();
    huePos[i] = random8(); 
    stepCount = 0U;
    currentRing = random(ringNb);
  }
}

bool EffectRingsLock::ringsRoutine(CRGB *leds, EffectWorker *param)
{
  uint8_t h, x, y;
  FastLED.clear();

  for (uint8_t i = 0; i < ringNb; i++)
  {
    if (i != currentRing) // если это не активное кольцо
    {
       h = shiftHueDir[i] & 0x0F; // сдвигаем оттенок внутри кольца
       if (h > 8U)
         //ringColor[i] += (uint8_t)(7U - h); // с такой скоростью сдвиг оттенка от вращения кольца не отличается
         ringColor[i]--;
       else
         //ringColor[i] += h;
         ringColor[i]++;
    } else {
      if (stepCount == 0) { // если сдвиг активного кольца завершён, выбираем следующее
        currentRing = random(ringNb);
        do {
          stepCount = WIDTH - 3U - random8((WIDTH - 3U) * 2U); // проворот кольца от хз до хз
        } while (stepCount < WIDTH / 5U || stepCount > 255U - WIDTH / 5U);
      } else {
        if (stepCount > 127U)
          {
            stepCount++;
            huePos[i] = (huePos[i] + 1U) % WIDTH;
          }
        else
          {
            stepCount--;
            huePos[i] = (huePos[i] - 1U + WIDTH) % WIDTH;
          }
      }
    }

    // отрисовываем кольца
    h = (shiftHueDir[i] >> 4) & 0x0F; // берём шаг для градиента вутри кольца
    if (h > 8U)
      h = 7U - h;
    for (uint8_t j = 0U; j < ((i == 0U) ? downRingHue : ((i == ringNb - 1U) ? upRingHue : ringWidth)); j++) // от 0 до (толщина кольца - 1)
    {
      y = i * ringWidth + j - ((i == 0U) ? 0U : ringWidth - downRingHue);
      for (uint8_t k = 0; k < WIDTH / 2U - 1; k++) // полукольцо
        {
          x = (huePos[i] + k) % WIDTH; // первая половина кольца
          EffectMath::setLed(myLamp.getPixelNumber(x, y), ColorFromPalette(*curPalette, ringColor[i]/* + k * h*/));
          x = (WIDTH - 1 + huePos[i] - k) % WIDTH; // вторая половина кольца (зеркальная первой)
          EffectMath::setLed(myLamp.getPixelNumber(x, y), ColorFromPalette(*curPalette, ringColor[i] + k * h));
        }
      if (WIDTH & 0x01) //(WIDTH % 2U > 0U) // если число пикселей по ширине матрицы нечётное, тогда не забываем и про среднее значение
      {
        x = (huePos[i] + WIDTH / 2U) % WIDTH;
        EffectMath::setLed(myLamp.getPixelNumber(x, y), ColorFromPalette(*curPalette, ringColor[i] + WIDTH / 2U * h));
      }
    }
  }
  return true;
}

// ------------------------------ ЭФФЕКТ КУБИК 2D ----------------------
// Classic (c) SottNick
// New (c) Vortigont
#define PAUSE_MAX 7

void EffectCube2d::swapBuff() {
  for (uint8_t x = 0; x < WIDTH; x++) { // переписываем виртуальный экран в настоящий
    for(uint8_t y = 0; y < HEIGHT; y++) {
      EffectMath::setLed(myLamp.getPixelNumber(x, y), ledbuff[myLamp.getPixelNumberBuff(x, y, fieldX, fieldY)]);
    }
  }
}

bool EffectCube2d::run(CRGB *ledarr, EffectWorker *opt){
  if (dryrun(classic ? 4. : 3., classic ? 3 : EFFECTS_RUN_TIMER))
    return false;
  if (classic)
    return cube2dClassicRoutine(*&ledarr, &*opt);
  else
    return cube2dRoutine(*&ledarr, &*opt);
}

String EffectCube2d::setDynCtrl(UIControl*_val)
{
  if(_val->getId()==4) sizeX = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) sizeY = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==6) classic = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  cubesize();
  return String();
}

void EffectCube2d::setscl(const byte _scl)
{
  EffectCalc::setscl(_scl); // сначала дергаем базовый, чтобы была обработка палитр/микрофона (если такая обработка точно не нужна, то можно не вызывать базовый метод)
  cubesize();
}

void EffectCube2d::load(){
  palettesload();    // подгружаем дефолтные палитры
  cubesize();
}

// задаем размерность кубов
void EffectCube2d::cubesize() {
  if (curPalette == nullptr) {
    return;
  }

  FastLED.clear();

  cntY = ceil((float)HEIGHT / (float)(sizeY + 1U));
	fieldY = (sizeY + 1U) * cntY;

  cntX = ceil((float)WIDTH / (float)(sizeX + 1U));
	fieldX = (sizeX + 1U) * cntX;

  ledbuff.resize(fieldX * fieldY); // создаем виртуальную матрицу, размером кратную размеру кубика+1
  for(uint16_t i = 0; i < (fieldX * fieldY); i++) // очищаем виртуальную матрицу. (может кто знает способ попроще?)
    ledbuff[i] = CRGB(0, 0, 0);

  //LOG(printf_P, PSTR("CUBE2D Size: scX=%d, scY=%d, scaleY=%d, cntX=%d, cntY=%d\n"), cubeScaleX, cubeScaleY, scaleY, cntX, cntY);
  uint8_t x=0, y = 0;
  CRGB color;

  for (uint8_t j = 0U; j < cntY; j++)
  {
    y = j * (sizeY + 1U);
    for (uint8_t i = 0U; i < cntX; i++)
    {
      x = i * (sizeX + 1U);
      if (scale == FASTLED_PALETTS_COUNT + 1U)
        color = CHSV(46U, 0U, 32U + random8(256U-32U));
      else

      while (1) // немного потенциально опасно, но у нас, если палитры не подгружены, - return, думаю это сводит опасность практически к нулю, иначе сработает вачдог
        // не вижу другого способа перестать получать почти черные кубики, это раздражает, впечатление будто лампе глаз выбили, или зуб :))
      {

        color = scale > 0 ? ColorFromPalette(*curPalette, random(1024)>>1, random8(128, 255)) : CRGB(random8(), random8(), random8());
        if (color >= CRGB(10,10,10)) break;  // Не хотелось бы получать слишком тёмные кубики
      }

      for (uint8_t k = 0U; k < sizeY; k++){
        for (uint8_t m = 0U; m < sizeX; m++){
          //EffectMath::setLed(myLamp.getPixelNumber(x+m, y+k), color);
          ledbuff[myLamp.getPixelNumberBuff(x+m, y+k, fieldX, fieldY)] = color;
        }
      }
    }
  }
  if (classic) {
    currentStep = 4U; // текущий шаг сдвига первоначально с перебором (от 0 до shiftSteps-1)
    shiftSteps = 4U; // всего шагов сдвига (от 3 до 4)
    pauseSteps = 0U; // осталось шагов паузы
    seamlessX = (fieldX == WIDTH);
    globalShiftX = 0;
    globalShiftY = 0;
    gX = 0;
    gY = 0;
  } else {
    pauseSteps = CUBE2D_PAUSE_FRAMES; // осталось шагов паузы
    shiftSteps = 0;
  }
}

bool EffectCube2d::cube2dRoutine(CRGB *leds, EffectWorker *param)
{
  if (curPalette == nullptr) {
    return false;
  }

  if (!pauseSteps){
    pauseSteps--;
    return false; // пропускаем кадры после прокрутки кубика (делаем паузу)
  }

  if (!shiftSteps) {  // если цикл вращения завершён
    // ====== определяем направление прокруток на будущий цикл
    pauseSteps = CUBE2D_PAUSE_FRAMES;
    direction = random8()%2;  // сдвиг 0 - строки, 1 - столбцы
    moveItems.resize(direction ? cntX : cntY, 0);

    for (uint8_t i=0; i<(direction ? cntX : cntY); i++){
      moveItems.at(i)= random8()%3; // 1 - fwd, 0 - bkw, 2 - none
    }

    shiftSteps = ((direction ? sizeY : sizeX)+1) * random8(direction ? cntY : cntX);  // такой рандом добавляет случайную задержку в паузу, попадая на "0"
    return false;
  }


  //двигаем, что получилось...
  shiftSteps--;

  for (uint8_t i=0; i<(direction ? cntX : cntY); i++){
    if (moveItems.at(i)==2) // skip some items
      continue;

    direction ? cube2dmoveCols(i, moveItems.at(i)) : cube2dmoveRows(i, moveItems.at(i));
  }

  swapBuff();
  return true;
}

// идём по горизонтали, крутим по вертикали (столбцы двигаются)
void EffectCube2d::cube2dmoveCols(uint8_t moveItem, bool movedirection){
      uint16_t x, anim0;
  CRGB color, color2;

      x = moveItem * (sizeX + 1U);
      anim0 = 0;

      // если крутим столбец вниз
      if (!movedirection)
        {
          //color = EffectMath::getPixColorXY(x, anim0);                                   // берём цвет от нижней строчки
          color = ledbuff[myLamp.getPixelNumberBuff(x, anim0, fieldX, fieldY)];
          for (uint8_t k = anim0; k < anim0+fieldY-1; k++)
          {
            //color2 = EffectMath::getPixColorXY(x,k+1);                                   // берём цвет от строчки над нашей
            color2 = ledbuff[myLamp.getPixelNumberBuff(x, k+1, fieldX, fieldY)];
            for (uint8_t m = x; m < x + sizeX; m++)
              //EffectMath::setLed(myLamp.getPixelNumber(m, k), color2);                           // копируем его на всю нашу строку
              ledbuff[myLamp.getPixelNumberBuff(m, k, fieldX, fieldY)] = color2;
          }
          for   (uint8_t m = x; m < x + sizeX; m++)
            //EffectMath::setLed(myLamp.getPixelNumber(m, anim0+fieldY-1), color);                  // цвет нижней строчки копируем на всю верхнюю
            ledbuff[myLamp.getPixelNumberBuff(m, anim0+fieldY-1, fieldX, fieldY)] = color;
          return;
        }

      // крутим столбец вверх
      //color = EffectMath::getPixColorXY(x,anim0+fieldY-1);                            // берём цвет от верхней строчки
      color = ledbuff[myLamp.getPixelNumberBuff(x,anim0+fieldY-1, fieldX, fieldY)];
      for (uint8_t k = anim0+fieldY-1; k > anim0 ; k--)
      {
        //color2 = EffectMath::getPixColorXY(x,k-1);                                   // берём цвет от строчки под нашей
        color2 = ledbuff[myLamp.getPixelNumberBuff(x, k-1, fieldX, fieldY)];
        for (uint8_t m = x; m < x + sizeX; m++)
          //EffectMath::setLed(myLamp.getPixelNumber(m,k), color2);                           // копируем его на всю нашу строку
          ledbuff[myLamp.getPixelNumberBuff(m, k , fieldX, fieldY)] = color2;
      }
      for   (uint8_t m = x; m < x + sizeX; m++)
        //EffectMath::setLed(myLamp.getPixelNumber(m, anim0), color);                         // цвет верхней строчки копируем на всю нижнюю
        ledbuff[myLamp.getPixelNumberBuff(m, anim0, fieldX, fieldY)] = color;
}

// идём по вертикали, крутим по горизонтали (строки двигаются)
void EffectCube2d::cube2dmoveRows(uint8_t moveItem, bool movedirection){
  uint16_t y, anim0;
  CRGB color, color2;

  y = moveItem * (sizeY + 1U);
  anim0 = 0;

  // крутим строку влево
  if (!movedirection)
  {
    //color = EffectMath::getPixColorXY(anim0, y);                            // берём цвет от левой колонки (левого пикселя)
    color = ledbuff[myLamp.getPixelNumberBuff(anim0, y, fieldX, fieldY)];
    for (uint8_t k = anim0; k < anim0+fieldX-1; k++)
    {
      //color2 = EffectMath::getPixColorXY(k+1, y);                           // берём цвет от колонки (пикселя) правее
      color2 = ledbuff[myLamp.getPixelNumberBuff(k+1, y, fieldX, fieldY)];
      for (uint8_t m = y; m < y + sizeY; m++)
        //EffectMath::setLed(myLamp.getPixelNumber(k, m), color2);                           // копируем его на всю нашу колонку
        ledbuff[myLamp.getPixelNumberBuff(k, m, fieldX, fieldY)] = color2;
    }
    for   (uint8_t m = y; m < y + sizeY; m++)
      //EffectMath::setLed(myLamp.getPixelNumber(anim0+fieldX-1, m), color);                  // цвет левой колонки копируем на всю правую
      ledbuff[myLamp.getPixelNumberBuff(anim0+fieldX-1, m, fieldX, fieldY)] = color;
   return;
  }

  //  крутим строку вправо
  //color = EffectMath::getPixColorXY(anim0+fieldX-1, y);                    // берём цвет от правой колонки
  color = ledbuff[myLamp.getPixelNumberBuff(anim0+fieldX-1, y, fieldX, fieldY)];
  for (uint8_t k = anim0+fieldX-1; k > anim0 ; k--)
  {
    //color2 = EffectMath::getPixColorXY(k-1, y);                           // берём цвет от колонки левее
    color2 = ledbuff[myLamp.getPixelNumberBuff(k-1, y, fieldX, fieldY)];
    for (uint8_t m = y; m < y + sizeY; m++)
      //EffectMath::setLed(myLamp.getPixelNumber(k, m), color2);                           // копируем его на всю нашу колонку
      ledbuff[myLamp.getPixelNumberBuff(k, m, fieldX, fieldY)] = color2;
  }
  for   (uint8_t m = y; m < y + sizeY; m++)
    //EffectMath::setLed(myLamp.getPixelNumber(anim0, m), color);                          // цвет правой колонки копируем на всю левую
    ledbuff[myLamp.getPixelNumberBuff(anim0, m, fieldX, fieldY)] = color;
}

bool EffectCube2d::cube2dClassicRoutine(CRGB *leds, EffectWorker *param)
{
  CRGB color, color2;
  int8_t shift, shiftAll;
  uint8_t anim0, x, y;

  //двигаем, что получилось...
  if (pauseSteps == 0 && currentStep < shiftSteps) // если пауза закончилась, а цикл вращения ещё не завершён
  {
    currentStep++;
    if (direction)
    {
      for (uint8_t i = 0U; i < cntX; i++)
      {
        x = (gX + i * (sizeX + 1U)) % WIDTH;
        if (storage[i][0] > 0) // в нулевой ячейке храним оставшееся количество ходов прокрутки
        {
          storage[i][0]--;
          shift = storage[i][1] - 1; // в первой ячейке храним направление прокрутки

          if (globalShiftY == 0)
            anim0 = (gY == 0U) ? 0U : gY - 1U;
          else if (globalShiftY > 0)
            anim0 = gY;
          else
            anim0 = gY - 1U;

          if (shift < 0) // если крутим столбец вниз
          {
            //color = myLamp.getPixColorXY(x, anim0); // берём цвет от нижней строчки
            color = ledbuff[myLamp.getPixelNumberBuff(x, anim0, fieldX, fieldY)];
            for (uint8_t k = anim0; k < anim0 + fieldY - 1; k++)
            {
              //color2 = myLamp.getPixColorXY(x, k + 1); // берём цвет от строчки над нашей
              color2 = ledbuff[myLamp.getPixelNumberBuff(x, k + 1, fieldX, fieldY)];
              for (uint8_t m = x; m < x + sizeX; m++)
                ledbuff[myLamp.getPixelNumberBuff(m % fieldX, k, fieldX, fieldY)] = color2;
                //myLamp.setLeds(myLamp.getPixelNumber(m % WIDTH, k), color2); // копируем его на всю нашу строку
            }
            for (uint8_t m = x; m < x + sizeX; m++)
              //myLamp.setLeds(myLamp.getPixelNumber(m % WIDTH, anim0 + fieldY - 1), color); // цвет нижней строчки копируем на всю верхнюю
              ledbuff[myLamp.getPixelNumberBuff(m % fieldX, anim0 + fieldY - 1, fieldX, fieldY)] = color;
          }
          else if (shift > 0) // если крутим столбец вверх
          {
            //color = myLamp.getPixColorXY(x, anim0 + fieldY - 1); // берём цвет от верхней строчки
            color = ledbuff[myLamp.getPixelNumberBuff(x, anim0 + fieldY - 1, fieldX, fieldY)];
            for (uint8_t k = anim0 + fieldY - 1; k > anim0; k--)
            {
              //color2 = myLamp.getPixColorXY(x, k - 1); // берём цвет от строчки под нашей
              color2 = ledbuff[myLamp.getPixelNumberBuff(x, k - 1, fieldX, fieldY)];
              for (uint8_t m = x; m < x + sizeX; m++)
                ledbuff[myLamp.getPixelNumberBuff(m % fieldX, k, fieldX, fieldY)] = color2;
                //myLamp.setLeds(myLamp.getPixelNumber(m % WIDTH, k), color2); // копируем его на всю нашу строку
            }
            for (uint8_t m = x; m < x + sizeX; m++)
              //myLamp.setLeds(myLamp.getPixelNumber(m % WIDTH, anim0), color); // цвет верхней строчки копируем на всю нижнюю
              ledbuff[myLamp.getPixelNumberBuff(m % fieldX, anim0, fieldX, fieldY)] = color;
          }
        }
      }
    }
    else
    {
      for (uint8_t j = 0U; j < cntY; j++)
      {
        y = gY + j * (sizeY + 1U);
        if (storage[0][j] > 0) // в нулевой ячейке храним оставшееся количество ходов прокрутки
        {
          storage[0][j]--;
          shift = storage[1][j] - 1; // в первой ячейке храним направление прокрутки

          if (seamlessX)
            anim0 = 0U;
          else if (globalShiftX == 0)
            anim0 = (gX == 0U) ? 0U : gX - 1U;
          else if (globalShiftX > 0)
            anim0 = gX;
          else
            anim0 = gX - 1U;

          if (shift < 0) // если крутим строку влево
          {
            //color = myLamp.getPixColorXY(anim0, y); // берём цвет от левой колонки (левого пикселя)
            color = ledbuff[myLamp.getPixelNumberBuff(anim0, y, fieldX, fieldY)];
            for (uint8_t k = anim0; k < anim0 + fieldX - 1; k++)
            {
              //color2 = myLamp.getPixColorXY(k + 1, y); // берём цвет от колонки (пикселя) правее
              color2 = ledbuff[myLamp.getPixelNumberBuff(k + 1, y, fieldX, fieldY)];
              for (uint8_t m = y; m < y + sizeY; m++)
                //myLamp.setLeds(myLamp.getPixelNumber(k, m), color2); // копируем его на всю нашу колонку
                ledbuff[myLamp.getPixelNumberBuff(k, m, fieldX, fieldY)] = color2;
            }
            for (uint8_t m = y; m < y + sizeY; m++)
              //myLamp.setLeds(myLamp.getPixelNumber(anim0 + fieldX - 1, m), color); // цвет левой колонки копируем на всю правую
              ledbuff[myLamp.getPixelNumberBuff(anim0 + fieldX - 1, m, fieldX, fieldY)] = color;
          }
          else if (shift > 0) // если крутим столбец вверх
          {
            //color = myLamp.getPixColorXY(anim0 + fieldX - 1, y); // берём цвет от правой колонки
            color = ledbuff[myLamp.getPixelNumberBuff(anim0 + fieldX - 1, y, fieldX, fieldY)];
            for (uint8_t k = anim0 + fieldX - 1; k > anim0; k--)
            {
              //color2 = myLamp.getPixColorXY(k - 1, y); // берём цвет от колонки левее
              color2 = ledbuff[myLamp.getPixelNumberBuff(k - 1, y, fieldX, fieldY)];
              for (uint8_t m = y; m < y + sizeY; m++)
                //myLamp.setLeds(myLamp.getPixelNumber(k, m), color2); // копируем его на всю нашу колонку
                ledbuff[myLamp.getPixelNumberBuff(k, m, fieldX, fieldY)] = color2;
            }
            for (uint8_t m = y; m < y + sizeY; m++)
              //myLamp.setLeds(myLamp.getPixelNumber(anim0, m), color); // цвет правой колонки копируем на всю левую
              ledbuff[myLamp.getPixelNumberBuff(anim0, m, fieldX, fieldY)] = color;
          }
        }
      }
    }
  }
  else if (pauseSteps != 0U) // пропускаем кадры после прокрутки кубика (делаем паузу)
    pauseSteps--;

  if (currentStep >= shiftSteps) // если цикл вращения завершён, меняем местами соотвествующие ячейки (цвет в них) и точку первой ячейки
  {
    currentStep = 0U;
    pauseSteps = PAUSE_MAX;
    //если часть ячеек двигалась на 1 пиксель, пододвигаем глобальные координаты начала
    gY = gY + globalShiftY; //+= globalShiftY;
    globalShiftY = 0;
    //gX += globalShiftX; для бесшовной не годится
    gX = (WIDTH + gX + globalShiftX) % WIDTH;
    globalShiftX = 0;

    //пришла пора выбрать следующие параметры вращения
    shiftAll = 0;
    direction = random8(2U);
    if (direction) // идём по горизонтали, крутим по вертикали (столбцы двигаются)
    {
      for (uint8_t i = 0U; i < cntX; i++)
      {
        storage[i][1] = random8(3);
        shift = storage[i][1] - 1; // в первой ячейке храним направление прокрутки
        if (shiftAll == 0)
          shiftAll = shift;
        else if (shift != 0 && shiftAll != shift)
          shiftAll = 50;
      }
      shiftSteps = sizeY + ((gY - shiftAll >= 0 && gY - shiftAll + fieldY < (int)HEIGHT) ? random8(2U) : 1U);

      if (shiftSteps == sizeY) // значит полюбому shiftAll было = (-1, 0, +1) - и для нуля в том числе мы двигаем весь куб на 1 пиксель
      {
        globalShiftY = 1 - shiftAll; //временно на единичку больше, чем надо
        for (uint8_t i = 0U; i < cntX; i++)
          if (storage[i][1] == 1U) // если ячейка никуда не планировала двигаться
          {
            storage[i][1] = globalShiftY;
            storage[i][0] = 1U; // в нулевой ячейке храним количество ходов сдвига
          }
          else
            storage[i][0] = shiftSteps; // в нулевой ячейке храним количество ходов сдвига
        globalShiftY--;
      }
      else
        for (uint8_t i = 0U; i < cntX; i++)
          if (storage[i][1] != 1U)
            storage[i][0] = shiftSteps; // в нулевой ячейке храним количество ходов сдвига
    }
    else // идём по вертикали, крутим по горизонтали (строки двигаются)
    {
      for (uint8_t j = 0U; j < cntY; j++)
      {
        storage[1][j] = random8(3);
        shift = storage[1][j] - 1; // в первой ячейке храним направление прокрутки
        if (shiftAll == 0)
          shiftAll = shift;
        else if (shift != 0 && shiftAll != shift)
          shiftAll = 50;
      }
      if (seamlessX)
        shiftSteps = sizeX + ((shiftAll < 50) ? random8(2U) : 1U);
      else
        shiftSteps = sizeX + ((gX - shiftAll >= 0 && gX - shiftAll + fieldX < (int)WIDTH) ? random8(2U) : 1U);

      if (shiftSteps == sizeX) // значит полюбому shiftAll было = (-1, 0, +1) - и для нуля в том числе мы двигаем весь куб на 1 пиксель
      {
        globalShiftX = 1 - shiftAll; //временно на единичку больше, чем надо
        for (uint8_t j = 0U; j < cntY; j++)
          if (storage[1][j] == 1U) // если ячейка никуда не планировала двигаться
          {
            storage[1][j] = globalShiftX;
            storage[0][j] = 1U; // в нулевой ячейке храним количество ходов сдвига
          }
          else
            storage[0][j] = shiftSteps; // в нулевой ячейке храним количество ходов сдвига
        globalShiftX--;
      }
      else
        for (uint8_t j = 0U; j < cntY; j++)
          if (storage[1][j] != 1U)
            storage[0][j] = shiftSteps; // в нулевой ячейке храним количество ходов сдвига
    }
  }
  swapBuff();
  return true;
}

//-------------- Эффект "Часы"
bool EffectTime::run(CRGB *ledarr, EffectWorker *opt){
  if(isDebug())
    return palleteTest(ledarr, opt);
  else {
    if((millis() - lastrun - EFFECTS_RUN_TIMER) < (unsigned)((255-speed)) && (speed==1 || speed==255)){
        EffectMath::dimAll(254);
      return true;
    } else {
      lastrun = millis();
      if (myLamp.isPrintingNow()) // если выводится бегущая строка, то эффект приостанавливаем! Специально обученный костыль, т.к. вывод статического и динамического текста одноверенно не совместимы
        return true;
    }
    return timePrintRoutine(ledarr, opt);
  }
}

void EffectTime::load(){
  FastLED.clear();
  palettesload();    // подгружаем дефолтные палитры

  if(((curTimePos<=(signed)LET_WIDTH*2-(LET_WIDTH/2)) || (curTimePos>=(signed)WIDTH+(LET_WIDTH/2))) )
  {
    curTimePos = random(LET_WIDTH*2,WIDTH);
    hColor[0] = ColorFromPalette(*curPalette, random8());
    mColor[0] = ColorFromPalette(*curPalette, random8());
  }
}

bool EffectTime::palleteTest(CRGB *leds, EffectWorker *param)
{
  FastLED.clear();
  float sf = 0.996078431372549+speed/255.; // смещение, для скорости 1 смещения не будет, т.к. суммарный коэф. == 1
  for(uint8_t y=0; y<HEIGHT; y++)
    for(uint8_t x=0; x<WIDTH; x++)
      EffectMath::drawPixelXY(WIDTH-1-x,HEIGHT-1-y,ColorFromPalette(*curPalette, (y*x*sf), 127));
  return true;
}

bool EffectTime::timePrintRoutine(CRGB *leds, EffectWorker *param)
{
  // #undef HEIGHT
  // #define HEIGHT 8
  // #undef WIDTH
  // #define WIDTH 8

  if (speed==254 || speed==1 || speed==255){
    EVERY_N_SECONDS(5){
      FastLED.clear();

      String tmp = embui.timeProcessor.getFormattedShortTime();
      if(HEIGHT>=16){
        int16_t xPos = speed!=254 ? random(LET_WIDTH*2,WIDTH) : (WIDTH+LET_WIDTH*2)/2;
        myLamp.sendStringToLamp(tmp.substring(0,2).c_str(), ColorFromPalette(*curPalette, speed!=254 ? random8() : 64), false, HEIGHT-LET_HEIGHT, xPos);
        myLamp.sendStringToLamp(tmp.substring(3,5).c_str(), ColorFromPalette(*curPalette, speed!=254 ? random8() : 127), false, (int8_t)HEIGHT-(int8_t)(LET_HEIGHT*2), xPos);
      } else if(WIDTH>=21){ // требуется минимум 5*4+1 символов
        int16_t xPos = speed!=254 ? random(LET_WIDTH*2+1,WIDTH) : (WIDTH+LET_WIDTH*2+1)/2;
        myLamp.sendStringToLamp(tmp.substring(0,2).c_str(), ColorFromPalette(*curPalette, speed!=254 ? random8() : 64), false, HEIGHT-LET_HEIGHT, xPos);
        myLamp.sendStringToLamp(tmp.substring(3,5).c_str(), ColorFromPalette(*curPalette, speed!=254 ? random8() : 127), false, HEIGHT-LET_HEIGHT, xPos-(LET_WIDTH*2+1));
      } else {
        //int16_t xPos = random(LET_WIDTH*2,WIDTH); // вывод часов/минут попеременно...
        int16_t xPos = speed!=254 ? random(LET_WIDTH*2,WIDTH) : (WIDTH+LET_WIDTH*2)/2;
        isMinute=!isMinute;
        myLamp.sendStringToLamp(isMinute?tmp.substring(3,5).c_str():tmp.substring(0,2).c_str(), ColorFromPalette(*curPalette, speed!=254 ? random8() : isMinute ? 64 : 127), false, HEIGHT-LET_HEIGHT, xPos);
      }
    }
  } else {
    //FastLED.clear();
    EVERY_N_SECONDS(5){
      isMinute=!isMinute;
    }
    EffectMath::dimAll(250-speed/3); // небольшой шлейф, чисто как визуальный эффект :)
    int16_t xPos = curTimePos;
    if((xPos<=(signed)LET_WIDTH*2-((signed)LET_WIDTH/2)) || (xPos>=(signed)WIDTH+((signed)LET_WIDTH/2))){
      if(xPos<=(signed)LET_WIDTH*2){
        timeShiftDir = false;
        xPos=LET_WIDTH*2-(LET_WIDTH/2); // будет на полсимвола выходить за пределы, так задумано :)
      } else {
        timeShiftDir = true;
        xPos=WIDTH+(LET_WIDTH/2); // будет на полсимвола выходить за пределы, так задумано :)
      }
      hColor[0] = ColorFromPalette(*curPalette, random8());
      mColor[0] = ColorFromPalette(*curPalette, random8());
    }
    String tmp = embui.timeProcessor.getFormattedShortTime();
    uint8_t shift = beatsin8(speed/5, -1, 1);
    if(HEIGHT>=16){
      myLamp.sendStringToLamp(tmp.substring(0,2).c_str(), hColor[0], false, HEIGHT-LET_HEIGHT+shift, xPos);
      myLamp.sendStringToLamp(tmp.substring(3,5).c_str(), mColor[0], false, HEIGHT-(LET_HEIGHT*2)+shift, xPos);
    } else if(WIDTH>=21){
      myLamp.sendStringToLamp(tmp.substring(0,2).c_str(), hColor[0], false, HEIGHT-LET_HEIGHT+shift, xPos+(LET_WIDTH*2+1));
      myLamp.sendStringToLamp(tmp.substring(3,5).c_str(), mColor[0], false, HEIGHT-LET_HEIGHT+shift, xPos);
    } else if(WIDTH>=10){
      myLamp.sendStringToLamp(isMinute ? tmp.substring(3,5).c_str() : tmp.substring(0,2).c_str(), hColor[0], false, HEIGHT-LET_HEIGHT+shift, xPos);
    } else {
        xPos = random(LET_WIDTH*2,WIDTH); // вывод часов/минут попеременно...
        myLamp.sendStringToLamp(isMinute ? tmp.substring(3,5).c_str() : tmp.substring(0,2).c_str(), ColorFromPalette(*curPalette, (int)color_idx%16), false, HEIGHT-LET_HEIGHT, xPos);
        color_idx=color_idx+(speed/256.0);
    }
    curTimePos=curTimePos+(0.23*(speed/255.0))*(timeShiftDir?-1:1); // смещаем
  }
  return true;

  // #undef HEIGHT
  // #define HEIGHT 16
  // #undef WIDTH
  // #define WIDTH 16
}

// ------------------------------ ЭФФЕКТ ДЫМ ----------------------
bool EffectMStreamSmoke::run(CRGB *ledarr, EffectWorker *opt){
  return multipleStreamSmokeRoutine(*&ledarr, &*opt);
}

// TODO: объединить функцию вместе с "EffectComet", сейчас лень возиться с указателями на массивы:)
void EffectMStreamSmoke::FillNoise(int8_t layer) {
  const uint8_t e_centerX =  (WIDTH / 2) - 1;
  const uint8_t e_centerY = (HEIGHT / 2) - 1;

  for (uint8_t i = 0; i < WIDTH; i++) {
    int32_t ioffset = e_scaleX[layer] * (i - e_centerX);
    for (uint8_t j = 0; j < HEIGHT; j++) {
      int32_t joffset = e_scaleY[layer] * (j - e_centerY);
      int8_t data = inoise16(e_x[layer] + ioffset, e_y[layer] + joffset, e_z[layer]) >> 8;
      int8_t olddata = noise3d[layer][i][j];
      int8_t newdata = scale8( olddata, eNs_noisesmooth ) + scale8( data, 255 - eNs_noisesmooth );
      data = newdata;
      noise3d[layer][i][j] = data;
    }
  }
}

String EffectMStreamSmoke::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) fillType = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

// (c) SottNick
// Относительно стартовой версии - переписан 20200521
bool EffectMStreamSmoke::multipleStreamSmokeRoutine(CRGB *leds, EffectWorker *param)
{
  CRGB color;
  if(isDebug())
    //EffectMath::dimAll(254);
  //else
    FastLED.clear();

  xSmokePos = xSmokePos + speed / 255.0 + 0.01; // смещение здесь
  xSmokePos2 = xSmokePos2 + scale / 512.0 + 0.01; // вращение заполнения тут

  bool isColored = scale!=255 && scale!=1; // 255 и 1 - белый цвет
  if (isColored)
  {
    if (smokeHue == scale)
      {
        smokeHue = 0U;
        rhue = random8();
      }
    color = CHSV(rhue, 255U, 255U);
    if ((int)xSmokePos & 0x01)//((smokeDeltaHue >> 2U) == 0U) // какой-то умножитель охота подключить к задержке смены цвета, но хз какой...
      smokeHue++;
  }
  else
    color = CHSV((scale - 1U) * 2.6, !isColored ? 0U : 255U, 255U);

  switch(fillType){
    case 1:
      for (uint8_t y = 0; y < HEIGHT; y++) {
        float v, f = xSmokePos-y;
        f = modff(f, &v); v = (int)v%WIDTH;
        v = v+f;

        EffectMath::drawPixelXYF_X(v, y, color);
        EffectMath::drawPixelXYF(WIDTH-1-v, y, color);
      }
      break;
    case 2:
      for (uint8_t y = 0; y < HEIGHT; y++) {
        myLamp.getUnsafeLedsArray()[myLamp.getPixelNumber((int)(xSmokePos-y)%WIDTH+(int)(xSmokePos2)%WIDTH, y)] += color; // на то что Х может оказаться отрицательным - ложим болт :)
        myLamp.getUnsafeLedsArray()[myLamp.getPixelNumber((WIDTH-(int)(xSmokePos-y)-1)%WIDTH+(int)(xSmokePos2)%WIDTH, y)] += color;
      }
      break;
    case 3:
      for (uint8_t y = 0; y < HEIGHT; y++) {
        float v, f = xSmokePos-y*1.5;
        f = modff(f, &v); v = (int)v%WIDTH;
        v = v+f;

        EffectMath::drawPixelXYF_X(v, y, color);
        EffectMath::drawPixelXYF(WIDTH-1-v, y, color);
      }
      break;
    case 4:
      for (uint8_t y = 0; y < HEIGHT; y++) {
        myLamp.getUnsafeLedsArray()[myLamp.getPixelNumber((int)((xSmokePos-y)*1.5)%WIDTH+(int)(xSmokePos2)%WIDTH, y)] += color; // на то что Х может оказаться отрицательным - ложим болт :)
        myLamp.getUnsafeLedsArray()[myLamp.getPixelNumber((WIDTH-(int)((xSmokePos-y)*1.5)-1)%WIDTH+(int)(xSmokePos2)%WIDTH, y)] += color; // увеличим частоту в 1.5
      }
      break;
    case 5:
      for (uint8_t y = 0; y < HEIGHT; y++) {
        myLamp.getUnsafeLedsArray()[myLamp.getPixelNumber(((int)xSmokePos - y) % WIDTH * y / (HEIGHT - 1) + WIDTH / 2, y)] += color; // на то что Х может оказаться отрицательным - ложим болт :)
        myLamp.getUnsafeLedsArray()[myLamp.getPixelNumber((WIDTH - ((int)xSmokePos - y) - 1) % WIDTH * y / (HEIGHT - 1) + WIDTH / 2, y)] += color;
      }
      break;
    case 6:
      for (uint8_t y = 0; y < HEIGHT; y++) {
        myLamp.getUnsafeLedsArray()[myLamp.getPixelNumber((xSmokePos-y)*y/(HEIGHT-1)+WIDTH/2, y)] += color; // на то что Х может оказаться отрицательным - ложим болт :)
        myLamp.getUnsafeLedsArray()[myLamp.getPixelNumber((WIDTH-(xSmokePos-y)-1)*y/(HEIGHT-1)+WIDTH/2, y)] += color;
      }
      break;
    default:
      break;
  }
  EffectMath::blur2d(35); // возможно размытие требуется до того как через шум пропускать, либо нужно заполнение через сабпиксель пропустить... хз, потом погляжу...
  if(!isDebug()){
    // Noise
    uint16_t sc = (uint16_t)scale * 60 + 4000; //64 + 1000;
    uint16_t sc2 = EffectMath::fmap(speed, 1, 255, 0.1, 1.); //(float)speed / 100.0 + 0.75; //1.5...3.5;
    e_x[0] += 1000*sc2; // 3000;
    e_y[0] += 1000*sc2; // 3000;
    e_z[0] += 1000*sc2; // 3000;
    e_scaleX[0] = sc; //8000;
    e_scaleY[0] = sc; //8000;
    FillNoise(0); 

    EffectMath::MoveFractionalNoise(MOVE_X, noise3d, 3);//4
    EffectMath::MoveFractionalNoise(MOVE_Y, noise3d, 3);//4
  }
  return true;
}

// ----------- Эффекты "Пикассо" (c) obliterator
void EffectPicasso::generate(bool reset){
  double minSpeed = 0.2, maxSpeed = 0.8;
  unsigned num = map(scale, 0U, 255U, 6U, sizeof(particles) / sizeof(*particles));

  for (unsigned i = numParticles; i < num; i++) {
    Particle *curr = (Particle *)&particles[i];
    curr->position_x = random8(0, WIDTH);
    curr->position_y = random8(0, HEIGHT);

    curr->speed_x = +((-maxSpeed / 3) + (maxSpeed * (float)random(1, 100) / 100));
    curr->speed_x += curr->speed_x > 0 ? minSpeed : -minSpeed;

    curr->speed_y = +((-maxSpeed / 2) + (maxSpeed * (float)random(1, 100) / 100));
    curr->speed_y += curr->speed_y > 0 ? minSpeed : -minSpeed;

    curr->color = CHSV(random8(1U, 255U), 255U, 255U);
    curr->hue_next = curr->color.h;
  };

  for (unsigned i = 0; i < num; i++) {
    if (reset) {
      particles[i].hue_next = random8(1U, 255U);
      particles[i].hue_step = (particles[i].hue_next - particles[i].color.h) / 25;
    }
    if (particles[i].hue_next != particles[i].color.h && particles[i].hue_step) {
      particles[i].color.h += particles[i].hue_step;
    }
  }
  numParticles = num;
}

void EffectPicasso::position(){
  float speedfactor = speed/255.0+0.1;
  for (unsigned i = 0; i < numParticles; i++) {
    Particle *curr = (Particle *)&particles[i];
    if (curr->position_x + curr->speed_x > WIDTH || curr->position_x + curr->speed_x < 0) {
      curr->speed_x = -curr->speed_x;
    }

    if (curr->position_y + curr->speed_y > HEIGHT || curr->position_y + curr->speed_y < 0) {
      curr->speed_y = -curr->speed_y;
    }

    curr->position_x += curr->speed_x*speedfactor;
    curr->position_y += curr->speed_y*speedfactor;
  };
}

bool EffectPicasso::picassoRoutine(CRGB *leds, EffectWorker *param){
  generate();
  position();

  for (unsigned i = 0; i < numParticles - 2; i+=2) {
    Particle *p1 = (Particle *)&particles[i];
    Particle *p2 = (Particle *)&particles[i + 1];
    EffectMath::drawLine(p1->position_x, p1->position_y, p2->position_x, p2->position_y, p1->color);
  }

  EVERY_N_MILLIS(20000){
    generate(true);
  }

  EffectMath::blur2d(80);
  return true;
}

bool EffectPicasso::picassoRoutine2(CRGB *leds, EffectWorker *param){
  generate();
  position();
  EffectMath::dimAll(180);

  for (unsigned i = 0; i < numParticles - 1; i++) {
    Particle *p1 = (Particle *)&particles[i];
    Particle *p2 = (Particle *)&particles[i + 1];
    EffectMath::drawLineF(p1->position_x, p1->position_y, p2->position_x, p2->position_y, p1->color);
  }

  EVERY_N_MILLIS(20000){
    generate(true);
  }

  EffectMath::blur2d(80);

  return true;
}

bool EffectPicasso::picassoRoutine3(CRGB *leds, EffectWorker *param){
  generate();
  position();
  EffectMath::dimAll(180);

  for (unsigned i = 0; i < numParticles - 2; i+=2) {
    Particle *p1 = (Particle *)&particles[i];
    Particle *p2 = (Particle *)&particles[i + 1];
    EffectMath::drawCircleF(fabs(p1->position_x - p2->position_x), fabs(p1->position_y - p2->position_y), fabs(p1->position_x - p1->position_y), p1->color);
  }

  EVERY_N_MILLIS(20000){
    generate(true);
  }

  EffectMath::blur2d(80);

  return true;
}


String EffectPicasso::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) pidx = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) {
    byte hue = EffectCalc::setDynCtrl(_val).toInt();
    TDynamicRGBGradientPalette_byte dynpal[20] = {
        0,  0,  0,  0,
        1,  0,  0,  0,
        80,  0,  0,  0,
        150,  0,  0,  0,
        255,  0,  0,  0
    };
    uint8_t *ptr = (uint8_t *)dynpal + 1;
    CRGB color = CHSV(hue + 255, 255U, 255U);
    memcpy(ptr, color.raw, sizeof(color.raw)); ptr += 4;
    color = CHSV(hue + 135, 255U, 200U);
    memcpy(ptr, color.raw, sizeof(color.raw)); ptr += 4;
    color = CHSV(hue + 160, 255U, 120U);
    memcpy(ptr, color.raw, sizeof(color.raw)); ptr += 4;
    color = CHSV(hue + 150, 255U, 255U);
    memcpy(ptr, color.raw, sizeof(color.raw)); ptr += 4;
    color = CHSV(hue  + 255, 255U, 255U);
    memcpy(ptr, color.raw, sizeof(color.raw)); ptr += 4;

    CRGBPalette32 pal;
    pal.loadDynamicGradientPalette(dynpal);
    palettes->add(0, pal, 0, 16);
  }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectPicasso::picassoRoutine4(CRGB *leds, EffectWorker *param){
  generate();
  position();

  GradientPalette *myPal = (*palettes)[pidx];

 // сила возмущения
  unsigned mx = EffectMath::fmap(scale, 0U, 255U, 200U, 80U);
  // радиус возмущения
  unsigned sc = EffectMath::fmap(scale, 0U, 255U, 12, 7);
  // отсечка расчетов (оптимизация скорости)
  unsigned tr = sc * 2 / 3;

  for (unsigned x = 0; x < WIDTH; x++) {
    for (unsigned y = 0; y < HEIGHT; y++) {
      float sum = 0;
      for (unsigned i = 0; i < numParticles; i += 2) {
        Particle *p1 = (Particle *)&particles[i];
        if ((unsigned)abs(x - p1->position_x) > tr || (unsigned)abs(y - p1->position_y) > tr) continue;
        float d = EffectMath::distance(x, y, p1->position_x, p1->position_y);
        if (d < 2) {
          // дополнительно подсвечиваем сердцевину
          sum += EffectMath::mapcurve(d, 0, 2, 255, mx, EffectMath::InQuad);
        } else if (d < sc) {
          // ореол резко демпфируем, во избежание размазывания и пересвета
          sum += EffectMath::mapcurve(d, 2, sc, mx, 0, EffectMath::OutQuart);
        }

        if (sum >= 255) { sum = 255; break; }
      }
      CRGB color = myPal->GetColor((uint8_t)sum, 255);
      EffectMath::drawPixelXY(x, y, color);
      }
  }

  return true;
}

bool EffectPicasso::run(CRGB *ledarr, EffectWorker *opt){

  if (effect == EFF_PICASSO) {
    return picassoRoutine(*&ledarr, &*opt);
  }
  if (effect == EFF_PICASSO2) {
    return picassoRoutine2(*&ledarr, &*opt);
  }
  if (effect == EFF_PICASSO3) {
    return picassoRoutine3(*&ledarr, &*opt);
  }
  return picassoRoutine4(*&ledarr, &*opt);
}

// -------- Эффект "Прыгуны" (c) obliterator
void EffectLeapers::restart_leaper(EffectLeapers::Leaper * l) {
  // leap up and to the side with some random component
#ifdef MIC_EFFECTS
  uint8_t mic = myLamp.getMicMaxPeak();
  float rand = random8(1, 50 + _rv * 4);
  l->xd = (1 * (float)(isMicOn() ? 25 + mic : rand) / 100.0);
  l->yd = (2 * (float)(isMicOn() ? 25 + mic : rand) / 100.0);
#else
  l->xd = (1 * (float)random8(1, 50 + _rv * 4) / 100);
  l->yd = (2 * (float)random8(1, 100 + _rv * 3) / 100);
#endif

  // for variety, sometimes go 50% faster
  if (random8() < 12) {
    l->xd += l->xd * 0.5;
    l->yd += l->yd * 0.5;
  }

  // leap towards the centre of the screen
  if (l->x > (WIDTH / 2)) {
    l->xd = -l->xd;
  }
}

void EffectLeapers::move_leaper(EffectLeapers::Leaper * l) {
#define GRAVITY            0.06
//float GRAVITY  = EffectMath::fmap(speed, 1, 255, 0.01f, 0.6f);
#define SETTLED_THRESHOLD  0.1
#define WALL_FRICTION      0.95
#define WIND               0.95    // wind resistance

  float speedfactor = (speed/512.0);
  float speedfactor2 = speedfactor + 0.33;

  l->x += l->xd*speedfactor2;
  l->y += l->yd*speedfactor2;

  // bounce off the floor and ceiling?
  if (l->y < 0 || l->y > HEIGHT - 1) {
    l->yd = (-l->yd * WALL_FRICTION);
    l->xd = ( l->xd * WALL_FRICTION);
    l->y += l->yd*speedfactor;
    if (l->y < 0) l->y = 0;
    // settled on the floor?
    if (l->y <= SETTLED_THRESHOLD && abs(l->yd) <= SETTLED_THRESHOLD) {
      restart_leaper(l);
    }
  }

  // bounce off the sides of the screen?
  if (l->x <= 0 || l->x >= WIDTH - 1) {
    l->xd = (-l->xd * WALL_FRICTION);
    if (l->x <= 0) {
      l->x = l->xd*speedfactor;
    } else {
      l->x = WIDTH - 1 - l->xd;
    }
  }

  l->yd -= GRAVITY;
  l->xd *= WIND;
  l->yd *= WIND;
}

void EffectLeapers::generate(bool reset){
  unsigned num = map(scale, 0U, 255U, 6U, sizeof(leapers) / sizeof(*leapers));

  for (unsigned i = numParticles; i < num; i++) {
    Leaper *curr = (Leaper *)&leapers[i];
    curr->x = random8(0, WIDTH);
    curr->y = random8(0, HEIGHT);

    curr->color = CHSV(random(1U, 255U), 255U, 255U);
  };
  numParticles = num;
}

String EffectLeapers::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) _rv = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  _rv = (_rv ? _rv : 13);
  return String();
}

bool EffectLeapers::leapersRoutine(CRGB *leds, EffectWorker *param){
  generate();

  EffectMath::dimAll(0);

  for (unsigned i = 0; i < numParticles; i++) {
    move_leaper(&leapers[i]);
    EffectMath::drawPixelXYF(leapers[i].x, leapers[i].y, leapers[i].color);
  };

  EffectMath::blur2d(20);
  return true;
}

bool EffectLeapers::run(CRGB *ledarr, EffectWorker *opt){
  return leapersRoutine(*&ledarr, &*opt);
}

// ----------- Эффекты "Лавовая лампа" (c) obliterator
void EffectLiquidLamp::generate(bool reset){
  unsigned num = map(scale, 0U, 255, 10, sizeof(particles) / sizeof(*particles));

  for (unsigned i = numParticles; i < num; i++) {
    Particle *curr = (Particle *)&particles[i];
    curr->position_x = random(0, WIDTH);
    curr->position_y = 0;
    curr->mass = random(MASS_MIN, MASS_MAX);
    curr->spf = EffectMath::fmap(curr->mass, MASS_MIN, MASS_MAX, 0.0015, 0.0005);
    curr->rad = EffectMath::fmap(curr->mass, MASS_MIN, MASS_MAX, 2, 3);
    curr->mx = map(curr->mass, MASS_MIN, MASS_MAX, 60, 80); // сила возмущения
    curr->sc = map(curr->mass, MASS_MIN, MASS_MAX, 6, 10); // радиус возмущения
    curr->tr = curr->sc  * 2 / 3; // отсечка расчетов (оптимизация скорости)
  };
  numParticles = num;
}

void EffectLiquidLamp::position(){
  float speedfactor = speed / 127.0 + 0.1;
  for (unsigned i = 0; i < numParticles; i++) {
    Particle *curr = (Particle *)&particles[i];
    curr->hot += EffectMath::mapcurve(curr->position_y, 0, HEIGHT, 5, -5, EffectMath::InOutQuad) * speedfactor;

    float heat = (curr->hot / curr->mass) - 1;
    if (heat > 0 && curr->position_y < HEIGHT) {
      curr->speed_y += heat * curr->spf;
    }
    if (curr->position_y > 0) {
      curr->speed_y -= 0.07;
    }

    if (curr->speed_y) curr->speed_y *= 0.85;
    curr->position_y += curr->speed_y * speedfactor;

    if (physic_on) {
      if (curr->speed_x) curr->speed_x *= 0.7;
      curr->position_x += curr->speed_x * speedfactor;
    }

    if (curr->position_x > WIDTH) curr->position_x -= WIDTH;
    if (curr->position_x < 0) curr->position_x += WIDTH;
    if (curr->position_y > HEIGHT) curr->position_y = HEIGHT;
    if (curr->position_y < 0) curr->position_y = 0;
  };
}

void EffectLiquidLamp::physic(){
  for (unsigned i = 0; i < numParticles; i++) {
    Particle *p1 = (Particle *)&particles[i];
    // отключаем физику на границах, чтобы не слипались шары
    if (p1->position_y < 3 || p1->position_y > HEIGHT - 1) continue;
    for (unsigned j = i + 1; j < numParticles; j++) {
      Particle *p2 = (Particle *)&particles[j];
      if (p2->position_y < 3 || p2->position_y > HEIGHT - 1) continue;
      float radius = 3;//(p1->rad + p2->rad);
      if (p1->position_x + radius > p2->position_x
					&& p1->position_x < radius + p2->position_x
					&& p1->position_y + radius > p2->position_y
					&& p1->position_y < radius + p2->position_y
      ){
          float dist = EffectMath::distance(p1->position_x, p1->position_y, p2->position_x, p2->position_y);
          if (dist <= radius) {
            float nx = (p2->position_x - p1->position_x) / dist;
            float ny = (p2->position_y - p1->position_y) / dist;
            float p = 2 * (p1->speed_x * nx + p1->speed_y * ny - p2->speed_x * nx - p2->speed_y * ny) / (p1->mass + p2->mass);
            float pnx = p * nx, pny = p * ny;
            p1->speed_x = p1->speed_x - pnx * p1->mass;
            p1->speed_y = p1->speed_y - pny * p1->mass;
            p2->speed_x = p2->speed_x + pnx * p2->mass;
            p2->speed_y = p2->speed_y + pny * p2->mass;
          }
			}
    }
  }
}

String EffectLiquidLamp::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) pidx = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) {
    byte hue = EffectCalc::setDynCtrl(_val).toInt();
    TDynamicRGBGradientPalette_byte dynpal[20] = {
        0,  0,  0,  0,
        1,  0,  0,  0,
        80,  0,  0,  0,
        150,  0,  0,  0,
        255,  0,  0,  0
    };
    uint8_t *ptr = (uint8_t *)dynpal + 1;
    CRGB color = CHSV(hue + 255, 255U, 255U);
    memcpy(ptr, color.raw, sizeof(color.raw)); ptr += 4;
    color = CHSV(hue + 135, 255U, 200U);
    memcpy(ptr, color.raw, sizeof(color.raw)); ptr += 4;
    color = CHSV(hue + 160, 255U, 120U);
    memcpy(ptr, color.raw, sizeof(color.raw)); ptr += 4;
    color = CHSV(hue + 150, 255U, 255U);
    memcpy(ptr, color.raw, sizeof(color.raw)); ptr += 4;
    color = CHSV(hue  + 255, 255U, 255U);
    memcpy(ptr, color.raw, sizeof(color.raw)); ptr += 4;

    CRGBPalette32 pal;
    pal.loadDynamicGradientPalette(dynpal);
    palettes->add(0, pal, 0, 16);
  }
  else if(_val->getId()==5) filter = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==6) physic_on = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectLiquidLamp::Routine(CRGB *leds, EffectWorker *param){
  generate();
  position();
  if (physic_on) {
    physic();
  }

  GradientPalette *myPal = (*palettes)[pidx];

  for (unsigned x = 0; x < WIDTH; x++) {
    for (unsigned y = 0; y < HEIGHT; y++) {
      float sum = 0;
      for (unsigned i = 0; i < numParticles; i++) {
        Particle *p1 = (Particle *)&particles[i];
        if ((unsigned)abs(x - p1->position_x) > p1->tr || (unsigned)abs(y - p1->position_y) > p1->tr) continue;
        float d = EffectMath::distance(x, y, p1->position_x, p1->position_y);
        if (d < p1->rad) {
          sum += EffectMath::mapcurve(d, 0, p1->rad, 255, p1->mx, EffectMath::InQuad);
        } else if (d < p1->sc){
          sum += EffectMath::mapcurve(d, p1->rad, p1->sc, p1->mx, 0, EffectMath::OutQuart);
        }
        if (sum > 255) { sum = 255; break; }
      }
      buff[x][y] = sum;
      CRGB color = myPal->GetColor((uint8_t)sum, 255);
      EffectMath::drawPixelXY(x, y, color);
    }
  }

  if (filter < 2) {
    for (unsigned x = 0; x < WIDTH; x++) {
      for (unsigned y = 0; y < HEIGHT; y++) {
        CRGB color = myPal->GetColor(buff[x][y], filter? buff[x][y] : 255);
        EffectMath::drawPixelXY(x, y, color);
      }
    }
  } else {
    float min =0, max = 0;
    // Оператор Щарра
    int oper_h[3][3] = {{3, 10, 3}, {0, 0, 0}, {-3, -10, -3}};
    int oper_v[3][3] = {{3, 0, -3}, {10, 0, -10}, {3, 0, -3}};
    for (unsigned x = 0; x < WIDTH - 1; x++) {
      for (unsigned y = 0; y < HEIGHT - 1; y++) {
        int valh = 0, valv = 0;
        for (int j = -1; j <= 1; j++) {
          for (int i = -1; i <= 1; i++) {
            valh += oper_h[j + 1][i + 1] * buff[(int)x + j][(int)y + i];
            valv += oper_v[j + 1][i + 1] * buff[(int)x + j][(int)y + i];
          }
        }
        float val = EffectMath::sqrt((valh * valh) + (valv * valv));
        buff2[x][y] = val;
        if (val < min) min = val;
        if (val > max) max = val;
      }
    }

    for (unsigned x = 0; x < WIDTH - 1; x++) {
      for (unsigned y = 0; y < HEIGHT - 1; y++) {
        float val = buff2[x][y];
        unsigned step = filter - 1;
        val = 1 - (val - min) / (max - min);
        while (step) { val *= val; --step; } // почему-то это быстрее чем pow
        CRGB color = myPal->GetColor(buff[x][y], val * 255);
        EffectMath::drawPixelXY(x, y, color);
      }
    }
  }
  return true;
}

bool EffectLiquidLamp::run(CRGB *ledarr, EffectWorker *opt){
  return Routine(*&ledarr, &*opt);
}

// ------- Эффект "Вихри"
// Based on Aurora : https://github.com/pixelmatix/aurora/blob/master/PatternFlowField.h
// Copyright(c) 2014 Jason Coon
bool EffectWhirl::run(CRGB *ledarr, EffectWorker *opt){

  return whirlRoutine(*&ledarr, &*opt);
}

void EffectWhirl::load(){
  palettesload();    // подгружаем дефолтные палитры
  ff_x = random16();
  ff_y = random16();
  ff_z = random16();
  for (uint8_t i = 0; i < AVAILABLE_BOID_COUNT; i++) {
      boids[i] = Boid(random(WIDTH), 0);
  }

}

bool EffectWhirl::whirlRoutine(CRGB *leds, EffectWorker *param) {
#ifdef MIC_EFFECTS
  micPick = isMicOn() ? myLamp.getMicMaxPeak() : 0;
 // EffectMath::dimAll(255U);
  fadeToBlackBy(leds, NUM_LEDS, 15);
#else
  fadeToBlackBy(leds, NUM_LEDS, 15);
#endif
  float speedfactor = EffectMath::fmap((float)speed, 1.0f, 255.0f, 0.5f, 1.1f);
  for (uint8_t i = 0; i < AVAILABLE_BOID_COUNT; i++) {
    Boid * boid = &boids[i];

    int ioffset = ff_scale * boid->location.x;
    int joffset = ff_scale * boid->location.y;

    byte angle = inoise8(ff_x + ioffset, ff_y + joffset, (uint16_t)ff_z);

    boid->velocity.x = ((float)sin8(angle) * 0.0078125 - speedfactor);
    boid->velocity.y = -((float)cos8(angle) * 0.0078125 - speedfactor);
    boid->update();
#ifdef MIC_EFFECTS
    if (!isMicOn())
      EffectMath::drawPixelXYF(boid->location.x, boid->location.y, ColorFromPalette(*curPalette, angle + (uint8_t)hue)); // + hue постепенно сдвигает палитру по кругу
    else
      EffectMath::drawPixelXYF(boid->location.x, boid->location.y, CHSV(myLamp.getMicMapFreq(), 255-micPick, constrain(micPick * EffectMath::fmap(scale, 1.0f, 255.0f, 1.25f, 5.0f), 48, 255))); // + hue постепенно сдвигает палитру по кругу

#else
    EffectMath::drawPixelXYF(boid->location.x, boid->location.y, ColorFromPalette(*curPalette, angle + (uint8_t)hue)); // + hue постепенно сдвигает палитру по кругу
#endif
    if (boid->location.x < 0 || boid->location.x >= WIDTH || boid->location.y < 0 || boid->location.y >= HEIGHT) {
      boid->location.x = random(WIDTH);
      boid->location.y = 0;
    }
  }
#ifdef MIC_EFFECTS
  EffectMath::blur2d((isMicOn() ? /*constrain(micPick * 2, 10, 100)*/ 30U : 30U));
#else
  EffectMath::blur2d(30U);
#endif
  //EVERY_N_MILLIS(200) {
    hue += speedfactor;
  //}
  ff_x += speedfactor;
  ff_y += speedfactor;
  ff_z += speedfactor;
  return true;
}

// ------------- Эффект "Блики на воде Цвета"
// Идея SottNick
// переписал на программные блики + паттерны - (c) kostyamat
// Генератор бликов (c) stepko

void EffectAquarium::load(){
  currentPalette=PartyColors_p;
}

String EffectAquarium::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) satur = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) glare = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectAquarium::run(CRGB *ledarr, EffectWorker *opt) {
  return aquariumRoutine(*&ledarr, &*opt);
}

void EffectAquarium::nPatterns() {
  if (glare == 1)
    iconIdx = (millis() >> 15) % 12;//beatsin88(3 * EffectMath::fmap((float)speed, 1., 255., 5., 15.), 0, 12);
  else
    iconIdx = glare - 3;
 #ifdef MIC_EFFECTS
  byte _video = isMicOn() ? constrain(myLamp.getMicMaxPeak() * EffectMath::fmap(scale, 1.0f, 255.0f, 1.25f, 5.0f), 48U, 255U) : 255;
#else
  byte _video = 255;
#endif
  for (byte x = 0; x < WIDTH * 2; x++)
  {
    for (byte y = 0; y < HEIGHT * 2; y++)
    {
      ledbuff[myLamp.getPixelNumberBuff(x, y, WIDTH * 2, HEIGHT * 2)] = (pgm_read_byte(&patterns[iconIdx][y % 10][x % 10]));
    }
  }

  for (byte x = 0; x < WIDTH; x ++)
  {
    for (byte y = 0; y < HEIGHT; y ++)
    {
      byte val = ledbuff[myLamp.getPixelNumberBuff((xsin + x) % (WIDTH * 2), (ysin + y) % (HEIGHT * 2), WIDTH * 2, HEIGHT * 2)];
      EffectMath::drawPixelXY(x, HEIGHT-1 - y, CHSV((uint8_t)((uint16_t)hue - val* 31), map((satur + 32 * val), 1, 510, 1, 255), _video));

    }
  }
}

void EffectAquarium::nGlare(CRGB *leds) {
/*#ifdef MIC_EFFECTS
  byte _video = isMicOn() ? constrain(myLamp.getMicMaxPeak() * EffectMath::fmap(scale, 1.0f, 255.0f, 1.25f, 5.0f), 48U, 255U) : 255;
#else
  byte _video = 255;
#endif
*/
  fill_solid(currentPalette, 16, CHSV(hue, satur, 200));
  currentPalette[10] = CHSV(hue, satur - 60, 225);
  currentPalette[9] = CHSV(hue, 255 - satur, 180);
  currentPalette[8] = CHSV(hue, 255 - satur, 180);
  currentPalette[7] = CHSV(hue, satur - 60, 225);

  fillNoiseLED(leds);
  
  blur2d(leds, LED_COLS, LED_ROWS, 100);
}

void EffectAquarium::fillNoiseLED(CRGB *leds) {
  uint8_t  dataSmoothing = 200 - (_speed * 4);
  for (uint8_t i = 0; i < MAX_DIMENSION; i++) {
    int32_t ioffset = _scale * i;
    for (uint8_t j = 0; j < MAX_DIMENSION; j++) {
      int32_t joffset = _scale * j;
      
      uint8_t data = inoise8(x + ioffset, y + joffset, z);
      
      data = qsub8(data, 16);
      data = qadd8(data, scale8(data, 39));
      
      uint8_t newdata = scale8(ledbuff[myLamp.getPixelNumberBuff(j, i, WIDTH, HEIGHT)], dataSmoothing) + scale8(data, 256 - dataSmoothing);
      data = newdata;
      
      ledbuff[myLamp.getPixelNumberBuff(j, i, WIDTH, HEIGHT)] = data;
    }
  }
  z += _speed;
  x += _speed / 16 * sin8(millis() / 10000);
  y += _speed / 16 * cos8(millis() / 10000);
  
  for (uint8_t i = 0; i < WIDTH; i++) {
    for (uint8_t j = 0; j < HEIGHT; j++) {
      uint8_t index = ledbuff[myLamp.getPixelNumberBuff(j, i, WIDTH, HEIGHT)];

      CRGB color = ColorFromPalette(currentPalette, index);
      EffectMath::drawPixelXY(i, j, color);
    }
  }
}

bool EffectAquarium::aquariumRoutine(CRGB *leds, EffectWorker *param) {
  float speedfactor = EffectMath::fmap((float)speed, 1., 255., 0.1, 1.);

  xsin = beatsin88(175 * EffectMath::fmap((float)speed, 1., 255., 1, 4.), 0, WIDTH * WIDTH);
  ysin = beatsin88(225 * EffectMath::fmap((float)speed, 1., 255., 1, 4.), 0, HEIGHT * HEIGHT);

  switch (glare) { //
  case 0:
    break;
  case 2:
    nGlare(leds);
    break;
  default:
    nPatterns();
    break;
  }

  if (!glare) {// если блики включены

    for (int16_t i = 0U; i < NUM_LEDS; i++)
    {
#ifdef MIC_EFFECTS
      if (isMicOn()) {
        hue = myLamp.getMicMapFreq();
        leds[i] = CHSV((uint8_t)hue,
          satur,
          constrain(myLamp.getMicMaxPeak() * EffectMath::fmap(scale, 1.0f, 255.0f, 1.25f, 5.0f),
          48,
          255)
          );
      }
      else
        leds[i] = CHSV((uint8_t)hue, satur, 255U);
#else
      leds[i] = CHSV((uint8_t)hue, satur, 255U);
#endif
    }
  }
  if (speed == 1) {
    hue = scale;
  }
  else {
    hue += speedfactor;
  }

  return true;
}

// ------- Эффект "Звезды"
bool EffectStar::run(CRGB *ledarr, EffectWorker *opt){

  return starRoutine(*&ledarr, &*opt);
}

void EffectStar::load(){
  palettesload();    // подгружаем дефолтные палитры
}

void EffectStar::drawStar(float xlocl, float ylocl, float biggy, float little, int16_t points, float dangle, uint8_t koler)// random multipoint star
{
  radius2 = 255.0 / points;
  for (int i = 0; i < points; i++)
  {
#ifdef MIC_EFFECTS
    EffectMath::drawLine(xlocl + ((little * (sin8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128), ylocl + ((little * (cos8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128), xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128), ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128), isMicOn() ? CHSV(koler+myLamp.getMicMapFreq(), 255-micPick, constrain(micPick * EffectMath::fmap(scale, 1.0f, 255.0f, 1.25f, 5.0f), 48, 255)) : ColorFromPalette(*curPalette, koler));
    EffectMath::drawLine(xlocl + ((little * (sin8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128), ylocl + ((little * (cos8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128), xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128), ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128), isMicOn() ? CHSV(koler+myLamp.getMicMapFreq(), 255-micPick, constrain(micPick * EffectMath::fmap(scale, 1.0f, 255.0f, 1.25f, 5.0f), 48, 255)) : ColorFromPalette(*curPalette, koler));

#else
    EffectMath::drawLine(xlocl + ((little * (sin8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128), ylocl + ((little * (cos8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128), xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128), ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128), ColorFromPalette(*curPalette, koler));
    EffectMath::drawLine(xlocl + ((little * (sin8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128), ylocl + ((little * (cos8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128), xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128), ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128), ColorFromPalette(*curPalette, koler));
#endif
    }
}

bool EffectStar::starRoutine(CRGB *leds, EffectWorker *param) {

if (setup) { // однократная настройка при старте эффекта
  float _speed = speed;
  counter = 0.0;

  // стартуем с центра
  driftx = (float)WIDTH/2.0;
  drifty = (float)HEIGHT/2.0;

  cangle = (float)(sin8(random8(25, 220)) - 128.0f) / 128.0f; //angle of movement for the center of animation gives a float value between -1 and 1
  sangle = (float)(sin8(random8(25, 220)) - 128.0f) / 128.0f; //angle of movement for the center of animation in the y direction gives a float value between -1 and 1

  stars_count = WIDTH / 2U;
  if (stars_count > STARS_NUM) stars_count = STARS_NUM;
  for (uint8_t num = 0; num < stars_count; num++) {
    points[num] = random8(3, 9); // количество углов в звезде
    delay[num] = _speed / 5 + (num << 2) + 1U; // задержка следующего пуска звезды
    color[num] = random8();
  }
  setup = false;
}

#ifdef MIC_EFFECTS
  micPick = myLamp.getMicMaxPeak();
  //EffectMath::dimAll(isMicOn() ? micPick*2 : 90);
  fadeToBlackBy(leds, NUM_LEDS, 255U - (isMicOn() ? micPick*2 : 90)); // работает быстрее чем dimAll
#else
  fadeToBlackBy(leds, NUM_LEDS, 165);
  //EffectMath::dimAll(90);
#endif

  float _scalefactor = ((float)speed/380.0+0.05);

  counter+=_scalefactor; // определяет то, с какой скоростью будет приближаться звезда

  if (driftx > (WIDTH - spirocenterX / 2U))//change directin of drift if you get near the right 1/4 of the screen
    cangle = 0 - fabs(cangle);
  if (driftx < spirocenterX / 2U)//change directin of drift if you get near the right 1/4 of the screen
    cangle = fabs(cangle);
  if ((uint16_t)counter % CENTER_DRIFT_SPEED == 0)
    driftx = driftx + (cangle * _scalefactor);//move the x center every so often

  if (drifty > ( HEIGHT - spirocenterY / 2U))// if y gets too big, reverse
    sangle = 0 - fabs(sangle);
  if (drifty < spirocenterY / 2U) // if y gets too small reverse
    sangle = fabs(sangle);
  //if ((counter + CENTER_DRIFT_SPEED / 2U) % CENTER_DRIFT_SPEED == 0)
  if ((uint16_t)counter % CENTER_DRIFT_SPEED == 0)
    drifty =  drifty + (sangle * _scalefactor);//move the y center every so often

  for (uint8_t num = 0; num < stars_count; num++) {
    if (counter >= delay[num])//(counter >= ringdelay)
    {
      if (counter - delay[num] <= WIDTH + 5) {
        EffectStar::drawStar(driftx, drifty, 2 * (counter - delay[num]), (counter - delay[num]), points[num], STAR_BLENDER + color[num], color[num]);
        color[num]+=_scalefactor; // в зависимости от знака - направление вращения
      }
      else
        delay[num] = counter + (stars_count << 1) + 1U; // задержка следующего пуска звезды
    }
  }
#ifdef MIC_EFFECTS
  EffectMath::blur2d(isMicOn() ? micPick/2 : 30U); //fadeToBlackBy() сам блурит, уменьшил блур под микрофон
#else
  EffectMath::blur2d(30U);
#endif
  return true;
}

//---------- Эффект "Фейерверк"
//адаптация и переписал - kostyamat
//https://gist.github.com/jasoncoon/0cccc5ba7ab108c0a373
String EffectFireworks::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) cnt = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) flashing = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

CRGB &Dot::piXY(CRGB *leds, byte x, byte y) {
  x -= PIXEL_X_OFFSET;
  y -= PIXEL_Y_OFFSET;
  if( x < WIDTH && y < HEIGHT) {
    return leds[myLamp.getPixelNumber(x, y)];
  } else
    return empty; // fixed
}

void Dot::Skyburst( accum88 basex, accum88 basey, saccum78 basedv, CRGB& basecolor, uint8_t dim)
  {
    yv = (int16_t)0 + (int16_t)random16(1500) - (int16_t)500;
    xv = basedv + (int16_t)random16(2000) - (int16_t)1000;
    y = basey;
    x = basex;
    color = basecolor;
    //EffectMath::makeBrighter(color, 50);
    color *= dim; //50;
    theType = 2;
    show = 1;
  }

void Dot::GroundLaunch(DOTS_STORE &store)
  {
    yv = 600 + random16(400 + (25 * HEIGHT));
    if(yv > 1200) yv = 1200;
    xv = (int16_t)random16(600) - (int16_t)300;
    y = 0;
    x = 0x8000;
    color = CHSV(0, 0, 130); // цвет запускаемого снаряда
    show = 1;
  }

  void Dot::Move(DOTS_STORE &store, bool Flashing)
  {
    if( !show) return;
    yv -= gGravity;
    xv = scale15by8_local( xv, gDrag);
    yv = scale15by8_local( yv, gDrag);

    if( theType == 2) {
      xv = scale15by8_local( xv, gDrag);
      yv = scale15by8_local( yv, gDrag);
      color.nscale8( 255);
      if( !color) {
        show = 0;
      }
    }
    // if we'd hit the ground, bounce
    if( yv < 0 && (y < (-yv)) ) {
      if( theType == 2 ) {
        show = 0;
      } else {
        yv = -yv;
        yv = scale15by8_local( yv, gBounce);
        if( yv < 500 ) {
          show = 0;
        }
      }
    }
    if( (yv < -300) /* && (!(oyv < 0))*/ ) {
      // pinnacle
      if( theType == 1 ) {

        if( (y > (uint16_t)(0x8000)) && (random8() < 32) && Flashing) {
          // boom
          LEDS.showColor( CRGB::Gray);
          LEDS.showColor( CRGB::Black);
        }

        show = 0;

        store.gSkyburst = true;
        store.gBurstx = x;
        store.gBursty = y;
        store.gBurstxv = xv;
        store.gBurstyv = yv;
        store.gBurstcolor = CRGB(random8(), random8(), random8());
      }
    }
    if( theType == 2) {
      if( ((xv >  0) && (x > xv)) ||
          ((xv < 0 ) && (x < (0xFFFF + xv))) )  {
        x += xv;
      } else {
        show = 0;
      }
    } else {
      x += xv;
    }
    y += yv;

  }

void Dot::Draw(CRGB *leds)
  {
    if( !show) return;
    byte ix, xe, xc;
    byte iy, ye, yc;
    screenscale( x, MODEL_WIDTH, ix, xe);
    screenscale( y, MODEL_HEIGHT, iy, ye);
    yc = 255 - ye;
    xc = 255 - xe;

    CRGB c00 = CRGB( dim8_video( scale8( scale8( color.r, yc), xc)),
                     dim8_video( scale8( scale8( color.g, yc), xc)),
                     dim8_video( scale8( scale8( color.b, yc), xc))
                     );
    CRGB c01 = CRGB( dim8_video( scale8( scale8( color.r, ye), xc)),
                     dim8_video( scale8( scale8( color.g, ye), xc)),
                     dim8_video( scale8( scale8( color.b, ye), xc))
                     );

    CRGB c10 = CRGB( dim8_video( scale8( scale8( color.r, yc), xe)),
                     dim8_video( scale8( scale8( color.g, yc), xe)),
                     dim8_video( scale8( scale8( color.b, yc), xe))
                     );
    CRGB c11 = CRGB( dim8_video( scale8( scale8( color.r, ye), xe)),
                     dim8_video( scale8( scale8( color.g, ye), xe)),
                     dim8_video( scale8( scale8( color.b, ye), xe))
                     );

    piXY(leds, ix, iy) += c00;
    piXY(leds, ix, iy + 1) += c01;
    piXY(leds, ix + 1, iy) += c10;
    piXY(leds, ix + 1, iy + 1) += c11;
  }

bool EffectFireworks::run(CRGB *ledarr, EffectWorker *opt)
{
  random16_add_entropy(millis());
  dim = beatsin8(100, 20, 100);
  EVERY_N_MILLISECONDS(EFFECTS_RUN_TIMER * 10) {
    valDim = random8(25, 50);
  }
  EVERY_N_MILLISECONDS(10) {
    return fireworksRoutine(*&ledarr, &*opt);
  }
  return false;
}

void EffectFireworks::sparkGen() {
  for (byte c = 0; c < cnt; c++) {
    if( gDot[c].show == 0 ) {
      if( launchcountdown[c] == 0) {
        gDot[c].GroundLaunch(this->store);
        gDot[c].theType = 1;
        launchcountdown[c] = random16(1200 - speed*4) + 1;
      } else {
        launchcountdown[c] --;
      }
    }
  }

  if( store.gSkyburst) {
    byte nsparks = random8( NUM_SPARKS / 2, NUM_SPARKS + 1);
    for( byte b = 0; b < nsparks; b++) {
      store.gBurstcolor = CHSV(random8(), 200, 100);
      gSparks[b].Skyburst( store.gBurstx, store.gBursty, store.gBurstyv, store.gBurstcolor, dim);
      store.gSkyburst = false;
    }
  }
  //EffectMath::blur2d(20);
}

bool EffectFireworks::fireworksRoutine(CRGB *leds, EffectWorker *param)
{

  fadeToBlackBy(leds, NUM_LEDS, valDim);
  sparkGen();
  //memset8( leds, 0, NUM_LEDS * 3);

  for (byte a = 0; a < cnt; a++) {
    gDot[a].Move(store, flashing);
    gDot[a].Draw(leds);
  }
  for( byte b = 0; b < NUM_SPARKS; b++) {
    gSparks[b].Move(store, flashing);
    gSparks[b].Draw(leds);
  }
    return true;
}

// ------------ Эффект "Тихий Океан"
//  "Pacifica" перенос кода kostyamat
//  Gentle, blue-green ocean waves.
//  December 2019, Mark Kriegsman and Mary Corey March.
//  For Dan.
// https://raw.githubusercontent.com/FastLED/FastLED/master/examples/Pacifica/Pacifica.ino
 bool EffectPacific::run(CRGB *ledarr, EffectWorker *opt)
{
    return pacificRoutine(*&ledarr, &*opt);
  return false;
}

// Add one layer of waves into the led array
void pacifica_one_layer(CRGB *leds, const TProgmemRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps(CRGB *leds)
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );

  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors(CRGB *leds)
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].blue = scale8( leds[i].blue,  145);
    leds[i].green= scale8( leds[i].green, 200);
    leds[i] |= CRGB( 2, 5, 7);
  }
}

bool EffectPacific::pacificRoutine(CRGB *leds, EffectWorker *param)
{
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / map(speed, 1, 255, 620, 60);
  uint32_t deltams2 = (deltams * speedfactor2) / map(speed, 1, 255, 620, 60);
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011,10,13));
  sCIStart2 -= (deltams21 * beatsin88(777,8,11));
  sCIStart3 -= (deltams1 * beatsin88(501,5,7));
  sCIStart4 -= (deltams2 * beatsin88(257,4,6));

  // Clear out the LED array to a dim background blue-green
  fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_one_layer(&*leds, pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301) );
  pacifica_one_layer(&*leds, pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
  pacifica_one_layer(&*leds, pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503));
  pacifica_one_layer(&*leds, pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601));

  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps(&*leds);

  // Deepen the blues and greens a bit
  pacifica_deepen_colors(&*leds);
  EffectMath::blur2d(20);
  return true;
}

#ifdef MIC_EFFECTS
//----- Эффект "Осциллограф" (c) kostyamat
String EffectOsc::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) gain = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectOsc::run(CRGB *ledarr, EffectWorker *opt) {
  if((millis() - lastrun ) <= (isMicOn() ? 15U : map(speed, speed <= 127 ? 1 : 128, speed <= 12 ? 128 : 255, 15, 60))) 
    return false;
  else {
    lastrun = millis();
  }
  //fpsmeter();
  return oscRoutine(*&ledarr, &*opt);
}

bool EffectOsc::oscRoutine(CRGB *leds, EffectWorker *param) {
  pointer = (float)myLamp.getMicScale() / _scaler;
  if (speed <= 127) {
    div = EffectMath::fmap(speed, 1., 127., 0.5, 4.);
    oscHV = HEIGHT;
    oscilLimit = WIDTH;
  } else{
    div = EffectMath::fmap(speed, 128., 255., 0.5, 4.);
    oscHV = WIDTH;
    oscilLimit = HEIGHT;
  }
  //memset8( leds, 0, NUM_LEDS * 3);
  fadeToBlackBy(leds, NUM_LEDS, 200);

  byte micPick = (isMicOn()? myLamp.getMicMaxPeak() : random8(200));
  color = CHSV(isMicOn()? myLamp.getMicFreq() : random(240), 255, scale == 1 ? 100 : constrain(micPick * EffectMath::fmap(scale, 1., 255., 1., 5.), 51, 255));
  
  for (float x = 0.; (uint8_t)x < oscHV; x += div) {
    if (speed <= 127)
      EffectMath::drawLineF(y[0], x, y[1], (x + div) >= oscHV ? oscHV - 1 : (x + div), color);
    else
      EffectMath::drawLineF(x, y[0], (x + div) >= oscHV ? oscHV - 1 : (x + div), y[1], color);

    y[0] = y[1];
    y[1] = EffectMath::fmap(
                          (isMicOn() ? analogRead(MIC_PIN) :  EffectMath::randomf(pointer - gain, pointer + gain)),
                          (double)gain,
                          pointer * 2. - (double)gain,
                          0., 
                          oscilLimit - 1);
    delayMicroseconds((uint16_t)(/*(isMicOn()? 1024U : 1568U)*/1024 * div));

  }

return true;
}
#endif

// ------ Эффект "Вышиванка" (с) проект Aurora "Munch"
void EffectMunch::load() {
  palettesload();
}

String EffectMunch::setDynCtrl(UIControl*_val){
  if(_val->getId()==4) rand = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  flag = rand ? false : true;
  return String();
}

 bool EffectMunch::run(CRGB *ledarr, EffectWorker *opt) {
   if (dryrun(2.0))
    return false;
  return munchRoutine(*&ledarr, &*opt);
}

bool EffectMunch::munchRoutine(CRGB *leds, EffectWorker *param) {
  fadeToBlackBy(leds, NUM_LEDS, 200);
  if (flag) rand = beatsin8(5, 0, 8); // Хрень, конечно, но хоть какое-то разнообразие.
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT; y++) {
      leds[myLamp.getPixelNumber(x, y)] = (x ^ y ^ flip) < count ? ColorFromPalette(*curPalette, ((x ^ y) << rand) + generation) : CRGB::Black;
    }
  }

  count += dir;

  if (count <= 0 || count >= mic[0]) {
    dir = -dir;
    if (count <= 0) {
      mic[0] = mic[1];
      if (flip == 0)
        flip = mic[1] - 1;
      else
        flip = 0;
    }
  }

  generation++;
#ifdef MIC_EFFECTS
  mic[1] = isMicOn() ? map(myLamp.getMicMapMaxPeak(), 0, 255, 0, WIDTH) : WIDTH;
#else
  mic[1] = WIDTH;
#endif
  return true;
}

// ------ Эффект "Цветной шум" (с) https://gist.github.com/StefanPetrick/c856b6d681ec3122e5551403aabfcc68
DEFINE_GRADIENT_PALETTE( pit ) {
  0,     3,   3,   3,
  64,   13,   13, 255,  //blue
  128,   3,   3,   3,
  192, 255, 130,   3 ,  //orange
  255,   3,   3,   3
};
bool EffectNoise::noiseRoutine(CRGB *leds, EffectWorker *param) {
  EffectMath::dimAll(200U);
    uint8_t layer = 0;

  CRGBPalette16 Pal( pit );

  //modulate the position so that it increases/decreases x
  //(here based on the top left pixel - it could be any position else)
  //the factor "2" defines the max speed of the x movement
  //the "-255" defines the median moving direction
  x[layer] = x[layer] + (map(speed, 1U, 255U, 2U, 16U ) * noise[layer][0][0]) - 255U;
  //modulate the position so that it increases/decreases y
  //(here based on the top right pixel - it could be any position else)
  y[layer] = y[layer] + (map(speed, 1U, 255U, 2U, 16U ) * noise[layer][WIDTH - 1][0]) - 255U;
  //z just in one direction but with the additional "1" to make sure to never get stuck
  //in case the movement is stopped by a crazy parameter (noise data) combination
  //(here based on the down left pixel - it could be any position else)
  z[layer] += 1 + ((noise[layer][0][HEIGHT - 1]) / 4);
  //set the scaling based on left and right pixel of the middle line
  //here you can set the range of the zoom in both dimensions
  scale_x[layer] = 8000 + (noise[layer][0][CentreY] * 16);
  scale_y[layer] = 8000 + (noise[layer][WIDTH - 1][CentreY] * 16);

  //calculate the noise data
  for (uint8_t i = 0; i < WIDTH; i++) {
    uint32_t ioffset = scale_x[layer] * (i - CentreX);
    for (uint8_t j = 0; j < HEIGHT; j++) {
      uint32_t joffset = scale_y[layer] * (j - CentreY);
      uint16_t data = inoise16(x[layer] + ioffset, y[layer] + joffset, z[layer]);
      // limit the 16 bit results to the interesting range
      if (data < 11000) data = 11000;
      if (data > 51000) data = 51000;
      // normalize
      data = data - 11000;
      // scale down that the result fits into a byte
      data = data / 161;
      // store the result in the array
      noise[layer][i][j] = data;
    }
  }

  //map the colors
  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      //I will add this overlay CRGB later for more colors
      //it´s basically a rainbow mapping with an inverted brightness mask
      CRGB overlay;
      if (palettepos == 14) overlay = CHSV(160,255 - noise[layer][x][y], noise[layer][WIDTH - 1][HEIGHT - 1] + noise[layer][x][y]);
      else overlay = CHSV(noise[layer][y][x], 255, noise[layer][x][y]);
      //here the actual colormapping happens - note the additional colorshift caused by the down right pixel noise[layer][15][15]
      if (palettepos == 4) EffectMath::drawPixelXYF(x, HEIGHT - y, CHSV(160, 0 , noise[layer][x][y]), 35);
      else leds[myLamp.getPixelNumber(x, y)] = ColorFromPalette(palettepos > 0 ? *curPalette : Pal, noise[layer][WIDTH - 1][HEIGHT - 1] + noise[layer][x][y]) + overlay;
    }
  }

  //make it looking nice
  if (palettepos != 4) {
    adjust_gamma(*&leds);
    EffectMath::blur2d(32);
  } else EffectMath::blur2d(48);
  //and show it!
 return true;
}

bool EffectNoise::run(CRGB *ledarr, EffectWorker *opt) {

  return noiseRoutine(*&ledarr, &*opt);
}

void EffectNoise::load() {
  palettesload();
}

// ---- Эффект "Мотыльки"
// (с) Сотнег, https://community.alexgyver.ru/threads/wifi-lampa-budilnik-obsuzhdenie-proekta.1411/post-49262
bool EffectButterfly::run(CRGB *ledarr, EffectWorker *opt) {
  return butterflyRoutine(*&ledarr, &*opt);
}

void EffectButterfly::load()
{
  for (uint8_t i = 0U; i < BUTTERFLY_MAX_COUNT; i++)
  {
    butterflysPosX[i] = random8(WIDTH);
    butterflysPosY[i] = random8(HEIGHT);
    butterflysSpeedX[i] = 0;
    butterflysSpeedY[i] = 0;
    butterflysTurn[i] = 0;
    butterflysColor[i] = (isColored) ? random8() : 0U;
    butterflysBrightness[i] = 255U;
  }
}

String EffectButterfly::setDynCtrl(UIControl*_val)
{
  if(_val->getId()==3) cnt = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) wings = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) {
    isColored = EffectCalc::setDynCtrl(_val).toInt();
    isColored = !isColored;

    for (uint8_t i = 0U; i < BUTTERFLY_MAX_COUNT; i++)
    {
      butterflysColor[i] = (isColored) ? random8() : 0U;
    }
  }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectButterfly::butterflyRoutine(CRGB *leds, EffectWorker *param)
{
  byte _scale = cnt;
  byte _speed = speed;

  if (isColored) // для режима смены цвета фона фиксируем количество мотыльков
    deltaValue = (_scale > BUTTERFLY_MAX_COUNT) ? BUTTERFLY_MAX_COUNT : _scale;
  else
    deltaValue = BUTTERFLY_FIX_COUNT;

  if (_scale != 1U and !isColored)
    hue = map(_scale, 2, BUTTERFLY_MAX_COUNT + 1U, 0, 255);
  hue2 = (_scale == 1U) ? 100U : 190U; // вычисление базового оттенка

  if (wings && isColored)
    //EffectMath::dimAll(35U); // для крылышков
    fadeToBlackBy(leds, NUM_LEDS, 200);
  else
    FastLED.clear();

  float maxspeed;
  uint8_t tmp;
  float speedfactor = (float)_speed / 2048.0f + 0.001f;
  if (++step >= deltaValue)
    step = 0U;
  for (uint8_t i = 0U; i < deltaValue; i++)
  {
    butterflysPosX[i] += butterflysSpeedX[i]*speedfactor;
    butterflysPosY[i] += butterflysSpeedY[i]*speedfactor;

    if (butterflysPosX[i] < 0)
      butterflysPosX[i] = (float)(WIDTH - 1) + butterflysPosX[i];
    if (butterflysPosX[i] > WIDTH - 1)
      butterflysPosX[i] = butterflysPosX[i] + 1 - WIDTH;

    if (butterflysPosY[i] < 0)
    {
      butterflysPosY[i] = -butterflysPosY[i];
      butterflysSpeedY[i] = -butterflysSpeedY[i];
      //butterflysSpeedX[i] = -butterflysSpeedX[i];
    }
    if (butterflysPosY[i] > HEIGHT - 1U)
    {
      butterflysPosY[i] = (HEIGHT << 1U) - 2U - butterflysPosY[i];
      butterflysSpeedY[i] = -butterflysSpeedY[i];
      //butterflysSpeedX[i] = -butterflysSpeedX[i];
    }

    //проворот траектории
    maxspeed = fabs(butterflysSpeedX[i])+fabs(butterflysSpeedY[i]); // максимальная суммарная скорость
    if (maxspeed == fabs(butterflysSpeedX[i] + butterflysSpeedY[i]))
      {
          if (butterflysSpeedX[i] > 0) // правый верхний сектор вектора
          {
            butterflysSpeedX[i] += butterflysTurn[i];
            if (butterflysSpeedX[i] > maxspeed) // если вектор переехал вниз
              {
                butterflysSpeedX[i] = maxspeed + maxspeed - butterflysSpeedX[i];
                butterflysSpeedY[i] = butterflysSpeedX[i] - maxspeed;
              }
            else
              butterflysSpeedY[i] = maxspeed - fabs(butterflysSpeedX[i]);
          }
          else                           // левый нижний сектор
          {
            butterflysSpeedX[i] -= butterflysTurn[i];
            if (butterflysSpeedX[i] + maxspeed < 0) // если вектор переехал вверх
              {
                butterflysSpeedX[i] = 0 - butterflysSpeedX[i] - maxspeed - maxspeed;
                butterflysSpeedY[i] = maxspeed - fabs(butterflysSpeedX[i]);
              }
            else
              butterflysSpeedY[i] = fabs(butterflysSpeedX[i]) - maxspeed;
          }
      }
    else //левый верхний и правый нижний секторы вектора
      {
          if (butterflysSpeedX[i] > 0) // правый нижний сектор
          {
            butterflysSpeedX[i] -= butterflysTurn[i];
            if (butterflysSpeedX[i] > maxspeed) // если вектор переехал наверх
              {
                butterflysSpeedX[i] = maxspeed + maxspeed - butterflysSpeedX[i];
                butterflysSpeedY[i] = maxspeed - butterflysSpeedX[i];
              }
            else
              butterflysSpeedY[i] = fabs(butterflysSpeedX[i]) - maxspeed;
          }
          else                           // левый верхний сектор
          {
            butterflysSpeedX[i] += butterflysTurn[i];
            if (butterflysSpeedX[i] + maxspeed < 0) // если вектор переехал вниз
              {
                butterflysSpeedX[i] = 0 - butterflysSpeedX[i] - maxspeed - maxspeed;
                butterflysSpeedY[i] = 0 - butterflysSpeedX[i] - maxspeed;
              }
            else
              butterflysSpeedY[i] = maxspeed - fabs(butterflysSpeedX[i]);
          }
      }

    if (butterflysBrightness[i] == 255U)
    {
      if (step == i && random8(2U) == 0U)//(step == 0U && ((pcnt + i) & 0x01))
      {
        butterflysBrightness[i] = random8(220U,244U);
        butterflysSpeedX[i] = (float)random8(101U) / 20.0f + 1.0f;
        if (random8(2U) == 0U) butterflysSpeedX[i] = -butterflysSpeedX[i];
        butterflysSpeedY[i] = (float)random8(101U) / 20.0f + 1.0f;
        if (random8(2U) == 0U) butterflysSpeedY[i] = -butterflysSpeedY[i];
        // проворот траектории
        //butterflysTurn[i] = (float)random8((fabs(butterflysSpeedX[i])+fabs(butterflysSpeedY[i]))*2.0+2.0) / 40.0f;
        butterflysTurn[i] = (float)random8((fabs(butterflysSpeedX[i])+fabs(butterflysSpeedY[i]))*20.0f+2.0f) / 200.0f;
        if (random8(2U) == 0U) butterflysTurn[i] = -butterflysTurn[i];
      }
    }
    else
    {
      if (step == i)
        butterflysBrightness[i]++;
      tmp = 255U - butterflysBrightness[i];
      if (tmp == 0U || ((uint16_t)(butterflysPosX[i] * tmp) % tmp == 0U && (uint16_t)(butterflysPosY[i] * tmp) % tmp == 0U))
      {
        butterflysPosX[i] = round(butterflysPosX[i]);
        butterflysPosY[i] = round(butterflysPosY[i]);
        butterflysSpeedX[i] = 0;
        butterflysSpeedY[i] = 0;
        butterflysTurn[i] = 0;
        butterflysBrightness[i] = 255U;
      }
    }

    if (wings)
      EffectMath::drawPixelXYF(butterflysPosX[i], butterflysPosY[i], CHSV(butterflysColor[i], 255U, (butterflysBrightness[i] == 255U) ? 255U : 128U + random8(2U) * 111U)); // это процедура рисования с нецелочисленными координатами. ищите её в прошивке
    else
      EffectMath::drawPixelXYF(butterflysPosX[i], butterflysPosY[i], CHSV(butterflysColor[i], 255U, butterflysBrightness[i])); // это процедура рисования с нецелочисленными координатами. ищите её в прошивке
  }

  // постобработка кадра
  if (isColored){
    for (uint8_t i = 0U; i < deltaValue; i++) // ещё раз рисуем всех Мотыльков, которые "сидят на стекле"
      if (butterflysBrightness[i] == 255U)
        EffectMath::drawPixelXY(butterflysPosX[i], butterflysPosY[i], CHSV(butterflysColor[i], 255U, butterflysBrightness[i]));
  }
  else {
    //теперь инверсия всей матрицы
    if (_scale == 1U)
      if (++deltaHue == 0U) hue++;
    for (uint16_t i = 0U; i < NUM_LEDS; i++)
      leds[i] = CHSV(hue, hue2, 255U - leds[i].r);
  }
  return true;
}

// ---- Эффект "Тени"
// https://github.com/vvip-68/GyverPanelWiFi/blob/master/firmware/GyverPanelWiFi_v1.02/effects.ino
bool EffectShadows::run(CRGB *ledarr, EffectWorker *opt) {
  return shadowsRoutine(*&ledarr, &*opt);
}

bool EffectShadows::shadowsRoutine(CRGB *leds, EffectWorker *param) {

  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 225), (40 * 256));
#ifdef MIC_EFFECTS
  uint8_t msmultiplier = isMicOn() ? myLamp.getMicMapMaxPeak() : getCtrlVal(3) == "false" ? beatsin88(map(speed, 1, 255, 100, 255), 32, map(speed, 1, 255, 60, 255)) : speed; // beatsin88(147, 32, 60);
  byte effectBrightness = isMicOn() ? myLamp.getMicMapMaxPeak() * 1.5f : scale;
#else
  uint8_t msmultiplier = getCtrlVal(3) == "false" ? beatsin88(map(speed, 1, 255, 100, 255), 32, map(speed, 1, 255, 60, 255)) : speed; // beatsin88(147, 32, 60);
  byte effectBrightness = scale;
#endif
  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;

  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;

  for( uint16_t i = 0 ; i < NUM_LEDS; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768U;

    uint32_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536U;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536U;
    bri8 += (255 - brightdepth);

    CRGB newcolor = CHSV( hue8, sat8, map8(bri8, map(effectBrightness, 1, 255, 32, 125), map(effectBrightness, 1, 255, 125, 250)));

    uint16_t pixelnumber = i;
    pixelnumber = (NUM_LEDS-1) - pixelnumber;

    nblend( leds[pixelnumber], newcolor, 64);
  }
  return true;
}

// ---- Эффект "Узоры"
// (c) kostyamat (Kostyantyn Matviyevskyy) 2020
// переделано kDn
// идея https://github.com/vvip-68/GyverPanelWiFi/blob/master/firmware/GyverPanelWiFi_v1.02/patterns.ino
String EffectPatterns::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) _speed = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) _sc = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==6) _subpixel = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectPatterns::run(CRGB *ledarr, EffectWorker *opt) {
  return patternsRoutine(*&ledarr, &*opt);
}

void EffectPatterns::drawPicture_XY() {
  float vx, vy, f;
  vx = modff(xsin, &f);
  vy = modff(ysin, &f);

  //FastLED.clear();
  EffectMath::dimAll(127);

  for (uint16_t x = 0; x < WIDTH; x++)
  {
    for (uint16_t y = 0; y < HEIGHT; y++)
    {
      byte in = buff[myLamp.getPixelNumberBuff((int)(xsin + x) % 20U, (int)(ysin + y) % 20U, 20U, 20U)];
      CHSV color2 = colorMR[in];

      if(_subpixel){
        if(_speed==33)
          EffectMath::drawPixelXYF_X(((float)x-vx), (float)(HEIGHT-1)-((float)y-vy), color2, 0);
        else if(_scale==33)
          EffectMath::drawPixelXYF_Y(((float)x-vx), (float)(HEIGHT-1)-((float)y-vy), color2, 0);
        else{
            EffectMath::drawPixelXYF(((float)x-vx), (float)(HEIGHT-1)-((float)y-vy), color2, 0);
        }
      } else {
        EffectMath::drawPixelXY(x, (HEIGHT-1)-y, color2);
      }
    }
  }
  for (uint16_t x = 0; x < WIDTH; x++){
    for (uint16_t y = 0; y < HEIGHT; y++){ // лучше вывод рамки фоном, чем мерцание по краю картинки, доп. цикл из-за особенностей сабпикселя
      if(_subpixel && _speed!=33 && _scale!=33 && (x==WIDTH-1 || x==0 || y==0 || y==HEIGHT-1)){
        EffectMath::drawPixelXY(x, (float)(HEIGHT-1)-y, colorMR[6]);
      }
    }
  }
}

void EffectPatterns::load() {
  if (_sc == 0)
    patternIdx = random(0, MAX_PATTERN);
   // Цвета с индексом 6 и 7 - случайные, определяются в момент настройки эффекта
  colorMR[6] = CHSV(random8(), 255U, 255U);
  colorMR[7].hue = colorMR[6].hue + 96; //(beatsin8(1, 0, 255, 0, 127), 255U, 255U);
}


bool EffectPatterns::patternsRoutine(CRGB *leds, EffectWorker *param)
{
  _speedX = EffectMath::fmap(_scale, 1, 65, -0.75, 0.75);
  _speedY = EffectMath::fmap(_speed, 1, 65, -0.75, 0.75);

  xsin += _speedX;
  ysin += _speedY;

  int8_t chkIdx = patternIdx;
  if (_sc == 0) {
    EVERY_N_SECONDS(5) {
      patternIdx ++;
      if (patternIdx >= MAX_PATTERN) patternIdx = 0;
    }
  } else patternIdx = _sc%(sizeof(patterns)/sizeof(Pattern));

  if(chkIdx != patternIdx){
    for (byte x = 0; x < 20U; x++)
    {
      for (byte y = 0; y < 20U; y++)
      {
        buff[myLamp.getPixelNumberBuff(x, y, 20U, 20U)] = (pgm_read_byte(&patterns[patternIdx][y % 10U][x % 10U]));
      }
    }
  }

  colorMR[6] = CHSV(beatsin88(EffectMath::fmap((fabs(_speedX) + fabs(_speedY)), 0.1, 1.5, 350., 1200.), 0, 255), 255, 255);
  colorMR[7].hue = colorMR[6].hue + 96; //(beatsin8(1, 0, 255, 0, 127), 255U, 255U);
  colorMR[7].sat = beatsin88(EffectMath::fmap((fabs(_speedX) + fabs(_speedY)), 0.1, 1.5, 150, 900), 0, 255);
  colorMR[7].val = beatsin88(EffectMath::fmap((fabs(_speedX) + fabs(_speedY)), 0.1, 1.5, 450, 1300), 0, 255);
  drawPicture_XY();

  return true;
}

// ***************************** "Стрелки" *****************************
void EffectArrows::load(){
    FastLED.clear();
    arrow_complete = false;
    arrow_mode_orig = _scale-1;
    arrow_mode = arrow_mode_orig == 0 ? random8(1,5) : arrow_mode_orig;
    arrow_play_mode_count_orig[0] = 0;
    arrow_play_mode_count_orig[1] = 4;  // 4 фазы - все стрелки показаны по кругу один раз - переходить к следующему ->
    arrow_play_mode_count_orig[2] = 4;  // 2 фазы - гориз к центру (1), затем верт к центру (2) - обе фазы повторить по 2 раза -> 4
    arrow_play_mode_count_orig[3] = 4;  // 1 фаза - все к центру (1) повторить по 4 раза -> 4
    arrow_play_mode_count_orig[4] = 4;  // 2 фазы - гориз к центру (1), затем верт к центру (2) - обе фазы повторить по 2 раза -> 4
    arrow_play_mode_count_orig[5] = 4;  // 1 фаза - все сразу (1) повторить по 4 раза -> 4
    for (byte i=0; i<6; i++) {
      arrow_play_mode_count[i] = arrow_play_mode_count_orig[i];
    }
    arrowSetupForMode(arrow_mode, true);
}

String EffectArrows::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) { _scale = EffectCalc::setDynCtrl(_val).toInt(); load();}
  else if(_val->getId()==4) subpixel = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectArrows::run(CRGB *ledarr, EffectWorker *opt) {
  if (_scale == 1) {
    EVERY_N_SECONDS((3000U / speed))
    {
      load();
    }
  }
  speedfactor = (float)speed / 768.0 + 0.15;
  return arrowsRoutine(*&ledarr, &*opt);
}

bool EffectArrows::arrowsRoutine(CRGB *leds, EffectWorker *param) {
    prevVal[0] += speedfactor;
    if((byte)prevVal[0] == (byte)prevVal[1]) {
      prevVal[1] = (byte)prevVal[1]+ 1U;
      EffectMath::fader(65);
      //fadeToBlackBy(leds, NUM_LEDS, 50);
    }

  CHSV color;

  // движение стрелки - cлева направо
  if ((arrow_direction & 0x01) > 0) {
    color = CHSV(arrow_hue[0], 255, 255);
    for (float x = 0.0f; x <= 4; x+= speedfactor) {
      for (float y = 0.0f; y <= x; y+= speedfactor) {
        if (subpixel ? arrow_x[0] - x >= 0 && arrow_x[0] - x <= stop_x[0] : (byte)arrow_x[0] - (byte)x >= 0 && (byte)arrow_x[0] - (byte)x <= (byte)stop_x[0]) {
          CHSV clr = ((byte)x < 4 || ((byte)x == 4 && (byte)y < 2)) ? color : CHSV(0,0,0);
          if (subpixel)
          {
            EffectMath::drawPixelXYF(arrow_x[0] - x, arrow_y[0] - y, clr);
            EffectMath::drawPixelXYF(arrow_x[0] - x, arrow_y[0] + y, clr);
          }
          else
          {
            EffectMath::drawPixelXY((byte)(arrow_x[0] - x), (byte)(arrow_y[0] - y), clr);
            EffectMath::drawPixelXY((byte)(arrow_x[0] - x), (byte)(arrow_y[0] + y), clr);
          }
        }
      }
    }
    arrow_x[0]+= speedfactor;
  }

  // движение стрелки - cнизу вверх
  if ((arrow_direction & 0x02) > 0) {
    color = CHSV(arrow_hue[1], 255, 255);
    for (float y = 0.0f; y <= 4; y+= speedfactor) {
      for (float x = 0.0f; x <= y; x+= speedfactor) {
        if (subpixel ? arrow_y[1] - y >= 0 && arrow_y[1] - y <= stop_y[1] : (byte)arrow_y[1] - (byte)y >= 0 && (byte)arrow_y[1] - (byte)y <= (byte)stop_y[1]) {
          CHSV clr = ((byte)y < 4 || ((byte)y == 4 && (byte)x < 2)) ? color : CHSV(0,0,0);
          if (subpixel)
          {
            EffectMath::drawPixelXYF(arrow_x[1] - x, arrow_y[1] - y, clr);
            EffectMath::drawPixelXYF(arrow_x[1] + x, arrow_y[1] - y, clr);
          }
          else
          {
            EffectMath::drawPixelXY((byte)(arrow_x[1] - x), (byte)(arrow_y[1] - y), clr);
            EffectMath::drawPixelXY((byte)(arrow_x[1] + x), (byte)(arrow_y[1] - y), clr);
          }
        }
      }
    }
    arrow_y[1]+= speedfactor;
  }

  // движение стрелки - cправа налево
  if ((arrow_direction & 0x04) > 0) {
    color = CHSV(arrow_hue[2], 255, 255);
    for (float x = 0.0f; x <= 4; x+= speedfactor) {
      for (float y = 0.0f; y <= x; y+= speedfactor) {
        if (subpixel ? arrow_x[2] + x >= stop_x[2] && arrow_x[2] + x < WIDTH : (byte)arrow_x[2] + (byte)x >= (byte)stop_x[2] && (byte)arrow_x[2] + (byte)x < WIDTH) {
          CHSV clr = ((byte)x < 4 || ((byte)x == 4 && (byte)y < 2)) ? color : CHSV(0,0,0);
          if (subpixel)
          {
            EffectMath::drawPixelXYF(arrow_x[2] + x, arrow_y[2] - y, clr);
            EffectMath::drawPixelXYF(arrow_x[2] + x, arrow_y[2] + y, clr);
          }
          else
          {
            EffectMath::drawPixelXY((byte)(arrow_x[2] + x), (byte)(arrow_y[2] - y), clr);
            EffectMath::drawPixelXY((byte)(arrow_x[2] + x), (byte)(arrow_y[2] + y), clr);
          }
        }
      }
    }
    arrow_x[2]-= speedfactor;
  }

  // движение стрелки - cверху вниз
  if ((arrow_direction & 0x08) > 0) {
    color = CHSV(arrow_hue[3], 255, 255);
    for (float y = 0.0f; y <= 4; y+= speedfactor) {
      for (float x = 0.0f; x <= y; x+= speedfactor) {
        if (subpixel ? arrow_y[3] + y >= stop_y[3] && arrow_y[3] + y < HEIGHT : (byte)arrow_y[3] + (byte)y >= (byte)stop_y[3] && (byte)arrow_y[3] + (byte)y < HEIGHT) {
          CHSV clr = ((byte)y < 4 || ((byte)y == 4 && (byte)x < 2)) ? color : CHSV(0,0,0);
          if (subpixel)
          {
            EffectMath::drawPixelXYF(arrow_x[3] - x, arrow_y[3] + y, clr);
            EffectMath::drawPixelXYF(arrow_x[3] + x, arrow_y[3] + y, clr);
          }
          else
          {
            EffectMath::drawPixelXY((byte)(arrow_x[3] - x), (byte)(arrow_y[3] + y), clr);
            EffectMath::drawPixelXY((byte)(arrow_x[3] + x), (byte)(arrow_y[3] + y), clr);
          }
        }
      }
    }
    arrow_y[3]-= speedfactor;
  }

  // Проверка завершения движения стрелки, переход к следующей фазе или режиму

  switch (arrow_mode) {

    case 1:
      // Последовательно - слева-направо -> снизу вверх -> справа налево -> сверху вниз и далее по циклу
      // В каждый сомент времени сктивна только одна стрелка, если она дошла до края - переключиться на следующую и задать ее начальные координаты
      arrow_complete = false;
      switch (arrow_direction) {
        case 1: arrow_complete = arrow_x[0] > stop_x[0]; break;
        case 2: arrow_complete = arrow_y[1] > stop_y[1]; break;
        case 4: arrow_complete = arrow_x[2] < stop_x[2]; break;
        case 8: arrow_complete = arrow_y[3] < stop_y[3]; break;
      }

      arrow_change_mode = false;
      if (arrow_complete) {
        arrow_direction = (arrow_direction << 1) & 0x0F;
        if (arrow_direction == 0) arrow_direction = 1;
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[1]--;
          if (arrow_play_mode_count[1] == 0) {
            arrow_play_mode_count[1] = arrow_play_mode_count_orig[1];
            arrow_mode = random8(1,5);
            arrow_change_mode = true;
          }
        }

        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;

    case 2:
      // Одновременно горизонтальные навстречу до половины экрана
      // Затем одновременно вертикальные до половины экрана. Далее - повторять
      arrow_complete = false;
      switch (arrow_direction) {
        case  5: arrow_complete = arrow_x[0] > stop_x[0]; break;   // Стрелка слева и справа встречаются в центре одновременно - проверять только стрелку слева
        case 10: arrow_complete = arrow_y[1] > stop_y[1]; break;   // Стрелка снизу и сверху встречаются в центре одновременно - проверять только стрелку снизу
      }

      arrow_change_mode = false;
      if (arrow_complete) {
        arrow_direction = arrow_direction == 5 ? 10 : 5;
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[2]--;
          if (arrow_play_mode_count[2] == 0) {
            arrow_play_mode_count[2] = arrow_play_mode_count_orig[2];
            arrow_mode = random8(1,5);
            arrow_change_mode = true;
          }
        }

        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;

    case 3:
      // Одновременно со всех сторон к центру
      // Завершение кадра режима - когда все стрелки собрались в центре.
      // Проверять стрелки по самой длинной стороне
      if (WIDTH >= HEIGHT)
        arrow_complete = arrow_x[0] > stop_x[0];
      else
        arrow_complete = arrow_y[1] > stop_y[1];

      arrow_change_mode = false;
      if (arrow_complete) {
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[3]--;
          if (arrow_play_mode_count[3] == 0) {
            arrow_play_mode_count[3] = arrow_play_mode_count_orig[3];
            arrow_mode = random8(1,5);
            arrow_change_mode = true;
          }
        }

        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;

    case 4:
      // Одновременно слева/справа от края до края со смещением горизонтальной оси на 1/3 высоты, далее
      // одновременно снизу/сверху от края до края со смещением вертикальной оси на 1/3 ширины
      // Завершение кадра режима - когда все стрелки собрались в центре.
      // Проверять стрелки по самой длинной стороне
      switch (arrow_direction) {
        case  5: arrow_complete = arrow_x[0] > stop_x[0]; break;   // Стрелка слева и справа движутся и достигают края одновременно - проверять только стрелку слева
        case 10: arrow_complete = arrow_y[1] > stop_y[1]; break;   // Стрелка снизу и сверху движутся и достигают края одновременно - проверять только стрелку снизу
      }

      arrow_change_mode = false;
      if (arrow_complete) {
        arrow_direction = arrow_direction == 5 ? 10 : 5;
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[4]--;
          if (arrow_play_mode_count[4] == 0) {
            arrow_play_mode_count[4] = arrow_play_mode_count_orig[4];
            arrow_mode = random8(1,5);
            arrow_change_mode = true;
          }
        }

        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;

    case 5:
      // Одновременно со всех сторон от края до края со смещением горизонтальной оси на 1/3 высоты, далее
      // Проверять стрелки по самой длинной стороне
      if (WIDTH >= HEIGHT)
        arrow_complete = arrow_x[0] > stop_x[0];
      else
        arrow_complete = arrow_y[1] > stop_y[1];

      arrow_change_mode = false;
      if (arrow_complete) {
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[5]--;
          if (arrow_play_mode_count[5] == 0) {
            arrow_play_mode_count[5] = arrow_play_mode_count_orig[5];
            arrow_mode = random8(1,5);
            arrow_change_mode = true;
          }
        }

        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;
  }
  return true;
}

void EffectArrows::arrowSetupForMode(byte mode, bool change) {
    switch (mode) {
      case 1:
        if (change) arrow_direction = 1;
        arrowSetup_mode1();    // От края матрицы к краю, по центру гориз и верт
        break;
      case 2:
        if (change) arrow_direction = 5;
        arrowSetup_mode2();    // По центру матрицы (гориз / верт) - ограничение - центр матрицы
        break;
      case 3:
        if (change) arrow_direction = 15;
        arrowSetup_mode2();    // как и в режиме 2 - по центру матрицы (гориз / верт) - ограничение - центр матрицы
        break;
      case 4:
        if (change) arrow_direction = 5;
        arrowSetup_mode4();    // От края матрицы к краю, верт / гориз
        break;
      case 5:
        if (change) arrow_direction = 15;
        arrowSetup_mode4();    // как и в режиме 4 от края матрицы к краю, на 1/3
        break;
    }
}
void EffectArrows::arrowSetup_mode1() {
  // Слева направо
  if ((arrow_direction & 0x01) > 0) {
    arrow_hue[0] = random8();
    arrow_x[0] = 0;
    arrow_y[0] = HEIGHT / 2;
    stop_x [0] = WIDTH + 7;      // скрывается за экраном на 7 пикселей
    stop_y [0] = 0;              // неприменимо
  }
  // снизу вверх
  if ((arrow_direction & 0x02) > 0) {
    arrow_hue[1] = random8();
    arrow_y[1] = 0;
    arrow_x[1] = WIDTH / 2;
    stop_y [1] = HEIGHT + 7;     // скрывается за экраном на 7 пикселей
    stop_x [1] = 0;              // неприменимо
  }
  // справа налево
  if ((arrow_direction & 0x04) > 0) {
    arrow_hue[2] = random8();
    arrow_x[2] = WIDTH - 1;
    arrow_y[2] = HEIGHT / 2;
    stop_x [2] = -7;             // скрывается за экраном на 7 пикселей
    stop_y [2] = 0;              // неприменимо
  }
  // сверху вниз
  if ((arrow_direction & 0x08) > 0) {
    arrow_hue[3] = random8();
    arrow_y[3] = HEIGHT - 1;
    arrow_x[3] = WIDTH / 2;
    stop_y [3] = -7;             // скрывается за экраном на 7 пикселей
    stop_x [3] = 0;              // неприменимо
  }
}

void EffectArrows::arrowSetup_mode2() {
  // Слева направо до половины экрана
  if ((arrow_direction & 0x01) > 0) {
    arrow_hue[0] = random8();
    arrow_x[0] = 0;
    arrow_y[0] = HEIGHT / 2;
    stop_x [0] = WIDTH / 2 - 1;  // до центра экрана
    stop_y [0] = 0;              // неприменимо
  }
  // снизу вверх до половины экрана
  if ((arrow_direction & 0x02) > 0) {
    arrow_hue[1] = random8();
    arrow_y[1] = 0;
    arrow_x[1] = WIDTH / 2;
    stop_y [1] = HEIGHT / 2 - 1; // до центра экрана
    stop_x [1] = 0;              // неприменимо
  }
  // справа налево до половины экрана
  if ((arrow_direction & 0x04) > 0) {
    arrow_hue[2] = random8();
    arrow_x[2] = WIDTH - 1;
    arrow_y[2] = HEIGHT / 2;
    stop_x [2] = WIDTH / 2;      // до центра экрана
    stop_y [2] = 0;              // неприменимо
  }
  // сверху вниз до половины экрана
  if ((arrow_direction & 0x08) > 0) {
    arrow_hue[3] = random8();
    arrow_y[3] = HEIGHT - 1;
    arrow_x[3] = WIDTH / 2;
    stop_y [3] = HEIGHT / 2;     // до центра экрана
    stop_x [3] = 0;              // неприменимо
  }
}

void EffectArrows::arrowSetup_mode4() {
  // Слева направо
  if ((arrow_direction & 0x01) > 0) {
    arrow_hue[0] = random8();
    arrow_x[0] = 0;
    arrow_y[0] = (HEIGHT / 3) * 2;
    stop_x [0] = WIDTH + 7;      // скрывается за экраном на 7 пикселей
    stop_y [0] = 0;              // неприменимо
  }
  // снизу вверх
  if ((arrow_direction & 0x02) > 0) {
    arrow_hue[1] = random8();
    arrow_y[1] = 0;
    arrow_x[1] = (WIDTH / 3) * 2;
    stop_y [1] = HEIGHT + 7;     // скрывается за экраном на 7 пикселей
    stop_x [1] = 0;              // неприменимо
  }
  // справа налево
  if ((arrow_direction & 0x04) > 0) {
    arrow_hue[2] = random8();
    arrow_x[2] = WIDTH - 1;
    arrow_y[2] = HEIGHT / 3;
    stop_x [2] = -7;             // скрывается за экраном на 7 пикселей
    stop_y [2] = 0;              // неприменимо
  }
  // сверху вниз
  if ((arrow_direction & 0x08) > 0) {
    arrow_hue[3] = random8();
    arrow_y[3] = HEIGHT - 1;
    arrow_x[3] = WIDTH / 3;
    stop_y [3] = -7;             // скрывается за экраном на 7 пикселей
    stop_x [3] = 0;              // неприменимо
  }
}

// ------ Эффект "Дикие шарики"
// (с) https://gist.github.com/bonjurroughs/9c107fa5f428fb01d484#file-noise-balls
 // EffectNBals::
bool EffectNBals::run(CRGB *ledarr, EffectWorker *opt) {
  return nballsRoutine(*&ledarr, &*opt);
}

String EffectNBals::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) beat2 = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectNBals::nballsRoutine(CRGB *leds, EffectWorker *param) {
  beat1 = map(speed, 1, 255, 8, 128);
  balls_timer();
  blur(*&leds);
  return true;
}

void EffectNBals::blur(CRGB *leds) {
  EffectMath::blur2d(beatsin8(2,0,60));
  // Use two out-of-sync sine waves
  // В общем те же фигуры Лиссажу, вид сбоку :), но выглядят хорошо
  uint8_t  i = beatsin8( beat1, 0, HEIGHT-1);
  uint8_t  j = beatsin8(fabs(beat1-beat2), 0, WIDTH-1);
  // Also calculate some reflections
  uint8_t ni = (WIDTH-1)-i;
  uint8_t nj = (WIDTH-1)-j;
  // The color of each point shifts over time, each at a different speed.
  uint16_t ms = millis();

  switch(balls){
  case 1:
    leds[myLamp.getPixelNumber(ni,nj)] += CHSV( ms / 17, 200, 255);
    break;
  case 3:
    leds[myLamp.getPixelNumber(ni,nj)] += CHSV( ms / 17, 200, 255);
    leds[myLamp.getPixelNumber(ni, j)] += CHSV( ms / 41, 200, 255);
    break;
  case 4:
    leds[myLamp.getPixelNumber(ni,nj)] += CHSV( ms / 17, 200, 255);
    leds[myLamp.getPixelNumber(ni, j)] += CHSV( ms / 41, 200, 255);
    leds[myLamp.getPixelNumber( i,nj)] += CHSV( ms / 37, 200, 255);
    leds[myLamp.getPixelNumber( i, j)] += CHSV( ms / 11, 200, 255);
    break;
  case 2:
    leds[myLamp.getPixelNumber(ni, j)] += CHSV( ms / 41, 200, 255);
    leds[myLamp.getPixelNumber( j, i)] += CHSV( ms / 13, 200, 255);
    break;
  }
}

void EffectNBals::balls_timer() {
  uint8_t secondHand = ((millis() / 1000)) % 60;

  if( lastSecond != secondHand) {
    lastSecond = secondHand;

    if(( secondHand == 30)||( secondHand == 0))  {
      balls += 1;
      if(balls > 4) {
        balls = 1;
      }
    }
  }
}

// ------ Эффект "Притяжение"
// Базовый Attract проект Аврора
bool EffectAttract::run(CRGB *ledarr, EffectWorker *opt) {
  return attractRoutine(*&ledarr, &*opt);
}

void EffectAttract::load() {
  palettesload();
  speedFactor = EffectMath::fmap((float)speed, 1., 255., 0.02, 1.);
  for (int i = 0; i < count; i++)
  {
    int direction = 1-2*random(0, 2); // -1 или 1
    Boid boid = Boid(15, 16 - i);
    boid.mass = (float)random(1, map(_mass, 1, 255, 128, 1024)) / 100.0f * speedFactor; //(1.0/speed);
    boid.velocity.x = (float)random(5, map(_energy, 1, 255, 16, 768)) / 500.0f; // * speedFactor; //(1.0/speed);
    boid.velocity.x *= direction;
    boid.velocity.y = 0;
    boid.colorIndex = i * 32;
    boid.location.x = EffectMath::randomf(0, WIDTH-1); // на малой скорости и максимальной энергии, почему-то формировалась только одна-две частицы,
    boid.location.y = EffectMath::randomf(0, HEIGHT-1); // остальные улетели за область экрана. И потом несколько минут добираються в область видимости.
    //Это вроде решает проблему
    boids[i] = boid;
  }

}

void EffectAttract::setspd(const byte _spd)
{
  EffectCalc::setspd(_spd);
  speedFactor = EffectMath::fmap((float)_spd, 1., 255., 0.02, 1.);
  setup();
}

void EffectAttract::setscl(const byte _scl)
{
  EffectCalc::setscl(_scl); // дернем базовый, где будет пересчет палитры
  _energy = scale; // получим внутренний коэф., ptPallete-palettescale == от меньшего к большему, palettescale - от большего к меньшему
  setup();
}

String EffectAttract::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) _mass = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  setup();
  return String();
}

void EffectAttract::setup(){
  for (int i = 0; i < count; i++)
  {
    int direction = 1-2*random(0, 2); // -1 или 1
    Boid boid = Boid(15, 16 - i);
    boid.mass = (float)random(1, map(_mass, 1, 255, 128, 1024)) / 100.0f * speedFactor; //(1.0/speed);
    boid.velocity.x = (float)random(5, map(_energy, 1, 255, 16, 768)) / 500.0f; // * speedFactor; //(1.0/speed);
    boid.velocity.x *= direction;
    boid.velocity.y = 0;
    boid.colorIndex = i * 32;
    boids[i] = boid;
  }
}

bool EffectAttract::attractRoutine(CRGB *leds, EffectWorker *param) {
  uint8_t dim = beatsin8(3, 170, 250);
  fadeToBlackBy(leds, NUM_LEDS, 255U - dim);

  for (int i = 0; i < count; i++) // count
  {

    Boid boid = boids[i];
    //boid.acceleration *= speedFactor/10;
    PVector force = attract(boid);
    boid.applyForce(force);

    boid.update();
    EffectMath::drawPixelXYF(boid.location.x, boid.location.y, ColorFromPalette(*curPalette, boid.colorIndex));

    boids[i] = boid;
  }
  return true;
}

//------------ Эффект "Змейки"
// вариант субпикселя и поведения от kDn
void EffectSnake::load() {
  palettesload();

  for(uint8_t i=0;i<MAX_SNAKES;i++){
    snakes[i].reset();
    snakes[i].pixels[0].x = WIDTH / 2; // пусть расползаются из центра
    snakes[i].pixels[0].y = HEIGHT / 2; // так будет интереснее
    snakes[i].direction = (EffectSnake::Direction)random(4);
    snakes[i].internal_speedf = ((random(2) ? 0.5 : 0.33)+1.0/(random(i+1)+1))+0.5;
  }
}

String EffectSnake::setDynCtrl(UIControl*_val) {
  if(_val->getId()==4) snakeCount = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) subPix = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==6) onecolor = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectSnake::snakeRoutine(CRGB *leds, EffectWorker *param) {
  speedFactor = (float)speed / 512.0 + 0.025;
  fadeToBlackBy(leds, NUM_LEDS, speed<25 ? 5 : speed/2 ); // длина хвоста будет зависеть от скорости
#ifdef MIC_EFFECTS
  hue+=(speedFactor/snakeCount+(isMicOn() ? myLamp.getMicMapFreq()/127.0 : 0));
#else
  hue+=speedFactor/snakeCount;
#endif
  hue = hue>255? hue-255 : hue;

  for (int i = snakeCount - 1; i >= 0; i--)
  {
    EffectSnake::Snake &snake = snakes[i];

    if(onecolor){
      fill_palette(colors, SNAKE_LENGTH, hue, 1, *curPalette, 255-(i*8), LINEARBLEND);
    } else {
      fill_palette(colors, SNAKE_LENGTH, (
        (speed<25 || speed>230) ? (i%2 ? hue : 255-hue) : (i%2 ? hue*(i+1) : (255-hue)*(i+1))
      ), 1, *curPalette, 255-(i*8), LINEARBLEND); // вообще в цикле заполнять палитры может быть немножко тяжело... но зато разнообразнее по цветам
    }
    snake.shuffleDown(speedFactor, subPix);

#ifdef MIC_EFFECTS
    if(myLamp.getMicMapMaxPeak()>speed/3.0+75.0 && isMicOn()) {
      snake.newDirection();
    } else if (random((speed<25)?speed*50:speed*10) < speed && !isMicOn()) {// как часто будут повороты :), логика загадочная, но на малой скорости лучше змейкам круги не наматывать :)
      snake.newDirection();
    }
#else
    if (random((speed<25)?speed*50:speed*10) < speed){ // как часто будут повороты :), логика загадочная, но на малой скорости лучше змейкам круги не наматывать :)
      snake.newDirection();
    }
#endif

    snake.move(speedFactor);
    snake.draw(colors, speedFactor, i, subPix, isDebug());
  }
  return true;
}

bool EffectSnake::run(CRGB *ledarr, EffectWorker *opt ) {
  return snakeRoutine(ledarr, &*opt);
}

void EffectSnake::Snake::draw(CRGB colors[SNAKE_LENGTH], float speedfactor, int snakenb, bool subpix, bool isDebug)
{
  int len= isDebug ? 1 : (int)SNAKE_LENGTH;
  for (int i = 0; i < len; i++) // (int)SNAKE_LENGTH
  {
    if(isDebug){ // тест сабпикселя
      FastLED.clear(); // врубаю очистку, чтобы видно было отрисовку одного пикселя (тестировать при сабпиксель отключено - иначе будет использован сабпиксель4)

      EffectMath::drawPixelXYF(9.21, 4.15, colors[i]);
      EffectMath::drawPixelXYF(7.22, 8.11, EffectMath::getPixColorXYF(9.21, 4.15));

      EffectMath::drawPixelXYF_X(11.21, 6, colors[i]);
      EffectMath::drawPixelXYF_X(9.22, 11, EffectMath::getPixColorXYF_X(11.21, 6));

      EffectMath::drawPixelXYF_Y(13, 8.15, colors[i]);
      EffectMath::drawPixelXYF_Y(11, 13.11, EffectMath::getPixColorXYF_Y(13, 8.15));
    }

    if (subpix){
      EffectMath::drawPixelXYF(pixels[i].x, pixels[i].y, colors[i]);
    }
    else {
      if(i!=0)
        EffectMath::drawPixelXY(pixels[i].x, pixels[i].y, colors[i]);
      else if(direction<LEFT)
        EffectMath::drawPixelXYF_Y(pixels[i].x, pixels[i].y, colors[i]);
      else
        EffectMath::drawPixelXYF_X(pixels[i].x, pixels[i].y, colors[i]);
    }
  }
}

//------------ Эффект "Nexus"
// (с) kostyamat 4.12.2020
void EffectNexus::reload() {
  for (byte i = 0; i < NEXUS; i++) {
    dotDirect[i] = random(0, 4);                     // задаем направление
    dotPosX[i] = random(0, WIDTH);                   // Разбрасываем частицы по ширине
    dotPosY[i] = random(0, HEIGHT);                  // и по высоте
    dotColor[i] = ColorFromPalette(RainbowColors_p, random8(0, 9) * 31, 255); // цвет капли
    dotAccel[i] = (float)random(5, 11) / 70;        // делаем частицам немного разное ускорение 
  }
}

String EffectNexus::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectNexus::load() {
  palettesload();
  randomSeed(millis());
  reload();
}

bool EffectNexus::run(CRGB *leds, EffectWorker *opt) {
  float speedfactor = EffectMath::fmap(speed, 1, 255, 0.1, .33);

  fadeToBlackBy(leds, NUM_LEDS, map(speed, 1, 255, 11, 33));

  for (byte i = 0; i < map(_scale, 1, 10, 4, NEXUS); i++) {
    switch (dotDirect[i])
    {
    case 0:   // вверх
      dotPosY[i] += (speedfactor + dotAccel[i]);
      break;
    case 1:   //  вниз 
      dotPosY[i] -= (speedfactor + dotAccel[i]);
      break;
    case 2:   // вправо
      dotPosX[i] += (speedfactor + dotAccel[i]);
      break;
    case 3:   // влево
      dotPosX[i] -= (speedfactor + dotAccel[i]);
      break;
    default:
      break;
    } 

    // Обеспечиваем бесшовность по Y. И переносим каплю в начало трека
    if (dotPosY[i] < 0) {
      dotPosY[i] = (float)HEIGHT - 1.;    
      resetDot(i);
    }

    if (dotPosY[i] > (HEIGHT - 1)) {
      dotPosY[i] = 0;
      resetDot(i);
    }

    // Обеспечиваем бесшовность по X.
    if (dotPosX[i] < 0) {
      dotPosX[i] = (WIDTH - 1);
      resetDot(i);
    }
    if (dotPosX[i] > (WIDTH - 1)) {
      dotPosX[i] = 0;
      resetDot(i);
    }

   switch (dotDirect[i])
  {
  case 0:   // вверх
  case 1:   //  вниз 
    EffectMath::drawPixelXYF_Y(dotPosX[i], dotPosY[i], dotColor[i], 0);
    break;
  case 2:   // вправо
  case 3:   // влево
    EffectMath::drawPixelXYF_X(dotPosX[i], dotPosY[i], dotColor[i], 0);
    break;
  default:
    break;
  } 


  }

  //fpsmeter();
  return true;
}

void EffectNexus::resetDot(uint8_t idx) {
  randomSeed(micros());
  dotDirect[idx] = random8(0, 4);                     // задаем направление
  dotColor[idx] = ColorFromPalette(*curPalette, random(0, 9) * 31, 255);              // цвет 
  dotAccel[idx] = (float)random(5, 10) / 70;     // делаем частицам немного разное ускорение 
  switch (dotDirect[idx])
  {
  case 0:   // вверх
    dotPosX[idx] = random8(0, WIDTH); // Разбрасываем капли по ширине
    dotPosY[idx] = 0;  // и по высоте
    break;
  case 1:   //  вниз 
    dotPosX[idx] = random8(0, WIDTH); // Разбрасываем капли по ширине
    dotPosY[idx] = HEIGHT-1;  // и по высоте
    break;
  case 2:   // вправо
    dotPosX[idx] = 0; // Разбрасываем капли по ширине
    dotPosY[idx] = random8(0, HEIGHT);  // и по высоте
    break;
  case 3:   // влево
    dotPosX[idx] = WIDTH-1; // Разбрасываем капли по ширине
    dotPosY[idx] = random8(0, HEIGHT);  // и по высоте
    break;
  default:
    break;
  } 
}

// --------------  Эффект "Цветение"
//Yaroslaw Turbin
//https://vk.com/ldirko
//https://www.reddit.com/user/ldirko/
bool EffectFlower::run(CRGB *ledarr, EffectWorker *opt ) {
  return flowerRoutine(*&ledarr, &*opt);
}

bool EffectFlower::flowerRoutine(CRGB *leds, EffectWorker *param) {
  float speedFactor = EffectMath::fmap((float)speed, 1., 255., 0.1, 2.55);
#ifdef MIC_EFFECTS
#define _Mic isMicOn() ? (float)peak / 50. : 0.1
  byte peak = myLamp.getMicMapMaxPeak();
 if (millis() - lastrun >= (isMicOn() ? 4000 - (uint16_t)peak * 20 : (unsigned int)(8000.0 / speedFactor)))
#else
#define _Mic 0.1
 if (millis() - lastrun >= (unsigned int)(8000.0 / speedFactor))
#endif
 {
   angle += _Mic;
   if (angle <= 1.) angle = 1.;
   lastrun = millis();
 }

  float n = triwave8((uint8_t)counter) * 2.2; // 2.2;
  //float n = quadwave8((uint8_t)counter) * 1.5;
  float a = n * angle;
  float r = c * sqrt16(n);
  float x = r * cos(a) + (float)WIDTH / 2.;
  float y = r * sin(a) + (float)HEIGHT / 2.;
#ifdef MIC_EFFECTS
    color = CHSV(
      isMicOn() ? myLamp.getMicFreq() : (millis()>>1),
      isMicOn() ? (255 - peak/2) : 250,
      isMicOn() ? constrain(peak *2, 96, 255) : 255
      );
#else
  color = CHSV(millis()>>1, 250, 255/*(uint16_t)counter >> 1*/);
#endif
  if ((uint16_t)ceil(x * y) < NUM_LEDS)
    EffectMath::drawPixelXYF(x, y, color) ;

  //EffectMath::blur2d( 20 );
  fadeToBlackBy(leds, NUM_LEDS, 5);

  counter += speedFactor;
  return true;
}

// ----------- Эфеект "ДНК"
// Yaroslaw Turbin 04.09.2020
// https://vk.com/ldirko
// https://pastebin.com/jwvC1sNF
// адаптация и доработки kostyamat
String EffectDNA::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) rotate = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectDNA::DNARoutine(CRGB *leds, EffectWorker *param)
{
  speeds = map(speed, 1, 255, 10, 60);
  fadeToBlackBy(leds, NUM_LEDS, speeds);

  int maxV, maxH;
  if(!rotate){
    maxV = LED_ROWS;
    maxH = LED_COLS;
  } else {
    maxV = LED_COLS;
    maxH = LED_ROWS;
  }

  for (uint8_t i = 0; i < maxV; i++)
  {
    uint16_t ms = millis();
    uint32_t x = beatsin16(speeds, 0, (maxH - 1) * 256, 0, i * freq);
    uint32_t y = i * 256;
    uint32_t x1 = beatsin16(speeds, 0, (maxH - 1) * 256, 0, i * freq + 32768);
    CRGB col = CHSV(ms / 29 + i * 255 / (maxV - 1), 255, beatsin8(speeds, 50, BRIGHTNESS, 0, (float)i * mn));
    CRGB col1 = CHSV(ms / 29 + i * 255 / (maxV - 1) + 128, 255, beatsin8(speeds, 50, BRIGHTNESS, 0, (float)i * mn + 127));

    if(!rotate){
      if(!isDebug())
        EffectMath::wu_pixel (x1 , y, col);
      EffectMath::wu_pixel (x , y, col1);
    } else {
      if(!isDebug())
        EffectMath::wu_pixel (y , x, col1);
      EffectMath::wu_pixel (y , x1, col);
    }
  }

  EffectMath::blur2d(16);
  return true;
}

bool EffectDNA::run(CRGB *ledarr, EffectWorker *opt) {

  return DNARoutine(*&ledarr, &*opt);
}

// ----------- Эффект "Огненная Лампа"
// Yaroslaw Turbin, 22.06.2020 
// https://vk.com/ldirko
// https://pastebin.com/eKqe4zzA
// переделака на субпиксель и доработки - kostyamat
void EffectFire2020::load() {
  regenNoise();
  palettesload();    // подгружаем палитры
}

void EffectFire2020::palettesload(){
  // собираем свой набор палитр для эффекта
  palettes.reserve(NUMPALETTES);
  palettes.push_back(&NormalFire_p);
  palettes.push_back(&HeatColors2_p);
  palettes.push_back(&NormalFire2_p);
  palettes.push_back(&CopperFireColors_p);
  palettes.push_back(&SodiumFireColors_p);
  palettes.push_back(&PotassiumFireColors_p);
  palettes.push_back(&RubidiumFireColors_p);
  palettes.push_back(&AlcoholFireColors_p);
  palettes.push_back(&LithiumFireColors_p);
  palettes.push_back(&WoodFireColors_p);
  usepalettes = true; // включаем флаг палитр
  scale2pallete();    // выставляем текущую палитру
}


void EffectFire2020::regenNoise() {
  uint16_t b = millis();
  for (uint8_t i = 0; i < LED_COLS; i++)
  {
    for (uint8_t j = 0; j < (NOISE_HEIGHT); j++)
    {
      noises[j * LED_COLS + i] = inoise8(i * (_scale+30), j * (_scale+30) + b / (_scale+30)); // init noise buffer
    }
  }
  for (uint8_t j = 0; j < LED_ROWS; j++)
  {
    colorfade[j] = abs8(j - (LED_ROWS - 1)) * 255 / (LED_ROWS - 1); // init colorfade table
    if (random8() < 100)
    {
      noises[myLamp.getPixelNumber(random8(WIDTH), j)] = qadd8(noises[j], random(156, 255)); // 196, 255
    }
  }

}

// нахрена кастомный мапинг тут???
// void EffectFire2020::palettemap(std::vector<PGMPalette*> &_pals, const uint8_t _val, const uint8_t _min, const uint8_t _max){
//   std::size_t idx = (_val-1); // т.к. сюда передается точное значение контрола, то приводим его к 0
//   if (!_pals.size() || idx>=_pals.size()) {
//     LOG(println,F("No palettes loaded or wrong value!"));
//     return;
//   }

//   curPalette = _pals.at(idx);
// }

String EffectFire2020::setDynCtrl(UIControl*_val)
{
  if(_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectFire2020::fire2020Routine(CRGB *leds, EffectWorker *param) {
  if(!curPalette) return false;

  float speedfactor = EffectMath::fmap((float)speed, 1., 255., 0.20, 1.);

  for (uint8_t i = 0; i < NUM_COLS; i++)
  {
#ifdef BIGMATRIX 
    for (uint8_t j = 0.; j < NUM_ROWS; j++)
#else
    for (float j = 0.; j < NUM_ROWS; j+= 0.5)
#endif
    {
      uint16_t index = ((uint8_t)(j + a) + random8(2)) % (NOISE_HEIGHT)*NUM_COLS; //roll index in noise buffer
      EffectMath::drawPixelXYF_Y(i, (float)(LED_ROWS - 1) - j, ColorFromPalette(*curPalette, qsub8(noises[i + index], colorfade[(uint8_t)j])), 35);

    }
  }
  blurRows(leds, WIDTH, HEIGHT, 15);
  a+= speedfactor; // как-то раньше не догадался так сделать. Мда...
  return true;
}


bool EffectFire2020::run(CRGB *ledarr, EffectWorker *opt) {
  EVERY_N_MILLISECONDS(EFFECTS_RUN_TIMER * NOISE_HEIGHT + NOISE_HEIGHT) {
    regenNoise();
  }

  //fpsmeter();
  return fire2020Routine(*&ledarr, &*opt);
}

// ----------- Эфеект "Змеиный Остров"
// (c) Сотнег
// База https://community.alexgyver.ru/threads/wifi-lampa-budilnik-obsuzhdenie-proekta.1411/post-53132
// адаптация и доработки kostyamat
String EffectTest::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) SnakeNum = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  regen();
  return String();
}

void EffectTest::setscl(const byte _scl){ // вот тут перегрузим масштаб
  EffectCalc::setscl(_scl); // вызываем функцию базового класса (т.е. заполнение scale и что еще она там умеет)
}

void EffectTest::regen() {

  for (uint8_t i = 0; i < map(SnakeNum, 1, 10, 2, MAX_SNAKES); i++)
  {
    snakeLast[i] = 0;
    snakePosX[i] = random8(WIDTH / 2 - WIDTH / 4, WIDTH/2 + WIDTH / 4);
    snakePosY[i] = random8(HEIGHT / 2 - HEIGHT / 4, HEIGHT / 2 + HEIGHT / 4);
    snakeSpeedX[i] = EffectMath::randomf(0.2, 1.5);//(255. + random8()) / 255.;
    snakeSpeedY[i] = EffectMath::randomf(0.2, 1.5);
    //snakeTurn[i] = 0;
    snakeColor[i] = random8(map(SnakeNum, 1, 10, 2, MAX_SNAKES) * 255/map(SnakeNum, 1, 10, 2, MAX_SNAKES));
    snakeDirect[i] = random8(4); //     B00           направление головы змейки
                                 // B10     B11
                                 //     B01
  }
}

bool EffectTest::testRoutine(CRGB *leds, EffectWorker *param) {
  FastLED.clear(); 
  float speedfactor = EffectMath::fmap((float)speed, 1., 255., 0.06, 0.5);
  int8_t dx = 0, dy = 0;
  for (uint8_t i = 0; i < map(SnakeNum, 1, 10, 2, MAX_SNAKES); i++)
  {
    snakeSpeedY[i] += snakeSpeedX[i] * speedfactor;
    if (snakeSpeedY[i] >= 1)
    {
      snakeSpeedY[i] = snakeSpeedY[i] - (int)snakeSpeedY[i];
      if (random8(8) <= 1U)
        if (random8(2U))
        {                                           // <- поворот налево
          snakeLast[i] = (snakeLast[i] << 2) | B01; // младший бит = поворот
          switch (snakeDirect[i])
          {
          case B10:
            snakeDirect[i] = B01;
            if (snakePosY[i] == 0U)
              snakePosY[i] = HEIGHT - 1U;
            else
              snakePosY[i]--;
            break;
          case B11:
            snakeDirect[i] = B00;
            if (snakePosY[i] >= HEIGHT - 1U)
              snakePosY[i] = 0U;
            else
              snakePosY[i]++;
            break;
          case B00:
            snakeDirect[i] = B10;
            if (snakePosX[i] == 0U)
              snakePosX[i] = WIDTH - 1U;
            else
              snakePosX[i]--;
            break;
          case B01:
            snakeDirect[i] = B11;
            if (snakePosX[i] >= WIDTH - 1U)
              snakePosX[i] = 0U;
            else
              snakePosX[i]++;
            break;
          }
        }
        else
        {                                           // -> поворот направо
          snakeLast[i] = (snakeLast[i] << 2) | B11; // младший бит = поворот, старший = направо
          switch (snakeDirect[i])
          {
          case B11:
            snakeDirect[i] = B01;
            if (snakePosY[i] == 0U)
              snakePosY[i] = HEIGHT - 1U;
            else
              snakePosY[i]--;
            break;
          case B10:
            snakeDirect[i] = B00;
            if (snakePosY[i] >= HEIGHT - 1U)
              snakePosY[i] = 0U;
            else
              snakePosY[i]++;
            break;
          case B01:
            snakeDirect[i] = B10;
            if (snakePosX[i] == 0U)
              snakePosX[i] = WIDTH - 1U;
            else
              snakePosX[i]--;
            break;
          case B00:
            snakeDirect[i] = B11;
            if (snakePosX[i] >= WIDTH - 1U)
              snakePosX[i] = 0U;
            else
              snakePosX[i]++;
            break;
          }
        }
      else
      { // двигаем без поворота
        snakeLast[i] = (snakeLast[i] << 2);
        switch (snakeDirect[i])
        {
        case B01:
          if (snakePosY[i] == 0U)
            snakePosY[i] = HEIGHT - 1U;
          else
            snakePosY[i]--;
          break;
        case B00:
          if (snakePosY[i] >= HEIGHT - 1U)
            snakePosY[i] = 0U;
          else
            snakePosY[i]++;
          break;
        case B10:
          if (snakePosX[i] == 0U)
            snakePosX[i] = WIDTH - 1U;
          else
            snakePosX[i]--;
          break;
        case B11:
          if (snakePosX[i] >= WIDTH - 1U)
            snakePosX[i] = 0U;
          else
            snakePosX[i]++;
          break;
        }
      }
    }
    switch (snakeDirect[i])
    {
    case B01:
      dy = 1;
      dx = 0;
      break;
    case B00:
      dy = -1;
      dx = 0;
      break;
    case B10:
      dy = 0;
      dx = 1;
      break;
    case B11:
      dy = 0;
      dx = -1;
      break;
    }

    long temp = snakeLast[i];
    uint8_t x = snakePosX[i];
    uint8_t y = snakePosY[i];
    EffectMath::drawPixelXYF(x, y, ColorFromPalette(*curPalette, snakeColor[i], snakeSpeedY[i] * 255));
    for (uint8_t m = 0; m < SNAKE_LENGTH; m++)
    { // 16 бит распаковываем, 14 ещё остаётся без дела в запасе, 2 на хвостик
      x = (WIDTH + x + dx) % WIDTH;
      y = (HEIGHT + y + dy) % HEIGHT;  
      EffectMath::drawPixelXYF(x, y, ColorFromPalette(*curPalette, snakeColor[i] + m * 4U, 255U));

      if (temp & B01)
      { // младший бит = поворот, старший = направо
        temp = temp >> 1;
        if (temp & B01)
        { // старший бит = направо
          if (dx == 0)
          {
            dx = 0 - dy;
            dy = 0;
          }
          else
          {
            dy = dx;
            dx = 0;
          }
        }
        else
        { // иначе налево
          if (dx == 0)
          {
            dx = dy;
            dy = 0;
          }
          else
          {
            dy = 0 - dx;
            dx = 0;
          }
        }
        temp = temp >> 1;
      }
      else
      { // если без поворота
        temp = temp >> 2;
      }
    }
    x = (WIDTH + x + dx) % WIDTH;
    y = (HEIGHT + y + dy) % HEIGHT;
    EffectMath::drawPixelXYF(x, y, ColorFromPalette(*curPalette, snakeColor[i] + SNAKE_LENGTH * 4U, (1 - snakeSpeedY[i]) * 255)); // хвостик
  }

  return true;
}

bool EffectTest::run(CRGB *ledarr, EffectWorker *opt) {
  return testRoutine(*&ledarr, &*opt);
}

void EffectTest::load() {
  palettesload();
  regen();
}

// ----------- Эфеект "Попкорн"
// (C) Aaron Gotwalt (Soulmate)
// адаптация и доработки kostyamat

bool EffectPopcorn::run(CRGB *ledarr, EffectWorker *opt) {
  //random16_add_entropy(micros());
  randomSeed(micros());
  return popcornRoutine(*&ledarr, &*opt);
}

void EffectPopcorn::restart_rocket(uint8_t r) {
  //tiltDirec = !tiltDirec; // "Мальчик" <> "Девочка"
  rockets[r].xd = (float)(random(-(WIDTH * HEIGHT + (WIDTH*2)), WIDTH * HEIGHT + (WIDTH*2))) / 256.0; // * tiltDirec; // Наклон. "Мальчики" налево, "девочки" направо. :)
  if ((rockets[r].x < 0 && rockets[r].xd < 0) || (rockets[r].x > (WIDTH-1) && rockets[r].xd > 0)) { // меняем направление только после выхода за пределы экрана
    // leap towards the centre of the screen
    rockets[r].xd = -rockets[r].xd;
  }
  // controls the leap height
  rockets[r].yd = EffectMath::randomf(0.9, EffectMath::fmap(speed, 1., 255., (float)HEIGHT / 1.50, (float)HEIGHT / 3.));// (float)(random8() * 8 + HEIGHT * 10.5) / 256.0;
  rockets[r].hue = random8();
  rockets[r].x = EffectMath::randomf(center - (float)WIDTH / 6, center + (float)WIDTH / 6);
}

void EffectPopcorn::reload(){
  //numRockets = map(scale, 1, 32, 6, WIDTH * 3);
  //rockets.resize(numRockets);
  speedfactor = EffectMath::fmap((float)speed, 1., 255., 0.25, 0.75);

  for (uint8_t r = 0; r < numRockets; r++) {
    rockets[r].x = random8(WIDTH);
    rockets[r].y = random8(HEIGHT);
    rockets[r].xd = 0;
    rockets[r].yd = -1;
    rockets[r].hue = random8();
  }
}

bool EffectPopcorn::popcornRoutine(CRGB *leds, EffectWorker *param) {
  speedfactor = EffectMath::fmap((float)speed, 1., 255., 0.25, 0.75);

  if (blurred) fadeToBlackBy(leds, NUM_LEDS, 30);
  else FastLED.clear();
  float popcornGravity = 0.1 * speedfactor;

  for (uint8_t r = 0; r < numRockets; r++) {
    // add the X & Y velocities to the positions
    rockets[r].x += rockets[r].xd ;
    if (rockets[r].x > WIDTH - 1)
      rockets[r].x = rockets[r].x - (WIDTH - 1);
    if (rockets[r].x < 0)
      rockets[r].x = WIDTH - 1 + rockets[r].x;
    rockets[r].y += rockets[r].yd * speedfactor;
    
    if (rockets[r].y >= (float)HEIGHT){
      //rockets[r].y = HEIGHT+HEIGHT - 2 - rockets[r].y;
      rockets[r].yd = -0.001; //rockets[r].yd;
    } 
    

    // bounce off the floor?
    if (rockets[r].y < 0 && rockets[r].yd < -0.7) { // 0.7 вычислено в экселе. скорость свободного падения ниже этой не падает. если ниже, значит ещё есть ускорение
      rockets[r].yd = (-rockets[r].yd) * 0.9375;//* 240) >> 8;
      rockets[r].y = rockets[r].yd; //чё это значило вообще?!
      rockets[r].y = -rockets[r].y;
    }

    // settled on the floor?
    if (rockets[r].y <= -1)
      restart_rocket(r);


    // bounce off the sides of the screen?
    /*if (rockets[r].x < 0 || rockets[r].x > (int)WIDTH * 256) {
      rockets[r].xd = (-rockets[r].xd * 248) >> 8;
      // force back onto the screen, otherwise they eventually sneak away
      if (rockets[r].x < 0) {
        rockets[r].x = rockets[r].xd;
        rockets[r].yd += rockets[r].xd;
      } else {
        rockets[r].x = (WIDTH * 256) - rockets[r].xd;
      }
    }*/

    // popcornGravity
    rockets[r].yd -= popcornGravity;

    // viscosity
    rockets[r].xd *= 0.875;
    rockets[r].yd *= 0.875;


    // make the acme gray, because why not
    if (-0.004 > rockets[r].yd and rockets[r].yd < 0.004)
      EffectMath::drawPixelXYF(rockets[r].x, rockets[r].y, revCol ?
                ColorFromPalette(*curPalette, rockets[r].hue) 
              : CRGB::Pink, blurred ? 35 : 0);
    else
      EffectMath::drawPixelXYF(rockets[r].x, rockets[r].y, revCol ? 
                CRGB::Gray 
              : ColorFromPalette(*curPalette, rockets[r].hue), blurred ? 35 : 0);
  }
  return true;
}

String EffectPopcorn::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) { uint8_t density = EffectCalc::setDynCtrl(_val).toInt(); numRockets = 5 + density; rockets.resize(numRockets); reload(); }
  else if(_val->getId()==5) blurred = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==6) revCol = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

// void EffectPopcorn::setscl(const byte _scl){ // вот тут перегрузим масштаб
//   EffectCalc::setscl(_scl); // вызываем функцию базового класса (т.е. заполнение scale и что еще она там умеет)
// }

void EffectPopcorn::load() {
  palettesload();
  reload();
}

//-------- Эффект "Детские сны"
// (c) Stepko https://editor.soulmatelights.com/gallery/505
String EffectSmokeballs::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  regen();
  return String();
}

void EffectSmokeballs::load(){
  palettesload();
  regen();
}

void EffectSmokeballs::regen() {
  randomSeed(millis());
  for (byte j = 0; j < WAVES_AMOUNT; j++) {
    reg[j] =  random((WIDTH * 10) - ((WIDTH / 3) * 20)); // сумма maxMin + reg не должна выскакивать за макс.Х
    sSpeed[j] = EffectMath::randomf(5., (float)(16 * WIDTH)); //random(50, 16 * WIDTH) / random(1, 10);
    maxMin[j] = random((WIDTH / 2) * 10, (WIDTH / 3) * 20);
    waveColors[j] = random(0, 9) * 28;
    pos[j] = reg[j];
  }
}

bool EffectSmokeballs::run(CRGB *ledarr, EffectWorker *opt){
  speedfactor = EffectMath::fmap(speed, 1., 255., .02, .1); // попробовал разные способы управления скоростью. Этот максимально приемлемый, хотя и сильно тупой.
  uint8_t _amount = map(_scale, 1, 16, 2, WAVES_AMOUNT);
  shiftUp();
  EffectMath::dimAll(240);
  EffectMath::blur2d(20);
  for (byte j = 0; j < _amount; j++) {
    pos[j] = beatsin16((uint8_t)(sSpeed[j] * (speedfactor * 5.)), reg[j], maxMin[j] + reg[j], waveColors[j]*256, waveColors[j]*8);
      EffectMath::drawPixelXYF((float)pos[j] / 10., 0.05, ColorFromPalette(*curPalette, waveColors[j]));
  }
  EVERY_N_SECONDS(20){
    for (byte j = 0; j < _amount; j++) {
      reg[j] += random(-20,20);
      waveColors[j] += 28;
    }
  }

  if (random8(255) > 253 ) regen();
  return true;
}

void EffectSmokeballs::shiftUp(){       
  for (byte x = 0; x < WIDTH; x++) {
    for (float y = (float)HEIGHT; y > 0.; y-= speedfactor) {
      EffectMath::drawPixelXY(x, y, EffectMath::getPixColorXY(x, y - 1));
    }
  }
}

// ----------- Эффект "Ёлки-Палки"
// "Cell" (C) Elliott Kember из примеров программы Soulmate
// Spaider и Spruce (c) stepko
void EffectCell::cell(CRGB *leds) {
  float speedfactor = EffectMath::fmap((float)speed, 1., 255., .33, 3.);
  offsetX = beatsin16(6. * speedfactor, -180, 180);
  offsetY = beatsin16(6. * speedfactor, -180, 180, 12000);
  for (uint x = 0; x < LED_COLS; x++) {
    for (uint y = 0; y < LED_ROWS; y++) {
      int16_t index = myLamp.getPixelNumber(x, y);

      if (index < 0) break;

      int16_t hue = x * beatsin16(10. * speedfactor, 1, 10) + offsetY;
      leds[index] = CHSV(hue, 200, sin8(x * 30 + offsetX));
      hue = y * 3 + offsetX;
      leds[index] += CHSV(hue, 200, sin8(y * 30 + offsetY));
    }
  }
  EffectMath::nightMode(leds); // пригасим немного, чтобы видить структуру, и убрать пересветы
} 

String EffectCell::setDynCtrl(UIControl*_val)
{
  if(_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  effId = _scale ? _scale : random(_val->getMin().toInt(), _val->getMax().toInt()+1)+1;
  return String();
}

bool EffectCell::run(CRGB *leds, EffectWorker *opt){
  
  if (_scale == 0) {
    EVERY_N_SECONDS(60) {
      effId ++;
      if (effId == 6)
        effId = 1;
    }
  } else effId = constrain(_scale, 1, 5);

  switch (effId)
  {
  case 1:
    cell(leds);
    break;
  case 2:
  case 3:
  case 4:
    spruce(leds);
    break;
  case 5:
    spider(leds);
    break;

  default:
    break;
  }
  //fpsmeter();
  return true;
}

void EffectCell::spruce(CRGB *leds) {
  hue++;
  fadeToBlackBy(leds, NUM_LEDS, map(speed, 1, 255, 1, 10));
  uint8_t z;
  if (effId == 3) z = triwave8(hue);
  else z = beatsin8(1, 1, 255);
  for (uint8_t i = 0; i < WIDTH - 1; i++) {
    x[i] = beatsin16(i * map(speed, 1, 255, 3, 20), i * 2, ((WIDTH - 1) * 4 - 2) - (i * 2 + 2));
    if (effId == 2) 
      EffectMath::drawPixelXYF_X(x[i] / 4, i, random8(10) == 0 ? CHSV(random8(), random8(32, 255), 255) : CHSV(100, 255, map(speed, 1, 255, 128, 100)));
    else
      EffectMath::drawPixelXYF_X(x[i] / 4, i, CHSV(hue + i * z, 255, 255));
  }
}

void EffectCell::spider(CRGB *leds) {
  fadeToBlackBy(leds, NUM_LEDS, 50);
  //FastLED.clear();
  float speedFactor = EffectMath::fmap(speed, 1, 255, 20., 2.); 
  for (uint8_t c = 0; c < Lines; c++) {
    float xx = 2. + sin8((float)millis() / speedFactor + 1000 * c * Scale) / 12.;
    float yy = 2. + cos8((float)millis() / speedFactor + 1500 * c * Scale) / 12.;
    EffectMath::drawLineF(xx, yy, (float)WIDTH - xx - 1, (float)HEIGHT - yy - 1, CHSV(c * (256 / Lines), 200, 255));
  }
}


// ----------- Эффект "Геометрический Вальс"
//F_lying 
//Fastled 16x16 rgb led matrix demo
//Yaroslaw Turbin, 27.10.2020 
//https://vk.com/ldirko
//https://www.reddit.com/user/ldirko/

//https://www.reddit.com/r/FastLED/comments/jj4oc9/new_fastled_matrix_example_f_lying_code_in
//code for arduino: https://wokwi.com/arduino/projects/280541577702539789
//                  https://wokwi.com/arduino/projects/280607115091902988
String EffectF_lying::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) _blur = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) type = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectF_lying::load() {
  palettesload();
}

bool EffectF_lying::run(CRGB *leds, EffectWorker *opt) {
  
  float speedfactor = EffectMath::fmap(speed, 1., 255., 0.2, 1.);

  EVERY_N_MILLISECONDS(30. / speedfactor) { hue++; } //30 - speed of hue change

  float x1 = (float)beatsin16(18. * speedfactor, 0, (NUM_COLS - 1) *deviator) / deviator;
  float x2 = (float)beatsin16(23. * speedfactor, 0, (NUM_COLS - 1) *deviator) / deviator;
  float x3 = (float)beatsin16(27. * speedfactor, 0, (NUM_COLS - 1) *deviator) / deviator; 
  float x4 = (float)beatsin16(31. * speedfactor, 0, (NUM_COLS - 1) *deviator) / deviator; 

  float y1 = (float)beatsin16(20. * speedfactor, 0, (NUM_ROWS - 1) *deviator) / deviator;
  float y2 = (float)beatsin16(26. * speedfactor, 0, (NUM_ROWS - 1) *deviator) / deviator;
  float y3 = (float)beatsin16(15. * speedfactor, 0, (NUM_ROWS - 1) *deviator) / deviator;
  float y4 = (float)beatsin16(27. * speedfactor, 0, (NUM_ROWS - 1) *deviator) / deviator;

  fadeToBlackBy (leds, NUM_LEDS, map(_blur, 1, 128, 128, 2));

  mydrawLine(leds, x1, y1,  x2, y2, 0);
  mydrawLine(leds, x2, y2,  x3, y3, 32);
  mydrawLine(leds, x2, y2,  x4, y4, 64);
  if (!type) {
    mydrawLine(leds, x3, y3,  x4, y4, 96);
    mydrawLine(leds, x3, y3,  x1, y1, 128);
    mydrawLine(leds, x4, y4,  x1, y1, 160);
  }
  
  blur2d(leds, NUM_COLS, NUM_ROWS, map(_blur, 1, 128, 2, 64));

  return true;
}

void EffectF_lying::mydrawLine(CRGB *leds, float x, float y, float x1, float y1, byte hueLamda)
{
  if (!type) {
    EffectMath::drawLineF(x, y, x1, y1, ColorFromPalette(*curPalette, hue + hueLamda, 200));
    EffectMath::drawPixelXYF(x, y, CRGB ::Gray);
    EffectMath::drawPixelXYF(x1, y1, CRGB ::Gray);
  } else {
    EffectMath::drawCircleF(x, y, x1, ColorFromPalette(*curPalette, hue + hueLamda, 200));
    EffectMath::drawCircleF(x1, y1, y1, ColorFromPalette(*curPalette, hue + hueLamda, 200));
  }
}

// ---------- Эффект "Тикси Ленд"
// (c)  Martin Kleppe @aemkei, https://github.com/owenmcateer/tixy.land-display
String EffectTLand::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) { animation = EffectCalc::setDynCtrl(_val).toInt(); isSeq = animation ? false : true; if(!animation) animation = random(_val->getMin().toInt()+1, _val->getMax().toInt()+1);}
  else if(_val->getId()==4) { hue = EffectCalc::setDynCtrl(_val).toInt(); ishue = hue; }
  else if(_val->getId()==5) { hue2 = EffectCalc::setDynCtrl(_val).toInt(); ishue2 = hue2; }
  else if(_val->getId()==6) fine = _val->getVal().toInt(); // качество не рандомим, а берем как есть
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectTLand::load() {
  // if (scale == 0)
  //   animation=random(1,35);
}

bool EffectTLand::run(CRGB *leds, EffectWorker *opt) {
  t = (double)(millis()&0xFFFFF) / map(speed, 1, 255, 1200, 128); ; // на больших значениях будет странное поведение, поэтому уменьшаем точность, хоть и будет иногда срыв картинки, но в 18 минут, так что - хрен с ним
  shift = (shift+1)%fine; // 0...3
  if(!ishue) hue++;
  if(!ishue2) hue2++;

  for( byte x = 0; x < WIDTH; x++) {
    for( byte y = 0; y < HEIGHT; y++) {
      if(myLamp.getPixelNumber(x,y)%fine==shift)
        processFrame(leds, t, x, y);
    }
  }
  
  if (isSeq) {
    EVERY_N_SECONDS(30) {
      animation++;
    }
  }

  //fpsmeter();
  return true;
}

void EffectTLand::processFrame(CRGB *leds, double t, double x, double y) {
  double i = (y * WIDTH) + x;
  int16_t frame = constrain(code(i, x, y), -1, 1) * 255;

  if (frame > 0) {
    EffectMath::drawPixelXY(x, y, CHSV(hue, frame, frame));
  }
  else if (frame < 0) {
    EffectMath::drawPixelXY(x, y, CHSV(hue2, frame * -1, frame * -1));
  } else EffectMath::drawPixelXY(x, y, CRGB::Black);
}

float EffectTLand::code(double i, double x, double y) {

  switch (animation) {
    /**
     * Motus Art
     * @motus_art
     */
    case 1:
      // Plasma
      //return sin(x + t) / 2 + sin(y + t) / 2 + sin(x + y + t) / 3;
      return (sin16((x + t) * 8192.0) * 0.5 + sin16((y + t) * 8192.0) * 0.5 + sin16((x + y + t) * 8192.0) * 0.3333333333333333) / 32767.0;
      break;

    case 2:
      // Up&Down
      //return sin(cos(x) * y / 8 + t);
      return sin16((cos16(x*8192.0) / 32767.0 * y / (HEIGHT/2.0) + t)*8192.0)/32767.0;
      break;

    case 3:
      //return sin(atan(y / x) + t);
      return sin16((EffectMath::atan_fast(y / x) + t)*8192.0)/32767.0;
      break;

    /**
     * tixy.land website
     */
    case 4:
      // Emitting rings
      //return sin(t - EffectMath::sqrt(((x - 7.5)*(x - 7.5)) + (y - 6)*(y - 6)));
      return sin16((t - EffectMath::sqrt((x - (WIDTH/2))*(x - (WIDTH/2)) + (y - (HEIGHT/2))*(y - (HEIGHT/2))))*8192.0)/32767.0;
      break;

    case 5:
      // Rotation
      //return sin(PI * 2 * atan((y - 8) / (x - 8)) + 5 * t);
      return sin16((PI * 2.5 * EffectMath::atan_fast((y - (HEIGHT/2)) / (x - (WIDTH/2))) + 5 * t) * 8192.0)/32767.0;
      break;

    case 6:
      // Vertical fade
      //return sin(y / 8 + t);
      return sin16((y / 8 + t)*8192.0)/32767.0;
      break;

    case 7:
      // Smooth noise
      //return cos(t + i + x * y);
      return cos16((t + i + x * y)*8192.0)/32767.0;
      break;

    case 8:
      // Waves
      //return sin(x / 2) - sin(x - t) - y + 6;
      return (sin16(x * 4096.0) - sin16((x - t) * 8192.0)) / 32767.0 - y + (HEIGHT/2);
      break;

    case 9:
      // Drop
      //return fmod(8 * t, 13) - hypot(x - 7.5, y - 7.5);
      return fmod(8 * t, 13) - EffectMath::sqrt((x - (WIDTH/2))*(x - (WIDTH/2))+(y - (HEIGHT/2))*(y - (HEIGHT/2))); //hypot(x - (WIDTH/2), y - (HEIGHT/2));
      break;

    case 10:
      // Ripples @thespite
      //return sin(t - EffectMath::sqrt(x * x + y * y));
      return sin16((t - EffectMath::sqrt(x * x + y * y))*8192.0)/32767.0;
      break;

    case 11:
      // Bloop bloop bloop @v21
      //return (x - 8) * (y - 8) - sin(t / 2.) * 64;
      return (x - (WIDTH/2)) * (y - (HEIGHT/2)) - sin16(t*4096.0)/512.0;
      break;


    /**
     * Reddit
     */
     case 12:
      // lurkerurke https://www.reddit.com/r/programming/comments/jpqbux/minimal_16x16_dots_coding_environment/gbgcwsn/
      //return sin((x - 7.5) * (y - 7.5) / 5 * t + t);
      return sin16(((x - (WIDTH/2)) * (y - (HEIGHT/2)) / 5 * t + t)*8192.0)/32767.0;
      break;

    case 13:
      // SN0WFAKER https://www.reddit.com/r/programming/comments/jpqbux/minimal_16x16_dots_coding_environment/gbgk7c0/
      //return sin(atan((y - 7.5) / (x - 7.5)) + t);
      return sin16((EffectMath::atan_fast((y - (HEIGHT/2)) / (x - (WIDTH/2))) + t) * 8192.0)/32767.0;
      break;

    case 14:
      //return  cos(((int)x ^ (int)y) * t); //sin(((int)(x / sin(t) / 50) ^ (int)(y / sin(t) / 50)) + t); //pow(cos(((int)y ^ (int)x) + t), cos((x > y) + t));
      return  cos16((((int)x ^ (int)y) * t)* 8192.0)/32767.0;
      break;

    case 15:
      // detunized https://www.reddit.com/r/programming/comments/jpqbux/minimal_16x16_dots_coding_environment/gbgk30l/
      //return sin(y / 8 + t * 0.5) + x / 16 - 0.5;
      return sin16((y / (HEIGHT/2) + t * 0.5)*8192.0)/32767.0 + x / 16 - 0.5;
      break;

    case 16:
      // Andres_A https://www.reddit.com/r/programming/comments/jpqbux/minimal_16x16_dots_coding_environment/gbgzdnj/
      //return 1. - hypot(sin(t) * 9 - x, cos(t) * 9 - y) / 9;
      //return 1. - hypot(sin(1.5*t) * 16 + x, cos(t*2) * 16 + y) / 4;
      //return 1. - hypot(8 * sin(1.5*t) + x - 8, (8 * cos(t*2) + y - 8))*(sin(0.5*t+1.0)+1.0); // https://tixy.land/?code=1.+-+hypot%288+*+sin%281.5*t%29+%2B+x+-+8%2C+%288+*+cos%28t*2%29+%2B+y+-+8%29%29*%28sin%280.5*t%2B1.0%29%2B1.0%29
      {
        float _x=sin16(12288.0*t)/32767.0 * WIDTH + x - (WIDTH/2);
        float _y=cos16(16384.0*t)/32767.0 * HEIGHT + y - (HEIGHT/2);
        //float _size=sin16((0.5*t+1.0)*8192.0)/32767.0+1.0;
        float _size=1.0/4.0; // 4.0 - постоянный размер шарика
        return 1. - EffectMath::sqrt(_x*_x+_y*_y)*_size; 
      }
      break;

    /**
     * @akella
     * https://twitter.com/akella/status/1323549082552619008
     */
    case 17:
      //return sin(6 * atan2(y - 8, x) + t);
      return sin16((6 * EffectMath::atan2_fast(y - (HEIGHT/2), x) + t)*8192.0)/32767.0;
      break;

    case 18:
      //return sin(i / 5 + t);
      return sin16((i / 5 + t)*16384.0)/32767.0;
      break;

    /**
     * Paul Malin
     * https://twitter.com/P_Malin/
     */

    case 19:
      // Matrix Rain https://twitter.com/P_Malin/status/1323583013880553472
      //return 1. - fmod((x * x - y + t * (fmod(1 + x * x, 5)) * 6), 16) / 16;
      return 1. - fmod((x * x - (HEIGHT - 1 - y) + t * (1 + fmod(x * x, 5)) * 3), WIDTH) / HEIGHT;
      break;

    case 20:
      // Burst https://twitter.com/P_Malin/status/1323605999274594304
      //return -10. / ((x - 8) * (x - 8) + (y - 8) * (y - 8) - fmod(t*0.3, 0.7) * 200);
      return -10. / ((x - (WIDTH/2)) * (x - (WIDTH/2)) + (y - (HEIGHT/2)) * (y - (HEIGHT/2)) - fmod(t*0.3, 0.7) * 200);
      break;

    case 21:
      // Rays
      //return sin(atan2(x, y) * 5 + t * 2);
      return sin16((EffectMath::atan2_fast(x, y) * 5 + t * 2)*8192.0)/32767.0;
      break;

    case 22:
      // Starfield https://twitter.com/P_Malin/status/1323702220320313346 
      //return !((int)(x + (t/2) * 50 / (fmod(y * y, 5.9) + 1)) & 15) / (fmod(y * y, 5.9) + 1);
      {
        uint16_t _y = (HEIGHT - 1 - y);
        float d = (fmod(_y * _y + 4, 4.1) + 0.85) * 0.5; // коэффициенты тут отвечают за яркость (размер), скорость, смещение, подбираются экспериментально :)
        return !((int)(x + t * 7.0 / d) & 15) / d; // 7.0 - множитель скорости
      }
      break;

    case 23:
      //return sin(3 * atan2(y - 7.5 + sin(t) * 5, x - 7.5 + sin(t) * 5) + t * 5);
      //return sin(3 * atan2(y - 7.5 + sin(t) * 4, x - 7.5 + sin(t) * 4) + t * 1.5 + 5);
      return sin16((3.5*EffectMath::atan2_fast(y - (HEIGHT/2) + sin16(t*8192.0) * 0.00006, x - (WIDTH/2) + sin16(t*8192.0) * 0.00006) + t * 1.5 + 5)*8192.0)/32767.0;
      break;

    case 24:
      //return (y - 8) / 3 - tan(x / 6 + 1.87) * sin(t * 2);
      //return (y - 8) / 3 - tan(x / 6 + 1.87) * sin16(t * 16834.0)/32767.0;
      return (y - 8) / 3 - EffectMath::tan2pi_fast((x / 6 + 1.87)/PI*2) * sin16(t * 16834.0)/32767.0;
      break;

    case 25:
      //return (y - 8) / 3 - (sin(x / 4 + t * 2));
      return (y - 8) / 3 - (sin16((x / 4 + t * 2)*8192.0)/32767.0);
      break;

    case 26:
      //return fmod(i, 4) - fmod(y, 4) + sin(t);
      return fmod(i, 4) - fmod(y, 4) + sin16(t*8192.0)/32767.0;
      break;

    case 27:
      //return cos(sin((x * t / 10)) * PI) + cos(sin(y * t / 10 + (EffectMath::sqrt(abs(cos(x * t))))) * PI);
      return cos(sin16(x * t * 819.2) / 32767.0 * PI) + cos16((sin16((y * t / 10 + (EffectMath::sqrt(abs(cos16(x * t * 8192.0)/32767.0))))*8192.0)/32767.0 * PI)*8192.0)/32767.0;
      break;

    case 28:
      //return -.4 / (hypot(x - fmod(t, 10), y - fmod(t, 8)) - fmod(t, 2) * 9);
      {
        float _x = x - fmod(t, WIDTH);
        float _y = y - fmod(t, HEIGHT);
        return -.4 / (EffectMath::sqrt(_x*_x+_y*_y) - fmod(t, 2) * 9);
      }
      break;

    case 29:
      //return sin(x / 3 * sin(t / 3) * 2) + cos(y / 4 * sin(t / 2) * 2);
      return sin16(x / 3 * sin16(t * 2730.666666666667) / 2.0) / 32767.0 + cos16(y / 4 * sin16(t * 4096.0) / 2.0) / 32767.0;
      break;

    case 30:
      //return sin(x * x * 3 * i / 1e4 - y / 2 + t * 2);
      return sin16((x * x * 3 * i / 1e4 - y / 2 + t * 2)*8192.0)/32767.0;
      break;

    case 31:
      //return 1. - fabs((x - 6) * cos(t) + (y - 6) * sin(t));
      return 1. - fabs((x - (WIDTH/2)) * cos16(t*8192.0)/32767.0 + (y - (HEIGHT/2)) * sin16(t*8192.0)/32767.0);
      break;

    case 32:
      //return 1. / 32 * tan(t / 64 * x * tan(i - x));
      //return (((x-8)/y+t)&1^1/y*8&1)*y/5;
      return ((((uint32_t)((x-8)/(HEIGHT-y)+t) & 1 ) ^ (uint32_t)((1./(HEIGHT-y)) * 8)) & 1) * (HEIGHT-y) / 8;
      break;

    case 33:
      return EffectMath::atan_fast((x - (WIDTH/2)) * (y - (HEIGHT/2))) - 2.5 * sin16(t*8192.0)/32767.0;
      break;

    case 34:
      //return sin(cos(y) * t) * cos(sin(x) * t);
      return sin16(cos16(y*8192.0)* 0.25 * t)/32767.0 * cos16(sin16(x*8192.0)* 0.25 * t)/32767.0;
      break;

    case 35:
      //return sin(y * (t/4)) * cos(x * (t/4));
      return sin16(y * t * 2048.0) / 32767.0 * cos16(x * t * 2048.0) / 32767.0;
      break;

    default:
      animation = 1;
      return sin16(t*8192.0)/32767.0;
      break;
  }
}

// -------- "LDIRKO Ленд"
//simple playground for fasled
//best use with 16x16 matrix and bigger
//like tixy.land ))
//...ldir... Yaroslaw Turbin, 18.11.2020 
//https://vk.com/ldirko
//https://www.reddit.com/user/ldirko/
String EffectLLand::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) select = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) { hue = EffectCalc::setDynCtrl(_val).toInt(); randColor = ((uint8_t)hue == 0); }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectLLand::load() {
  palettesload();
}

bool EffectLLand::run(CRGB *leds, EffectWorker *opt) {
  if (_scale > 0) {
    effnumber = _scale - 1;
  } else {
    EVERY_N_MILLISECONDS(30000) { effnumber = (effnumber + 1) % effects; } //speed of  effect change
  }
  uint16_t i = 0;
  t = (float)millis() / EffectMath::fmap(speed, 1., 255., 20., 1.);
  if (randColor) hue += 0.2;
  for (uint8_t y = 0; y < LED_ROWS; y++) {
    for (uint16_t x = 0; x < LED_COLS; x++) {
      EffectMath::drawPixelXY(x, y, select ? CHSV(code(x, y, i, t) + (uint8_t)hue, 255, 255) : ColorFromPalette (*curPalette, code(x,y,i,t) + (uint8_t)hue, 255)); 
      i++;
    }
  }
  //fpsmeter();
  return true;
}

uint16_t EffectLLand::code(byte x, byte y, uint16_t i, float t) {
  uint16_t outputcode = 0;
  
  switch (effnumber) {
    case 0: outputcode = (x * 8 + sin8(t / 2)) ^ (y * 8 - cos8(t / 1.33)); // oldschool demoscene xor pattern with move
      break;
    case 1: outputcode = sin8(x * 3 + t / 2) * cos8(y * 3 + t / 3) / 64 + cos8(x * 8); // sin plasma 3
      break;
    case 2: outputcode = sin8(x * 8 + sin8(x * 2 + t / 2)) / 2 + sin8(y * 8 + sin8(y * 2 + t / 2) / 2); // sin plasma 
      break;
    case 3: outputcode = sin8((x << 4) + t) / 2 + sin8((y << 4) + t) / 2 + t; //sin plasma 4 
      break;
    case 4: outputcode = ((x * 8) ^ (y * 8)) + t ; // oldschool demoscene xor pattern static
      break;
    case 5: outputcode = (x * 8 + sin8(t / 3)) & (y * 8 - cos8(t / 2)); //Serpinski triangle with move
      break;
    case 6: outputcode = inoise8(x * 42, y * 42, t * 2) + t / 3; // perlin noise plasma
      break;
    case 7: outputcode = sin8(x * sin8(y + t) / 16 + t) / 5 + sin8(y * 15 + t) / 3 + t / 10; // sin plasma 2
      break;
    case 8: outputcode = sin8(y * 5 + 30) / 2 + sin8(x * 5 + 30) / 2 + sin8(t / 5);
      break;
    case 9: outputcode = map(inoise8(x * 50, y * 50 - t * 16, 0) - y * 255 / (LED_ROWS - 1), 0, 255, 205, 255); // CHSV fire
      break;
  }
  return outputcode;
}


// ----------- Эффект "Осцилятор"
// (c) Сотнег (SottNick)
bool EffectOscilator::run(CRGB *leds, EffectWorker *opt) {
  if (millis() - timer < (unsigned)map(speed, 1U, 255U, 70, 15)) return true;
  else timer = millis(); // не могу сообразить, как по другому скоростью управлять
  CRGB currColors[3];
  for (uint8_t c = 0; c < 3; c++)
    currColors[c] = ColorFromPalette(*curPalette, c * 85U + hue);

  // расчёт химической реакции и отрисовка мира
  uint16_t colorCount[3] = {0U, 0U, 0U};
  hue++;
  FastLED.clear();
  for (uint8_t x = 0; x < WIDTH; x++) {
      for (uint8_t y = 0; y < HEIGHT; y++) {
          if (oscillatingWorld[x][y].red){
             colorCount[0]++;
             if (greenNeighbours(x, y) > 2)
                oscillatingWorld[x][y].color = 1U;
          }
          else if (oscillatingWorld[x][y].green){
             colorCount[1]++;
             if (blueNeighbours(x, y) > 2)
                oscillatingWorld[x][y].color = 2U;
          }
          else {//if (oscillatingWorld[x][y].blue){
             colorCount[2]++;
             if (redNeighbours(x, y) > 2)
                oscillatingWorld[x][y].color = 0U;
          }
          drawPixelXYFseamless((float)x + 0.5, (float)y + 0.5, currColors[oscillatingWorld[x][y].color]);
      }
  }


  // проверка зацикливания
  if (colorCount[0] == deltaHue && colorCount[1] == deltaHue2 && colorCount[2] == deltaValue) {
    step++;
    if (step > 10U){
      if (colorCount[0] < colorCount[1])
        step = 0;
      else
        step = 1;
      if (colorCount[2] < colorCount[step])
        step = 2;
      colorCount[step] = 0U;
      step = 0U;
    }
  }
  else
    step = 0U;
  
  // вброс хаоса
  if (hue == hue2){// чтобы не каждый ход
    hue2 += random8(220U) + 36U;
    uint8_t tx = random8(WIDTH);
    deltaHue = oscillatingWorld[tx][0U].color + 1U;
    if (deltaHue > 2U) deltaHue = 0U;
    oscillatingWorld[tx][0U].color = deltaHue;
    oscillatingWorld[(tx + 1U) % WIDTH][0U].color = deltaHue;
    oscillatingWorld[(tx + 2U) % WIDTH][0U].color = deltaHue;
  }

  deltaHue = colorCount[0];
  deltaHue2 = colorCount[1];
  deltaValue = colorCount[2];

  // вброс исчезнувшего цвета
  for (uint8_t c = 0; c < 3; c++)
  {
    if (colorCount[c] < 6U){
      uint8_t tx = random8(WIDTH);
      uint8_t ty = random8(HEIGHT);
      if (random8(2U)){
        oscillatingWorld[tx][ty].color = c;
        oscillatingWorld[(tx + 1U) % WIDTH][ty].color = c;
        oscillatingWorld[(tx + 2U) % WIDTH][ty].color = c;
      }
      else {
        oscillatingWorld[tx][ty].color = c;
        oscillatingWorld[tx][(ty + 1U) % HEIGHT].color = c;
        oscillatingWorld[tx][(ty + 2U) % HEIGHT].color = c;
      }
    }
  }

  // перенос на следующий цикл
  for (uint8_t x = 0; x < WIDTH; x++) {
      for (uint8_t y = 0; y < HEIGHT; y++) {
          setCellColors(x, y);
      }
  }

  //fpsmeter();
  return true;
}

void EffectOscilator::load() {
  palettesload();
  step = 0U;
 //случайное заполнение
  for (uint8_t i = 0; i < WIDTH; i++) {
    for (uint8_t j = 0; j < HEIGHT; j++) {
      oscillatingWorld[i][j].color = random8(3);
      setCellColors(i, j);
    }
  }
  timer = millis();
}

void EffectOscilator::drawPixelXYFseamless(float x, float y, CRGB color)
{
  uint8_t xx = (x - (int)x) * 255, yy = (y - (int)y) * 255, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  #define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t xn = (int8_t)(x + (i & 1)) % WIDTH;
    uint8_t yn = (int8_t)(y + ((i >> 1) & 1)) % HEIGHT;
    CRGB clr = EffectMath::getPixColorXY(xn, yn);
    clr.r = qadd8(clr.r, (color.r * wu[i]) >> 8);
    clr.g = qadd8(clr.g, (color.g * wu[i]) >> 8);
    clr.b = qadd8(clr.b, (color.b * wu[i]) >> 8);
    EffectMath::drawPixelXY(xn, yn, clr);
  }
}

int EffectOscilator::redNeighbours(uint8_t x, uint8_t y) {
  return (oscillatingWorld[(x + 1) % WIDTH][y].red) +
         (oscillatingWorld[x][(y + 1) % HEIGHT].red) +
         (oscillatingWorld[(x + WIDTH - 1) % WIDTH][y].red) +
         (oscillatingWorld[x][(y + HEIGHT - 1) % HEIGHT].red) +
         (oscillatingWorld[(x + 1) % WIDTH][(y + 1) % HEIGHT].red) +
         (oscillatingWorld[(x + WIDTH - 1) % WIDTH][(y + 1) % HEIGHT].red) +
         (oscillatingWorld[(x + WIDTH - 1) % WIDTH][(y + HEIGHT - 1) % HEIGHT].red) +
         (oscillatingWorld[(x + 1) % WIDTH][(y + HEIGHT - 1) % HEIGHT].red);
    }

int EffectOscilator::blueNeighbours(uint8_t x, uint8_t y) {
  return (oscillatingWorld[(x + 1) % WIDTH][y].blue) +
         (oscillatingWorld[x][(y + 1) % HEIGHT].blue) +
         (oscillatingWorld[(x + WIDTH - 1) % WIDTH][y].blue) +
         (oscillatingWorld[x][(y + HEIGHT - 1) % HEIGHT].blue) +
         (oscillatingWorld[(x + 1) % WIDTH][(y + 1) % HEIGHT].blue) +
         (oscillatingWorld[(x + WIDTH - 1) % WIDTH][(y + 1) % HEIGHT].blue) +
         (oscillatingWorld[(x + WIDTH - 1) % WIDTH][(y + HEIGHT - 1) % HEIGHT].blue) +
         (oscillatingWorld[(x + 1) % WIDTH][(y + HEIGHT - 1) % HEIGHT].blue);
}
  
int EffectOscilator::greenNeighbours(uint8_t x, uint8_t y) {
  return (oscillatingWorld[(x + 1) % WIDTH][y].green) +
         (oscillatingWorld[x][(y + 1) % HEIGHT].green) +
         (oscillatingWorld[(x + WIDTH - 1) % WIDTH][y].green) +
         (oscillatingWorld[x][(y + HEIGHT - 1) % HEIGHT].green) +
         (oscillatingWorld[(x + 1) % WIDTH][(y + 1) % HEIGHT].green) +
         (oscillatingWorld[(x + WIDTH - 1) % WIDTH][(y + 1) % HEIGHT].green) +
         (oscillatingWorld[(x + WIDTH - 1) % WIDTH][(y + HEIGHT - 1) % HEIGHT].green) +
         (oscillatingWorld[(x + 1) % WIDTH][(y + HEIGHT - 1) % HEIGHT].green);
}

void EffectOscilator::setCellColors(uint8_t x, uint8_t y) {
  oscillatingWorld[x][y].red = (oscillatingWorld[x][y].color == 0U);
  oscillatingWorld[x][y].green = (oscillatingWorld[x][y].color == 1U);
  oscillatingWorld[x][y].blue = (oscillatingWorld[x][y].color == 2U);
}

//------------ Эффект "Дождь с ветром" 
// (с) kostyamat 1.12.2020
String EffectWrain::setDynCtrl(UIControl*_val)
{
  if(_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) { uint8_t val = EffectCalc::setDynCtrl(_val).toInt(); white = (val == 1); randColor = (val == 0); }
  else if(_val->getId()==5) clouds = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==6) storm = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==7) type = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectWrain::reload() {
  for (byte i = 0; i < counts; i++) {
    dotPosX[i] = EffectMath::randomf(0, WIDTH-1); // Разбрасываем капли по ширине
    dotPosY[i] = EffectMath::randomf(0, HEIGHT);  // и по высоте
    dotColor[i] = random(0, 9) * 31;              // цвет капли
    dotAccel[i] = (float)random(5, 10) / 100;     // делаем частицам немного разное ускорение 
    dotBri[i] = random(128, 255);
  }
}

void EffectWrain::load() {
  palettesload();
  randomSeed(millis());
  reload();
}

bool EffectWrain::run(CRGB *leds, EffectWorker *opt) {
  float speedfactor = EffectMath::fmap(speed, 1, 255, 0.1, .75);

  if (_flash and (millis() - timer < 500)) 
    _flash = true;
  else _flash = false;

  if (_flash == false) {
    switch (type)
    {
    case 1:
    case 5:
      FastLED.clear();
      break;
    case 2:
    case 6:
      fadeToBlackBy(leds, NUM_LEDS, 200. * speedfactor);
      break;
    case 3:
    case 7:
      fadeToBlackBy(leds, NUM_LEDS, 100. * speedfactor);
      break;
    case 4:
    case 8:
      fadeToBlackBy(leds, NUM_LEDS, 50. * speedfactor);
      break;
    default:
      break;
    } 
  }

  //
  for (byte i = 0; i < map(_scale, 1, 45, 2, counts); i++) {
    dotColor[i]++;
    dotPosX[i] += (speedfactor * dotChaos + dotAccel[i]) * dotDirect; // смещение по горизонтали
    dotPosY[i] -= (speedfactor + dotAccel[i]);

    // Обеспечиваем бесшовность по Y.
    if (dotPosY[i] < 0)
    {                                                             // достигли низа, обновляем каплю
      dotPosY[i] = ((float)HEIGHT - (clouds ? cloudHeight : 1.)); // переносим каплю в начало трека
      dotPosX[i] += EffectMath::randomf(-1, 1);                   // сдвигаем каплю туда-сюда по горизонтали
      dotBri[i] = random(170, 200);                               // задаем капле новое значение яркости
    }
    if (dotPosY[i] > (HEIGHT - 1))
      dotPosY[i] = 0;

    // Обеспечиваем бесшовность по X.
    if (dotPosX[i] < 0)
      dotPosX[i] = (WIDTH - 1);
    if (dotPosX[i] > (WIDTH - 1))
      dotPosX[i] = 0;

    if (randColor) {
      if (dotDirect) EffectMath::drawPixelXYF(dotPosX[i], dotPosY[i], CHSV(dotColor[i], 256U - beatsin88(2 * speed, 1, 196), beatsin88(1 * speed, 64, 255)));
      else EffectMath::drawPixelXYF_Y(dotPosX[i], dotPosY[i], CHSV(dotColor[i], 256U - beatsin88(2 * speed, 1, 196), beatsin88(1 * speed, 64, 255)));
    } else if (white) {
      CHSV color = rgb2hsv_approximate(CRGB::Gray);
      color.value = dotBri[i] - 48;
      if (dotDirect) EffectMath::drawPixelXYF(dotPosX[i], dotPosY[i], color);
      else EffectMath::drawPixelXYF_Y(dotPosX[i], dotPosY[i], color);
    }
    else {
      CHSV color = rgb2hsv_approximate(ColorFromPalette(*curPalette, dotColor[i], dotBri[i]));
      color.sat = 128;
      if (dotDirect) EffectMath::drawPixelXYF(dotPosX[i], dotPosY[i], color);
      else EffectMath::drawPixelXYF_Y(dotPosX[i], dotPosY[i], color);
    }
  }

  // Раздуваем\угасаем ветер
  if (type <= 4) {
    uint8_t val = triwave8(windProgress += speedfactor);
    dotChaos = (float)val / 254;
    if (val == 0) {
      dotDirect = random(-1, 2); //выбираем направление ветра лево-право, рандом 2 не возвращает (как не странно).
    }
  } else dotDirect = 0;

    // Рисуем тучку и молнию
  if (clouds) {
    if (randColor) curPalette = palettes.at(6);  // устанавливаем палитру RainbowColors_p
    if (white) curPalette = palettes.at(17);     // WaterfallColors_p
    if (storm) _flash = Lightning(200);
    Clouds((storm ? _flash : false));
  } else if (storm) {
    _flash = Lightning(200);
  }

  //fpsmeter();
  return true;
}

bool EffectWrain::Lightning(uint16_t chanse)
{
  CRGB lightningColor = CHSV(30,90,255);
  //uint8_t lightning[WIDTH][HEIGHT];
  // ESP32 does not like static arrays  https://github.com/espressif/arduino-esp32/issues/2567
if (random16() < chanse)
  {            
    timer = millis();
    //uint8_t *lightning = (uint8_t *)malloc(WIDTH * HEIGHT);                                                           // Odds of a lightning bolt
    lightning[scale8(random8(), WIDTH - 1) + (HEIGHT - 1) * WIDTH] = 255; // Random starting location
    for (uint8_t ly = HEIGHT - 1; ly > 1; ly--)
    {
      for (uint8_t lx = 1; lx < WIDTH - 1; lx++)
      {
        if (lightning[lx + ly * WIDTH] == 255)
        {
          lightning[lx + ly * WIDTH] = 0;
          uint8_t dir = random8(4);
          switch (dir)
          {
          case 0:
            EffectMath::setLed(myLamp.getPixelNumber(lx + 1, ly - 1), lightningColor);
            lightning[(lx + 1) + (ly - 1) * WIDTH] = 255; // move down and right
            break;
          case 1:
            EffectMath::setLed(myLamp.getPixelNumber(lx, ly - 1), lightningColor); 
            lightning[lx + (ly - 1) * WIDTH] = 255;                                 // move down
            break;
          case 2:
            EffectMath::setLed(myLamp.getPixelNumber(lx - 1, ly - 1), lightningColor);
            lightning[(lx - 1) + (ly - 1) * WIDTH] = 255; // move down and left
            break;
          case 3:
            EffectMath::setLed(myLamp.getPixelNumber(lx - 1, ly - 1), lightningColor);
            lightning[(lx - 1) + (ly - 1) * WIDTH] = 255; // fork down and left
            EffectMath::setLed(myLamp.getPixelNumber(lx - 1, ly - 1), lightningColor);
            lightning[(lx + 1) + (ly - 1) * WIDTH] = 255; // fork down and right
            break;
          }
        }
      }
    }

    //free(lightning);
    return true;
  }
  return false;
}

// Функция рисует тучу в верхней части матрицы 
void EffectWrain::Clouds(bool flash)
{
  uint8_t dataSmoothing = 50; //196
  uint16_t noiseX = beatsin16(1, 10, 4000, 0, 150);
  uint16_t noiseY = beatsin16(1, 1000, 10000, 0, 50);
  uint16_t noiseZ = beatsin16(1, 10, 4000, 0, 100);
  uint16_t noiseScale = 50; // A value of 1 will be so zoomed in, you'll mostly see solid colors. A value of 4011 will be very zoomed out and shimmery
  //uint8_t *_noise = (uint8_t *)malloc(WIDTH * cloudHeight);

  // This is the array that we keep our computed noise values in
  //static uint8_t _noise[WIDTH][cloudHeight];
  for (uint8_t x = 0; x < WIDTH; x++)
  {
    int xoffset = noiseScale * x;

    for (uint8_t z = 0; z < cloudHeight; z++) {
      int yoffset = noiseScale * z ;
      uint8_t noiseData = qsub8(inoise8(noiseX + xoffset, noiseY + yoffset, noiseZ), 16);
      noiseData = qadd8(noiseData, scale8(noiseData, 39));
      _noise[x * cloudHeight + z] = scale8(_noise[x * cloudHeight + z], dataSmoothing) + scale8(noiseData, 256 - dataSmoothing);
      if (flash)
        EffectMath::drawPixelXY(x, HEIGHT - z - 1, CHSV(random8(20,30), 250, random8(64, 100)));
      else 
        nblend(myLamp.getUnsafeLedsArray()[myLamp.getPixelNumber(x, HEIGHT - z - 1)], ColorFromPalette(*curPalette, _noise[x * cloudHeight + z], _noise[x * cloudHeight + z]), (500 / cloudHeight));
    }
    noiseZ++;
  }

  if (millis() - timer < 300) {
    for (uint8_t i = 0; i < WIDTH; i++)
    {
      //for (byte z = 0; z < 10; z++)
        EffectMath::drawPixelXYF(i, EffectMath::randomf((float)HEIGHT - 4.5, (float)HEIGHT - 2.5), CHSV(0, 250, random8(120, 200)), 0);
    }
  }
}

// ------------- Эффект "Цветные драже"
void EffectPile::load() {
  FastLED.clear();
  //palettesload();

  for (uint8_t i = 0; i < MAXDOTS; i++) {
    dots[i].x = EffectMath::randomf((float)WIDTH /2 - 1.5, (float)WIDTH /2 + 1.5); //random(2)/2.0 + WIDTH/2 - 1;
    dots[i].y = (float)HEIGHT + 1. + (i + 2);
    dots[i].hue = random(0, 255);
  }
  memset(widthPos,0,sizeof(widthPos)/sizeof(float));

  // for(int16_t i=0;i<WIDTH*HEIGHT; i++){
  //     if(!random(map(scale,1,255,5,2))){
  //       leds[i]=CHSV(random(0,255),255,255);
  //     }
  // }
}
bool EffectPile::run(CRGB *leds, EffectWorker *opt)
{
  speedfactor = EffectMath::fmap(speed, 1, 255, 0.1, 0.5);

  if (!done)
  {
    if (!clearrows(false))
      return true;
  }

  for (uint8_t i = 0; i < map(scale, 1, 255, 1, MAXDOTS); i++) {
    uint8_t rx = dots[i].x;
    uint8_t ry = ceil(dots[i].y);
    if (ry < HEIGHT)
      EffectMath::drawPixelXY(rx, ry, CRGB::Black);
    dots[i].y -= speedfactor;
    ry = ceil(dots[i].y);

    EffectMath::drawPixelXY(rx, ry, CHSV(dots[i].hue, 255, 255));
    uint16_t pos = constrain(rx, 0, (int16_t)WIDTH - 1);
    if (dots[i].y < 0.1 || dots[i].y <= widthPos[pos])
    {
      int8_t shift = random(2) ? -1 : 1;
      if (widthPos[constrain(pos + shift, 0, (int16_t)WIDTH - 1)] > widthPos[constrain(pos - shift, 0, (int16_t)WIDTH - 1)])
        shift *= -1;
      if (pos < WIDTH && pos + shift >= 0 && pos + shift < (int16_t)WIDTH && widthPos[constrain(pos + shift, 0, (int16_t)WIDTH - 1)] < widthPos[pos])
      {
        EffectMath::drawPixelXY(rx, ry, CRGB::Black);
        dots[i].x = constrain(dots[i].x + shift, 0, (int16_t)WIDTH - 1);
        rx = dots[i].x;
        EffectMath::drawPixelXY(rx, ry, CHSV(dots[i].hue, 255, 255));
      }
      else
      {
        if (EffectMath::getPixColorXY(pos, widthPos[pos] - 1 > 0 ? widthPos[pos] - 1 : widthPos[pos]))
          widthPos[pos] += 1;
        else
        {
          // костыльное заполнение отверстий для случаев когда две точки падают подряд с минимальным смещением
          EffectMath::drawPixelXY(rx, ry, CRGB::Black);
          dots[i].y = widthPos[pos] - 1;
          ry = ceil(dots[i].y);
          EffectMath::drawPixelXY(rx, ry, CHSV(dots[i].hue, 255, 255));
        }
        // EVERY_N_SECONDS(1){
        //   Serial.print("widthPos["); Serial.print(pos); Serial.print("]="); Serial.print(widthPos[pos]); Serial.println();
        // }
        dots[i].y = (float)HEIGHT + 1 + (i + 2);
        dots[i].x = EffectMath::randomf((float)WIDTH /2 - 1.5, (float)WIDTH /2 + 1.5); //random(2)/2.0 + WIDTH/2 - 1;
        dots[i].hue = random(0, 255);
        //done = false;
        //clearrows(false);
      }
    }
  }

  EVERY_N_SECONDS(5){
    clearrows(true);
  }

  return true;
}

bool EffectPile::clearrows(bool clear)
{
  CRGB *leds= myLamp.getUnsafeLedsArray();
  
  bool state = false;
  if(clear){
    uint8_t ypos = 0;
    for(uint16_t i=0;i<MAXDOTS;i++){
      if(dots[i].y<HEIGHT){
        dots[i].y+=HEIGHT;
        dots[i].x = random(2)/2.0 + WIDTH/2 - 1;
        dots[i].hue = random(0,255);
      }
    }

    for(uint8_t x=0; x<WIDTH; x++)
      if(ypos<widthPos[x]) ypos=widthPos[x];

    for(uint8_t y=0; y<ypos; y++){
      for(uint8_t x=0; x<WIDTH; x++){
        if(random(map(ypos,1,HEIGHT,10,3))<2){
          leds[myLamp.getPixelNumber(x,y)]=CRGB::Black;
        }
      }
    }

    done = false;
  } else {
    // проверяем все ли осыпались
    //state = true;
    internal_counter += speedfactor;
    float v;
    internal_counter = modff(internal_counter, &v);
    if(v) {
      state = true;
      // смещаемся
      for(uint16_t x=0; x<WIDTH; x++){
        for(uint16_t y=1; y<HEIGHT; y++){
          if(leds[myLamp.getPixelNumber(x,y)] && !leds[myLamp.getPixelNumber(x,y-1)]){
            leds[myLamp.getPixelNumber(x,y-1)] = leds[myLamp.getPixelNumber(x,y)];
            leds[myLamp.getPixelNumber(x,y)] = CRGB::Black;
            widthPos[x] = y;
            state = false;
          } else if(leds[myLamp.getPixelNumber(x,y)] && x>0 && y>widthPos[x-1]) {
            leds[myLamp.getPixelNumber(x-1,y-1)] = leds[myLamp.getPixelNumber(x,y)];
            leds[myLamp.getPixelNumber(x,y)] = CRGB::Black;
            widthPos[x-1] = y;
            state = false;
          }
          else if(leds[myLamp.getPixelNumber(x,y)] && x<WIDTH-1 && y>widthPos[x+1]) {
            leds[myLamp.getPixelNumber(x+1,y-1)] = leds[myLamp.getPixelNumber(x,y)];
            leds[myLamp.getPixelNumber(x,y)] = CRGB::Black;
            widthPos[x+1] = y;
            state = false;
          } else if(!leds[myLamp.getPixelNumber(x,y)] && widthPos[x] > y){
            widthPos[x] = y;
          }
        }
      }
    }
    if(state) done = true;
  }
  return state;
}


//-------- по мотивам Эффектов Particle System -------------------------
// https://github.com/fuse314/arduino-particle-sys
// https://github.com/giladaya/arduino-particle-sys
// https://www.youtube.com/watch?v=S6novCRlHV8&t=51s

//при попытке вытащить из этой библиотеки только минимально необходимое выяснилось, что там очередной (третий) вариант реализации субпиксельной графики.
//ну его нафиг. лучше будет повторить визуал имеющимися в прошивке средствами.

void EffectFairy::particlesUpdate2(uint8_t i) {
  trackingObjectState[i] -= 1 * speedFactor; //ttl // ещё и сюда надо speedfactor вкорячить. удачи там!

  //apply velocity
  trackingObjectPosX[i] += trackingObjectSpeedX[i] * speedFactor;
  trackingObjectPosY[i] += trackingObjectSpeedY[i] * speedFactor;
  if(trackingObjectState[i] == 0 || trackingObjectPosX[i] <= -1 || trackingObjectPosX[i] >= WIDTH || trackingObjectPosY[i] <= -1 || trackingObjectPosY[i] >= HEIGHT) 
    trackingObjectIsShift[i] = false;
}

// ============= ЭФФЕКТ ИСТОЧНИК ===============
// (c) SottNick
// выглядит как https://github.com/fuse314/arduino-particle-sys/blob/master/examples/StarfieldFastLED/StarfieldFastLED.ino

void EffectFairy::fountEmit(uint8_t i) {
  if (hue++ & 0x01)
    hue2++;

  trackingObjectPosX[i] = WIDTH * 0.5;
  trackingObjectPosY[i] = HEIGHT * 0.5;


  trackingObjectSpeedX[i] = (((float)random8()-127.)/512.); 
  trackingObjectSpeedY[i] = EffectMath::sqrt(0.0626-trackingObjectSpeedX[i] * trackingObjectSpeedX[i]); 
  
  if(random8(2U)) trackingObjectSpeedY[i]=-trackingObjectSpeedY[i];

  trackingObjectState[i] = EffectMath::randomf(50, 250); 
  
  if (type)
    trackingObjectHue[i] = hue2;
  else 
    trackingObjectHue[i] = random8();
  trackingObjectIsShift[i] = true; 
}

void EffectFairy::fount(CRGB *leds){
  speedFactor = EffectMath::fmap(speed, 1, 255, 0.2, 1.);
  step = deltaValue; //счётчик количества частиц в очереди на зарождение в этом цикле

  EffectMath::dimAll(EffectMath::fmap(speed, 1, 255, 180, 127)); //ахах-ха. очередной эффект, к которому нужно будет "подобрать коэффициенты"

  //go over particles and update matrix cells on the way
  for (int i = 0; i < enlargedObjectNUM; i++) {
    if (!trackingObjectIsShift[i] && step) {
      fountEmit(i);
      step--;
    }
    if (trackingObjectIsShift[i]) { 
      particlesUpdate2(i);

      //generate RGB values for particle
      CRGB baseRGB;
        baseRGB = CHSV(trackingObjectHue[i], 255,255); 

      baseRGB.nscale8(trackingObjectState[i]);
      EffectMath::drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], baseRGB, 0);
    }
  }
  if (blur) EffectMath::blur2d(blur * 10); // Размытие 
}

// ============= ЭФФЕКТ ФЕЯ ===============
// (c) SottNick
#define FAIRY_BEHAVIOR //типа сложное поведение

void EffectFairy::fairyEmit(uint8_t i) {
    if (deltaHue++ & 0x01)
      if (hue++ & 0x01)
        hue2++;//counter++;
    trackingObjectPosX[i] = boids[0].location.x;
    trackingObjectPosY[i] = boids[0].location.y;

    //хотите навставлять speedfactor? - тут не забудьте
    //trackingObjectSpeedX[i] = ((float)random8()-127.)/512./0.25*speedfactor; 
    trackingObjectSpeedX[i] = ((float)random8()-127.)/512.; 
    trackingObjectSpeedY[i] = EffectMath::sqrt(0.0626-trackingObjectSpeedX[i]*trackingObjectSpeedX[i]); 
    if(random8(2U)) { trackingObjectSpeedY[i]=-trackingObjectSpeedY[i]; }

    trackingObjectState[i] = random8(20, 80); 
    trackingObjectHue[i] = hue2;
    trackingObjectIsShift[i] = true; 
}

bool EffectFairy::fairy(CRGB *leds) {
  speedFactor = EffectMath::fmap(speed, 1, 255, 0.05, .25);
  step = deltaValue; //счётчик количества частиц в очереди на зарождение в этом цикле
  
#ifdef FAIRY_BEHAVIOR
  if (!deltaHue && deltaHue2 && fabs(boids[0].velocity.x) + fabs(boids[0].velocity.y) < 0.15){ 
    deltaHue2 = 0U;
    
    boids[1].velocity.x = (((float)random8()+255.) / 4080.);
    boids[1].velocity.y = (((float)random8()+255.) / 2040.);
    if (boids[0].location.x > WIDTH * 0.5) boids[1].velocity.x = -boids[1].velocity.x;
    if (boids[0].location.y > HEIGHT * 0.5) boids[1].velocity.y = -boids[1].velocity.y;
  }
  if (!deltaHue2){
    step = 1U;
    
    boids[0].location.x += boids[1].velocity.x * speedFactor;
    boids[0].location.y += boids[1].velocity.y * speedFactor;
    deltaHue2 = (boids[0].location.x <= 0 || boids[0].location.x >= WIDTH-1 || boids[0].location.y <= 0 || boids[0].location.y >= HEIGHT-1);
  }
  else
#endif // FAIRY_BEHAVIOR
  {  
    PVector attractLocation = PVector(WIDTH * 0.5, HEIGHT * 0.5);
    //float attractMass = 10;
    //float attractG = .5;
    // перемножаем и получаем 5.
    Boid boid = boids[0];
    PVector force = attractLocation - boid.location;      // Calculate direction of force
    float d = force.mag();                                // Distance between objects
    d = constrain(d, 5.0f, HEIGHT);//видео снято на 5.0f  // Limiting the distance to eliminate "extreme" results for very close or very far objects
//d = constrain(d, modes[currentMode].Scale / 10.0, HEIGHT);

    force.normalize();                                    // Normalize vector (distance doesn't matter here, we just want this vector for direction)
    float strength = (5. * boid.mass) / (d * d);          // Calculate gravitional force magnitude 5.=attractG*attractMass
//float attractMass = (modes[currentMode].Scale) / 10.0 * .5;
    force *= strength * speedFactor;                                    // Get force vector --> magnitude * direction
    boid.applyForce(force);
    boid.update();
    
    if (boid.location.x <= -1) boid.location.x = -boid.location.x;
    else if (boid.location.x >= WIDTH) boid.location.x = -boid.location.x+WIDTH+WIDTH;
    if (boid.location.y <= -1) boid.location.y = -boid.location.y;
    else if (boid.location.y >= HEIGHT) boid.location.y = -boid.location.y+HEIGHT+HEIGHT;
    boids[0] = boid;

    if (!deltaHue) {
      if (random8(3U)){
        d = ((random8(2U)) ? boids[0].velocity.x : boids[0].velocity.y) * ((random8(2U)) ? .2 : -.2);
        boids[0].velocity.x += d * speedFactor;
        boids[0].velocity.y -= d * speedFactor;
      }
      else {
        if (fabs(boids[0].velocity.x) < 0.02)
          boids[0].velocity.x = -boids[0].velocity.x;
        else if (fabs(boids[0].velocity.y) < 0.02)
          boids[0].velocity.y = -boids[0].velocity.y;
      }
    }
  }

  //dimAll(255-128/.25*speedfactor); очередной эффект, к которому нужно будет "подобрать коэффициенты"
    //EffectMath::dimAll(127);  
    EffectMath::dimAll(EffectMath::fmap(speed, 1, 255, 180, 127));

  //go over particles and update matrix cells on the way
  for(int i = 0; i<enlargedObjectNUM; i++) {
    if (!trackingObjectIsShift[i] && step) {
      fairyEmit(i);
      step--;
    }
    if (trackingObjectIsShift[i]){ 
      // вернуться и поглядеть, что это
      if (type && trackingObjectSpeedY[i] > -1) trackingObjectSpeedY[i] -= 0.05; //apply acceleration
      particlesUpdate2(i);

      //generate RGB values for particle
      CRGB baseRGB = /*ColorFromPalette(*curPalette, trackingObjectHue[i], 255); */CHSV(trackingObjectHue[i], 255,255); // particles[i].hue

      baseRGB.nscale8(trackingObjectState[i]);//эквивалент
      EffectMath::drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], baseRGB, 0);
    }
  }
  /*CHSV temp = rgb2hsv_approximate(ColorFromPalette(*curPalette, hue, 255));
  temp.sat = 160;*/
  EffectMath::drawPixelXYF(boids[0].location.x, boids[0].location.y, CHSV(hue, 160U, 255U) /*temp*/, 0);  
  return true;
}

bool EffectFairy::run(CRGB *leds, EffectWorker *opt) {

  switch (effect)
  {
  case EFF_FAIRY:
    return fairy(leds);
    break;
  case EFF_FOUNT:
    fount(leds);
    break; 
  default:
    break;
  }
  //fpsmeter();
  return true;
}


void EffectFairy::setscl(const byte _scl){
  EffectCalc::setscl(_scl);

  // if(effect==EFF_FAIRY)
  //   deltaValue = 10; // количество зарождающихся частиц за 1 цикл //perCycle = 1;
  // else
  //   deltaValue = enlargedObjectNUM / (EffectMath::sqrt(CENTER_X_MAJOR * CENTER_X_MAJOR + CENTER_Y_MAJOR * CENTER_Y_MAJOR) * 4U) + 1U; // 4 - это потому что за 1 цикл частица пролетает ровно четверть расстояния между 2мя соседними пикселями

  enlargedObjectNUM = map(scale, 1, 255, 4, trackingOBJECT_MAX_COUNT);
  if (enlargedObjectNUM > trackingOBJECT_MAX_COUNT)
    enlargedObjectNUM = trackingOBJECT_MAX_COUNT;
}

void EffectFairy::load(){
  //---- Общее для двух эффектов
  if(effect==EFF_FAIRY)
    deltaValue = 10; // количество зарождающихся частиц за 1 цикл //perCycle = 1;
  else
    deltaValue = enlargedObjectNUM / (EffectMath::sqrt(CENTER_X_MAJOR * CENTER_X_MAJOR + CENTER_Y_MAJOR * CENTER_Y_MAJOR) * 4U) + 1U; // 4 - это потому что за 1 цикл частица пролетает ровно четверть расстояния между 2мя соседними пикселями

  enlargedObjectNUM = map(scale, 1, 255, 4, trackingOBJECT_MAX_COUNT);
  if (enlargedObjectNUM > trackingOBJECT_MAX_COUNT)
    enlargedObjectNUM = trackingOBJECT_MAX_COUNT;
  for (uint16_t i = 0; i < enlargedOBJECT_MAX_COUNT; i++)
    trackingObjectIsShift[i] = false; 

  //---- Только для эффекта Фея
  // лень было придумывать алгоритм для таектории феи, поэтому это будет нулевой "бойд" из эффекта Притяжение
  boids[0] = Boid(random8(WIDTH), random8(HEIGHT));
  boids[0].mass = 0.5;//((float)random8(33U, 134U)) / 100.; // random(0.1, 2); // сюда можно поставить регулятор разлёта. чем меньше число, тем дальше от центра будет вылет
  boids[0].velocity.x = ((float) random8(46U, 100U)) / 500.0;
  if (random8(2U)) boids[0].velocity.x = -boids[0].velocity.x;
  boids[0].velocity.y = 0;
  hue = random8();
  #ifdef FAIRY_BEHAVIOR
    deltaHue2 = 1U;
  #endif
}

String EffectFairy::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) type = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) blur = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

// ---------- Эффект "Бульбулятор"
// "Circles" (C) Elliott Kember https://editor.soulmatelights.com/gallery/11
// адаптация и переделка - kostyamat
void EffectCircles::load() {
  palettesload();
  for (uint8_t i = 0; i < NUMBER_OF_CIRCLES; i++) {
    circles[i].reset();
  }
}

void EffectCircles::drawCircle(CRGB *leds, Circle circle) {
  int16_t centerX = circle.centerX;
  int16_t centerY = circle.centerY;
  int hue = circle.hue;
  float radius = circle.radius();

  int16_t startX = centerX - ceil(radius);
  int16_t endX = centerX + ceil(radius);
  int16_t startY = centerY - ceil(radius);
  int16_t endY = centerY + ceil(radius);

  for (int16_t x = startX; x < endX; x++) {
    for (int16_t y = startY; y < endY; y++) {
      if (y < 0 or y > (int)(HEIGHT - 1)) continue;
      if (x < 0 or y > (int)(WIDTH - 1)) continue;
      int16_t index = myLamp.getPixelNumber(x, y);
      if (index < 0 || index > NUM_LEDS - 1)
        continue;
      float distance = EffectMath::sqrt(sq(x - centerX) + sq(y - centerY));
      if (distance > radius)
        continue;

      uint16_t brightness;
      if (radius < 1) { // last pixel
        brightness = 255.0 * radius;
      } else {
        float percentage = distance / radius;
        float fraction = 1.0 - percentage;
        brightness = 255.0 * fraction;
      }
      leds[index] += ColorFromPalette(*curPalette, hue, brightness);
    }
  }
}

bool EffectCircles::run(CRGB *leds, EffectWorker *opt) {
  randomSeed(millis());
  FastLED.clear();
  byte count = map(scale, 1, 255, 3, NUMBER_OF_CIRCLES);
  for (uint8_t i = 0; i < count; i++) {
    circles[i].bpm += EffectMath::fmap(speed, 1, 255, 0.5, 3.33);
    if (circles[i].radius() < 0.001) {
      circles[i].hue = random(0, NUMBER_OF_CIRCLES) * 255 / count;
      circles[i].move();
    }
    drawCircle(leds, circles[i]);
  }
  return true;
}

// ----------- Эффект "Бенгальские Огни"
// (c) stepko https://wokwi.com/arduino/projects/289797125785520649
// 06.02.2021
void EffectBengalL::load() {
  for (byte i = 0; i < sparksNum; i++) {
    regen(i);
  }
  speedFactor = EffectMath::fmap(speed, 1, 255, 0.1, 1);
}

String EffectBengalL::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) centerRun = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectBengalL::phisics(byte id) {
  sparksPos[0][id] += sparksSpeed[0][id] * speedFactor;
  sparksPos[1][id] += sparksSpeed[1][id] * speedFactor;
  sparksSpeed[1][id] -= .98 * speedFactor;
  sparksSat[id] += (255. / (float)WIDTH) * speedFactor;            // остывание искор
  sparksFade[id] -= (255. / (float)(HEIGHT*1.5)) * speedFactor;    // угасание искор
  if (sparksSpeed[0][id] > 0)
    sparksSpeed[0][id] -= 0.1 * speedFactor;
  else
    sparksSpeed[0][id] += 0.1 * speedFactor;
  if (sparksPos[0][id] <= 0 || sparksPos[0][id] >= WIDTH * 10 || sparksPos[1][id] < 0) {
    regen(id);
  }
}

void EffectBengalL::regen(byte id) {
  sparksPos[0][id] = gPos[0];
  sparksPos[1][id] = gPos[1];
  sparksSpeed[0][id] = random(-10, 10);
  sparksSpeed[1][id] = random(-5, 20);
  sparksColor[id] = random8();
  sparksSat[id] = 10;
  sparksFade[id] = 255;
}


void EffectBengalL::setspd(const byte _spd)
{
  EffectCalc::setspd(_spd); // дернем базовый, где будет пересчет палитры
  speedFactor = EffectMath::fmap(speed, 1, 255, 0.1, 1);
}


bool EffectBengalL::run(CRGB *leds, EffectWorker *opt) {
  fadeToBlackBy(leds, NUM_LEDS, beatsin8(5, 20, 100));
  if (centerRun) {
    gPos[0] = beatsin16(_x, 0, (WIDTH - 1) * 10);
    gPos[1] = beatsin16(_y, 0, (HEIGHT - 1) * 10);
  } else {
    gPos[0] = WIDTH / 2 * 10;
    gPos[1] = HEIGHT / 2 * 10;
  }
  for (byte i = 0; i < map(scale, 1, 255, 8, sparksNum); i++) {
    phisics(i);
    if (sparksPos[1][i] < ((HEIGHT - 1) * 10) and sparksPos[1][i] >= 0)
      if (sparksPos[0][i] < ((WIDTH - 1) * 10) and sparksPos[0][i] >= 0)
        EffectMath::drawPixelXYF(sparksPos[0][i] / 10,  sparksPos[1][i] / 10, CHSV(sparksColor[i], constrain(sparksSat[i], 5, 255), constrain(sparksFade[i], 32, 255)));
  }

  EVERY_N_SECONDS(period) {
    for (byte i = 0; i < map(scale, 1, 255, 1, sparksNum); i++) regen(i);
    period = random(10, 60);
    _x = random(1, 10);
    _y = random(1, 10);
  }
  return true;
}

// ----------- Эффект "Шары"
// (c) stepko and kostyamat https://wokwi.com/arduino/projects/289839434049782281
// 07.02.2021
void EffectBalls::load() {
  randomSeed(millis());
  palettesload();

  speedFactor = EffectMath::fmap(speed, 1, 255, 0.15, 0.5);

  for (byte i = 0; i < ballsAmount; i++) {
    radius[i] = EffectMath::randomf(0.5, radiusMax);
    ball[i][2] = EffectMath::randomf(0.5, 1.1) * speedFactor;
    ball[i][3] = EffectMath::randomf(0.5, 1.1) * speedFactor;
    ball[i][0] = random(0, WIDTH);
    ball[i][1] = random(0, HEIGHT);
    color[i] = random(0, 255);
  }
}

/*String EffectBalls::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) centerRun = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}*/

void EffectBalls::setspd(const byte _spd)
{
  EffectCalc::setspd(_spd); // дернем базовый, где будет пересчет палитры
  speedFactor = EffectMath::fmap(speed, 1, 255, 0.15, 0.5);
}

void EffectBalls::fill_circle(float cx, float cy, float radius, CRGB col) {
  radius -= 0.5;
  for (int y = -radius; y <= radius; y++) {
    for (int x = -radius; x <= radius; x++) {
      if (x * x + y * y <= radius * radius)
        EffectMath::drawPixelXYF(cx + x, cy + y, col);
    }
  }
}

bool EffectBalls::run(CRGB *leds, EffectWorker *opt) {
  fadeToBlackBy(leds, NUM_LEDS, map(speed, 1, 255, 5, 20));

  for (byte i = 0; i < map(scale, 1, 255, 2, ballsAmount); i++) {
    if (rrad[i]) {  // тут у нас шарики надуваются\сдуваются по ходу движения
      radius[i] += (fabs(ball[i][2]) > fabs(ball[i][3])? fabs(ball[i][2]) : fabs(ball[i][3])) * 0.1 * speedFactor;
      if (radius[i] >= radiusMax) {
        rrad[i] = false;
      }
    } else {
      radius[i] -= (fabs(ball[i][2]) > fabs(ball[i][3])? fabs(ball[i][2]) : fabs(ball[i][3])) * 0.1 * speedFactor;
      if (radius[i] < 1.) {
        rrad[i] = true;
        color[i] = random(0, 255);
      }
    }


    //EffectMath::drawCircleF(ball[i][1], ball[i][0], radius[i], ColorFromPalette(*curPalette, color[i]), 0.5);
    if (radius[i] > 1) 
      fill_circle(ball[i][1], ball[i][0], radius[i], ColorFromPalette(*curPalette, color[i]));
    else 
      EffectMath::drawPixelXYF(ball[i][1], ball[i][0], ColorFromPalette(*curPalette, color[i]));


    if (ball[i][0] + radius[i] >= HEIGHT - 1)
      ball[i][0] += (ball[i][2] * ((HEIGHT - 1 - ball[i][0]) / radius[i] + 0.005));
    else if (ball[i][0] - radius[i] <= 0)
      ball[i][0] += (ball[i][2] * (ball[i][0] / radius[i] + 0.005));
    else
      ball[i][0] += ball[i][2];
    //-----------------------
    if (ball[i][1] + radius[i] >= WIDTH - 1)
      ball[i][1] += (ball[i][3] * ((WIDTH - 1 - ball[i][1]) / radius[i] + 0.005));
    else if (ball[i][1] - radius[i] <= 0)
      ball[i][1] += (ball[i][3] * (ball[i][1] / radius[i] + 0.005));
    else
      ball[i][1] += ball[i][3];
    //------------------------
    if (ball[i][0] < 0.01) {
      ball[i][2] = EffectMath::randomf(0.5, 1.1) * speedFactor;
      ball[i][0] = 0.01;
    }
    else if (ball[i][0] > HEIGHT - 1.01) {
      ball[i][2] = EffectMath::randomf(0.5, 1.1) * speedFactor;
      ball[i][2] = -ball[i][2];
      ball[i][0] = HEIGHT - 1.01;
    }
    //----------------------
    if (ball[i][1] < 0.01) {
      ball[i][3] = EffectMath::randomf(0.5, 1.1) * speedFactor;
      ball[i][1] = 0.01;
    }
    else if (ball[i][1] > WIDTH - 1.01) {
      ball[i][3] = EffectMath::randomf(0.5, 1.1) * speedFactor;
      ball[i][3] = -ball[i][3];
      ball[i][1] = WIDTH - 1.01;
    }
  }
  blur2d(leds, WIDTH, HEIGHT, 48);
  return true;
}

// ---------- Эффект-игра "Лабиринт"
void EffectMaze::newGameMaze() {
  playerPos[0] = !MAZE_SHIFT;
  playerPos[1] = !MAZE_SHIFT;

  gameOverFlag = false;
  buttons = 4;

  GenerateMaze(maze, MAZE_WIDTH, MAZE_HEIGHT);    // генерировать лабиринт обычным способом
  SolveMaze(maze, MAZE_WIDTH, MAZE_HEIGHT);       // найти путь

  if (!(GAMEMODE || mazeMode)) {
    for (byte y = 0; y < MAZE_HEIGHT; y++) {
      for (byte x = 0; x < MAZE_WIDTH; x++) {
        switch (maze[(y + MAZE_SHIFT) * MAZE_WIDTH + (x + MAZE_SHIFT)]) {
          case 1:  EffectMath::drawPixelXY(x, y, color); break;
          case 2:
            EffectMath::drawPixelXY(x, y, 0x000000);
            break;
          default: EffectMath::drawPixelXY(x, y, 0x000000); break;
        }
      }
      // Отображаем сгенерированный лабиринт строка за строкой
      FastLED.show();  // стоит убрать, и все начинает мерцать
      delay(50);
    }

  } else {
    for (byte y = 0; y < FOV; y++) {
      for (byte x = 0; x < FOV; x++) {
        switch (maze[(y + MAZE_SHIFT) * MAZE_WIDTH + (x + MAZE_SHIFT)]) {
          case 1:  EffectMath::drawPixelXY(x, y, color);  break;
          case 2:
            EffectMath::drawPixelXY(x, y, 0x000000);
            break;
          default: EffectMath::drawPixelXY(x, y, 0x000000);  break;
        }
      }
    }
  }

  // Отрисовка - с видимыми границами по периметру (настройки MAZE_SHIFT выше)
  // Слева от начальной позиции делаем дыру - это вход
  if (playerPos[0]>0) {
    playerPos[0] = playerPos[0] - 1;
    EffectMath::drawPixelXY(playerPos[0], playerPos[1], 0x000000);
  }
  
  EffectMath::drawPixelXY(playerPos[0], playerPos[1],  playerColor);

  mazeStarted = false;  
}

bool EffectMaze::run(CRGB *ledarr, EffectWorker *opt) {
  if (loadingFlag || gameOverFlag) {  
    if (loadingFlag) FastLED.clear();
    gameTimer.setInterval(map(speed, 1, 255, 500, 50));   // установить начальную скорость
    loadingFlag = false;
    newGameMaze();
    // modeCode = MC_MAZE;
  }

  if (gameDemo && !gamePaused) demoMaze();
  buttonsTickMaze();
  return true;
}

void EffectMaze::buttonsTickMaze() {
  if (gameDemo && !gameTimer.isReady()) { // тут крутим скорость в демо-режиме
    return;
  }

  if (checkButtons()) {
    bool btnPressed = false;
    if (buttons == 3) {   // кнопка нажата
      btnPressed = true;
      int8_t newPos = playerPos[0] - 1;
      if (newPos >= 0 && newPos <= (int)WIDTH - 1)
        if (EffectMath::getPixColorXY(newPos, playerPos[1]) == 0) {
          movePlayer(newPos, playerPos[1], playerPos[0], playerPos[1]);
          playerPos[0] = newPos;
        }
    }
    if (buttons == 1) {   // кнопка нажата
      btnPressed = true;
      int8_t newPos = playerPos[0] + 1;
      if (newPos >= 0 && newPos <= (int)WIDTH - 1)
        if (EffectMath::getPixColorXY(newPos, playerPos[1]) == 0) {
          movePlayer(newPos, playerPos[1], playerPos[0], playerPos[1]);
          playerPos[0] = newPos;
        }
    }
    if (buttons == 0) {   // кнопка нажата
      btnPressed = true;
      int8_t newPos = playerPos[1] + 1;
      if (newPos >= 0 && newPos <= (int)HEIGHT - 1)
        if (EffectMath::getPixColorXY(playerPos[0], newPos) == 0) {
          movePlayer(playerPos[0], newPos, playerPos[0], playerPos[1]);
          playerPos[1] = newPos;
        }
    }
    if (buttons == 2) {   // кнопка нажата
      btnPressed = true;
      int8_t newPos = playerPos[1] - 1;
      if (newPos >= 0 && newPos <= (int)HEIGHT - 1)
        if (EffectMath::getPixColorXY(playerPos[0], newPos) == 0) {
          movePlayer(playerPos[0], newPos, playerPos[0], playerPos[1]);
          playerPos[1] = newPos;
        }
    }
    if (btnPressed && !mazeStarted) {
      mazeStarted = true;
      labTimer = millis();
    }
    buttons = 4;
  }
}

void EffectMaze::movePlayer(int8_t nowX, int8_t nowY, int8_t prevX, int8_t prevY) {
  if (!track) EffectMath::drawPixelXY(prevX, prevY, 0x000000);
  EffectMath::drawPixelXY(nowX, nowY,  playerColor);

  if ((nowX == (MAZE_WIDTH - 2) - MAZE_SHIFT) && (nowY == (MAZE_HEIGHT - 1) - MAZE_SHIFT)) {
    gameOverFlag = true;

  /*  FastLED.show();
    delay(250);
    FastLED.clear();
    //if (!gameDemo) {
      displayScore((millis() - labTimer) / 1000);
      FastLED.show();
    //}
    delay(1500);*/
    return;
  }

  if (GAMEMODE || mazeMode) {
    for (int8_t y = nowY - FOV; y < nowY + FOV; y++) {
      for (int8_t x = nowX - FOV; x < nowX + FOV; x++) {
        if (x < 0 || x > (int)WIDTH - 1 || y < 0 || y > (int)HEIGHT - 1) continue;
        if (maze[(y + MAZE_SHIFT) * MAZE_WIDTH + (x + MAZE_SHIFT)] == 1) {
          EffectMath::drawPixelXY(x, y, GLOBAL_COLOR_1);
        }
      }
      FastLED.show();
    }
  }
}

void EffectMaze::demoMaze() {
  if (checkPath(0, 1)) buttons = 0;
  if (checkPath(1, 0)) buttons = 1;
  if (checkPath(0, -1)) buttons = 2;
  if (checkPath(-1, 0)) buttons = 3;
}

bool EffectMaze::checkPath(int8_t x, int8_t y) {
  // если проверяемая клетка является путью к выходу
  if ( (maze[(playerPos[1] + y + MAZE_SHIFT) * MAZE_WIDTH + (playerPos[0] + x + MAZE_SHIFT)]) == 2) {
    maze[(playerPos[1] + MAZE_SHIFT) * MAZE_WIDTH + (playerPos[0] + MAZE_SHIFT)] = 4;   // убираем текущую клетку из пути (2 - метка пути, ставим любое число, например 4)
    return true;
  }
  else return false;
}

// копаем лабиринт
void EffectMaze::CarveMaze(char *maze, int width, int height, int x, int y) {
  int x1, y1;
  int x2, y2;
  int dx, dy;
  int dir, count;

  dir = random(10) % 4;
  count = 0;
  while (count < 4) {
    dx = 0; dy = 0;
    switch (dir) {
      case 0:  dx = 1;  break;
      case 1:  dy = 1;  break;
      case 2:  dx = -1; break;
      default: dy = -1; break;
    }
    x1 = x + dx;
    y1 = y + dy;
    x2 = x1 + dx;
    y2 = y1 + dy;
    if (   x2 > 0 && x2 < width && y2 > 0 && y2 < height
           && maze[y1 * width + x1] == 1 && maze[y2 * width + x2] == 1) {
      maze[y1 * width + x1] = 0;
      maze[y2 * width + x2] = 0;
      x = x2; y = y2;
      dir = random(10) % 4;
      count = 0;
    } else {
      dir = (dir + 1) % 4;
      count += 1;
    }
  }
}

// генерацтор лабиринта
void EffectMaze::GenerateMaze(char *maze, int width, int height) {
  int x, y;
  for (x = 0; x < width * height; x++) {
    maze[x] = 1;
  }
  maze[1 * width + 1] = 0;
  for (y = 1; y < height; y += 2) {
    for (x = 1; x < width; x += 2) {
      CarveMaze(maze, width, height, x, y);
    }
  }
  // вход и выход
  maze[0 * width + 1] = 0;
  maze[(height - 1) * width + (width - 2)] = 0;

  track = random8(0,2);
  color = CHSV(hue += 8, random8(192, 255), 192);
  
 /* CHSV tmp = rgb2hsv_approximate(color);
  tmp.s = 255;
  tmp.hue += 100;
  tmp.val = 255; */

  playerColor = CHSV(hue + random(63, 127), random8(127, 200), 255);;
  
}

// решатель (ищет путь)
void EffectMaze::SolveMaze(char *maze, int width, int height) {
  int dir, count;
  int x, y;
  int dx, dy;
  int forward;
  // Remove the entry and exit. 
  maze[0 * width + 1] = 1;
  maze[(height - 1) * width + (width - 2)] = 1;

  forward = 1;
  dir = 0;
  count = 0;
  x = 1;
  y = 1;
  unsigned int attempts = 0;
  while (x != width - 2 || y != height - 2) {
    if (attempts++ > maxSolves) {   // если решатель не может найти решение (maxSolves в 5 раз больше числа клеток лабиринта)
      gameOverFlag = true;          // перегенерировать лабиринт
      break;                        // прервать решение
    }
    dx = 0; dy = 0;
    switch (dir) {
      case 0:  dx = 1;  break;
      case 1:  dy = 1;  break;
      case 2:  dx = -1; break;
      default: dy = -1; break;
    }
    if (   (forward  && maze[(y + dy) * width + (x + dx)] == 0)
           || (!forward && maze[(y + dy) * width + (x + dx)] == 2)) {
      maze[y * width + x] = forward ? 2 : 3;
      x += dx;
      y += dy;
      forward = 1;
      count = 0;
      dir = 0;
    } else {
      dir = (dir + 1) % 4;
      count += 1;
      if (count > 3) {
        forward = 0;
        count = 0;
      }
    }
  }
  
  // Replace the entry and exit.
  maze[(height - 2) * width + (width - 2)] = 2;
  maze[(height - 1) * width + (width - 2)] = 2;
}

void EffectMaze::setspd(const byte _spd)
{
  EffectCalc::setspd(_spd);
  gameTimer.setInterval(map(speed, 1, 255, 500, 50));   // установить скорость
}

// --------- Эффект "Вьющийся Цвет"
// (c) Stepko https://wokwi.com/arduino/projects/283705656027906572
// доработка - kostyamat
bool EffectFrizzles::run(CRGB *leds, EffectWorker *opt) {
  _speed = EffectMath::fmap(speed, 1, 255, 0.25, 3);
  if (scale <= 127) _scale = EffectMath::fmap(scale, 1, 255, 1, 8);
  else _scale = EffectMath::fmap(scale, 1, 255, 8, 1);

  for(float i= (float)8 * _scale; i> 0; i--)
    leds[myLamp.getPixelNumber(beatsin8(12. * _speed + i * _speed, 0, WIDTH - 1), beatsin8(15. * _speed + i * _speed, 0, HEIGHT - 1))] = CHSV(beatsin8(12. * _speed, 0, 255), scale > 127 ? 255 - i*8 : 255, scale > 127 ? 127 + i*8 : 255);
  blur2d(leds, WIDTH, HEIGHT, 16);
  return true;
}

// --------- Эффект "Северное Сияние"
// (c) kostyamat 05.02.2021
// идеи подсмотрены тут https://www.reddit.com/r/FastLED/comments/jyly1e/challenge_fastled_sketch_that_fits_entirely_in_a/
// особая благодарность https://www.reddit.com/user/ldirko/ Yaroslaw Turbin aka ldirko
void EffectPolarL::load() {
  adjastHeight = EffectMath::fmap((float)HEIGHT, 8, 32, 28, 12);
  adjScale = map((int)WIDTH, 8, 64, 310, 63);
  palettesload();
}

void EffectPolarL::palettesload(){
  // собираем свой набор палитр для эффекта
  palettes.reserve(numpalettes);
  palettes.push_back(&AuroraColors_p); 
  palettes.push_back(&RainbowColors_p);
  palettes.push_back(&PartyColors_p);
  palettes.push_back(&HeatColors_p);
  palettes.push_back(&RainbowColors_p);
  palettes.push_back(&HeatColors_p);
  palettes.push_back(&LithiumFireColors_p);
  palettes.push_back(&GreenAuroraColors_p); 
  palettes.push_back(&BlueAuroraColors_p);
  palettes.push_back(&NeonAuroraColors_p);
  palettes.push_back(&PotassiumFireColors_p);
  palettes.push_back(&WoodFireColors_p);
  palettes.push_back(&SodiumFireColors_p);
  palettes.push_back(&WaterfallColors_p); 
   
  usepalettes = true; // включаем флаг палитр
  scale2pallete();    // выставляем текущую палитру
}

// нахрена кастомный мапинг тут???
// void EffectPolarL::palettemap(std::vector<PGMPalette*> &_pals, const uint8_t _val, const uint8_t _min, const uint8_t _max){
//   std::size_t idx = (_val-1); // т.к. сюда передается точное значение контрола, то приводим его к 0
//   if (!_pals.size() || idx>=_pals.size()) {
//     LOG(println,F("No palettes loaded or wrong value!"));
//     return;
//   }

//     flag = idx;

//   curPalette = _pals.at(idx);
// }

void EffectPolarL::setscl(const byte _scl)
{
  EffectCalc::setscl(_scl); // дернем базовый, где будет пересчет палитры
  _scale = map(scale, 1, 255, 30, adjScale);
}

bool EffectPolarL::run(CRGB *leds, EffectWorker *opt) {
  byte _speed = map(speed, 1, 255, 128, 16);
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT; y++) {
      timer++;
      //uint16_t i = x*y;
      leds[myLamp.getPixelNumber(x, y)]= 
          ColorFromPalette(*curPalette,
            qsub8(
              inoise8(/*i*/timer % 2 + x * _scale,
                y * 16 + timer % 16,
                timer / _speed
              ),
              fabs((float)HEIGHT/2 - (float)y) * adjastHeight
            )
          );
      if (flag == 1) { // Тут я модифицирую стандартные палитры 
        CRGB tmpColor = leds[myLamp.getPixelNumber(x, y)];
        leds[myLamp.getPixelNumber(x, y)].g = tmpColor.r;
        leds[myLamp.getPixelNumber(x, y)].r = tmpColor.g;
        leds[myLamp.getPixelNumber(x, y)].g /= 6;
        leds[myLamp.getPixelNumber(x, y)].r += leds[myLamp.getPixelNumber(x, y)].r < 206 ? 48 : 0;;
      } else if (flag == 3) {
        leds[myLamp.getPixelNumber(x, y)].b += 48;
        leds[myLamp.getPixelNumber(x, y)].g += leds[myLamp.getPixelNumber(x, y)].g < 206 ? 48 : 0;
      }
    }
  }
  //if (flag) blur2d(leds, WIDTH, HEIGHT, 20);

  return true;
}

// --------- Эффект "Гонщик"
// (c) Stepko + kostyamat https://editor.soulmatelights.com/my-patterns/655
void EffectRacer::setspd(const byte _spd) {
  EffectCalc::setspd(_spd);
  speedFactor = _speed * EffectMath::fmap(speed, 1, 255, 0.33, 2);
  addRadius = _addRadius * EffectMath::fmap(speed, 1, 255, 0.33, 2);
}

bool EffectRacer::run(CRGB *leds, EffectWorker *opt) {
  fadeToBlackBy(leds, NUM_LEDS, 16 * speedFactor);

  if (round(posX / 4) > aimX) {
    posX -= speedFactor;
  }
  if (round(posY / 4) > aimY) {
    posY -= speedFactor;
  }
  if (round(posX / 4) < aimX) {
    posX += speedFactor;
  }
  if (round(posY / 4) < aimY) {
    posY += speedFactor;
  }
  if (round(posX / 4) == aimX && round(posY / 4) == aimY) {
    aimChange();
  }
  radius += addRadius;
  angle += radius;
  switch (hue%3)
  {
  case 0:
    EffectMath::drawCircleF(aimX, aimY, radius, color); // рисуем круг
    break;
  
  case 1:
    drawStarF(aimX, aimY, 1.3 * radius, radius, 4, angle, color); // рисуем квадрат
    break;
  case 2:
    drawStarF(aimX, aimY, 2 * radius, radius, starPoints, angle, color); // рисуем звезду
    break;
  }
  
  EffectMath::drawPixelXYF(posX / 4, posY / 4, CHSV(0, 0, 255)); // отрисовываем бегуна

  return true;
}

void EffectRacer::load() {
  palettesload();
}

void EffectRacer::aimChange() {
  aimX = random(0, WIDTH-1);  // позиция цели 
  aimY = random(0, HEIGHT-1);
  radius = 1; // начальный размер цели = 1 пиксель
  hue = millis()>>1; //random(0, 255);
  color = ColorFromPalette(*curPalette, hue, 180);
  starPoints = random(3, 7); // количество лучей у звезды
}

void EffectRacer::drawStarF(float x, float y, float biggy, float little, int16_t points, float dangle, CRGB color) {
  float radius2 = 255.0 / points;
  for (int i = 0; i < points; i++) {
    EffectMath::drawLineF(x + ((little * (sin8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128), y + ((little * (cos8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128), x + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128), y + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128), color);
    EffectMath::drawLineF(x + ((little * (sin8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128), y + ((little * (cos8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128), x + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128), y + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128), color);

  }
}
