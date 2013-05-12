#include <cstdlib> //atoi
#include <cassert> //assert macro
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include <sys/time.h> //gettimeofday

using namespace boost;

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "<usage>: ./program <host> <port>\n";
    return 1;
  }
  asio::io_service io_service;
  asio::ip::tcp::resolver resolver(io_service);
  asio::ip::tcp::resolver::query query(asio::ip::tcp::v4(), argv[1], argv[2]);
  asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
  asio::ip::tcp::socket s(io_service);
  asio::connect(s, iterator);
  int64_t message[2];
  struct timeval tv;
  gettimeofday(&tv, NULL);
  message[0] = tv.tv_sec * 1000000 + tv.tv_usec;
  asio::write(s, asio::buffer(static_cast<void*>(message), sizeof(message))); 
  asio::read(s, asio::buffer(static_cast<void*>(message), sizeof(message))); 
  int64_t send = message[0];
  int64_t their = message[1];
  gettimeofday(&tv, NULL);
  int64_t back = tv.tv_sec * 1000000 + tv.tv_usec; 
  int64_t mine = (back + send) / 2;
  std::cout << "round trip = " << back - send << "\n";
  std::cout << "clock error = " << their - mine << "\n";
  return 0;
}

