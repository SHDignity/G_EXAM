/*
* @file vosThread.h 
* @author Park si hyeon (devicepark@naver.com) 
* @date 2006-01-13
* 
* @brief for using thread in Windows and Linux
* 
* @license This code is licensed under the MIT License.
*          Notify the original author when modifying the source code.
*/


#ifndef __VOSTHREAD_H__
#define __VOSTHREAD_H__

#ifdef _WIN32
#	include <Windows.h>
#else
#	include <pthread.h>
#	include <signal.h>
#	include <mutex>
#	include <condition_variable>
#endif

/*
* @example vosThread th;
* @example th.start();		// start thread
* @example ...
* @example th.isRunning();  // fast look up
* @example ...
* @example th.wait();		// wait thread end
*/

class vosThread {
public:
	vosThread& operator=(const vosThread& in_thread) {
		return *this;
	}

	vosThread(const vosThread& in_thread) {
#ifdef _WIN32
		_handle = 0x00;
#else
#endif
		_isRunning = false;
	}

#ifdef _WIN32
	HANDLE _handle;
	bool _paused = false;
#else
	pthread_t _thread;
	std::mutex _mutex;
	std::condition_variable _cv;
	bool _startSuspended = false;
	bool _paused = false;
#endif

	bool _isRunning;

#ifdef _WIN32
	static DWORD WINAPI Starter(LPVOID in_thread) {
#else
	static void* Starter(void* in_thread) {
		vosThread* thread = static_cast<vosThread*>(in_thread);
		{
			std::unique_lock<std::mutex> lock(thread->_mutex);
			while (thread->_startSuspended) {
				thread->_cv.wait(lock);
			}
		}
#endif
		vosThread* thread = static_cast<vosThread*>(in_thread);
		thread->_isRunning = true;
		thread->run(in_thread);
		thread->_isRunning = false;
		return 0x00;
	}

public:
	vosThread() {
#ifdef _WIN32
		_handle = 0x00;
#else
#endif
		_isRunning = false;
	}

	virtual ~vosThread() {
		if (_isRunning) {
			wait();
#ifdef _WIN32
			CloseHandle(_handle);
#else
			if (_thread) {
				pthread_join(_thread, NULL);
				_thread = 0;
			}
#endif
		}

	}

	bool start() {
		if (_isRunning) return false;
#ifdef _WIN32
		_handle = CreateThread(0x00, 0x00, vosThread::Starter, static_cast<void*>(this), 0x00, 0x00);
		return _handle != NULL;
#else
		_startSuspended = false;
		return pthread_create(&_thread, NULL, vosThread::Starter, static_cast<void*>(this)) == 0;
#endif
	}

	//서스펜드로 시작하는게 없어서...추가!
	bool start_suspended() {
		if (_isRunning) return false;
#ifdef _WIN32
		_handle = CreateThread(0x00, 0x00, vosThread::Starter, static_cast<void*>(this), CREATE_SUSPENDED, 0x00);
		return _handle != NULL;
#else
		_startSuspended = true;
		return pthread_create(&_thread, NULL, vosThread::Starter, static_cast<void*>(this)) == 0;
#endif
	}

	bool isRunning() const {
		return _isRunning;
	}

	bool wait() const {
		if (!_isRunning) return false;
#ifdef _WIN32
		return WaitForSingleObject(_handle, INFINITE) == 0x00000000L;
#else
		return pthread_join(_thread, NULL) == 0;
#endif
	}


#ifdef _WIN32
	virtual void run(LPVOID pParam) = 0;
#else
	virtual void run(void* pParam) = 0;
#endif

	bool kill() {
		_isRunning = false;
#ifdef _WIN32
		bool success = TerminateThread(_handle, 1) && CloseHandle(_handle);
		_handle = 0x00;
		return success;
#else
		return pthread_kill(_thread, SIGKILL) == 0;
#endif
	}

#ifdef _WIN32
	bool resume(HANDLE handle) {
		DWORD dwRet;
		dwRet = ::ResumeThread(handle);
		if (dwRet == (DWORD)-1) {
			return false;
		}
		_paused = false;
		return true;
	}
#else
	bool resume() {
		std::lock_guard<std::mutex> lock(_mutex);
		_startSuspended = false;
		_paused = false;
		_cv.notify_one();
		return true;
	}
#endif

#ifdef _WIN32
	bool pause(HANDLE handle) {
		DWORD dwRet;
		dwRet = ::SuspendThread(handle);
		if (dwRet == 0xFFFFFFFF) {
			return false;
		}
		_paused = true;
		return true;
	}
#else
	bool pause() {
		std::lock_guard<std::mutex> lock(_mutex);
		_paused = true;
		return true;
	}
#endif

#ifdef _WIN32
	bool stop() {
		//스레드가 종료 요청을 처리하도록 신호를 보내자
		_isRunning = false;

		//pause 상태라면 ResumeThread를 호출하여 대기 상태를 해제
		if (_paused) {
			ResumeThread(_handle);
			_paused = false;
		}

		//스레드가 종료될 때까지 대기하자
		if (_handle) {
			WaitForSingleObject(_handle, INFINITE);
			CloseHandle(_handle);
			_handle = 0x00;
		}
		return true;
	}
#else
	bool stop() {
		std::lock_guard<std::mutex> lock(_mutex);
		_isRunning = false;

		//pause 상태라면 대기 상태 해제
		if (_paused) {
			_paused = false;
			_cv.notify_one();
		}

		//스레드가 종료될 때가지 대기
		if (_thread) {
			pthread_join(_thread, NULL);
			_thread = 0;
		}

		return true;
	}
#endif

public:
#ifdef _WIN32
	HANDLE getHandle() const {
		return _handle;
	}
#else
	pthread_t getHandle() const {
		return _thread;
	}
#endif
};

#endif



