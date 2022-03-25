#ifndef __thread__pool__h__
#define __thread__pool__h__

#include <aRibeiroCore/MethodPointer.h>
#include <aRibeiroPlatform/ObjectQueue.h>
#include <aRibeiroPlatform/PlatformThread.h>
#include <aRibeiroPlatform/PlatformSemaphore.h>

namespace aRibeiro {

	DefineMethodPointer(TaskMethod_Fnc, void) VoidMethodCall();

	class ThreadPool {

		std::vector<PlatformThread*> threads;
		ObjectQueue<TaskMethod_Fnc> task_queue;

		void run();

	public:

		ThreadPool();
		~ThreadPool();
		void postTask(const TaskMethod_Fnc& fnc);

	};
}

#endif