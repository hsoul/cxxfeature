#include <map>
#include <stdio.h>
#include <vector>

#include "external/access_private/include/access_private.hpp"

class TTS
{
private:
    void test(int a, std::map<int, int> b)
    {
        printf("well done\n");
    }
    int ads_ = 7;
    std::map<int, int> dmapd_{
        {2, 1},
        {7, 9}
    };
};

using t = std::map<int, int>;
ACCESS_PRIVATE_FIELD(TTS, t, dmapd_)

using T = void(int a, std::map<int, int> b);
ACCESS_PRIVATE_FUN(TTS, T, test)

int main(int argc, char* argv[])
{

    TTS a;
    auto& i = access_private::dmapd_(a);
    for (auto& elem : i)
    {
        printf("%d <-> %d\n", elem.first, elem.second);
    }

    call_private::test(a, 1, i);

    return 0;
}