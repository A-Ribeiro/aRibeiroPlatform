#ifndef __queue__H__
#define __queue__H__

#include <aRibeiroCore/common.h>
//#include <queue>
#include <list>
#include <aRibeiroPlatform/PlatformAutoLock.h>
#include <aRibeiroPlatform/PlatformSemaphore.h>

namespace aRibeiro {

    /*

    Example of use

#include <aRibeiroPlatform/aRibeiroPlatform.h>
using namespace aRibeiro;

ObjectQueue<int> queue;

void thread() {
    while ( !PlatformThread::getCurrentThread()->isCurrentThreadInterrupted() ) {
        int v = queue.dequeue();
        if (queue.isSignaled())
            break;
        printf("[thread]: %i\n", v);
    }
    printf("end thread\n");
}

int main(int argc, char* argv[]){
    PlatformPath::setWorkingPath(PlatformPath::getExecutablePath(argv[0]));
    // initialize self referencing of the main thread.
    PlatformThread::getMainThread();

    aRibeiro::PlatformThread thread_(thread);
    thread_.start();

    printf("--- Type any number to the Thread (-1 to exit) ---\n");

    char data[1024];
    while (true) {

        fgets(data, 1024, stdin);
        int i = atoi(data);

        if (i == -1)
            break;

        for(int j=0;j<5;j++)
        queue.enqueue( i + j );
    }

    thread_.interrupt();
    thread_.wait();

    return 0;
}

    */

    /*
    template <typename T>
    int ObjectQueue_default_comparer(const T &a, const T & b) {
        if (a > b)
            return 1;
        else if (a < b)
            return -1;
        else
            return 0;
    }

    template <typename T>
    int ObjectQueue_default_comparer_reverse(const T &a, const T & b) {
        if (a > b)
            return -1;
        else if (a < b)
            return 1;
        else
            return 0;
    }
    */

    template <typename T>
    class ObjectQueue {
        aRibeiro::PlatformMutex mutex;
        aRibeiro::PlatformSemaphore semaphore;
        bool blocking;
        bool ordered;

        std::list<T> list;

        //private copy constructores, to avoid copy...
        ObjectQueue(const ObjectQueue& v) {}
        void operator=(const ObjectQueue& v) {}

    public:

        ObjectQueue(bool blocking = true) :semaphore(0) {
            this->blocking = blocking;
            ordered = false;
        }

        //int(*compareFunction)(const T&a, const T&b) = ObjectQueue_default_comparer
        T removeInOrder(const T &v, bool ignoreSignal = false) {

            bool blockinAquireSuccess = false;
            if (blocking) {
                blockinAquireSuccess = semaphore.blockingAcquire();
                if (semaphore.isSignaled() && !ignoreSignal)
                    return T();
            }

            PlatformAutoLock autoLock(&mutex);

            if (list.size() == 0) {
                // not found the data to remove... 
                // incr semaphore for another thread
                if (blockinAquireSuccess)
                    semaphore.release();
                return T();
            }
            else {
                {
                    T begin = list.front();
                    if (begin == v) {
                        list.pop_front();
                        if (blockinAquireSuccess)
                            semaphore.release();
                        return begin;
                    }
                    else if (v < begin) {
                        // not found the data to remove... 
                        // incr semaphore for another thread
                        if (blockinAquireSuccess)
                            semaphore.release();
                        return T();
                    }

                    T end = list.back();
                    if (end == v) {
                        list.pop_back();
                        if (blockinAquireSuccess)
                            semaphore.release();
                        return end;
                    }
                    else if (v > end) {
                        // not found the data to remove... 
                        // incr semaphore for another thread
                        if (blockinAquireSuccess)
                            semaphore.release();
                        return T();
                    }
                }

                //binary search
                int size = list.size();
                typename std::list<T>::iterator aux;
                typename std::list<T>::iterator it = list.begin();
                while (true) {
                    int half_size = size / 2;

                    if (half_size == 0) {
                        T result = (*it);
                        if (!(result == v)) {
                            it++;
                            result = (*it);
                        }
                        list.erase(it);
                        return result;
                    }

                    aux = it;
                    std::advance(aux, half_size);

                    if (v < *aux) {
                        size = half_size;
                    }
                    else if (v > *aux) {
                        it = aux;
                        size = size - half_size;
                    }
                    else {
                        if ((*aux) == v) {
                            T result = (*aux);
                            list.erase(aux);
                            return result;
                        }
                    }
                }
            }

            // not found the data to remove... 
            // incr semaphore for another thread
            if (blockinAquireSuccess)
                semaphore.release();
            return T();
        }

