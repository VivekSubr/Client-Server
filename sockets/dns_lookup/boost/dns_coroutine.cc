#include "utils.h"
#include <boost/asio.hpp>
using namespace boost;
using boost::asio::ip::tcp;

asio::awaitable<void> do_resolve(tcp::resolver& resolver, tcp::resolver::query& query)
{
    co_await resolver.async_resolve(query, asio::use_awaitable);
}

int main(int argc, char **argv)
{
    std::string host = getHostFromArg(1, argc, argv);
    std::cout<<"Resolving "<<host<<"\n";

    asio::io_service io_service;
    asio::io_service::work work(io_service);

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(host, "http");
    asio::co_spawn(io_service, do_resolve(resolver, query), asio::detached);

    while(true)
    {
    }

    return 0;
}