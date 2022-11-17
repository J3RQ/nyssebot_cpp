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
```
cmake -S . -B build
cmake --build build
cmake --build build -t test
```

## Config (.env)
Add the token of your bot and Mapbox API key
```
TOKEN=example_bot_token
MB_APIKEY=key_example
```

## Commands

#### /stop
Gets realtime departures from a stop. Results can be filtered down to include only certain line numbers.
#### /route
Gets route instructions from point A to point B. 
#### /map
Gets a map tile to display the location of a stop.
