# Plan doprowadzenia firmware do pelnego dzialania

Ten plik opisuje stan refaktoru projektu `NUCLEO-L152RE (STM32 CMake)` wzgledem starego projektu CubeIDE `NUCLEO-L152RE (STM32)`. Celem jest firmware, ktory buduje sie z CMake, uruchamia sie na NUCLEO-L152RE i ma czytelna architekture pod portfolio.

## Aktualny stan

Nowy projekt ma juz:

- konfiguracje CubeMX/CMake, linker script, startup, HAL, FatFS i wygenerowane peryferia,
- dzialajacy preset Debug na CMake/Ninja z toolchainem `arm-none-eabi-gcc` z pakietu STM32Cube,
- build Debug przechodzacy do `active-load.elf`,
- szkielety modulow: `app`, `logger`, `measurement`, `param_table`, `comms`,
- driver `DAC8830`,
- driver `MCP3561T` z rejestrami, zapisem/odczytem konfiguracji, komendami fast command i podstawowym `Deinit`,
- konfiguracje ADC dla pomiaru pradu i napiecia,
- kompatybilny rdzen stanu aplikacji z legacy `data[67]` i tymi samymi offsetami pol co w starym `myData`,
- `param_table` opisujacy stare offsety, typy, zakresy i wartosci domyslne,
- parser starej ramki PC w `comms`, zapisujacy dane do tych samych pol co stare `saveDataToStruct()`,
- czesciowy zapis/odczyt EEPROM pod tymi samymi adresami co stary projekt,
- odczyt surowych probek MCP3561T przez `mcp3561t_ReadAdcRaw()`,
- `measurement` podpiete do petli 50 ms: odczyt pradu/napiecia, stare przeliczenia z `mystruct.c`, aktualizacja `current`, `battery voltage`, `parameter` i przekaznikow zakresu,
- poprawione legacy mapowanie ADC: napiecie na `CS_1`, prad na `CS_2`,
- modul `load_output` obudowujacy DAC8830 i zapisujacy `DAC output` do legacy offsetu `data[16]`,
- regulator PI/PID przeniesiony ze starego `regulator.c`: tryby `C/P/R`, kompensacja temperaturowa, anty-windup, saturacja 0..15 A, mapping DAC `((output / 15) * 37683) + 27852`,
- modul `safety` z warunkami ze starego `main.c`: undervoltage, limit Ah, overtemperature przez dwa kolejne odczyty, `timeStop`; przy stopie zeruje DAC i ustawia `onOff = 0`,
- modul `temp_sensor` dla DS18B20: start konwersji, odczekanie ok. 710 ms, odczyt `temp_1` i `temp_3` do legacy offsetow `data[20]` i `data[28]`,
- SD logging w `logger_sd`: ten sam naglowek CSV, ten sam format wiersza i harmonogram 100 ms przez pierwsza minute, potem `loggingSpeed`,
- backend FatFS SPI (`user_diskio_spi`) przeniesiony ze starego projektu i podpiety pod `hspi3`/`SD_CS`,
- LCD/menu/enkoder w `lcd_ui`: port starego HD44780 4-bit, ekrany default/turn/mode/logging, odswiezanie po zmianie albo ok. co 2 s oraz reakcje na B1, iCLK i iSW,
- minimalny start aplikacji: `app_Init()`, start `TIM2`, start `TIM6`, start UART RX, DAC ustawiany na zero,
- cienki `main()` z `app_Process()` w petli glownej i flaga 50 ms wystawiana z callbacka TIM2.

Stary projekt zawiera jeszcze nieprzeniesiona logike:

- `Core/Src/mystruct.c/h` - centralny stan, gettery/settery, pomiary temperatury, ladunek `qlmb`,
- `Core/Src/regulator.c/h` - PI/PID, kompensacja temperaturowa, inicjalizacja MCP3561T w starej formie,
- `Core/Src/komunikacjaPC2.c/h` - protokol UART, callback RX, EEPROM save/load,
- `Core/Src/sd.c/h` - logowanie CSV na SD,
- `Core/Src/interface.c/h` - LCD, menu, przyciski/enkoder,
- `Core/Src/main.c` - orkiestracja petli, safety checks i harmonogram logowania.

