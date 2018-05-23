#include <boost/asio/spawn.hpp>
#include <boost/asio.hpp>
#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

void accept_connection(tcp::acceptor &listener) {
  spawn([&](yield_context yield) {
      tcp::socket sock(listener.get_io_context());
      listener.async_accept(sock, yield);
      accept_connection(listener);
      
      string headline;
      async_read_until(sock, dynamic_buffer(headline), "\r\n\r\n", yield);
      clog << headline << endl;
      string response = "HTTP/1.1 200 OK\r\n"
                        "\r\n"
                        "hello world\n";
      async_write(sock, buffer(response), yield);
    });
}

int main() {
  io_context io;
  tcp::acceptor lis(io, tcp::endpoint(tcp::v6(), 1234));
  accept_connection(lis);
  io.run();
  return 0;
}

  
  
