#include "ThreadPool.h"

namespace aRibeiro {

	void ThreadPool::run() {
		while (!PlatformThread::isCurrentThreadInterrupted()) {
			TaskMethod_Fnc job = task_queue.dequeue();
			if (task_queue.isSignaled())
				return;
			job();
		}
	}

	ThreadPool::ThreadPool() {

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