## Stan po pierwszym buildzie

Zamkniete blokery startowe:

- CMake preset zostal przelaczony na `cmake/gcc-arm-none-eabi.cmake`, a toolchain znajduje kompilator z `~/.local/share/stm32cube/bundles/gnu-tools-for-stm32/14.3.1+st.2/bin`.
- `adc_current_config.c` i `adc_voltage_config.c` sa dodane do `target_sources()` w glownym `CMakeLists.txt`.
- `Core/Src/main.c` wywoluje `app_Init()`, startuje `TIM2`, `TIM6` i UART RX, a w petli glownej wola `app_Process()`.
- `HAL_TIM_PeriodElapsedCallback()` dla TIM2 wystawia tylko flage 50 ms przez `app_Notify50msFromIsr()`.
- `app_init.c` includuje bezposrednio potrzebne naglowki HAL/Cube, inicjalizuje logger, DAC i ADC.
- DAC8830 jest inicjalizowany na `hspi2` z `CS_3` i dostaje kod `0` na starcie.
- `comms`, `logger_sd` i `param_table` maja minimalne szkielety API zamiast pustych plikow.
- `logger_uart.c` ma poprawiony warunek inicjalizacji i przekazywanie wskaznika do bufora DMA.
- `measurement.c` ma poprawiony typ `MCP3561T_Config_t`, jawne CS z `main.h` i komentarz o topologii SPI.
- `mcp3561t_SaveConfig()` kopiuje pola z `cfg`, `mcp3561t_WriteConfig()` uzywa `||`, a `mcp3561t_Deinit()` jest zaimplementowany.
- `cube-cmake --build --preset Debug` linkuje `active-load.elf`.
- `cube-cmake --build --preset Release` linkuje `active-load.elf`.

Pozostale ryzyka przed testami na plytce:

- `measurement.c` nadal uzywa `dev_id = 1` dla obu ADC; to pasuje do starego kodu przy osobnych CS, ale warto potwierdzic na hardware.
- `comms` ma juz ring buffer i parser starej ramki, ale trzeba potwierdzic faktyczna dlugosc ramki/CRC z aplikacja PC. Nowy kod przyjmuje spojny wariant 37 bajtow: bajty 0..32 liczone do CRC, CRC w bajtach 33..36.
- `param_table` opisuje legacy offsety, ale nie ma jeszcze walidacji zakresow uzytej przez parser PC.
- `logger_sd` jest tylko placeholderem; nie ma jeszcze FatFS/CSV/bufora.
- `app_Process()` obsluguje flage 50 ms, `COMMS_Process()`, `MEAS_Poll()`, `TempSensor_Process()`, `Safety_Process()`, `Regulator_Run50ms()`, `log_sd_Process50ms()` i `LcdUi_Process()`.
- Odczyt ADC buduje sie, ale wymaga testu na plytce: sprawdzic polaryzacje, wartosci raw i czy statyczny odczyt `ADCDATA` wystarcza przy aktualnym `DATA_FORMAT_24_BIT`.
- Regulator buduje sie i zachowuje stary wzor, ale wymaga testu na obciazeniu/sztucznym sygnale przed wlaczeniem realnego obciazenia.
- Safety buduje sie i zachowuje stare progi, ale wymaga testu scenariuszy: napiecie <= 11.2 V, `qlmb / 3600 > 250`, `temp_1 >= 170` dwa razy, koniec `timeStop`.
- DS18B20 zachowuje stary bit-banging i brak CRC scratchpada; trzeba sprawdzic na plytce timing TIM6 oraz poprawnosc odczytow `temp_1`/`temp_3`.
- SD logging buduje sie i ma podpiety backend SPI, ale wymaga testu z karta SD. Bufor CSV ma kompatybilny limit 64x256 B, co podnosi RAM do ok. 22 KB.
- LCD/menu/enkoder buduje sie i jest wpiete w EXTI oraz petle 50 ms. Po objawie "jedna linia + krzaki" start LCD zostal wzmocniony: jawny reset linii RS/RW/EN/DB4..DB7 przed inicjalizacja, dluzszy start po zasilaniu i dodatkowy delay po `clear/home`. Po krzakach po wcisnieciu przyciskow timing impulsu `EN` zostal przywrocony do starego, wolnego 5 ms, a reakcje menu robia pelne odrysowanie aktywnego ekranu zamiast punktowych zmian kursora. Nadal wymaga testu na plytce: kolejnosc pinow DB4..DB7, polaryzacja enkodera, odbicia stykow i odbicia przyciskow.
- EEPROM zachowuje stare adresy i pola, ale brakuje jeszcze wersjonowania/CRC konfiguracji oraz migracji pustych/uszkodzonych danych.

