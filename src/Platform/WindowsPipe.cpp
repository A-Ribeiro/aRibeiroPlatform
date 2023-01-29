#include "WindowsPipe.h"

#if defined(_WIN32)

#include <aRibeiroPlatform/PlatformThread.h>

#include <inttypes.h>

namespace aRibeiro
{

	static std::string _GetLastErrorToString_pipe() {
		std::string result;

		wchar_t* s = NULL;
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&s, 0, NULL);

		// WCHAR TO CONSOLE CP (Code Page)
		if (s) {
			/*
			wchar_t *aux = new wchar_t[lstrlenW(s)+1];
			memset(aux, 0, (lstrlenW(s)+1) * sizeof(wchar_t));

			//MultiByteToWideChar(CP_ACP, 0L, s, lstrlenA(s) + 1, aux, strlen(s));
			MultiByteToWideChar(
				CP_OEMCP //GetConsoleCP()
				, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS , s, lstrlenA(s) + 1, aux, lstrlenA(s));
				result = StringUtil::toString(aux);
			*/

			char* aux = new char[lstrlenW(s) + 1];
			memset(aux, 0, (lstrlenW(s) + 1) * sizeof(char));
			WideCharToMultiByte(
				GetConsoleOutputCP(), //GetConsoleCP(),
				WC_COMPOSITECHECK,
				s, lstrlenW(s) + 1,
				aux, lstrlenW(s),
				NULL, NULL
			);
			result = aux;
			delete[] aux;
			LocalFree(s);
		}

