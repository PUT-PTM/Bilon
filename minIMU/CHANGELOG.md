BiloCopter
==========

Problematyka 
------------
Gotowy kod wykonany przez kolegów, działa błędnie.
Na załączonych print screenach widać to wyraźnie:
- [zjdecie1][screen1] minIMU zostało unieruchomione
pomimo to wyniki pomiarów rozjeżdżają się.
- [zdjecie2][screen2] obracamy modułem o 90 stopni wokół osi X(Roll),
odwzorowanie na wykresie nie przypomina tego.
- [screen3] USB w trybie SPI przesyła/odbiera
losowo wybrane dane. Co sprawia ze są one źle przetwarzane.

Co może być źle?
----------------
Żyroskop z minIMU (L3GD20) zwraca wartość w stopniach
filtr Madgwick dokonuje obliczeń na radianach, nie jest 
to uwzględnione w implementacji projektu.

Wątpliwa jest jakość konwersji kwaternionów na kąty eulera [słaba dokładność]
// Nadal nie działa ...

Co dalej ?
----------
W związku z licznymi problemami które napotkaliśmy, postanowiliśmy porzucić
rozwój pracy kolegów i zacząć projekt od nowa. Zamierzamy zaimplementować
wszystkie obliczenia na STM32 (co i tak by musiało nastąpić), co do komunikacji
z komputerem STM Studio wydaje się wystarczającym narzędziem.
Jeśli uda nam się uzyskać poprawne wyniki będziemy myśleć co dalej.
