#include <boost/asio.hpp>
#include <utils.h>

using namespace boost::asio;
using namespace std;
using ip::tcp;

int main(int argc, char **argv) 
{
    boost::asio::io_service io_service;

    auto h = getHostFromArg(argc, argv, 1, 2);
    if(h == nullptr)
    {
      std::cout<<"Defaulting to localhost:3000\n";
      h = std::make_unique<host>("127.0.0.1", 3000);
    }

    tcp::socket socket(io_service);//socket creation
    auto endpoint = tcp::endpoint(boost::asio::ip::address::from_string(h->ip), h->port);
    try 
    {
        socket.connect(endpoint);
    }
    catch(boost::system::system_error& err)
    {
        cout << "connect failed: " << err.code().message() <<" on "<<endpoint<<endl;
        return -1;
    }

    const string msg = "Hello from Client!\n";
    boost::system::error_code error;
    boost::asio::write( socket, boost::asio::buffer(msg), error );
    if(!error) {
        cout << "Client sent hello message!" << endl;
    }
    else {
        cout << "send failed: " << error.message() << endl;
    }

    std::vector<char> buf(1024);
    size_t len = socket.read_some(boost::asio::buffer(buf), error);
    std::cout<<"Revieved len : "<<len<<"\n";
    if( error && error != boost::asio::error::eof ) {
        cout << "receive failed: " << error.message() << endl;
    }
    else {
       std::string s{buf.begin(), buf.end()};
       std::cout<<s<<endl;
    }
    
    return 0;
}