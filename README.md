# stocks_dl

Library for downloading stock data at boerse-frankfurt.de and finance.yahoo.com. Note that there are python libraries to download stock data from yahoo for free, which are much more comprehensible. The advantage of using boerse-frankfurt is that there is also balance sheet data available, as well as the quality of the price data is better because splits are included.

Disclaimer: Please note that using this library may not be legal.

Downloadable data:

 * daily prices
 * dividends
 * balance sheets (only from boerse-frankfurt.de)

I have tested the library only under Linux (Debian 9, Debian 10, Ubuntu 18.04), but it should also run under Windows.

## Building the library and the example

To build, you need cmake, the boost library (sublibs system, signals, thread and property_tree) and the CURL C library. Install them for example under debian with

    apt install cmake libboost-dev libcurl4-openssl-dev

Then clone the repository.

    git clone https://github.com/msrst/stocks_dl

Now in the cloned folder, run

    mkdir build
    cd build
    cmake ..
    make

Now you can test the example:

    ./example/example1 bf_prices DE0007236101

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

    ./example/example1 bf_dividends DE0007236101

Downloads all dividends listed on www.boerse-frankfurt.de (boerse-frankfurt has only dividends data for european companies and only lists the dividends that were paid in 1999 or later).

    ./example/example1 bf_share_data DE0007236101

![screenshot bf_share_data](https://github.com/msrst/stocks_dl/blob/master/screenshots/bf_share_data_20.png)

Displays balance sheets from www.boerse-frankfurt.de (boerse-frankfurt has only balance sheets for german companies), since 1999. Also gets the WKN and the stock exchange symbol. *DE0007236101* is the ISIN of the share to query data for.

    ./example/example1 bf_prices DE0007236101 01.03.2020_27.03.2020

![screenshot bf_prices](https://github.com/msrst/stocks_dl/blob/master/screenshots/bf_prices_20.png)

## The boerse frankfurt "api"

In the second half of 2019, boerse-frankfurt completely changed their way of delivering stocks data to website users. Before, they seemed to do everything to confuse developers. For example, the server sent a calculation like 5 * 2 + 3 an which the web browser (or download tool) had to evaluate. The result was then sent back to the server when gathering stocks data.

The new api seems to be a spring boot (java, tomcat) server. In 2020, it was very easy to use. Since 2021, a few extra headers have to be passed.
Here are some examples (they will only work with headers):

https://api.boerse-frankfurt.de/v1/data/price_history?limit=50&offset=0&isin=DE0007236101&mic=XFRA&minDate=2019-03-27&maxDate=2020-03-27

 -> high, low, open, close, volume (JSON) (min year: depending on share, maybe 1990)

https://api.boerse-frankfurt.de/v1/data/dividend_information?isin=DE0007236101&limit=50

 -> dividends JSON

https://api.boerse-frankfurt.de/v1/data/historical_key_data?isin=DE0007236101&limit=50

 -> historical key figures, like total assets or other important balance sheet figures (JSON) (min year: 1999)
There are more endpoints, these are only some examples.

## License

Copyright (C) Matthias Rosenthal

License: [GPL](./LICENSE)

Applies to all files
