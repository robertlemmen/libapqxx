#ifndef APQXX_H
#define APQXX_H

#include <string>

#include <boost/asio.hpp>

// forward defs for libpq
struct pg_conn;
typedef struct pg_conn PGconn;

/* the general idea is to template this with an IO library adapter as well as a
 * parameter that is also passed on to that adapter on construction. for
 * boost::asio that would be an adapter that takes an io_service as argument.
 *
 * an adapter allows constructing a watcher over a file descriptor, which can
 * then be used to wait until it becomes readable
 *
 * this stuff needs to move to a different file over time
 * */

namespace apqxx {

class AsioWatcher {
};

class AsioAdapter {
    public:
        virtual apqxx::AsioWatcher* new_watcher(int fd) = 0;
};

class BoostAsioWatcher {
    public:
        BoostAsioWatcher(
};

class BoostAsioAdapter : public AsioAdapter {
    public:
        BoostAsioAdapter(boost::asio::io_service &ios);
        virtual ~BoostAsioAdapter();
        virtual apqxx::AsioWatcher* new_watcher(int fd);

    private:
        boost::asio::io_service &ios_;
};

class Connection;

class Transaction {
    public:
        Transaction(Connection *conn); // XXX perhaps only called by Connection...
        ~Transaction();

        void exec(const std::string &sql);

    private:
        Connection *conn_;
};

class Connection {
    public:
        Connection(const std::string &options = std::string{});
        ~Connection();

        Transaction start_tx();

    friend Transaction;

    private:
        PGconn *pg_conn_;
};

class ConnectionPool {
    public:
        ConnectionPool(apqxx::AsioAdapter *asio, const std::string &options = std::string{});
        ~ConnectionPool();

        // XXX this should be some sort of rvalue copy unique_ptr thing
        Connection& borrow();
        void recycle(Connection &conn);

    private:
        AsioAdapter *asio_;
        Connection conn_;   // XXX not really a "pool", but let's get it working functionally and
                            // around the async parts first...
        
};

} // namespace apqxx

#endif /* APQXX_H */
