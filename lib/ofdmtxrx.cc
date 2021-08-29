/*
 * Copyright (c) 2013 Joseph Gaeddert
 *
 * This file is part of liquid.
 *
 * liquid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * liquid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with liquid.  If not, see <http://www.gnu.org/licenses/>.
 */

//
// ofdmtxrx.cc
//

#include <math.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <complex>
#include <liquid/liquid.h>

#include "ofdmtxrx.h"

#define DEBUG 0

// debug print
#if DEBUG == 1
#   define dprintf(s) printf(s)
#else
#   define dprintf(s) /* s */
#endif

// default constructor
//  _M              :   OFDM: number of subcarriers
//  _cp_len         :   OFDM: cyclic prefix length
//  _taper_len      :   OFDM: taper prefix length
//  _p              :   OFDM: subcarrier allocation
//  _callback       :   frame synchronizer callback function
//  _userdata       :   user-defined data structure
ofdmtxrx::ofdmtxrx(unsigned int       _M,
                   unsigned int       _cp_len,
                   unsigned int       _taper_len,
                   unsigned char *    _p,
                   framesync_callback _callback,
                   void *             _userdata)
{
    // validate input
    if (_M < 8) {
        fprintf(stderr,"error: ofdmtxrx::ofdmtxrx(), number of subcarriers must be at least 8\n");
        throw 0;
    } else if (_cp_len < 1) {
        fprintf(stderr,"error: ofdmtxrx::ofdmtxrx(), cyclic prefix length must be at least 1\n");
        throw 0;
    } else if (_taper_len > _cp_len) {
        fprintf(stderr,"error: ofdmtxrx::ofdmtxrx(), taper length cannot exceed cyclic prefix length\n");
        throw 0;
    }

    // set internal properties
    M            = _M;
    cp_len       = _cp_len;
    taper_len    = _taper_len;
    debug_enabled= false;

    // create frame generator
    unsigned char * p = NULL;   // subcarrier allocation (default)
    ofdmflexframegenprops_init_default(&fgprops);
    fgprops.check           = LIQUID_CRC_32;
    fgprops.fec0            = LIQUID_FEC_NONE;
    fgprops.fec1            = LIQUID_FEC_HAMMING128;
    fgprops.mod_scheme      = LIQUID_MODEM_QPSK;
    fg = ofdmflexframegen_create(M, cp_len, taper_len, p, &fgprops);

    // allocate memory for frame generator output (single OFDM symbol)
    fgbuffer_len = M + cp_len;
    fgbuffer = (std::complex<float>*) malloc(fgbuffer_len * sizeof(std::complex<float>));

    // create frame synchronizer
    fs = ofdmflexframesync_create(M, cp_len, taper_len, p, _callback, _userdata);
    // TODO: create buffer

    // create usrp objects
    uhd::device_addr_t dev_addr;
    usrp_tx = uhd::usrp::multi_usrp::make(dev_addr);
    usrp_rx = uhd::usrp::multi_usrp::make(dev_addr);

    // initialize default tx values
    set_tx_freq(462.0e6f);
    set_tx_rate(500e3);
    set_tx_gain_soft(-12.0f);
    set_tx_gain_uhd(40.0f);

    // initialize default rx values
    set_rx_freq(462.0e6f);
    set_rx_rate(500e3);
    set_rx_gain_uhd(20.0f);

    // reset transceiver
    reset_tx();
    reset_rx();

    // create and start rx thread
    rx_running = false;                     // receiver is not running initially
    rx_thread_running = true;               // receiver thread IS running initially
    pthread_mutex_init(&rx_mutex, NULL);    // receiver mutex
    pthread_mutex_init(&rx_buffer_mutex, NULL);    // receiver buffer mutex
    pthread_cond_init(&rx_cond,   NULL);    // receiver condition
    pthread_create(&rx_process,   NULL, ofdmtxrx_rx_worker, (void*)this);
    
    // TODO: create and start tx thread
}

