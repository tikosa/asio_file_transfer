#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/system/error_code.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>

using namespace boost::asio;
using namespace boost::system;


io_service service_;
ip::tcp::socket tcp_socket{service_};
char data[1024];
std::string filename;
size_t filesize;
size_t bytes_sent;
int fd;

void client_start(std::string host);
void connect_handler(const boost::system::error_code &ec);
void filename_write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred);
void filesize_write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred);
void write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred);
void show_progress(double progress);

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
    fd = open(filename.c_str(), O_RDONLY);
    if(-1 == fd) {
        std::cout << "Could not open the file: " << strerror(errno) << std::endl;
        close(fd);
        tcp_socket.shutdown(ip::tcp::socket::shutdown_send);
        return;
    }
    int bytes = 0;
    if( (bytes = ::read(fd, data, 1024 )) > 0) {
        // std::string kb(data, bytes);
        async_write(tcp_socket, buffer(data, bytes), write_handler);
        // tcp_socket.async_write_some(buffer(data, bytes), write_handler);
    } else {
        close(fd);
        tcp_socket.shutdown(ip::tcp::socket::shutdown_send);
    }
}

void write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    if (ec){
        tcp_socket.shutdown(ip::tcp::socket::shutdown_send);
    }

    // Show progress
    bytes_sent += bytes_transferred;
    double percent = (double)bytes_sent / filesize;
    show_progress(percent);

    // std::this_thread::sleep_for(std::chrono::milliseconds(10));

    int bytes = 0;
    if( (bytes = ::read(fd, data, 1024) ) > 0) {
        // std::string kb(data, bytes);
        async_write(tcp_socket, buffer(data, bytes), write_handler);
        // tcp_socket.async_write_some(buffer(data, bytes), write_handler);
    } else {
        close(fd);
        tcp_socket.shutdown(ip::tcp::socket::shutdown_send);
    }
}

void show_progress(double progress)
{
    int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %" << ((progress == 1) ? "\n" : "\r");
    std::cout.flush();

    //std::cout << std::endl;
}