        // int(*compareFunction)(const T&a, const T&b) = ObjectQueue_default_comparer
        void enqueueInOrder(const T &v) {
            //PlatformAutoLock autoLock(&mutex);
            mutex.lock();

            ARIBEIRO_ABORT(ordered == false && list.size() != 0, "Trying to enqueue element in a non-ordered queue.\n");
            ordered = true;

            if (list.size() == 0) {
                list.push_back(v);
            }
            else {
                bool compareResult_it_less_equal = (v < list.front()) ? true : (!(v > list.front()) ? true : false);
                if (compareResult_it_less_equal) {
                    list.push_front(v);

                    mutex.unlock();
                    if (blocking)
                        semaphore.release();
                    return;
                }
                bool compareResult_it_greated_last = (v > list.back()) ? true : (!(v < list.back()) ? true : false);
                if (compareResult_it_greated_last) {
                    list.push_back(v);

                    mutex.unlock();
                    if (blocking)
                        semaphore.release();
                    return;
                }

                //binary search
                int size = list.size();
                typename std::list<T>::iterator aux;
                typename std::list<T>::iterator it = list.begin();
                while (true) {
                    int half_size = size / 2;

                    if (half_size == 0) {
                        it++;
                        list.insert(it, v);
                        break;
                    }

                    aux = it;
                    std::advance(aux, half_size);

                    if (v < *aux) {
                        size = half_size;
                    }
                    else if (v > *aux) {
                        it = aux;
                        size = size - half_size;
                    }
                    else {
                        list.insert(aux, v);
                        break;
                    }
                }

            }


            mutex.unlock();
            if (blocking)
                semaphore.release();


            // */
            /*

                    typename std::list<T>::iterator it = list.begin();
                    while ( it !=  list.end()) {
                        int compareResult = compareFunction(v,*it);
                        if ( compareResult <= 0 )
                            break;
                        it++;
                    }
                    list.insert(it, v);

            // */

            /*
                    typename std::list<T>::reverse_iterator rit = list.rbegin();
                    while ( rit !=  list.rend()) {
                        int compareResult = compareFunction(v,*rit);
                        if ( compareResult >= 0 )
                            break;
                        rit++;
                    }
                    list.insert(rit.base(), v);
            // */
        }

        //int(*compareFunction)(const T&a, const T&b) = ObjectQueue_default_comparer_reverse
        void enqueueInOrderReverse(const T &v) {
            //PlatformAutoLock autoLock(&mutex);

            mutex.lock();

            ARIBEIRO_ABORT(ordered == false && list.size() != 0, "Trying to enqueue element in a non-ordered queue.\n");
            ordered = true;

            if (list.size() == 0) {
                list.push_back(v);
            }
            else {
                bool compareResult_it_less_equal = (v > list.front()) ? true : (!(v < list.front()) ? true : false);
                if (compareResult_it_less_equal) {
                    list.push_front(v);

                    mutex.unlock();
                    if (blocking)
                        semaphore.release();
                    return;
                }
                bool compareResult_it_greated_last = (v < list.back()) ? true : (!(v > list.back()) ? true : false);
                if (compareResult_it_greated_last) {
                    list.push_back(v);

                    mutex.unlock();
                    if (blocking)
                        semaphore.release();
                    return;
                }

                //binary search
                int size = list.size();
                typename std::list<T>::iterator aux;
                typename std::list<T>::iterator it = list.begin();
                while (true) {
                    int half_size = size / 2;

                    if (half_size == 0) {
                        it++;
                        list.insert(it, v);
                        break;
                    }

                    aux = it;
                    std::advance(aux, half_size);

                    if (v > *aux) {
                        size = half_size;
                    }
                    else if (v < *aux) {
                        it = aux;
                        size = size - half_size;
                    }
                    else {
                        list.insert(aux, v);
                        break;
                    }
                }

            }


            mutex.unlock();
            if (blocking)
                semaphore.release();


            // */
            /*

                    typename std::list<T>::iterator it = list.begin();
                    while ( it !=  list.end()) {
                        int compareResult = compareFunction(v,*it);
                        if ( compareResult <= 0 )
                            break;
                        it++;
                    }
                    list.insert(it, v);

            // */

            /*
                    typename std::list<T>::reverse_iterator rit = list.rbegin();
                    while ( rit !=  list.rend()) {
                        int compareResult = compareFunction(v,*rit);
                        if ( compareResult >= 0 )
                            break;
                        rit++;
                    }
                    list.insert(rit.base(), v);
            // */
        }

        void enqueue(const T &v) {
            //PlatformAutoLock autoLock(&mutex);

            mutex.lock();

            ARIBEIRO_ABORT(ordered == true, "Trying to enqueue element in an ordered queue.\n");

            list.push_back(v);
            mutex.unlock();

            if (blocking)
                semaphore.release();
        }

        uint32_t size() {
            PlatformAutoLock autoLock(&mutex);
            return (uint32_t)list.size();
        }

        T peek() {
            PlatformAutoLock autoLock(&mutex);
            if (list.size() > 0)
                return list.front();
            return T();
        }

        T dequeue(bool *isSignaled = NULL, bool ignoreSignal = false) {

            if (blocking) {
                if (!semaphore.blockingAcquire() && !ignoreSignal){
                    if (isSignaled != NULL)
                        *isSignaled = true;
                    return T();
                }

                mutex.lock();
                T result = list.front();
                list.pop_front();
                mutex.unlock();

                if (isSignaled != NULL)
                    *isSignaled = false;
                return result;
            }

            //PlatformAutoLock autoLock(&mutex);
            mutex.lock();
            if (list.size() > 0) {
                T result = list.front();
                list.pop_front();
                mutex.unlock();
                if (isSignaled != NULL)
                    *isSignaled = false;
                return result;
            }
            mutex.unlock();

            if (isSignaled != NULL)
                *isSignaled = false;
            return T();
        }

        T rdequeue(bool *isSignaled = NULL, bool ignoreSignal = false) {

            if (blocking) {
                if (!semaphore.blockingAcquire() && !ignoreSignal) {
                    if (isSignaled != NULL)
                        *isSignaled = true;
                    return T();
                }

                mutex.lock();
                T result = list.back();
                list.pop_back();
                mutex.unlock();
                if (isSignaled != NULL)
                    *isSignaled = false;
                return result;
            }

            mutex.lock();
            if (list.size() > 0) {
                T result = list.back();
                list.pop_back();
                mutex.unlock();
                if (isSignaled != NULL)
                    *isSignaled = false;
                return result;
            }
            mutex.unlock();
            if (isSignaled != NULL)
                *isSignaled = false;
            return T();
        }

        bool isSignaledFromCurrentThread() {
            return semaphore.isSignaled();
        }
    };

}

#endif