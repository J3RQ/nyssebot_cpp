# Nyssebot

## Dependencies
```
mkdir libs
git clone NAME_OF_DEPENDENCY.git
```
Clone these dependencies into the 'libs'-folder:
```
brainboxdotcc/DPP
libfmt/fmt
libcpr/cpr
```

## Building
How to build:
```cpp
cmake -S . -B build
cmake --build build
cmake --build build -t test
```

## Commands

#### /stop
Gets realtime departures from a stop. Results can be filtered down to include only certain line numbers.
#### /route
Gets route instructions from point A to point B. 
#### /map
Gets a map tile to display the location of a stop.

## Miscellaneous
> I have no idea whether or not this can be built on Windows. I write and run the bot exclusively on linux machines, so I haven't had the chance to test that.
