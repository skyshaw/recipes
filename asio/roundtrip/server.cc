#include <cstdlib> //atoi
#include <cassert> //assert macro
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include <sys/time.h> //gettimeofday

using namespace boost;

class Connection: public boost::enable_shared_from_this<Connection> {
 public:
  typedef boost::shared_ptr<Connection> ConnPtr;
  static ConnPtr Create(asio::io_service& io_service) {
    return ConnPtr(new Connection(io_service));
  }
  asio::ip::tcp::socket& GetSocket() {
    return socket_;
  }
  void Start() {
    asio::async_read(socket_, asio::buffer(static_cast<void*>(message), sizeof(message)), 
                     boost::bind(&Connection::HandleRead, 
                                 shared_from_this(), //inherit from enable_shared_from_this
                                 asio::placeholders::error,
                                 asio::placeholders::bytes_transferred));

  }
  void HandleRead(const boost::system::error_code& e, size_t bytes_read) {
    assert(bytes_read == sizeof(message));
    if (!e) {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      message[1] = tv.tv_sec * 1000000 + tv.tv_usec; 
      asio::async_write(socket_, asio::buffer(static_cast<void*>(message), sizeof(message)),
                        boost::bind(&Connection::HandleWrite,
                                    shared_from_this(),
                                    asio::placeholders::error,
                                    asio::placeholders::bytes_transferred));
    }
  }
  void HandleWrite(const boost::system::error_code& e, size_t bytes_write) {
    assert(bytes_write == sizeof(message));
  }
 private:
  asio::ip::tcp::socket socket_;
  int64_t message[2];


  Connection(asio::io_service& io_service) : socket_(io_service) {

  }
  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;
  Connection(Connection&&) = delete;
  Connection& operator=(Connection&&) = delete;
  
};


class Server {
 public:
  Server(asio::io_service& io_service, int port) 
    : io_service_(io_service),
      acceptor_(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {
    StartAccept();
  }
  void StartAccept() {
    auto conn = Connection::Create(io_service_);
    acceptor_.async_accept(conn->GetSocket(), 
                           boost::bind(&Server::HandleAccept, this, 
                                       asio::placeholders::error, conn));
  }
  void HandleAccept(const boost::system::error_code& e, Connection::ConnPtr conn) {
    if (!e) {
      conn->Start();
    }
    StartAccept();
  }
 private:
  asio::io_service& io_service_;
  asio::ip::tcp::acceptor acceptor_;
};



int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "<usage>: ./program port\n";
    return 1;
  }
  int port = atoi(argv[1]);
  asio::io_service io_service;
  Server server(io_service, port);
  io_service.run();
  return 0;
}

