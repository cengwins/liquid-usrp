import sys
sys.path.append('/Users/eronur/Documents/GitHub/liquid-usrp')

import pybind11_wrapper

def test_add():
    assert(pybind11_wrapper.add(3, 4) == 7)
    print(pybind11_wrapper.add(3, 4))

if __name__ == '__main__':
    test_add()
    p = pybind11_wrapper.Pet('Molly')
    print(p)
    print(p.getName())

    o = pybind11_wrapper.ofdmtxrx()
    o.set_tx_freq(2000)

    while(True):
        pass
