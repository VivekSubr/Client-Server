#include <vector>
#include <string>
#include <cassert>

struct PoD 
{
    int         i;
    double      j;
    std::string str;

    PoD(int a, double b, const std::string& s): i(a), j(b), str(s)
    {
    }
};

int main()
{
    std::vector<PoD> testV;
    for(int i=0; i<100; i++) 
    {
        testV.emplace_back(PoD(i, i, std::to_string(i)));
    }

    assert(0);
    return 0;
}