// custom constructor that allows selection between original ofdmtxrx_rx_worker()
// and ofdmtxrx_rx_worker_blocking().
ofdmtxrx::ofdmtxrx(unsigned int       _M,
                   unsigned int       _cp_len,
                   unsigned int       _taper_len,
                   unsigned char *    _p,
                   framesync_callback _callback,
                   void *             _userdata,
                   bool		      _blocking_rx_worker)
{
    // validate input
    if (_M < 8) {
        fprintf(stderr,"error: ofdmtxrx::ofdmtxrx(), number of subcarriers must be at least 8\n");
        throw 0;
    } else if (_cp_len < 1) {
        fprintf(stderr,"error: ofdmtxrx::ofdmtxrx(), cyclic prefix length must be at least 1\n");
        throw 0;
    } else if (_taper_len > _cp_len) {
        fprintf(stderr,"error: ofdmtxrx::ofdmtxrx(), taper length cannot exceed cyclic prefix length\n");
        throw 0;
    }

    // set internal properties
    M            = _M;
    cp_len       = _cp_len;
    taper_len    = _taper_len;
    debug_enabled= false;

    // create frame generator
    unsigned char * p = NULL;   // subcarrier allocation (default)
    ofdmflexframegenprops_init_default(&fgprops);
    fgprops.check           = LIQUID_CRC_32;
    fgprops.fec0            = LIQUID_FEC_NONE;
    fgprops.fec1            = LIQUID_FEC_HAMMING128;
    fgprops.mod_scheme      = LIQUID_MODEM_QPSK;
    fg = ofdmflexframegen_create(M, cp_len, taper_len, p, &fgprops);

    // allocate memory for frame generator output (single OFDM symbol)
    fgbuffer_len = M + cp_len;
    fgbuffer = (std::complex<float>*) malloc(fgbuffer_len * sizeof(std::complex<float>));

    // create frame synchronizer
    fs = ofdmflexframesync_create(M, cp_len, taper_len, p, _callback, _userdata);
    // TODO: create buffer

    // create usrp objects
    uhd::device_addr_t dev_addr;
    usrp_tx = uhd::usrp::multi_usrp::make(dev_addr);
    usrp_rx = uhd::usrp::multi_usrp::make(dev_addr);

    // initialize default tx values
    set_tx_freq(462.0e6f);
    set_tx_rate(500e3);
    set_tx_gain_soft(-12.0f);
    set_tx_gain_uhd(40.0f);

    // initialize default rx values
    set_rx_freq(462.0e6f);
    set_rx_rate(500e3);
    set_rx_gain_uhd(20.0f);

    // reset transceiver
    reset_tx();
    reset_rx();

    // create and start rx thread
    rx_running = false;                     // receiver is not running initially
    rx_thread_running = true;               // receiver thread IS running initially
    pthread_mutex_init(&rx_mutex, NULL);    // receiver mutex
    pthread_mutex_init(&rx_buffer_mutex, NULL);    // receiver buffer mutex
    pthread_cond_init(&rx_cond,   NULL);    // receiver condition
    pthread_cond_init(&rx_buffer_filled_cond,   NULL);    // receiver buffer filled condition
    pthread_cond_init(&rx_buffer_modified_cond,   NULL);    // receiver buffer modified condition
    pthread_cond_init(&esbrs_ready, NULL);

    if (_blocking_rx_worker)
    {
        pthread_create(&rx_process,   NULL, ofdmtxrx_rx_worker_blocking, (void*)this);
    }
    else
    {
        pthread_create(&rx_process,   NULL, ofdmtxrx_rx_worker, (void*)this);
    }
    
    // TODO: create and start tx thread
}
// destructor
ofdmtxrx::~ofdmtxrx()
{
    dprintf("waiting for process to finish...\n");

    // ensure reciever thread is not running
    if (rx_running) stop_rx();

    // signal condition (tell rx worker to continue)
    dprintf("destructor signaling condition...\n");
    rx_thread_running = false;
    pthread_cond_signal(&rx_cond);

    dprintf("destructor joining rx thread...\n");
    void * exit_status;
    pthread_join(rx_process, &exit_status);

    // destroy threading objects
    dprintf("destructor destroying rx mutex...\n");
    pthread_mutex_destroy(&rx_mutex);
    dprintf("destructor destroying rx condition...\n");
    pthread_cond_destroy(&rx_cond);
    dprintf("destructor destroying rx buffer mutex...\n");
    pthread_mutex_destroy(&rx_buffer_mutex);
    dprintf("destructor destroying rx buffer filled condition...\n");
    pthread_cond_destroy(&rx_buffer_filled_cond);
    dprintf("destructor destroying rx buffer modified condition...\n");
    pthread_cond_destroy(&rx_buffer_modified_cond);
    
    // TODO: output debugging file
    if (debug_enabled)
        ofdmflexframesync_debug_print(fs, "ofdmtxrx_framesync_debug.m");

    dprintf("destructor destroying other objects...\n");
    // destroy framing objects
    ofdmflexframegen_destroy(fg);
    ofdmflexframesync_destroy(fs);

    // free other allocated arrays
    free(fgbuffer);
    
    dprintf("destructor finished\n");
}


