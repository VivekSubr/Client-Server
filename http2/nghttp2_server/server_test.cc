#include "server_test.h"
#include "server_impl.h"

void ServerTest::SetUpTestSuite() 
{

}

void ServerTest::TearDownTestSuite() 
{

} 

TEST_F(ServerTest, Test_RST_STREAM)
{
  
}

int main(int argc, char **argv) {
   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}