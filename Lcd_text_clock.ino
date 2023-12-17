#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <microDS3231.h>
#include <stdio.h>
#include <EncButton.h>


// Пины RTC модуля 
# define RST 6
# define DAT 7
# define CLC 8

// Пины энкодера
# define CLK 3
# define DT 2
# define SW 4

 
// Создаём объект часов
MicroDS3231 rtc;  

// Создаём объект дисплея
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Создаём объект энеодер, c кнопкой
EncButton enc(CLK, DT, SW, INPUT, INPUT_PULLUP);


// Объявим переменные 
int last_day = 0;           // для ежедневного рбновления даты на дисплее
float current_temp =0.0;
short menu_level = -3;       // уровень меню
short line_lcd = 0;         // строка дисплея
int dt[6];                  // хранит текущие значения времени и даты для настройки
uint32_t sec_blink = millis(); // для работы с задержками

/*
 DS3231 согласно datasheet имеен погрешность +/- 3 гр. 
 Опытным путём была утановлена погрешность данного модуля в 1.2 град в большую сторону.
*/
const float correction_temp = 1.2;


// Пункты меню
const int ml_hour_and_day = 0;
const int ml_minute_and_month = 3;
const int ml_year = 6;

// флаги
bool is_settings = false;  // флаг дла работы с меню настроек
bool on_off_lcd = true;    // флаг для мигания курсором
bool change_min = false;   // флаг для определения были ли изменины минуты



void setup() {
  // Настройки lCD
  lcd.begin(); 
  lcd.backlight();
   
  // Настройки порта
   Serial.begin(9600);
  
  // Настройки часов
  if (rtc.lostPower()) {            // выполнится при сбросе батарейки
    Serial.println("lost power!");
        // визуально громоздкий, но более "лёгкий" с точки зрения памяти способ установить время компиляции
    rtc.setTime(BUILD_SEC, BUILD_MIN, BUILD_HOUR, BUILD_DAY, BUILD_MONTH, BUILD_YEAR);
  }
}



void loop() { 
  // обязательная функция отработки. Должна постоянно опрашиваться
  enc.tick();
  setup_rtc();

  if(!is_settings){
  // если прошла 1 секунда выводим время
    if (millis() % 1000 == 0) {
      // вывод времени
      lcd.setCursor(0, 0);
      lcd.print(rtc.getTimeString());
      Serial.println(rtc.getTemperatureFloat());

      if(last_day != rtc.getDay()){
        // вывод даты, обновляется раз в сутки
        lcd.setCursor(0, 1);
        lcd.print(rtc.getDateString());
        last_day = rtc.getDay();
      }

      if(current_temp != rtc.getTemperatureFloat()){
        // вывод температуры, при езменении
        current_temp = rtc.getTemperatureFloat() - correction_temp;
        lcd.setCursor(9, 0);
        lcd.print(current_temp);
        lcd.print(" C");
      }
    }
  }
}


void setup_rtc(){
  /* функция установки даты и времени 
    Проверяет нажатие кнопки Энкодера.
    Проверяет пворот ручки энкодера.
    Изменент текущие значение времени и даты в 
  */

  if(enc.press() ){

    if(!is_settings){ 
      update_initial_values_dt();     
    }
    menu_level += 3;                  // переход на следующий уровень меню
  }

  if(menu_level >= 6 && line_lcd == 0){
    // переход на слюдующую строку дисплея
      ++line_lcd;
      menu_level = 0;
    }

  if(menu_level > 7 && line_lcd == 1){
    // выход из настроек и сохранение
    save_settings_and_exit();
  }

  if(is_settings){

    if (menu_level == ml_hour_and_day && line_lcd == 0 ){
      // Настройка часов
      change_the_numbers(menu_level, line_lcd, 0, 0, 24);
    }

    if (menu_level == ml_minute_and_month && line_lcd == 0 ){
      // настройка минут
      change_the_numbers(menu_level, line_lcd, 1, 0, 60);

      // Провека былили изменины минуты
      if( enc.turn() && !change_min) change_min = true;   
    }

    if (menu_level == ml_hour_and_day && line_lcd == 1 ){
      // Настройка числа
      change_the_numbers(menu_level, line_lcd, 2, 1, 31);
    }

    if (menu_level == ml_minute_and_month && line_lcd == 1 ){
      // настройка месяца
      change_the_numbers(menu_level, line_lcd, 3, 1, 12);   
    }

    if (menu_level == ml_year && line_lcd == 1 ){
      // настройка года
      change_the_numbers(menu_level, line_lcd, 4, 1988, 2099);
    
    }
  
  }
}

void change_the_numbers(int menu_lvl, int line, short pos, short min_value, short max_value){
  // Функция меняет и отображает числа

  /*
    - menu_lvl, line позиция на дисплее и строка
    - pos позиция в списке для замены.
    - max_value, min_value максимольное и минимальное значение даты времени.
     
  */

  lcd.setCursor(menu_level, line_lcd);
  // изменение знаяения 
  if(enc.right()) dt[pos] += 1;
  if(enc.left()) dt[pos] -= 1;

  // проверка на переполнение
  if (dt[pos] > max_value) dt[pos] = min_value;
  if (dt[pos] < min_value) dt[pos] = max_value;

  // добавление лидирующиго нуля при необходимости
  if(dt[pos] < 10){
    print_on_lcd("0%d", dt[pos]);
  }

  else{
    print_on_lcd("%d", dt[pos]);
  }
     

}


void print_on_lcd(char *format_str, int value){
 // Вывод на дисплей во время настройки чвсов

  char* buffer = malloc(4);
  sprintf(buffer, format_str, value);
  lcd.print(buffer);
  // if(millis() - sec_blink < 3500){
  //   on_off_lcd = !on_off_lcd;
  //   sec_blink =  millis();
  // }
    
  // if(on_off_lcd) lcd.print(buffer);
  // if(!on_off_lcd) lcd.print("  ");

  free(buffer);
}

void save_settings_and_exit(){
  // Сброс пременных на начальные значения и сохранение значений

  if (!change_min) dt[1] = rtc.getMinutes();  // если минуты не изменяли, обновляем данные в массиве, для сохранения точности хода

  // сброс значений на исходные 
  menu_level = -3;
  line_lcd = 0;
  is_settings = false;
  change_min = false; 

  // обновление параметров
  rtc.setTime(rtc.getSeconds(), dt[1], dt[0], dt[2], dt[3], dt[4]);
}
void update_initial_values_dt(){
    // Обновление начальных заначений перед установкой даты и времени
    DateTime now = rtc.getTime();
    dt[0] = now.hour;
    dt[1] = now.minute;
    dt[2] = now.date;
    dt[3] = now.month;
    dt[4] = now.year;

    // изменяем флаг входа в меню
    is_settings = true;              // при первом нажатии входим в меню  
}