// 
// transmitter methods
//

// set transmitter frequency
void ofdmtxrx::set_tx_freq(float _tx_freq)
{
    usrp_tx->set_tx_freq(_tx_freq);
}

// set transmitter sample rate
void ofdmtxrx::set_tx_rate(float _tx_rate)
{
    usrp_tx->set_tx_rate(_tx_rate);
}

// set transmitter software gain
void ofdmtxrx::set_tx_gain_soft(float _tx_gain_soft)
{
    tx_gain = powf(10.0f, _tx_gain_soft/20.0f);
}

// set transmitter hardware (UHD) gain
void ofdmtxrx::set_tx_gain_uhd(float _tx_gain_uhd)
{
    usrp_tx->set_tx_gain(_tx_gain_uhd);
}

// set transmitter antenna
void ofdmtxrx::set_tx_antenna(char * _tx_antenna)
{
    usrp_rx->set_tx_antenna(_tx_antenna);
}

// reset transmitter objects and buffers
void ofdmtxrx::reset_tx()
{
    ofdmflexframegen_reset(fg);
}

// update payload data on a particular channel
void ofdmtxrx::transmit_packet(unsigned char * _header,
                               unsigned char * _payload,
                               unsigned int    _payload_len,
                               int             _mod,
                               int             _fec0,
                               int             _fec1)
{
    // set up the metadata flags
    metadata_tx.start_of_burst = false; // never SOB when continuous
    metadata_tx.end_of_burst   = false; // 
    metadata_tx.has_time_spec  = false; // set to false to send immediately
    //TODO: flush buffers
    uhd::stream_args_t stream_args("fc32"); //complex floats
    // vector buffer to send data to device
    std::vector<std::complex<float> > usrp_buffer(fgbuffer_len);

    // set properties
    fgprops.mod_scheme  = _mod;
    fgprops.fec0        = _fec0;
    fgprops.fec1        = _fec1;
    ofdmflexframegen_setprops(fg, &fgprops);

    // assemble frame
    ofdmflexframegen_assemble(fg, _header, _payload, _payload_len);

    // generate a single OFDM frame
    bool last_symbol=false;
    unsigned int i;
    while (!last_symbol) {

        // generate symbol
        last_symbol = ofdmflexframegen_write(fg, fgbuffer, fgbuffer_len);

        // copy symbol and apply gain
        for (i=0; i<fgbuffer_len; i++)
            usrp_buffer[i] = fgbuffer[i] * tx_gain;

        // send samples to the device
        usrp_tx->get_device()->get_tx_stream(stream_args)->send(
            &usrp_buffer.front(), usrp_buffer.size(),
            metadata_tx);

    } // while loop

    // send a few extra samples to the device
    // NOTE: this seems necessary to preserve last OFDM symbol in
    //       frame from corruption
    usrp_tx->get_device()->get_tx_stream(stream_args)->send(
        &usrp_buffer.front(), usrp_buffer.size(),
        metadata_tx);
    
    // send a mini EOB packet
    metadata_tx.start_of_burst = false;
    metadata_tx.end_of_burst   = true;

    usrp_tx->get_device()->get_tx_stream(stream_args)->send("", 0, metadata_tx);

}

