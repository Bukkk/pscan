# pscan - cpu usage tracker

## standard
C11

## build system
meson
działa czytanie envvar CC oraz -Weverything dla clanga

## działanie
przetestowany na dwóch różnych distro

## valgrind
zgłasza, że wszystkie allokacje zostały zwolnione

## wątki
pierwszy, drugi, trzeci, oraz sygnał

## buforowanie
za pomocą RingBuffer

## threads.h
używam standardowej nakładki C11

## featury
można doszukać się kilku FAM i CompoundLiterals