## Minimalna kolejnosc prac

### 1. Toolchain i build

- [x] Udostepnic CMake/Ninja/ARM GCC z pakietow STM32Cube.
- [x] Ustalic docelowy preset na `gcc-arm-none-eabi.cmake`.
- [x] Dodac `adc_current_config.c` i `adc_voltage_config.c` do `target_sources()`.
- [x] Doprowadzic projekt do czystego `cube-cmake --build --preset Debug`.
- [x] Zweryfikowac `Release`.
- Dodac prosta instrukcje build/flash do `README.md` albo osobnego `BUILD.md`.

### 2. Naprawic szkielety, zeby firmware w ogole startowal

- [x] Naprawic wymienione blokery kompilacji w `logger`, `measurement` i `MCP3561T`.
- [x] W `main.c` dodac minimalna sekwencje startowa:
  - `app_Init()`,
  - `HAL_TIM_Base_Start_IT(&htim2)`,
  - `HAL_TIM_Base_Start(&htim6)`,
  - start odbioru UART RX po stronie modulu komunikacji,
  - bezpieczne ustawienie DAC na zero.
- [x] Przeniesc `HAL_TIM_PeriodElapsedCallback()` tak, aby TIM2 wystawial flage 50 ms, a nie wykonywal dlugich operacji w ISR.
- Ustalona zasada: ISR tylko zbiera bajty/flagi; parsowanie UART, SD, LCD i regulacja ida z petli glownej albo z malych funkcji cyklicznych.

### 3. Stan aplikacji i parametry

- Zastapic stare `myData.data[index]` czytelna struktura, np. `AppState` + `AppConfig`.
- Zachowac mapowanie protokolu UART ze starego projektu, ale ukryc je w `comms_pc`.
- Przeniesc pola: setpoint, mode `C/P/R`, on/off, logging, logging speed, buffer size, wire resistance, `Kp`, `Ki`, `Kd`, limit czasu, napiecie, prad, temperatury, DAC output, `qlmb`.
- `param_table` wykorzystac jako jedna liste parametrow: ID, typ, zakres, wartosc domyslna, wskaznik do pola konfiguracji.
- Dodac domyslne wartosci po pustym/niepoprawnym EEPROM.

### 4. Komunikacja PC i EEPROM

- Z `komunikacjaPC2.c` przeniesc parser ramki `[0x77][length][payload][crc]` do `Modules/comms`.
- Poprawic bug/ryzyko ze starego kodu: CRC jest liczony po bajtach, ale odbior `rx_crc` uzywa 4 bajtow z pozycji `buf_pos - 3`; trzeba jednoznacznie zdefiniowac format CRC i dlugosc ramki.
- Odbior UART powinien miec ring buffer, a nie bezposrednie zapisy do globalnej struktury z callbacka.
- Po poprawnej ramce: walidacja zakresow, aktualizacja `AppConfig`, zapis EEPROM, ACK/NACK.
- EEPROM wydzielic do osobnego modulu, np. `Modules/storage/config_store.c/h`.
- Dodac naglowek, wersje struktury i CRC konfiguracji, zeby zmiana formatu nie psula urzadzenia.

