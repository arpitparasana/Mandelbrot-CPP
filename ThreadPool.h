#pragma once
#include <iostream>
#include <functional>
#include <condition_variable>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>

using namespace std;


class ThreadPool {
	vector<thread> threads;
	queue<function<void()>> jobQueue;
	mutex mtx;
	condition_variable cv;
	bool quit;
	bool stopped;
	void Run(int); 
	
public:
	ThreadPool(int);
	~ThreadPool();
	void AddJob(function<void()> job);
	void ShutDown();
	
};

class Timer
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> start;
		std::chrono::time_point<std::chrono::high_resolution_clock> stop;
	public:
		Timer(){}

		void Start()
		{
			start  = std::chrono::high_resolution_clock::now();
		}
		void Stop()
		{
			stop   = std::chrono::high_resolution_clock::now();
		}
		uint64_t MSsecs()
		{
			typedef std::chrono::duration<int,std::milli> millisecs_t ;
			millisecs_t duration_get( std::chrono::duration_cast<millisecs_t>(stop-start) ) ;
			return duration_get.count();
		}
		uint64_t USecs()
		{
			typedef std::chrono::duration<int,std::micro> microsecs_t ;
			microsecs_t duration_get( std::chrono::duration_cast<microsecs_t>(stop-start) ) ;
			return duration_get.count();
		}
	};