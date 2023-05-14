#include <coroutine>
#include <utility>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <cstdio>
#include <utils.h>

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
namespace ip = boost::asio::ip;
namespace this_coro = boost::asio::this_coro;

awaitable<void> echo(tcp::socket socket)
{
  try
  {
    char data[1024];
    for (;;)
    {
      std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data), use_awaitable);
      co_await async_write(socket, boost::asio::buffer(data, n), use_awaitable);
    }
  }
  catch (std::exception& e)
  {
    std::printf("echo Exception: %s\n", e.what());
  }
}

awaitable<void> listener(std::unique_ptr<host> host)
{
  //https://stackoverflow.com/questions/65821773/what-is-the-implement-of-asiothis-coroexecutor
  auto executor = co_await this_coro::executor; 

  //https://www.boost.org/doc/libs/1_82_0/doc/html/boost_asio/reference/basic_socket_acceptor/basic_socket_acceptor/overload6.html
  //First arg is execution context, second is tcp endpoint 
  tcp::acceptor acceptor(executor, ip::tcp::endpoint{ip::address::from_string(host->ip), host->port});
  for (;;)
  {
    tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
    co_spawn(executor, echo(std::move(socket)), detached);
  }
}

int main(int argc, char **argv)
{
  try
  {
    //https://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio/overview/core/basics.html
    //"This I/O execution context represents your program's link to the operating system's I/O services"
    boost::asio::io_context io_context(1);

    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto){ io_context.stop(); });

    auto h = getHostFromArg(argc, argv, 1, 2);
    if(h == nullptr)
    {
      std::cout<<"Defaulting to localhost:3000\n";
      h = std::make_unique<host>("localhost", 3000);
    }

    //https://www.boost.org/doc/libs/1_82_0/doc/html/boost_asio/reference/co_spawn.html
    //"Spawn a new coroutined-based thread of execution." (not actual thread)
    //Here, io_context is the execution context, listener is the awaitable and completion token is detached
    //https://think-async.com/Asio/asio-1.22.2/doc/asio/overview/model/completion_tokens.html
    //https://think-async.com/Asio/asio-1.22.2/doc/asio/reference/detached_t.html
    co_spawn(io_context, listener(std::move(h)), detached);

    std::cout<<"Listening...\n";
    //"While inside the call to io_context::run(), the I/O execution context dequeues the result of the operation, 
    // translates it into an error_code, and then passes it to your completion handler."
    io_context.run(); //blocking call
  }
  catch (std::exception& e)
  {
    std::printf("Exception: %s\n", e.what());
  }

  return 0;
}