### 5. Pomiar ADC i DAC

- Driver `MCP3561T` doprowadzic do pelnego cyklu:
  - reset,
  - zapis konfiguracji,
  - odczyt konfiguracji do weryfikacji,
  - odczyt surowej probki ADC,
  - konwersja `raw -> voltage/current`,
  - timeout/error status.
- `measurement` ma wystawiac API aplikacyjne, np. `MEAS_Init()`, `MEAS_Poll()`, `MEAS_GetVoltage()`, `MEAS_GetCurrent()`.
- Zweryfikowac konfiguracje rejestrow MCP3561T wzgledem starego kodu: w nowym kodzie sa komentarze o poprawkach `0x63 -> 0x23` i `0x8B -> 0x8C`; trzeba potwierdzic to na hardware albo w datasheet.
- Driver `DAC8830` obudowac modulem wyjscia, np. `load_output.c/h`, ktory przelicza ampery na kod DAC i pilnuje zakresu.
- Start firmware musi zawsze ustawiac DAC na zero przed wlaczeniem regulacji.

### 6. Regulator i safety

- Przeniesc algorytm PI/PID ze starego `regulator.c`, ale bez zaleznosci od globalnych makr i `myData.data[]`.
- Zachowac tryby:
  - `C`: prad = setpoint,
  - `P`: prad = moc / napiecie,
  - `R`: prad = napiecie / rezystancja.
- Przeniesc kompensacje temperaturowa `compensate(current, temperature)` do osobnego pliku albo statycznej czesci regulatora.
- Dodac anty-windup, saturacje 0..15 A, reset integratora przy `onOff == 0`.
- Safety checks przeniesc ze starego `main.c` do modulu `system_state` albo `safety`:
  - napiecie <= 11.2 V -> stop regulacji,
  - `qlmb / 3600 > 250` -> stop regulacji,
  - temperatura >= 170 C przez dwa kolejne odczyty -> DAC zero i stop regulacji,
  - koniec `timeStop` -> stop regulacji i logowania.
- Rozdzielic "stop regulacji" od "stop logowania"; stary kod czasem wylacza jedno i drugie.

### 7. Temperatura DS18B20

- Wydzielic stary kod z `mystruct.c` do `temp_sensor.c/h`.
- Zachowac asynchroniczny cykl: start konwersji, odczekanie ok. 710-750 ms, odczyt scratchpada.
- Obsluzyc brak czujnika, CRC scratchpada i timeout.
- API powinno zwracac status pomiaru, a nie tylko wpisywac globalna wartosc.

### 8. SD logging

- Przeniesc `sd.c` do `logger_sd` albo osobnego `sd_log`.
- Zachowac format CSV: `time,setpoint,parameter,current,battery voltage,DAC,temp_1,temp_2,ON/OFF,qlmb`.
- Utworzenie folderu/pliku robic przy zboczu `logging: 0 -> 1`.
- Bufor logowania ograniczyc do stalego maksimum; `getLoggingBufferSize()` nie moze przekroczyc fizycznego rozmiaru bufora.
- Nie montowac/odmontowywac karty przy kazdym flushu, jesli stabilnosc SPI/FatFS pozwoli trzymac mount podczas sesji.
- Bledy SD logowac przez `logger`, ale nie blokowac petli regulacji.

### 9. LCD i enkoder

- [x] Przeniesc `interface.c/h` do `Modules/ui/lcd_ui.c/h`.
- [x] Zachowac stary tryb HD44780 4-bit i te same ekrany: default, turn, mode, logging.
- [x] Zachowac zachowanie B1/enkodera: wybor ekranu, mnoznik setpointu, wybor trybu, wlaczanie/wylaczanie loggingu.
- [x] Odwiezenie LCD co ok. 2 s albo po zmianie istotnego parametru.
- [ ] Oddzielic niski poziom HD44780 (`lcd_hd44780`) od logiki menu (`lcd_ui`), jesli bedziemy chcieli mocniej porzadkowac UI.
- [ ] Po tescie hardware zdecydowac, czy zostawiamy operacje LCD w callbacku EXTI dla maksymalnej zgodnosci ze starym projektem, czy przenosimy je na eventy obslugiwane w petli glownej.

