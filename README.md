# Часы реального времени. 

  Используемые модули:
  - МК Arduino Nano
  - Модуль часов DS3231
  - LCD 1602 i2c
  - Энкодер с обвязкой (круглый).
## Схема подключения
 ![Схема подключения](https://github.com/AbramtsevFV/RTC_I2C_Enc/blob/master/connection%20diagram.jpg)

## Настройка даты и времени
  Нажатие на энкодер переводит часы в режим настройки. Каждое нажатие переключает меню
  1) Часы
  2) Минуты
  3) День
  4) Месяц
  5) Год
  6) Выход из меню и сохранение параметров
Если минуты не изменялись, то минуты в момент сохранения  синронизируются с модулем RTC.
