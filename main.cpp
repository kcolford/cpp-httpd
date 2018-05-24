// simple high performance http server

// FIXME: this should be moved to Boost.Coroutine2
#define BOOST_COROUTINES_NO_DEPRECATION_WARNING

#include <boost/asio/spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <boost/thread.hpp>

namespace httpd {

namespace asio = boost::asio;
using boost::asio::ip::tcp;

void accept_connection(tcp::acceptor &);
void run();

struct socket {
  tcp::socket sock;
  asio::yield_context &yield;

  socket(tcp::acceptor &listener, asio::yield_context &y): sock(listener.get_io_context()), yield(y) {
    listener.async_accept(sock, yield);
  }

  // Return the next character from the socket.
  char next();

  // Return a string of all characters up to delim.
  std::string next(char delim);

  // Send all the characters in buf to the output.
  void write(std::string buf);

 private:
  unsigned short i = 0;
  unsigned short n = 0;
  char buf[1024];
};

void handle_conversation(socket &);

struct request {
  std::string path;
};

}

void httpd::socket::write(std::string buf) {
  asio::async_write(sock, asio::buffer(buf), yield);
}

char httpd::socket::next() {
  if (i >= n) {
    i = 0;
    n = sock.async_read_some(asio::buffer(buf), yield);
  }
  return buf[i++];
}

std::string httpd::socket::next(char delim) {
  std::string out;
  char c;
  while ((c = next()) != delim)
    out += c;
  return out;
}

void httpd::handle_conversation(httpd::socket &sock) {
  auto action = sock.next(' ');
  auto path = sock.next(' ');
  auto version = sock.next('\r');
  std::string header;
  do {
    sock.next();
    header = sock.next('\r');
  } while (header != "");

  sock.write("HTTP/1.1 200 OK\r\n"
             "\r\n"
             "Hello, World!\n"
             );
}

void httpd::accept_connection(tcp::acceptor &listener) {
  asio::spawn(listener.get_executor(), [&](asio::yield_context yield) {
      socket sock(listener, yield);
      httpd::accept_connection(listener);

      httpd::handle_conversation(sock);
    });
}

void httpd::run() {
  asio::io_context io;
  
  tcp::acceptor lis(io, tcp::endpoint(tcp::v6(), 1234));
  httpd::accept_connection(lis);

  // Listening on ipv6 usually covers ipv4 too but sometimes it
  // doesn't.
  try {
    tcp::acceptor lis4(io, tcp::endpoint(tcp::v4(), 1234));
    httpd::accept_connection(lis4);
  } catch (...) {}

  // Start a thread pool based on the number of cores to maximize the
  // connection handling capability.
  std::vector<boost::thread> thread_pool;
  while (thread_pool.size()
#if 0
         < 1 && 1
#endif
         < std::thread::hardware_concurrency())
    thread_pool.push_back(boost::thread([&io]() {
          io.run();
        }));
  for (auto &t : thread_pool)
    t.join();
}

int main() {
  httpd::run();
  return 0;
}