### 10. Orkiestracja aplikacji

- Dodac `app_run_50ms()` albo podobna funkcje wolana z petli glownej po fladze TIM2.
- Proponowany przeplyw co 50 ms:
  1. obsluz odebrane ramki PC,
  2. zaktualizuj pomiary ADC,
  3. zaktualizuj temperature, jesli gotowa,
  4. wykonaj safety checks,
  5. wykonaj regulator, jesli `onOff == 1` i safety OK,
  6. dopisz probke do bufora SD zgodnie z harmonogramem,
  7. odswiez LCD wedlug harmonogramu.
- `main.c` ma zostac cienkim wrapperem HAL + `app_Init()` + `while(1) app_Process()`.

## Proponowana architektura katalogow

Docelowo warto dojsc do takiego ukladu:

```text
Modules/
  app/
    src/app.c
    src/app.h
  state/
    src/app_state.c
    src/app_state.h
    src/param_table.c
    src/param_table.h
  measurement/
    src/measurement.c
    src/measurement.h
    src/temp_sensor.c
    src/temp_sensor.h
  control/
    src/regulator.c
    src/regulator.h
    src/safety.c
    src/safety.h
    src/load_output.c
    src/load_output.h
  comms/
    src/comms_pc.c
    src/comms_pc.h
  storage/
    src/config_store.c
    src/config_store.h
    src/sd_log.c
    src/sd_log.h
  ui/
    src/lcd_hd44780.c
    src/lcd_hd44780.h
    src/lcd_ui.c
    src/lcd_ui.h
  logger/
    src/logger_uart.c
    src/logger.h
AdDrivers/
  MCP3561T/
  DAC8830/
```

Nie trzeba tego robic jednym commitem. Najpierw lepiej miec budujacy sie firmware z kilkoma modulami, potem przenosic odpowiedzialnosci.

## Definicja "pelny FW dziala"

Firmware mozna uznac za funkcjonalnie domkniety, gdy:

- build Debug i Release przechodzi z CMake,
- po flashu DAC startuje od zera,
- UART PC przyjmuje ramke konfiguracyjna, waliduje CRC i odsyla ACK/NACK,
- konfiguracja zapisuje sie i odtwarza po resecie,
- ADC zwraca sensowne napiecie i prad,
- regulator utrzymuje zadany prad w trybie `C`,
- tryby `P` i `R` przeliczaja setpoint na prad,
- safety odcina regulacje przy undervoltage, overtemperature, Ah limit i time limit,
- SD tworzy katalog/plik i dopisuje CSV,
- LCD pokazuje aktualne wartosci i stan ON/OFF/logging,
- bledy ida przez jeden logger UART,
- w `main.c` nie ma juz logiki domenowej poza uruchomieniem aplikacji.

## Najblizszy sensowny sprint

1. Dodac krotka instrukcje build/flash do `README.md` albo `BUILD.md`.
2. Potwierdzic z aplikacja PC dokladna dlugosc ramki i format CRC; w razie potrzeby dodac tryb zgodnosci z legacy bugiem CRC.
3. Zweryfikowac na hardware odczyt raw MCP3561T i przeliczenia pradu/napiecia; w razie potrzeby dodac sign-extension albo korekte formatu danych.
4. Doprowadzic `MCP3561T + measurement` do pelnego API: reset, verify config, status/timeout i diagnostyka bledow.
5. Podpiac walidacje zakresow z `param_table` w parserze PC i EEPROM load.
6. Przetestowac SD na plytce: mount, folder, plik CSV, append i zachowanie po wyjeciu karty.
7. Potem przeniesc LCD/menu/enkoder.
