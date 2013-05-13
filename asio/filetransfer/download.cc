#include <iostream>

#include <cassert>

#include <sys/stat.h>
#include <fcntl.h>


#define BOOST_ASIO_DISABLE_EPOLL

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>




using namespace boost;

const char *g_file = NULL;

//do not inherit
class FileHandle {
 public:
  using FileHandlePtr = boost::shared_ptr<FileHandle>;
  FileHandle(asio::io_service& io_service, const char *filename)
    : stream_descriptor_{io_service}, fd_{-1} {
    if ((fd_ = open(filename, O_RDONLY)) < 0) {
      throw boost::system::error_code{};
    }
    stream_descriptor_.assign(fd_); // filesystem fd is not support by epoll
  }
  asio::posix::stream_descriptor& GetStreamDescriptor() {
    return stream_descriptor_;
  }
  asio::mutable_buffers_1 GetBuffer(size_t size) {
    return asio::buffer(data_.data(), size);
  }
  asio::mutable_buffers_1 GetBuffer() {
    return asio::buffer(data_);
  }
  size_t GetBufferSize() {
    return data_.size();
  }
  ~FileHandle() {
    if (fd_ >= 0) {
      close(fd_);
    }
  }
 private:
  int fd_;
  asio::posix::stream_descriptor stream_descriptor_;
  boost::array<char, 1024> data_;
  FileHandle(const FileHandle&) = delete;
  FileHandle& operator=(const FileHandle&) = delete;
  FileHandle(FileHandle&&) = delete;
  FileHandle& operator=(FileHandle&&) = delete;
};



class Connection : public enable_shared_from_this<Connection> {
 public:
  using ConnPtr = boost::shared_ptr<Connection>;
  static ConnPtr Create(asio::io_service& io_service) {
    return ConnPtr(new Connection{io_service});
  }
  asio::ip::tcp::socket& GetSocket() {
    return socket_;
  }
  void Start() {
    try {
      auto filehandle = boost::make_shared<FileHandle>(socket_.get_io_service(), g_file);
      StartRead(filehandle);
    } catch (boost::system::error_code& e) {
      std::cout << "can not open file " << g_file << "\n";
    }
  }

  void StartRead(FileHandle::FileHandlePtr filehandle) {
    asio::async_read(filehandle->GetStreamDescriptor(), filehandle->GetBuffer(), 
                     boost::bind(&Connection::HandleRead, shared_from_this(),
                                 filehandle,
                                 asio::placeholders::error,
                                 asio::placeholders::bytes_transferred));
  }

  void HandleRead(FileHandle::FileHandlePtr filehandle, const boost::system::error_code& e, 
                  size_t bytes_read) {
    if (e && e != boost::asio::error::eof) {
      std::cout << e.message() << "\n";
    } else {
      if (bytes_read > 0) {
        asio::async_write(socket_, filehandle->GetBuffer(bytes_read),
                          boost::bind(&Connection::HandleWrite, shared_from_this(),
                                      filehandle,
                                      asio::placeholders::error,
                                      asio::placeholders::bytes_transferred));
      } else {
        std::cout << "file is transferred\n";
      }
    }
  }
  void HandleWrite(FileHandle::FileHandlePtr filehandle, const boost::system::error_code& e,
                   size_t bytes_written) {
    if (bytes_written == filehandle->GetBufferSize()) {
      StartRead(filehandle);
    } else {
      std::cout << "file is transferred\n";
    }
  }
 private:

  Connection(asio::io_service& io_service) : socket_{io_service} {
  }

  asio::ip::tcp::socket socket_;

  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;
  Connection(Connection&&) = delete;
  Connection& operator=(Connection&&) = delete;
};

class FileServer {
 public:
  FileServer(asio::io_service& io_service, int port) 
    : io_service_{io_service}, 
      acceptor_{io_service, asio::ip::tcp::endpoint{asio::ip::tcp::v4(), port}} {
    StartAccept();
  }
  void StartAccept() {
    auto conn = Connection::Create(io_service_);
    acceptor_.async_accept(conn->GetSocket(),
                           boost::bind(&FileServer::HandleAccept, this,
                                       conn,
                                       asio::placeholders::error));
  }
  void HandleAccept(Connection::ConnPtr conn, const boost::system::error_code& e) {
    if (e) {
      std::cout << e.message() << "\n";
    } else {
      conn->Start();
    }
    StartAccept();
  }
 private:
  asio::io_service& io_service_;
  asio::ip::tcp::acceptor acceptor_;

  FileServer(const FileServer&) = delete;
  FileServer& operator=(const FileServer&) = delete;
  FileServer(FileServer&&) = delete;
  FileServer& operator=(FileServer&&) = delete;
};

int main(int argc, const char** argv) {
  if (argc != 3) {
    std::cout << "usage: ./program port <filename>\n";
    return 1;
  }
  boost::asio::io_service io_service;
  g_file = argv[2];
  FileServer server(io_service, ::atoi(argv[1]));
  io_service.run();
  return 0;
}
