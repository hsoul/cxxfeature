#include "thread_analysis.h"

class BankAccount
{
public:
    Mutex mutex_;
    int balance_ GUARDED_BY(mutex_);
public:
    void withDraw(int amount)
    {
        mutex_.lock();
        withDrawImpl(amount);
        mutex_.unlock();
    }
    void transferFrom(BankAccount& b, int amount)
    {
        mutex_.lock();
        b.withDraw(amount);
        b.mutex_.lock();
        b.withDrawImpl(amount);
        b.mutex_.unlock();
        depositTmpl(amount);
        mutex_.unlock();
    }
private:
    void depositTmpl(int amount) REQUIRES(mutex_)
    {
        balance_ += amount;
    }
    void withDrawImpl(int amount) REQUIRES(mutex_)
    {
        balance_ -= amount;
    }
};

int main()
{
    return 0;
}