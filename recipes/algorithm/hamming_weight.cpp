#include "hamming_weight.h"
#include <iostream>

namespace hamming_weight
{
int count1_move(uint32_t x) // 每次右移一位遇到 1 二进制位计数
{
    int count = 0;
    int times = 0;
    while (x)
    {
        if (x & 0x1)
            ++count;
        x = (x >> 1);
        ++times;
    }
    printf("times %d\n", times);
    return count;
}

int count1_sub(uint32_t x)
{
    int count = 0;
    int times = 0;
    while (x)
    {
        x = x & (x - 1); // x&(x-1)将会减少x二进制中最右边的1，直至x变为0
        count++;
        ++times;
    }
    printf("times %d\n", times);
    return count;
}

int count1_c_0(int x) // 与之相似的变形是可以先统计出二进制数中0的个数，统计方法是x = x | (x + 1) 的作用是每次循环把x的二进制中从右往左数的第一个0变成1，直道变成全1的时候x + 1就溢出为全0，循环结束。
{
    int n = 0;
    while ((x + 1))
    {
        n++;
        x |= (x + 1); // x = x | (x + 1) 的作用是每次循环把x的二进制中从右往左数的第一个0变成1
    }
    return 32 - n;
}

int Hamming_weight(uint32_t n)
{
    n = (n & 0x55555555) + ((n >> 1) & 0x55555555);
    n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
    n = (n & 0x0f0f0f0f) + ((n >> 4) & 0x0f0f0f0f);
    n = (n & 0x00ff00ff) + ((n >> 8) & 0x00ff00ff);
    n = (n & 0x0000ffff) + ((n >> 16) & 0x0000ffff);

    return n;
}

/*
这个方法是Hamming weight Wikipedia上面提出来的，很高效，比上面的两种方法都要高效。采用了分治法的思想

代码解析： 乍一看，立马懵逼，很难看懂为何那么写。先将代码中用到的几个常数对比一下其特点，再联想到分治的思想，你可能就懂了。

    0x5555……这个换成二进制之后就是01 01 01 01 01 01 01 01……
    0x3333……这个换成二进制之后就是0011 0011 0011 0011……
    0x0f0f………这个换成二进制之后就是00001111 00001111……
    看出来点什么了吗？
    如果把这些二进制序列看作一个循环的周期序列的话，那么第一个序列的周期是2，每个周期是01，第二个序列的周期是4，每个周期是0011，第三个的周期是8，每个是00001111，第四个和第五个以此类推。
    看出了这些数的特点，再回头看代码你会轻松的发现代码的意义。
    算法的实现原理是将32位无符号整数分成32个段，每个段即1bit，段的取值可表示当前段中1的个数，所以将32个段的数值累加在一起就是二进制中1的个数，如何累加呢？
    这就是代码做的事情。 (n & 0x55555555) + ((n >> 1) & 0x55555555) 将32位数中的32个段从左往右把相邻的两个段的值相加后放在2bits中，就变成了16个段，每段2位。
    同理(n & 0x33333333) + ((n >> 2) & 0x33333333) 将16个段中相邻的两个段两两相加，存放在4bits中，就变成了8个段，每段4位。
    以此类推，最终求得数中1的个数就存放在一个段中，这个段32bits，就是最后的n。

    看懂了上面几行的代码，你会情不自禁的想说：妙，太妙了！算法的世界总是那么奇妙。

    你也许可能会问，有没有更优的方法了，还真有，Hamming weight Wikipedia还有对该方法的优化，有心者继续探究吧，我就此打住，不和大家一同前行啦。
*/

int count1_group(unsigned char n)
{
    n = (n & 0x55) + ((n >> 1) & 0x55); // 2 位分组 01010101
    n = (n & 0x33) + ((n >> 2) & 0x33); // 4 位分组 00110011
    n = (n & 0x0f) + ((n >> 4) & 0x0f); // 8 位分组 00001111
    return n;
}

void Test()
{
    int input = 0;
    int num = count1_move(uint32_t(input));
    std::cout << "bit 1 nums " << num << std::endl;
    num = count1_sub(uint32_t(input));
    std::cout << "bit 1 nums " << num << std::endl;
    num = count1_group(input);
    std::cout << "bit 1 nums " << num << std::endl;
}
} // namespace hamming_weight
