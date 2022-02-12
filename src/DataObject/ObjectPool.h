#ifndef __pool__H__
#define __pool__H__

#include <aRibeiroCore/common.h>
//#include <queue>
#include <list>
#include <aRibeiroPlatform/PlatformAutoLock.h>
#include <aRibeiroPlatform/ObjectQueue.h>

namespace aRibeiro {

    template <class T>
    class ObjectPool {

        struct ObjectPoolElement {
            bool ignore_placement_new_delete;
            T* data;

            ObjectPoolElement() {
                ignore_placement_new_delete = false;
                data = NULL;
            }

            bool operator >(const ObjectPoolElement&b)const {
                return (uintptr_t)data > (uintptr_t)b.data;
            }
            bool operator <(const ObjectPoolElement&b)const {
                return (uintptr_t)data < (uintptr_t)b.data;
            }
            bool operator ==(const ObjectPoolElement&b)const {
                return (uintptr_t)data == (uintptr_t)b.data;
            }
        };

        aRibeiro::PlatformMutex mutex;

        //std::vector<Element> list;

        ObjectQueue< ObjectPoolElement > available;
        //ObjectQueue< ObjectPoolElement > in_use;
        std::map< T*, ObjectPoolElement > in_use;


        //std::vector<T*> used;
        //std::vector<T*> available;
        // int posCreate;
        // int posRelease;
        // int count;

        bool released;

        //private copy constructores, to avoid copy...
        ObjectPool(const ObjectPool& v) {}
        void operator=(const ObjectPool& v) {}

    public:

        ObjectPool() : available(false)
            //    , in_use(false) 
        {
            released = false;
        }

        virtual ~ObjectPool() {
            PlatformAutoLock autoLock(&mutex);

            while (available.size() > 0) {
                ObjectPoolElement element = available.dequeue(true);
                delete element.data;
            }

            typename std::map< T*, ObjectPoolElement >::iterator it = in_use.begin();
            while (it != in_use.end()) {
                ObjectPoolElement element = it->second;
                if (!element.ignore_placement_new_delete)
                    element.data->~T();
                delete element.data;
            }
            in_use.clear();

            /*
            while (in_use.size() > 0) {
                ObjectPoolElement element = in_use.dequeue(true);
                if (!element.ignore_placement_new_delete)
                    element.data->~T();
                delete element.data;
            }
            */

            released = true;
        }

        T* create(bool ignore_placement_new_delete = false) {
            PlatformAutoLock autoLock(&mutex);

            ARIBEIRO_ABORT(released, "ERROR: trying to create element from a deleted pool");

            if (available.size() > 0) {
                ObjectPoolElement element = available.dequeue();
                element.ignore_placement_new_delete = ignore_placement_new_delete;

                //in_use.enqueueInOrder(element);
                in_use[element.data] = element;

                //placement new operator
                if (!element.ignore_placement_new_delete)
                    new (element.data) T();
                return element.data;
            }
            else {
                ObjectPoolElement newElement;
                newElement.ignore_placement_new_delete = ignore_placement_new_delete;
                newElement.data = new T();

                in_use[newElement.data] = newElement;
                //in_use.enqueueInOrder(newElement);

                return newElement.data;
            }
        }

        void release(T* data) {
            PlatformAutoLock autoLock(&mutex);

            ARIBEIRO_ABORT(released, "ERROR: trying to release element from a deleted pool\n");

            //check in_use elements
            typename std::map< T*, ObjectPoolElement >::iterator it = in_use.find(data);

            if (it != in_use.end()) {
                ObjectPoolElement removedElement = it->second;

                in_use.erase(it);

                //placement delete operator
                if (!removedElement.ignore_placement_new_delete)
                    removedElement.data->~T();

                available.enqueue(removedElement);
            }
            else {
                ARIBEIRO_ABORT(true, "ERROR: deleting unknown element...\n");
            }

            /*
            ObjectPoolElement toSearch;
            toSearch.data = data;
            ObjectPoolElement removedElement = in_use.removeInOrder(toSearch, true);
            if (removedElement.data != NULL) {
                //placement delete operator
                if (!removedElement.ignore_placement_new_delete)
                    removedElement.data->~T();

                available.enqueue(removedElement);
            }
            else {
                ARIBEIRO_ABORT(true, "ERROR: deleting unknown element...\n");
            }
            */

        }
    };

}

#endif