// Assemble the frame so it is ready to be written as samples
void ofdmtxrx::assemble_frame(unsigned char * _header,
                            unsigned char * _payload,
                            unsigned int    _payload_len,
                            int             _mod,
                            int             _fec0,
                            int             _fec1)
{
    // set properties
    fgprops.mod_scheme  = _mod;
    fgprops.fec0        = _fec0;
    fgprops.fec1        = _fec1;
    ofdmflexframegen_setprops(fg, &fgprops);

    // assemble frame
    ofdmflexframegen_assemble(fg, _header, _payload, _payload_len);
}

// Write one symbol from the assembled frame.
// Returns true if last symbol.
bool ofdmtxrx::write_symbol()
{
    bool last_symbol=false;
    last_symbol= ofdmflexframegen_write(fg, fgbuffer, 1);
    return last_symbol;
}

// update payload data on a particular channel
void ofdmtxrx::transmit_symbol()
{
    // vector buffer to send data to device
    std::vector<std::complex<float> > usrp_buffer(fgbuffer_len);
    uhd::stream_args_t stream_args("fc32"); //complex floats
    // generate a single OFDM frame
    //bool last_symbol=false;
    unsigned int i;
    //while (!last_symbol) {

        // generate symbol
        //last_symbol = ofdmflexframegen_writesymbol(fg, fgbuffer);

        // copy symbol and apply gain
        for (i=0; i<fgbuffer_len; i++)
            usrp_buffer[i] = fgbuffer[i] * tx_gain;

        // send samples to the device
        usrp_tx->get_device()->get_tx_stream(stream_args)->send(
            &usrp_buffer.front(), usrp_buffer.size(),
            metadata_tx);

    //} // while loop

}

void ofdmtxrx::end_transmit_frame()
{
    // vector buffer to send data to device
    std::vector<std::complex<float> > usrp_buffer(fgbuffer_len);
    uhd::stream_args_t stream_args("fc32"); //complex floats
    unsigned int i;
    // copy symbol and apply gain
    for (i=0; i<fgbuffer_len; i++)
        usrp_buffer[i] = fgbuffer[i] * tx_gain;

    // send a few extra samples to the device
    // NOTE: this seems necessary to preserve last OFDM symbol in
    //       frame from corruption
    usrp_tx->get_device()->get_tx_stream(stream_args)->send(
        &usrp_buffer.front(), usrp_buffer.size(),
        metadata_tx);
    
    // send a mini EOB packet
    metadata_tx.start_of_burst = false;
    metadata_tx.end_of_burst   = true;

    usrp_tx->get_device()->get_tx_stream(stream_args)->send("", 0, metadata_tx);

}

// 
// receiver methods
//

// set receiver frequency
void ofdmtxrx::set_rx_freq(float _rx_freq)
{
    usrp_rx->set_rx_freq(_rx_freq);
}

// set receiver sample rate
void ofdmtxrx::set_rx_rate(float _rx_rate)
{
    usrp_rx->set_rx_rate(_rx_rate);
}

// set receiver hardware (UHD) gain
void ofdmtxrx::set_rx_gain_uhd(float _rx_gain_uhd)
{
    usrp_rx->set_rx_gain(_rx_gain_uhd);
}

// set receiver antenna
void ofdmtxrx::set_rx_antenna(char * _rx_antenna)
{
    usrp_rx->set_rx_antenna(_rx_antenna);
}

// reset receiver objects and buffers
void ofdmtxrx::reset_rx()
{
    ofdmflexframesync_reset(fs);
}

// start receiver
void ofdmtxrx::start_rx()
{
    dprintf("usrp rx start\n");
    // set rx running flag
    rx_running = true;

    // tell device to start
    usrp_rx->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);

    // signal condition (tell rx worker to start)
    pthread_cond_signal(&rx_cond);
}

// stop receiver
void ofdmtxrx::stop_rx()
{
    dprintf("usrp rx stop\n");
    // set rx running flag
    rx_running = false;

    // tell device to stop
    usrp_rx->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);
}

//
// additional methods
//

// enable debugging
void ofdmtxrx::debug_enable()
{
    debug_enabled = true;
    ofdmflexframesync_debug_enable(fs);
}

// disable debugging
void ofdmtxrx::debug_disable()
{
    debug_enabled = false;
    ofdmflexframesync_debug_disable(fs);
}

