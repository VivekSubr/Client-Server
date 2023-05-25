#include "utils.h"
#include <boost/asio.hpp>
using namespace boost;
using boost::asio::ip::tcp;

int main(int argc, char **argv)
{
    std::string host = getHostFromArg(1, argc, argv);
    std::cout<<"Resolving "<<host<<"\n";

    try
    {
        asio::io_service io_service;
        asio::io_service::work work(io_service); //without this in scope, run is non-blocking!
        std::thread worker_thread([&io_service](){ io_service.run(); });
        worker_thread.detach();

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(host, "http");
        std::future<asio::ip::basic_resolver_results<tcp>> dns_result_future = resolver.async_resolve(query, asio::use_future);

        asio::ip::basic_resolver_results<tcp> dns_result = dns_result_future.get();
        
        //it is of type 'ip::basic_resolver_entry'
        for(auto it = dns_result.begin(); it != dns_result.end(); it++)
        { 
           std::cout<<"Endpoint "<<it->endpoint().address().to_string()<<"\n";
        }
    }
    catch (std::system_error& e)
    {
        std::cout << e.what() << std::endl;
    }
    catch(...)
    {
        std::cout<<"Uncaught exception\n";
    }
 
    return 0;
}