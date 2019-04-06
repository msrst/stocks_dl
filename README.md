# stocks_dl

Library for downloading stock data at boerse-frankfurt.de and finance.yahoo.com.

Downloadable data:

 * daily prices
 * dividends
 * balance sheets (only from boerse-frankfurt.de)

I have tested the library only under Linux (Debian), but it should also run under Windows.

## Building the library and the example

To build, you need cmake, the boost library (sublibs system, signals, thread and spirit) and the CURL C library. Install them for example under debian with

    apt install cmake libboost-dev libcurl4-openssl-dev

Then clone the repository.

    git clone https://github.com/msrst/stocks_dl

Now in the cloned folder, run

    cmake .
    make

Now you can test the example:

    cd example
    ./example1 bf_prices DE0007236101 28.1.2018_01.03.2019

(see section "Using the example" for more information on how to use the sample)

## More configuration options for cmake

(It is more intuitive to configure them via cmake-gui)

    -DBOOST_ROOT=/your/boost/root

Configure your root directory for the boost library if you do not use the package of your linux distribution

    -DSTOCKS_DL_DEBUG_OUTPUT=ON

Turn on debug output of the stocks_dl-library (default off)

## Using the example

    ./example1 y_dividends BAS.DE BASF

![screenshot y_dividends](https://github.com/msrst/stocks_dl/blob/master/screenshots/y_dividends.png)

Downloads dividends from finance.yahoo.com until 01 march 2019

    ./example1 bf_dividends DE0007236101

Downloads all dividends listed on www.boerse-frankfurt.de (boerse-frankfurt has only dividends data for european companies and only lists the dividends of the last five years).

    ./example1 bf_share_data DE0007236101

![screenshot bf_share_data](https://github.com/msrst/stocks_dl/blob/master/screenshots/bf_share_data.png)

Displays balance sheets from www.boerse-frankfurt.de (boerse-frankfurt has only balance sheets for german companies). Also gets the WKN and the stock exchange symbol. *DE0007236101* is the ISIN of the share to query data for.

    ./example/example1 bf_prices DE0007236101 28.1.2018_01.03.2019

![screenshot bf_prices](https://github.com/msrst/stocks_dl/blob/master/screenshots/bf_prices.png)

Querys prices from 28th January 2018 till 01st march 2019.

## License

Copyright (C) Matthias Rosenthal

License:_ [GPL](./LICENSE)

Applies to all files
