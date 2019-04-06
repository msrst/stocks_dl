# stocks_dl
library for downloading stock data at boerse-frankfurt.de and finance.yahoo.com

## Building the library and the example
To build, you need cmake, the boost library (sublibs system, signals, thread and spirit) and the CURL C library. Install them for example under debian with
_Code apt install cmake libboost-dev libcurl4-openssl-dev
Firstly, clone the repository.
_Code git clone https://github.com/msrst/stocks_dl
Now in the cloned folder, run
_Code cmake .
_Code make
Now you can test the example:
cd example
./example1 bf_prices DE0007236101 28.1.2018_01.03.2019
(see section below for more information on how to use the sample)

## More configuration options for the cmake
(It is more intuitive to configure them via cmake-gui)
_Code -DBOOST_ROOT=/your/boost/root
Configure your root directory for the boost library if you do not use a linux package
_Code -DSTOCKS_DL_DEBUG_OUTPUT=ON
Turn on debug output of the stocks_dl-library (default off)

## Using the example
_Code ./example1 y_dividends BAS.DE BASF
_Code ./example1 bf_dividends DE0007236101
_Code ./example1 bf_share_data DE0007236101
_Code ./example/example1 bf_prices DE0007236101 28.1.2018_01.03.2019

## License
Copyright (C) Matthias Rosenthal
_Code License:_ [GPL](./LICENSE)
Applies to all files
