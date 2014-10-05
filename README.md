BiloCopter
==========

Madgwick ahrs
-------------
http://www.x-io.co.uk/open-source-imu-and-ahrs-algorithms/
Nie udało nam się tego okiełznać problemy były z dryffem (wartości uciekały) brak dokładności.

Pololu ahrs
-----------

https://github.com/pololu/minimu-9-ahrs-arduino

Przeportowaliśmy kod dla arduino na stm32 -> rezultat zadawalający (Działa lepiej niż na arduino)

Co jest ważne ?
- Implementacja komunikacji USB
- Obsługa I2C
- Typy zmiennych (signed/unsigned)
- Konfiguracja wstępna
