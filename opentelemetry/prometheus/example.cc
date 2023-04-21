#include <string>
#include <iostream>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include "metrics.h"
namespace po = boost::program_options;
using M = std::map<std::string, std::string>;

void createSyncInstrument(Metrics& m, const std::string& name);
void incSyncInstrument(Metrics& m, const std::string& name);
void createObservableInstrument(Metrics& m, const std::string& name);

int main(int argc, char *argv[])
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    Metrics m("example", "0.1", "127.0.0.1:4317");
    std::cout<<"Started...\n";
    while(true) 
    {
       std::string tok1, tok2, tok3;
       std::cin>>tok1>>tok2>>tok3;
       std::cout<<"Parsed to "<<tok1<<" : "<<tok2<<" : "<<tok3<<"\n";
       if(tok1 == "CREATE") {
        if(tok2 == "SYNC") {
           createSyncInstrument(m, tok3);
        }
        else if(tok2 == "OBS") {

        }  
        else {
           std::cout<<"Unknown type\n";
        }
       }
       else if(tok1 == "INC") {
        if(tok2 == "SYNC") {
           incSyncInstrument(m, tok3);
        }
       } 
       else {
         std::cout<<"Unknown command\n";
       }
    }

    return 0;
}

void createSyncInstrument(Metrics& m, const std::string& name)
{
    if(name.empty()) {
        std::cout<<"name empty!\n";
        return;
    }

    m.CreateSyncInstrument(Metrics::InstrumentType::UIntCounter, name, "test counter", "units");

    if(m.GetSyncInstrument(name) != nullptr) std::cout<<"Created successfully\n";
}

void incSyncInstrument(Metrics& m, const std::string& name)
{
    auto inst = m.GetSyncInstrument(name);
    if(inst == nullptr) {
        std::cout<<"Invalid name\n";
        return;
    }

    static_cast<metrics::Counter<uint64_t>*>(inst)->Add(10, 
            opentelemetry::common::KeyValueIterableView<M>({{"test-label", "test"}, {"abc", "123"}}));

    std::cout<<"incremented!\n";
}

void createObservableInstrument(Metrics& m, const std::string& name)
{

}