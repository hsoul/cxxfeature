#ifndef THREAD_SAFETY_ANALYSIS_MUTEX_H
#define THREAD_SAFETY_ANALYSIS_MUTEX_H

#include <assert.h>
#include <pthread.h>
#include <sys/cdefs.h>

// https://clang.llvm.org/docs/ThreadSafetyAnalysis.html

// Enable thread safety attributes only with clang.
// The attributes can be safely erased when compiling with other compilers.
#if defined(__clang__) && (!defined(SWIG))
    #define THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#else
    #define THREAD_ANNOTATION_ATTRIBUTE__(x) // no-op
#endif

// CAPABILITY is an attribute on classes, which specifies that objects of the class can be used as a capability. The string argument specifies the kind of capability in error messages, e.g. "mutex". See the Container example given above, or the Mutex class in mutex.h.
#define CAPABILITY(x) \
    THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define SCOPED_CAPABILITY \
    THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

// GUARDED_BY is an attribute on data members, which declares that the data member is protected by the given capability. Read operations on the data require shared access, while write operations require exclusive access.
#define GUARDED_BY(x) \
    THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

// PT_GUARDED_BY is similar, but is intended for use on pointers and smart pointers. There is no constraint on the data member itself, but the data that it points to is protected by the given capability.
#define PT_GUARDED_BY(x) \
    THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))
// ACQUIRED_BEFORE and ACQUIRED_AFTER are attributes on member declarations, specifically declarations of mutexes or other capabilities. These declarations enforce a particular order in which the mutexes must be acquired, in order to prevent deadlock.
#define ACQUIRED_BEFORE(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define ACQUIRED_AFTER(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

// REQUIRES is an attribute on functions or methods, which declares that the calling thread must have exclusive access to the given capabilities.More than one capability may be specified.The capabilities must be held on entry to the function, and must still be held on exit.
#define REQUIRES(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define REQUIRES_SHARED(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

// ACQUIRE and ACQUIRE_SHARED are attributes on functions or methods declaring that the function acquires a capability, but does not release it. The given capability must not be held on entry, and will be held on exit (exclusively for ACQUIRE, shared for ACQUIRE_SHARED).
#define ACQUIRE(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define ACQUIRE_SHARED(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

// RELEASE, RELEASE_SHARED, and RELEASE_GENERIC declare that the function releases the given capability. The capability must be held on entry (exclusively for RELEASE, shared for RELEASE_SHARED, exclusively or shared for RELEASE_GENERIC), and will no longer be held on exit.
#define RELEASE(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define RELEASE_SHARED(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define RELEASE_GENERIC(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(release_generic_capability(__VA_ARGS__))

#define TRY_ACQUIRE(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define TRY_ACQUIRE_SHARED(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

// EXCLUDES is an attribute on functions or methods, which declares that the caller must not hold the given capabilities.This annotation is used to prevent deadlock.Many mutex implementations are not re - entrant, so deadlock can occur if the function acquires the mutex a second time.
#define EXCLUDES(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASSERT_CAPABILITY(x) \
    THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define ASSERT_SHARED_CAPABILITY(x) \
    THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define RETURN_CAPABILITY(x) \
    THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define NO_THREAD_SAFETY_ANALYSIS \
    THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

#define CHECK_PTHREAD_RETURN_VALUE

#ifdef CHECK_PTHREAD_RETURN_VALUE
__BEGIN_DECLS
extern void __assert_perror_fail(int errnum, const char *file, unsigned int line, const char *function) noexcept __attribute__((__noreturn__));
__END_DECLS

    #define MCHECK(ret) ({                                              \
        __typeof__(ret) errnum = (ret);                                 \
        if (__builtin_expect(errnum != 0, 0))                           \
        {                                                               \
            __assert_perror_fail(errnum, __FILE__, __LINE__, __func__); \
        }                                                               \
    })
#else
    #define MCHECK(ret) ({              \
        __typeof__(ret) errnum = (ret); \
        assert(errnum == 0);            \
        (void)errnum;                   \
    })
#endif

// Defines an annotated interface for mutexes.
// These methods can be implemented to use any internal mutex implementation.
class CAPABILITY("mutex") Mutex
{
private:
    pthread_mutex_t mutex_;
    pid_t holder_;
public:
    Mutex()
        : holder_(0)
    {
        // pthread_mutex_init(&mutex_, NULL);
        MCHECK(pthread_mutex_init(&mutex_, NULL));
    }
    ~Mutex()
    {
        assert(holder_ == 0);
        MCHECK(pthread_mutex_destroy(&mutex_));
    }

    bool isLockedByThisThread() const
    {
        return false; // holder_ == current thread tid
    }

    // Acquire/lock this mutex exclusively.  Only one thread can have exclusive
    // access at any one time.  Write operations to guarded data require an
    // exclusive lock.
    void lock() ACQUIRE()
    {
        MCHECK(pthread_mutex_lock(&mutex_));
        assignHolder();
    }

    // Release/unlock an exclusive mutex.
    void unlock() RELEASE()
    {
        MCHECK(pthread_mutex_unlock(&mutex_));
        unassignHolder();
    }

    // Assert that this mutex is currently held by the calling thread.
    void AssertLocked() ASSERT_CAPABILITY(this)
    {
        assert(isLockedByThisThread());
    }

    pthread_mutex_t *getPthreadMutex()
    {
        return &mutex_;
    }
private:
    void assignHolder()
    {
        holder_ = 0;
    }
    void unassignHolder()
    {
        // holder_= current thread id
    }
};

// MutexGuard is an RAII class that acquires a mutex in its constructor, and
// releases it in its destructor.
class SCOPED_CAPABILITY MutexGuard
{
private:
    Mutex &mutex_;
public:
    // Acquire mu, implicitly acquire *this and associate it with mu.
    explicit MutexGuard(Mutex &mutex) ACQUIRE(mutex_)
        : mutex_(mutex)
    {
        mutex_.lock();
    }
    // Release *this and all associated mutexes, if they are still held.
    // There is no warning if the scope was already unlocked before.
    ~MutexGuard() RELEASE()
    {
        mutex_.unlock();
    }
};

#endif // THREAD_SAFETY_ANALYSIS_MUTEX_H