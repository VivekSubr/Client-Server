#include <coroutine>
#include <utility>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <cstdio>
#include <utils.h>

using boost::asio::ip::tcp;
using namespace boost;
namespace ip = boost::asio::ip;

asio::awaitable<void> echo(tcp::socket socket)
{
  try
  {
    char data[1024];
    for(;;)
    {
      std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data), asio::use_awaitable);
      std::cout<<"echo "<<data<<", size "<<n<<"\n";
      
      co_await async_write(socket, boost::asio::buffer(data, n), asio::use_awaitable);
    }
  }
  catch (std::exception& e)
  {
    std::printf("echo Exception: %s\n", e.what());
  }
}

asio::awaitable<void> udp_echo(ip::udp::socket socket)
{
  try
  {
    char data[1024];
    for(;;)
    {
      std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data), asio::use_awaitable);
      std::cout<<"echo "<<data<<", size "<<n<<"\n";
      
      co_await async_write(socket, boost::asio::buffer(data, n), asio::use_awaitable);
    }
  }
  catch (std::exception& e)
  {
    std::printf("echo Exception: %s\n", e.what());
  }
}

asio::awaitable<void> listener(std::unique_ptr<host> host)
{
  //https://stackoverflow.com/questions/65821773/what-is-the-implement-of-asiothis-coroexecutor
  auto executor = co_await asio::this_coro::executor; 

  //https://www.boost.org/doc/libs/1_82_0/doc/html/boost_asio/reference/basic_socket_acceptor/basic_socket_acceptor/overload6.html
  //First arg is execution context, second is tcp endpoint 
  auto endpoint = ip::tcp::endpoint{ip::address::from_string(host->ip), host->port};
  tcp::acceptor acceptor(executor, endpoint);

  std::cout<<"Listening...\n";
  for(;;)
  {
    tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
    asio::co_spawn(executor, echo(std::move(socket)), asio::detached);
  }
}

asio::awaitable<void> udp_listener(std::unique_ptr<host> host)
{
  auto executor = co_await asio::this_coro::executor; 
  auto endpoint = ip::udp::endpoint{ip::address::from_string(host->ip), host->port};
  
  std::cout<<"Listening...\n";
  for(;;)
  {
    ip::udp::socket socket(executor, endpoint);  
    asio::co_spawn(executor, udp_echo(std::move(socket)), asio::detached);
  }
}

int main(int argc, char **argv)
{
  try
  {
    //https://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio/overview/core/basics.html
    //"This I/O execution context represents your program's link to the operating system's I/O services"
    asio::io_context io_context(1); //concurrency_hint = 1, https://stackoverflow.com/questions/40829143/what-does-concurrency-hint-of-boostasioio-service-means

    asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto){ io_context.stop(); });

    auto h = getHostFromArg(argc, argv, 2, 3);
    if(h == nullptr)
    {
      std::cout<<"Defaulting to localhost:3000\n";
      h = std::make_unique<host>("127.0.0.1", 3000);
    }

    //https://www.boost.org/doc/libs/1_82_0/doc/html/boost_asio/reference/co_spawn.html
    //"Spawn a new coroutined-based thread of execution." (not actual thread)
    //Here, io_context is the execution context, listener is the awaitable and completion token is detached
    //https://think-async.com/Asio/asio-1.22.2/doc/asio/overview/model/completion_tokens.html
    //https://think-async.com/Asio/asio-1.22.2/doc/asio/reference/detached_t.html
    asio::co_spawn(io_context, listener(std::move(h)), asio::detached);

    //"While inside the call to io_context::run(), the I/O execution context dequeues the result of the operation, 
    // translates it into an error_code, and then passes it to your completion handler."
    io_context.run(); //blocking call
  }
  catch(std::exception& e)
  {
    std::printf("Exception: %s\n", e.what());
  }

  return 0;
}