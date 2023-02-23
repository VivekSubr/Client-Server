#include "server_test.h"
#include "server_impl.h"

std::shared_ptr<std::thread> ServerTest::m_tServer;

void ServerTest::SetUpTestSuite() 
{
   m_tServer.reset(new std::thread([&](){
      std::cout<<"Starting server\n";
      run("8888");
   }));
}

void ServerTest::TearDownTestSuite() 
{
   m_tServer->join();
} 

TEST_F(ServerTest, Test_RST_STREAM)
{
  
}

int main(int argc, char **argv) {
   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}