//
// private methods
//

// set timespec for timeout
//  _ts         :   pointer to timespec structure
//  _timeout    :   time before timeout
void ofdmtxrx::set_timespec(struct timespec * _ts,
                            float             _timeout)
{
    // get current time (timeval)
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);

    // add offset (timespec)
    _ts->tv_sec  = tv_now.tv_sec;                       // seconds
    _ts->tv_nsec = tv_now.tv_usec*1000 + _timeout*1e9;  // nanoseconds

    // accumulate nanoseconds into seconds
    while (_ts->tv_nsec > 1000000000) {
        _ts->tv_nsec -= 1000000000;
        _ts->tv_sec++;
    }
}

// receiver worker thread
void * ofdmtxrx_rx_worker(void * _arg)
{
    // type cast input argument as ofdmtxrx object
    ofdmtxrx * txcvr = (ofdmtxrx*) _arg;
    uhd::stream_args_t stream_args("fc32", "sc16");
    // set up receive buffer
    const size_t max_samps_per_packet = txcvr->usrp_rx->get_device()->get_rx_stream(stream_args)->get_max_num_samps();
    std::vector<std::complex<float> > buffer(max_samps_per_packet);
    stream_args.args["spp"] = max_samps_per_packet;
    // receiver metadata object
    uhd::rx_metadata_t md;

    while (txcvr->rx_thread_running) {
        // wait for signal to start; lock mutex
        pthread_mutex_lock(&(txcvr->rx_mutex));

        // this function unlocks the mutex and waits for the condition;
        // once the condition is set, the mutex is again locked
        dprintf("rx_worker waiting for condition...\n");
        //int status =
        pthread_cond_wait(&(txcvr->rx_cond), &(txcvr->rx_mutex));
        dprintf("rx_worker received condition\n");

        // unlock the mutex
        dprintf("rx_worker unlocking mutex\n");
        pthread_mutex_unlock(&(txcvr->rx_mutex));

        // condition given; check state: run or exit
        dprintf("rx_worker running...\n");
        if (!txcvr->rx_running) {
            dprintf("rx_worker finished\n");
            break;
        }

        // run receiver
        while (txcvr->rx_running) {

            // grab data from device
            //dprintf("rx_worker waiting for samples...\n");
            size_t num_rx_samps = txcvr->usrp_rx->get_device()->get_rx_stream(stream_args)->recv(
                &buffer.front(), buffer.size(), md);
            //dprintf("rx_worker processing samples...\n");

            // ignore error codes for now
#if 0
// 'handle' the error codes
switch(md.error_code){
case uhd::rx_metadata_t::ERROR_CODE_NONE:
case uhd::rx_metadata_t::ERROR_CODE_OVERFLOW:
break;

default:
std::cerr << "Error code: " << md.error_code << std::endl;
std::cerr << "Unexpected error on recv, exit test..." << std::endl;
//return 1;
//std::cerr << "rx_worker exiting prematurely" << std::endl;
//pthread_exit(NULL);
}
#endif

            // push data through frame synchronizer
            // TODO : use arbitrary resampler?
            unsigned int j;
            for (j=0; j<num_rx_samps; j++) {
                // grab sample from usrp buffer
                std::complex<float> usrp_sample = buffer[j];

                // push resulting samples through synchronizer
                ofdmflexframesync_execute(txcvr->fs, &usrp_sample, 1);
            }

        } // while rx_running
        dprintf("rx_worker finished running\n");

    } // while true
    
    //
    dprintf("rx_worker exiting thread\n");
    pthread_exit(NULL);
}

