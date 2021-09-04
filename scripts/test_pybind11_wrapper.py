import sys
#sys.path.append('/Users/eronur/Documents/GitHub/liquid-usrp') #if did not call make install!

sys.path.append('/usr/local/lib')

import time

import liquid_usrp_pybind11_wrapper as wrapper

class x(object):
    pass

def mycallback_txcvr1(header:bytes,headervalid:int,payload:bytes,payloadlen:int,payloadvalid:int, rssi:float, evm:float):
    try:
        print("mycallback_txcvr1", header, headervalid, payload, payloadlen, payloadvalid, rssi, evm)
    except Exception as e:
        print("Exception: UnicodeDecodeError {}".format(e))
    return 0

def mycallback_txcvr2(header:bytes,headervalid:int,payload:bytes,payloadlen:int,payloadvalid:int, rssi:float, evm:float):
    try:
        print("mycallback_txcvr2", header, headervalid, payload, payloadlen, payloadvalid, rssi, evm)
    except Exception as e:
        print("Exception: UnicodeDecodeError {}".format(e))
    return 0



if __name__ == '__main__':
    try:
        frequency = 462.0e6         # carrier frequency
        bandwidth = 1000e3         # bandwidth
        num_frames = 2000          # number of frames to transmit
        txgain_dB = -12.0          # software tx gain [dB]
        uhd_txgain = 40.0           # uhd (hardware) tx gain
        #device1 = "30E623A"
        #device2 = "30E6248"
        device1 = "winslab_b210_1"
        device2 = "winslab_b210_2"
        # ofdm properties
        M = 512
        cp_len = 64
        taper_len = 64

        txcvr1 = wrapper.ofdmtxrx(x, M, cp_len, taper_len, device1)
        txcvr1.debug_enable()
        txcvr1.set_callback(mycallback_txcvr1)
        txcvr1.start_rx()

        txcvr2 = wrapper.ofdmtxrx(x, M, cp_len, taper_len, device2)
        txcvr2.debug_enable()
        txcvr2.set_callback(mycallback_txcvr2)
        txcvr2.start_rx()

    #    txcvr.set_tx_freq(frequency);
    #    txcvr.set_tx_rate(bandwidth);
    #    txcvr.set_tx_gain_soft(txgain_dB);
    #    txcvr.set_tx_gain_uhd(uhd_txgain);

        time.sleep(3)
        header = "myheader"
        payload = "mypayload"
        #byte_payload = bytes(payload, 'utf-8')
        payload_len = 9
        ms = wrapper.LIQUID_MODEM_QPSK
        fec0 = wrapper.LIQUID_FEC_NONE # fec(inner)
        fec1 = wrapper.LIQUID_FEC_GOLAY2412 # fec(outer)

        for x in range(20):
            header = "H" + str(x) + "eader"
            txcvr2.transmit_packet_python(header, payload, payload_len, ms, fec0, fec1);
            time.sleep(0.1)

    except Exception as e:
        print("Exception: UnicodeDecodeError {}".format(e))

