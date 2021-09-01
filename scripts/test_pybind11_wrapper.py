import sys
sys.path.append('/Users/eronur/Documents/GitHub/liquid-usrp')

import pybind11_wrapper
from typing import *

def mycallback(header:str,headervalid:int,payload:str,payloadlen:int,payloadvalid:int):
    print("mycallback", header, headervalid, payload, payloadlen, payloadvalid)
    return 0

def test_add():
    assert(pybind11_wrapper.add(3, 4) == 7)
    print(pybind11_wrapper.add(3, 4))

if __name__ == '__main__':
    test_add()
    p = pybind11_wrapper.Pet('Molly')
    print(p)
    print(p.getName())




    frequency = 462.0e6         # carrier frequency
    bandwidth = 1000e3         # bandwidth
    num_frames = 2000          # number of frames to transmit
    txgain_dB = -12.0          # software tx gain [dB]
    uhd_txgain = 40.0           # uhd (hardware) tx gain

    # ofdm properties
    M = 48
    cp_len = 6
    taper_len = 4

    txcvr = pybind11_wrapper.ofdmtxrx(M,cp_len,taper_len)
    txcvr.debug_enable()
    txcvr.set_callback(mycallback)
    txcvr.try_callback(1)
    txcvr.try_callback(2)

    txcvr.set_tx_freq(frequency);
    txcvr.set_tx_rate(bandwidth);
    txcvr.set_tx_gain_soft(txgain_dB);
    txcvr.set_tx_gain_uhd(uhd_txgain);

    header = "myheader"
    payload = "mypayload"
    #byte_payload = bytes(payload, 'utf-8')
    payload_len = 9
    ms = pybind11_wrapper.LIQUID_MODEM_QPSK
    fec0 = pybind11_wrapper.LIQUID_FEC_NONE # fec(inner)
    fec1 = pybind11_wrapper.LIQUID_FEC_GOLAY2412 # fec(outer)

    for x in range(6):
        header = "myheader" + str(x)
        txcvr.transmit_packet_python(header, payload, payload_len, ms, fec0, fec1);

    #o = pybind11_wrapper.ofdmtxrx()

    # o.set_tx_freq(462.0e6)
    # o.set_tx_rate(500e3)
    # o.set_tx_gain_soft(-12.0)
    # o.set_tx_gain_uhd(40.0)
    #
    # o.set_rx_freq(462.0e6)
    # o.set_rx_rate(500e3)
    # o.set_rx_gain_uhd(20.0)

    while(True):
        pass
