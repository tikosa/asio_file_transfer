#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/system/error_code.hpp>

#include <iostream>
#include <sstream>
#include <string>

using namespace boost::asio;
using namespace boost::system;

io_service service_;
ip::tcp::endpoint tcp_endpoint{ip::tcp::v4(), 8090};
ip::tcp::acceptor tcp_acceptor{service_, tcp_endpoint};
ip::tcp::socket tcp_socket{service_};
char data[1024];
std::string filename;
size_t filesize;
int fd;

void accept_handler(const boost::system::error_code &ec);
void filename_read_handler(const boost::system::error_code &ec, size_t bytes_read);
void filesize_read_handler(const boost::system::error_code &ec, size_t bytes_read);
void file_read_handler(const boost::system::error_code &ec, size_t bytes_read);

int main()
{
    tcp_acceptor.listen();
    tcp_acceptor.async_accept(tcp_socket, accept_handler);
    service_.run();
}

void accept_handler(const boost::system::error_code &ec)
{
  if (ec) {
      return;
  }
  tcp_socket.async_read_some(buffer(data), filename_read_handler);
}

void filename_read_handler(const boost::system::error_code &ec, size_t bytes_read)
{
  if (ec) {
      return;
  }
  filename.append(data, bytes_read);
  std::cout << "filename = " << filename << std::endl;
  tcp_socket.async_read_some(buffer(data), filesize_read_handler);
}

void filesize_read_handler(const boost::system::error_code &ec, size_t bytes_read)
{
  if (ec) {
      return;
  }
  filesize = std::stoi(std::string(data, bytes_read-1));
  std::cout << "filesize = " << filesize << std::endl;
  fd = open(filename.c_str(), O_CREAT | O_RDWR | O_APPEND, 0666);
  if (fd < 0) {
      close(fd);
      return;
  }

  tcp_socket.async_read_some(buffer(data), file_read_handler);
}

void file_read_handler(const boost::system::error_code &ec, size_t bytes_read)
{
  if (ec) {
      return;
  }
  ::write(fd, data, bytes_read);
  tcp_socket.async_read_some(buffer(data), file_read_handler);
}
