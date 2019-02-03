#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/system/error_code.hpp>

#include <iostream>
#include <sstream>
#include <string>

using namespace boost::asio;
using namespace boost::system;

io_service service_;
ip::tcp::socket tcp_socket{service_};
char data[1024];
std::string filename;
size_t filesize;

void client_start(std::string host);
void connect_handler(const boost::system::error_code &ec);
void filename_write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred);
void filesize_write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred);

int main(int argc, char **argv)
{
    filename.append(argv[2]);
    client_start(argv[1]);
}

void client_start(std::string host)
{
    ip::tcp::endpoint ep( ip::address::from_string(host), 8090);
    tcp_socket.async_connect(ep, connect_handler);
    service_.run();
}

void connect_handler(const boost::system::error_code &ec)
{
    if (ec){
      return;
    }

    async_write(tcp_socket, buffer(filename), filename_write_handler);
    // tcp_socket.async_read_some(buffer(bytes), read_handler);
}

void filename_write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    if (ec) {
      return;
    }

    struct stat st;
    stat(filename.c_str(), &st);
    filesize = st.st_size;
    std::string str_filesize = std::to_string(filesize);
    async_write(tcp_socket, buffer(str_filesize), filesize_write_handler);

    // tcp_socket.shutdown(tcp::socket::shutdown_send);
}

void filesize_write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    tcp_socket.shutdown(ip::tcp::socket::shutdown_send);
    return;
}