// Special version of receiver worker thread that waits for 
// a pthread_cond_signal() before sending samples to synchronizer.
// Allows the samples to be modified by another thread before being
// sent to the synchronizer.
void * ofdmtxrx_rx_worker_blocking(void * _arg)
{
    // type cast input argument as ofdmtxrx object
    ofdmtxrx * txcvr = (ofdmtxrx*) _arg;
    uhd::stream_args_t stream_args("fc32", "sc16");
    // set up receive buffer
    const size_t max_samps_per_packet = txcvr->usrp_rx->get_device()->get_rx_stream(stream_args)->get_max_num_samps();
    std::vector<std::complex<float> > _buffer(max_samps_per_packet);
    stream_args.args["spp"] = max_samps_per_packet;
    //std::vector<std::complex<float> > * rx_buffer = &_buffer;
    txcvr->rx_buffer = &_buffer;

    // receiver metadata object
    uhd::rx_metadata_t md;

    while (txcvr->rx_thread_running) {
        // wait for signal to start; lock mutex
        pthread_mutex_lock(&(txcvr->rx_mutex));

        // this function unlocks the mutex and waits for the condition;
        // once the condition is set, the mutex is again locked
        dprintf("rx_worker waiting for condition...\n");
        //int status =
        pthread_cond_wait(&(txcvr->rx_cond), &(txcvr->rx_mutex));
        dprintf("rx_worker received condition\n");

        // unlock the mutex
        dprintf("rx_worker unlocking mutex\n");
        pthread_mutex_unlock(&(txcvr->rx_mutex));

        // condition given; check state: run or exit
        dprintf("rx_worker running...\n");
        if (!txcvr->rx_running) {
            dprintf("rx_worker finished\n");
            break;
        }

        // run receiver
        while (txcvr->rx_running) {

            // grab data from device
            //dprintf("rx_worker waiting for samples...\n");
            dprintf("rx_worker locking buffer mutex\n");
            pthread_mutex_lock(&(txcvr->rx_buffer_mutex));
            size_t num_rx_samps = txcvr->usrp_rx->get_device()->get_rx_stream(stream_args)->recv(
                &(txcvr->rx_buffer->front()), txcvr->rx_buffer->size(), md
                //&rx_buffer.front(), rx_buffer.size(), md,
            );
	    dprintf("rx_worker signalling buffer filled cond");
            pthread_cond_signal(&(txcvr->rx_buffer_filled_cond));
            //dprintf("rx_worker processing samples...\n");




            // ignore error codes for now
#if 0
            // 'handle' the error codes
            switch(md.error_code){
            case uhd::rx_metadata_t::ERROR_CODE_NONE:
            case uhd::rx_metadata_t::ERROR_CODE_OVERFLOW:
                break;

            default:
                std::cerr << "Error code: " << md.error_code << std::endl;
                std::cerr << "Unexpected error on recv, exit test..." << std::endl;
                //return 1;
                //std::cerr << "rx_worker exiting prematurely" << std::endl;
                //pthread_exit(NULL);
            }
#endif

            dprintf("rx_worker waiting for buffer modified cond\n");
            pthread_cond_wait(&txcvr->rx_buffer_modified_cond, &txcvr->rx_buffer_mutex);
            // push data through frame synchronizer
            // TODO : use arbitrary resampler?
            unsigned int j;
            for (j=0; j<num_rx_samps; j++) {
                // grab sample from usrp buffer
                std::complex<float> usrp_sample = (*txcvr->rx_buffer)[j];

                // push resulting samples through synchronizer
                ofdmflexframesync_execute(txcvr->fs, &usrp_sample, 1);
            }
            dprintf("rx_worker unlocking buffer mutex\n");
            pthread_mutex_unlock(&(txcvr->rx_buffer_mutex));

        } // while rx_running
	pthread_mutex_unlock(&(txcvr->rx_buffer_mutex));
        dprintf("rx_worker finished running\n");

    } // while true
    
    //
    dprintf("rx_worker exiting thread\n");
    pthread_exit(NULL);
}

#if 0
// callback function
int ofdmtxrx_callback(unsigned char *  _header,
                      int              _header_valid,
                      unsigned char *  _payload,
                      unsigned int     _payload_len,
                      int              _payload_valid,
                      framesyncstats_s _stats,
                      void *           _userdata)
{
    // type case pointer
    //ofdmtxrx * txcvr = (ofdmtxrx*) _userdata;
    
    printf("***** rssi=%7.2fdB evm=%7.2fdB, header[%4s], payload[%4s]",
            _stats.rssi, _stats.evm,
            _header_valid  ? "pass" : "FAIL",
            _payload_valid ? "pass" : "FAIL");

    return 0;
}
#endif
