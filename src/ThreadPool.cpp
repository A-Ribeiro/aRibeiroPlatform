#include "ThreadPool.h"

namespace aRibeiro {

	void ThreadPool::run() {

#ifdef _WIN32
		//SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
		//SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
		//SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
			printf("SetThreadPriority WorkerThread ERROR...\n");
#endif

		while (!PlatformThread::isCurrentThreadInterrupted()) {
			TaskMethod_Fnc job = task_queue.dequeue();
			if (task_queue.isSignaled())
				return;
			job();
		}

	}

	ThreadPool::ThreadPool() {

#ifdef _WIN32
		//SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
		//SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
		if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
			printf("SetPriorityClass  MainThread ERROR...\n");
		if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
			printf("SetThreadPriority MainThread ERROR...\n");
#endif

		for (int i = 0; i < PlatformThread::QueryNumberOfSystemThreads(); i++)
			threads.push_back(new PlatformThread(this, &ThreadPool::run));
		for (int i = 0; i < threads.size(); i++)
			threads[i]->start();

		printf("[ThreadPool] Created %i threads on the pool.\n", (int)threads.size());
	}

	ThreadPool::~ThreadPool() {

		for (int i = 0; i < threads.size(); i++)
			threads[i]->interrupt();
		for (int i = 0; i < threads.size(); i++)
			threads[i]->wait();
		for (int i = 0; i < threads.size(); i++)
			delete threads[i];
		threads.clear();

	}

	void ThreadPool::postTask(const TaskMethod_Fnc& fnc) {
		task_queue.enqueue(fnc);
	}
}
