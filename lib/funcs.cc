
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <complex>
#include <liquid/liquid.h>
#include<string>
#include "funcs.h"


int add(int i, int j) {
    return i + j;
};



eonofdmtxrx::eonofdmtxrx(void )
{
    fprintf(stderr,"constructed ofdmtxrx\n");
    // create usrp objects
    uhd::device_addr_t dev_addr;
    usrp_tx = uhd::usrp::multi_usrp::make(dev_addr);
    usrp_rx = uhd::usrp::multi_usrp::make(dev_addr);
};


// set transmitter frequency
void eonofdmtxrx::set_tx_freq(float _tx_freq)
{
    fprintf(stderr,"eonofdmtxrx:set_tx_freq%f\n", _tx_freq);
}

// set transmitter sample rate
void eonofdmtxrx::set_tx_rate(float _tx_rate)
{
    fprintf(stderr,"eonofdmtxrx:set_tx_freq%f\n", _tx_rate);
}
