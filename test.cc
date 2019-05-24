#include <iostream>

#include "apqxx.h"

using namespace std;
using namespace apqxx;

int main(int argc, char **argv) {

    cout << "Running tests for libapqxx (" << VERSION << ")..." << endl; 

    boost::asio::io_service ios;
    BoostAsioAdapter *adapter = new BoostAsioAdapter(ios);

    ConnectionPool cp(adapter);
    Connection &conn = cp.borrow();
    Transaction tx = conn.start_tx();
    tx.exec("SELECT 1");

    cp.recycle(conn);

    cout << "All done!" << endl;

    delete adapter;

    return 0;
}
