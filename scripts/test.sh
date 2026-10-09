#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p .tests
cc -std=c11 -Wall -Wextra -Werror -pedantic -g -fsanitize=address,undefined \
 -Iinclude tests/test_coeur.c src/parametres.c src/mesures.c src/modes.c src/nmea.c -lm -o .tests/test_coeur
ASAN_OPTIONS=detect_leaks=1 .tests/test_coeur
truncate -s 16M .tests/sd-test.img
mkfs.fat -F 16 .tests/sd-test.img >/dev/null
cc -std=c11 -Wall -Wextra -g -fsanitize=address,undefined \
 -Ilib/fatfs/include -Ilib/cesi_grove/include \
 tests/test_sd.c lib/cesi_grove/src/sd_files.c lib/fatfs/src/ff.c lib/fatfs/src/unicode.c -o .tests/test_sd
.tests/test_sd .tests/sd-test.img
cc -std=c11 -Wall -Wextra -g -fsanitize=address,undefined \
 -Itests/stubs -Iinclude -Ilib/cesi_grove/include \
 tests/test_station.c src/station.c src/parametres.c src/mesures.c src/modes.c src/nmea.c -lm -o .tests/test_station
.tests/test_station
cc -std=c11 -Wall -Wextra -g -fsanitize=address,undefined \
 -Itests/stubs -Iinclude tests/test_eeprom.c src/eeprom.c src/parametres.c -o .tests/test_eeprom
.tests/test_eeprom
