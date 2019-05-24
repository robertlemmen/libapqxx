#include "apqxx.h"

#include <libpq-fe.h>

#include <iostream> // XXX for debugging

using namespace std;

namespace apqxx {



BoostAsioAdapter::BoostAsioAdapter(boost::asio::io_service &ios) :
    ios_(ios) {
}

BoostAsioAdapter::~BoostAsioAdapter() {
}

AsioWatcher* BoostAsioAdapter::new_watcher(int fd) {
    return 0;
}

Transaction::Transaction(Connection *conn) :
    conn_{conn} {
}

Transaction::~Transaction() {
}

void Transaction::exec(const std::string &sql) {
    int ret = PQsendQuery(conn_->pg_conn_, sql.c_str());
    if (!ret) {
        cout << "# error exec statemnt:" << PQerrorMessage(conn_->pg_conn_) << endl;
    }
    PGresult *result;
    do {
        int fd = PQsocket(conn_->pg_conn_);
        cout << "# fd: " << fd << endl;
        // XXX here we need to wait for action on fd!
        PQconsumeInput(conn_->pg_conn_);
        cout << "# PGgetResult will block: " << PQisBusy(conn_->pg_conn_) << endl;
        result = PQgetResult(conn_->pg_conn_);
        cout << "# got stuff from query" << endl;
    } while (result);
    cout << "# exec status: " << PQresStatus(PQresultStatus(result)) << endl;
}

// this should really be inside libpq
const char* PQconnStatus(ConnStatusType status) {
    switch (status) {
        case CONNECTION_OK:
            return "OK";
        case CONNECTION_BAD:
            return "BAD";
        case CONNECTION_STARTED:
            return "STARTED";
        case CONNECTION_MADE:
            return "MADE";
        case CONNECTION_AWAITING_RESPONSE:
            return "AWAITING_RESPONSE";
        case CONNECTION_AUTH_OK:
            return "AUTH_OK";
        case CONNECTION_SETENV:
            return "SETENV";
        case CONNECTION_SSL_STARTUP:
            return "SSL_STARTUP";
        case CONNECTION_NEEDED:
            return "NEEDED";
        case CONNECTION_CHECK_WRITABLE:
            return "CHECK_WRITABLE";
        case CONNECTION_CONSUME:
            return "CONSUME";
        default:
            return "??";
    }
}

Connection::Connection(const string &options) :
    pg_conn_(PQconnectStart(options.c_str())) {
    int fd = PQsocket(pg_conn_);
    cout << "# db name: " << PQdb(pg_conn_) << endl;
    cout << "# fd: " << fd << endl;
    ConnStatusType status = PQstatus(pg_conn_);
    ConnStatusType prev_status = status;
    // XXX obviously we should not poll, but use async to check for fd changes,
    // also means callback lambda is required
    do {
        PQconnectPoll(pg_conn_);
        status = PQstatus(pg_conn_);
        if (prev_status != status) {
            cout << "# status: " << PQconnStatus(status) << endl;
            prev_status = status;
        }
    } while ((status != CONNECTION_BAD) && (status != CONNECTION_OK));
    if (status == CONNECTION_BAD) {
        cout << "# last message: " << PQerrorMessage(pg_conn_) << endl;
    }
    PQsetnonblocking(pg_conn_, 1);
    cout << "# connection is non-blocking: " << PQisnonblocking(pg_conn_) << endl;
}

Connection::~Connection() {
}

Transaction Connection::start_tx() {
    return Transaction(this);
}

ConnectionPool::ConnectionPool(AsioAdapter *asio, const string &options) :
    conn_{options} {
}

ConnectionPool::~ConnectionPool() {
}

Connection& ConnectionPool::borrow() {
    return conn_;
}

void ConnectionPool::recycle(Connection &conn) {
    // XXX once we have a real pool
}

} // namespace apqxx
