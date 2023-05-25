#include "utils.h"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
using namespace boost;
using boost::asio::ip::tcp;

void pollDNS(const system::error_code& e, asio::steady_timer* t, int* count, asio::io_context* io)
{
    std::cout<<"***Hit pollDNS\n";
    (*count)++;
    int nReady = io->poll_one();
    t->expires_at(t->expiry() + std::chrono::seconds(2));
}

void dnsCallback(const system::error_code& error, tcp::resolver::results_type results)
{
    for(auto it = results.begin(); it != results.end(); it++)
    { 
        std::cout<<"Endpoint "<<it->endpoint().address().to_string()<<"\n";
    }
}

int main(int argc, char **argv)
{
    std::string host = getHostFromArg(1, argc, argv);
    std::cout<<"Resolving "<<host<<"\n";

    asio::io_context timer_io;
    asio::io_context dns_io;
    asio::io_service::work work(timer_io);

    tcp::resolver        resolver(dns_io);
    tcp::resolver::query query(host, "http");   
    resolver.async_resolve(query, boost::bind(dnsCallback, asio::placeholders::error, placeholders::_2));

    int nCount{0};
    asio::steady_timer t(timer_io, std::chrono::seconds(2));
    t.async_wait(boost::bind(pollDNS, asio::placeholders::error, &t, &nCount, &dns_io));

    timer_io.run();
    return 0;
}