		return result;
	}

	std::string WindowsPipe::randomNamedPipeName() {
		static PlatformMutex mutex;
		static uint64_t uid = 0;
		PlatformAutoLock autoLock(&mutex);
		char aux[64];
		sprintf(aux, "\\\\.\\pipe\\%" PRIu64, uid++);
		return aux;
	}

	WindowsPipe::WindowsPipe()
	{
		read_is_blocking = true;
		read_signaled = false;
		write_signaled = false;
		read_fd = INVALID_FD;
		write_fd = INVALID_FD;

		read_event = NULL;



		read_event = CreateEvent(0, TRUE, FALSE, NULL);
		ARIBEIRO_ABORT(read_event == NULL, "Error to create Event. Message: %s", _GetLastErrorToString_pipe().c_str());

		ZeroMemory(&overlapped, sizeof(OVERLAPPED));
		overlapped.hEvent = read_event;


		SECURITY_DESCRIPTOR sd;
		InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
		SetSecurityDescriptorDacl(&sd, TRUE, (PACL)0, FALSE);

		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = &sd;
		sa.bInheritHandle = TRUE;

		//bool bool_result = CreatePipe(&read_fd, &write_fd, &sa, 0);

		//LPCTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");

		std::string named_pipe_name = WindowsPipe::randomNamedPipeName();

		read_fd = CreateNamedPipe(
			named_pipe_name.c_str(),
			PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,       // read/write access //PIPE_ACCESS_DUPLEX | PIPE_ACCESS_OUTBOUND 
			PIPE_TYPE_BYTE | //PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_BYTE | //PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			512,                  // output buffer size 
			512,                  // input buffer size 
			0,                        // client time-out 
			&sa);                    // default security attribute 

		ARIBEIRO_ABORT(read_fd == INVALID_HANDLE_VALUE, "%s", _GetLastErrorToString_pipe().c_str());


		write_fd = ::CreateFile(
			named_pipe_name.c_str(), 
			GENERIC_WRITE,
			0, 
			&sa, 
			OPEN_EXISTING, 
			FILE_FLAG_OVERLAPPED, 
			NULL);

		ARIBEIRO_ABORT(write_fd == INVALID_HANDLE_VALUE, "%s", _GetLastErrorToString_pipe().c_str());

	}

	WindowsPipe::~WindowsPipe()
	{
		close();
	}

	size_t WindowsPipe::write(const void* buffer, size_t size)
	{
		if (write_fd == INVALID_FD || write_signaled)
			return 0;

		//https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
		//WriteFile
		DWORD written = 0;

		bool bool_result = ::WriteFile(write_fd, buffer, (DWORD)size, &written, NULL);

		// can have another returns
		if (!bool_result)
		{
			//if (errno == EAGAIN)
			//{
			//    // non blocking
			//}
			//else
			{
				// any other error
				printf("[WindowsPipe] write ERROR: %s", _GetLastErrorToString_pipe().c_str());
				write_signaled = true;
			}
			return 0;
		}

		return (size_t)written;
	}

	// returns true if read something...
	// false the other case ...
	// need to call isSignaled if receive false
	// https://learn.microsoft.com/en-us/windows/win32/fileio/testing-for-the-end-of-a-file
	bool WindowsPipe::read(aRibeiro::ObjectBuffer* outputBuffer)
	{
		if (read_fd == INVALID_FD || read_signaled)
			return false;
		if (outputBuffer->size == 0)
			outputBuffer->setSize(64 * 1024); // 64k

		//https://stackoverflow.com/questions/42402673/createprocess-and-capture-stdout

		//https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
		//ReadFile

		aRibeiro::PlatformThread* currentThread = aRibeiro::PlatformThread::getCurrentThread();

		DWORD received = 0;
		bool bool_result = ::ReadFile(read_fd, outputBuffer->data, outputBuffer->size, &received, &overlapped);
		//ARIBEIRO_ABORT(!bool_result, "%s", _GetLastErrorToString_pipe().c_str());

		if (bool_result) {
			if (received > 0) {
				outputBuffer->setSize(received);
				return true;
			}
			else if (received == 0) {
				outputBuffer->setSize(0);
				// EOF
				closeReadFD();
				return false;
			}
			else {
				// any other error
				printf("[WindowsPipe] read ERROR: unkown error\n");
				read_signaled = true;
			}
		}

		// bool result is false from here
		DWORD dwError = GetLastError();

		bool continue_waiting = true;

		while (continue_waiting) {

			continue_waiting = false;

			switch (dwError)
			{
			case ERROR_BROKEN_PIPE:
			{
				outputBuffer->setSize(0);
				// EOF
				closeReadFD();
				return false;
			}
			case ERROR_HANDLE_EOF:
			{
				outputBuffer->setSize(0);
				// EOF
				closeReadFD();
				return false;
			}
			case ERROR_IO_INCOMPLETE:
			{
				printf("[WindowsPipe] I/O incomplete.\n");
				continue_waiting = true;
				break;
			}
			case ERROR_IO_PENDING:
			{
				// need to wait the overlapped event
				DWORD dwWaitResult;
				HANDLE handles_threadInterrupt_sem[2] = {
					read_event, // WAIT_OBJECT_0 + 0
					currentThread->m_thread_interrupt_event // WAIT_OBJECT_0 + 1
				};

				DWORD dwWaitTime = INFINITE;
				if (!read_is_blocking)
					dwWaitTime = 0;

				dwWaitResult = WaitForMultipleObjects(
					2,   // number of handles in array
					handles_threadInterrupt_sem,     // array of thread handles
					FALSE,          // wait until all are signaled
					dwWaitTime // INFINITE //INFINITE
				);

				if (dwWaitResult == WAIT_TIMEOUT) {
					// NOT EOF, the wait gives timeout... trying to wait again
					printf("[WindowsPipe] Timeout...\n");
					CancelIo(read_fd);
					{
						//check if have any already read data after call to cancelIO
						bool_result = GetOverlappedResult(read_fd, &overlapped, &received, FALSE);
						if (bool_result) {
							ResetEvent(read_event);
							outputBuffer->setSize(received);
							return (received > 0);
						}
					}
					outputBuffer->setSize(0);
					return false;
				}
				else if (dwWaitResult == WAIT_OBJECT_0 + 1) {
					// true if the interrupt is signaled (and only the interrupt...) 
					// Thread Interrupted
					printf("thread interrupted...\n");
					CancelIo(read_fd);
					{
						//check if have any already read data after call to cancelIO
						bool_result = GetOverlappedResult(read_fd, &overlapped, &received, FALSE);
						if (bool_result) {
							ResetEvent(read_event);
							outputBuffer->setSize(received);
							return true;
						}
					}
					outputBuffer->setSize(0);
					return false;
				}
				else if (dwWaitResult == WAIT_OBJECT_0 + 0) {
					// true if the read signaled the IO event...
					bool_result = GetOverlappedResult(read_fd, &overlapped, &received, FALSE);
					if (bool_result) {
						ResetEvent(read_event);
						outputBuffer->setSize(received);
						return true;
					}
					else {
						//forward the error to the next iteration...
						dwError = GetLastError();
						continue_waiting = true;
					}
				}
				break;
			}

			default:
			{
				printf("[WindowsPipe] read UNHANDLED ERROR: %s", _GetLastErrorToString_pipe().c_str());
				break;
			}

			}
		}







		//if (received > 0)
		//{
		//    outputBuffer->setSize(received);
		//    return true;
		//}
		//outputBuffer->setSize(0);
		//if (received == 0)
		//{
		//    // EOF
		//    closeReadFD();
		//    return false;
		//}

		//// can have another returns
		//if (!bool_result)
		//{
		//    //if (errno == EAGAIN)
		//    //{
		//    //    // non blocking
		//    //    return false;
		//    //}
		//    //else
		//    {
		//        // any other error
		//        printf("[WindowsPipe] read ERROR: %s", _GetLastErrorToString_pipe().c_str());
		//        read_signaled = true;
		//    }
		//}

		return false;
	}

	bool WindowsPipe::isReadSignaled()
	{
		return read_fd == INVALID_FD || read_signaled || aRibeiro::PlatformThread::isCurrentThreadInterrupted();
	}

	bool WindowsPipe::isWriteSignaled()
	{
		return write_fd == INVALID_FD || write_signaled || aRibeiro::PlatformThread::isCurrentThreadInterrupted();
	}

	//// STDIN_FILENO
	//void WindowsPipe::aliasReadAs(int fd)
	//{
	//    dup2(read_fd, fd);
	//}

	//// STDOUT_FILENO | STDERR_FILENO
	//void WindowsPipe::aliasWriteAs(int fd)
	//{
	//    dup2(write_fd, fd);
	//}

	bool WindowsPipe::isReadBlocking()
	{
		/*auto flags = fcntl(read_fd, F_GETFL);
		return (flags & O_NONBLOCK) != 0;*/
		return read_is_blocking;
	}

	bool WindowsPipe::isWriteBlocking()
	{
		/*auto flags = fcntl(write_fd, F_GETFL);
		return (flags & O_NONBLOCK) != 0;*/
		return true;
	}

	void WindowsPipe::setReadBlocking(bool v)
	{
		/*auto flags = fcntl(read_fd, F_GETFL);
		fcntl(read_fd, F_SETFL, (v) ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK));*/
		read_is_blocking = v;
	}

	void WindowsPipe::setWriteBlocking(bool v)
	{
		/*auto flags = fcntl(write_fd, F_GETFL);
		fcntl(write_fd, F_SETFL, (v) ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK));*/
	}

	bool WindowsPipe::isReadFDClosed()
	{
		return read_fd == INVALID_FD;
	}

	bool WindowsPipe::isWriteFDClosed()
	{
		return write_fd == INVALID_FD;
	}

	void WindowsPipe::closeReadFD()
	{
		if (read_fd == INVALID_FD)
			return;
		CloseHandle(read_fd);
		read_fd = INVALID_FD;
	}

	void WindowsPipe::closeWriteFD()
	{
		if (write_fd == INVALID_FD)
			return;
		CloseHandle(write_fd);
		write_fd = INVALID_FD;
	}

	void WindowsPipe::close()
	{
		closeReadFD();
		closeWriteFD();

		if (read_event != NULL) {
			CloseHandle(read_event);
			read_event = NULL;
		}
	}
}

#endif
