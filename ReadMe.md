# Test Kernel Modul für AVM Assessment

Es wird im procfs die Datei `/proc/avm-kernel-logger` erstellt.
In diese kann eine beliebige Zeichenkette geschrieben werden.  
Diese Zeichenkette wird anschließend Wort für Wort jede Sekunde im Kernel Log ausgegeben.  
Das ausgegebene Wort wird aus dem internen Speicher gelöscht.  
Die Wörter die sich zurzeit im internen Speicher befinden, können aus der procfs Datei ausgelesen werden.

Getestet mit Ubuntu 24.04 LTS Server Standardinstallation

## Build

Das Modul wird als externes Modul gegen den aktuellen Kernel gebaut.

Zusätzliche Pakete:

`sudo apt install build-essential`

Kompilieren:

`make`

## Test

`./test.sh`
