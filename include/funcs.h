
#include <complex>
#include <pthread.h>
#include <liquid/liquid.h>
#include <uhd/usrp/multi_usrp.hpp>


int add(int i, int j);


struct Pet {
    Pet(const std::string &name) : name(name) { }
    void setName(const std::string &name_) { name = name_; }
    const std::string &getName() const { return name; }

    std::string name;
};


class eonofdmtxrx {
public:
    eonofdmtxrx( ); // test constructor for python binding

    void set_tx_freq(float _tx_freq);
    void set_tx_rate(float _tx_rate);


private:
    // RF objects and properties
    uhd::usrp::multi_usrp::sptr usrp_tx;
    uhd::usrp::multi_usrp::sptr usrp_rx;
    uhd::tx_metadata_t          metadata_tx;
};