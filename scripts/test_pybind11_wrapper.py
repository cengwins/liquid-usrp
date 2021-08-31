import sys
sys.path.append('/Users/eronur/Documents/GitHub/liquid-usrp')

import pybind11_wrapper

def mycallback(header,headervalid,payload,payloadlen,payloadvalid):
    print("mycallback", header, headervalid, payload, payloadlen, payloadvalid)

def test_add():
    assert(pybind11_wrapper.add(3, 4) == 7)
    print(pybind11_wrapper.add(3, 4))

if __name__ == '__main__':
    test_add()
    p = pybind11_wrapper.Pet('Molly')
    print(p)
    print(p.getName())




    M = 48
    cp_len = 6
    taper_len = 4

    o = pybind11_wrapper.ofdmtxrx(M,cp_len,taper_len)
    o.debug_enable()
    o.set_callback(mycallback)

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
