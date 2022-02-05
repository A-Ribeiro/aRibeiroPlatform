#ifndef APPLE_SPECIAL_SEMAPHORE
#define APPLE_SPECIAL_SEMAPHORE
#if defined(OS_TARGET_mac) && defined(__APPLE__)

// for unamed semaphores on mac
//#include <dispatch/dispatch.h>
//dispatch_semaphore_t semaphore
//semaphore = dispatch_semaphore_create(count);
//dispatch_release(semaphore);
//return dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, 0)) == 0;
//uint64_t timeout = NSEC_PER_MSEC * timeout_ms;
//long s = dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, timeout));
//signaled = signaled || (dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER) != 0);
//dispatch_semaphore_signal(semaphore);



#include <aRibeiroCore/common.h>
#include <aRibeiroPlatform/PlatformMutex.h>
#include <aRibeiroPlatform/PlatformThread.h>

namespace aRibeiro
{

    // based on: https://stackoverflow.com/questions/641126/posix-semaphores-on-mac-os-x-sem-timedwait-alternative

    typedef struct
    {
        pthread_mutex_t mutex;
        pthread_cond_t cond_var;
        unsigned count;
    } fake_sem_t;

    int fake_sem_init(fake_sem_t* psem, int flags, unsigned count) {
        fake_sem_t pnewsem;
        int result;
        result = pthread_mutex_init(&pnewsem.mutex, NULL);
        if (result)
            return result;
        result = pthread_cond_init(&pnewsem.cond_var, NULL);
        if (result) {
            pthread_mutex_destroy(&pnewsem.mutex);
            return result;
        }
        pnewsem.count = count;
        *psem = pnewsem;
        return 0;
    }

    int fake_sem_destroy(fake_sem_t *psem) {
        if (!psem)
            return EINVAL;
        pthread_mutex_destroy(&psem->mutex);
        pthread_cond_destroy(&psem->cond_var);
        return 0;
    }

    int fake_sem_post(fake_sem_t *pxsem) {
        if (!pxsem)
            return EINVAL;
        int result, xresult;
        result = pthread_mutex_lock(&pxsem->mutex);
        if (result)
            return result;
        pxsem->count = pxsem->count + 1;
        xresult = pthread_cond_signal(&pxsem->cond_var);
        result = pthread_mutex_unlock(&pxsem->mutex);
        if (result)
            return result;
        if (xresult) {
            errno = xresult;
            return -1;
        }
        return 0;
    }

    int fake_sem_wait(fake_sem_t *pxsem) {
        int result, xresult;
        if (!pxsem)
            return EINVAL;
        result = pthread_mutex_lock(&pxsem->mutex);
        if (result)
            return result;
        xresult = 0;
        while (!PlatformThread::isCurrentThreadInterrupted() && pxsem->count == 0) {
            xresult = pthread_cond_wait(&pxsem->cond_var, &pxsem->mutex);
            if (xresult) // any error...
                break;
        }
        if (PlatformThread::isCurrentThreadInterrupted())
            xresult = EINTR;
        if (!xresult && pxsem->count > 0)
            pxsem->count--;
        result = pthread_mutex_unlock(&pxsem->mutex);
        if (result)
            return result;
        if (xresult) {
            errno = xresult;
            return -1;
        }
        return 0;
    }

    int fake_sem_trywait(fake_sem_t *pxsem) {
        int result, xresult;
        if (!pxsem)
            return EINVAL;
        result = pthread_mutex_lock(&pxsem->mutex);
        if (result)
            return result;
        xresult = 0;
        if (pxsem->count > 0)
            pxsem->count--;
        else
            xresult = EAGAIN;
        result = pthread_mutex_unlock(&pxsem->mutex);
        if (result)
            return result;
        if (xresult) {
            errno = xresult;
            return -1;
        }
        return 0;
    }

    int fake_sem_timedwait(fake_sem_t *pxsem, const struct timespec *abstim) {
        int result, xresult;
        if (!pxsem)
            return EINVAL;
        result = pthread_mutex_lock(&pxsem->mutex);
        if (result)
            return result;
        xresult = 0;
        while (!PlatformThread::isCurrentThreadInterrupted() && pxsem->count == 0) {
            xresult = pthread_cond_timedwait(&pxsem->cond_var, &pxsem->mutex, abstim);
            if (xresult) //ETIMEDOUT or any other error...
                break;
        }
        if (PlatformThread::isCurrentThreadInterrupted())
            xresult = EINTR;
        if (!xresult && pxsem->count > 0)
            pxsem->count--;
        result = pthread_mutex_unlock(&pxsem->mutex);
        if (result)
            return result;
        if (xresult) {
            errno = xresult;
            return -1;
        }
        return 0;
    }


}

#endif
#endif