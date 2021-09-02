import sys
sys.path.append('/Users/eronur/Documents/GitHub/liquid-usrp')
sys.path.append('/usr/local/lib')

import time

import pybind11_wrapper

def mycallback_txcvr1(header:str,headervalid:int,payload:str,payloadlen:int,payloadvalid:int):
    print("mycallback_txcvr1", header, headervalid, payload, payloadlen, payloadvalid)
    return 0

def mycallback_txcvr2(header:str,headervalid:int,payload:str,payloadlen:int,payloadvalid:int):
    print("mycallback_txcvr2", header, headervalid, payload, payloadlen, payloadvalid)
    return 0



if __name__ == '__main__':

    frequency = 462.0e6         # carrier frequency
    bandwidth = 1000e3         # bandwidth
    num_frames = 2000          # number of frames to transmit
    txgain_dB = -12.0          # software tx gain [dB]
    uhd_txgain = 40.0           # uhd (hardware) tx gain
    device1 = "30E623A"
    device2 = "30E6248"
    # ofdm properties
    M = 48
    cp_len = 6
    taper_len = 4

    txcvr1 = pybind11_wrapper.ofdmtxrx(M, cp_len, taper_len, device1)
    txcvr1.debug_enable()
    txcvr1.set_callback(mycallback_txcvr1)
    txcvr1.try_callback(1)
    txcvr1.try_callback(2)
    txcvr1.start_rx()

    txcvr2 = pybind11_wrapper.ofdmtxrx(M, cp_len, taper_len, device2)
    txcvr2.debug_enable()
    txcvr2.set_callback(mycallback_txcvr2)
    txcvr2.try_callback(1)
    txcvr2.try_callback(2)
    txcvr2.start_rx()

#    txcvr.set_tx_freq(frequency);
#    txcvr.set_tx_rate(bandwidth);
#    txcvr.set_tx_gain_soft(txgain_dB);
#    txcvr.set_tx_gain_uhd(uhd_txgain);

    header = "myheader"
    payload = "mypayload"
    #byte_payload = bytes(payload, 'utf-8')
    payload_len = 9
    ms = pybind11_wrapper.LIQUID_MODEM_QPSK
    fec0 = pybind11_wrapper.LIQUID_FEC_NONE # fec(inner)
    fec1 = pybind11_wrapper.LIQUID_FEC_GOLAY2412 # fec(outer)

    for x in range(5):
        header = "myheader" + str(x)
        txcvr1.transmit_packet_python(header, payload, payload_len, ms, fec0, fec1);
        time.sleep(0.01)

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
