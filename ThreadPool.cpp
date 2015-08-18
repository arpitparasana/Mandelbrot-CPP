#include <iostream>
#include <sstream>
#include "ThreadPool.h"


ThreadPool::ThreadPool(int numberOfThreads):quit(false),stopped(false){
	for(int t=0;t<numberOfThreads;t++)
		threads.emplace_back(std::thread(&ThreadPool::Run,this,t));
}

ThreadPool::~ThreadPool(){
	// If not stopped call shutdown.
	if(!stopped)    ShutDown();
}

void ThreadPool::AddJob(function<void(void)> job){
	if(stopped) throw "";
	if(quit)  throw "";
	// Wake up one thread.
	{
		std::unique_lock<std::mutex> smartLock(mtx);
		jobQueue.push(job);
		cv.notify_one();
	}
}


void ThreadPool::ShutDown(){
	if(!stopped){
		// Wake up all threads.
		{
			std::unique_lock<std::mutex> smartLock(mtx);
			quit = true;
			cv.notify_all();
		}
	
	for(auto& e : threads)  e.join();
	threads.clear();
	stopped = true;
	}
}


void ThreadPool::Run(int threadNumber){
	
	std::function<void(void)> job;
	
	Timer t;
	Timer tJob;
	uint64_t cummulativeTime = 0;
	uint64_t activeTime = 0;
	
	t.Start();
	
	while(true){
		{
			std::unique_lock<std::mutex> smartLock(mtx);
			auto f = [this] () {return !jobQueue.empty() || quit;};
			cv.wait(smartLock,f);
		
			if(quit && jobQueue.empty()){
				goto Quit;	
			}

			job=jobQueue.front();
			jobQueue.pop();
		
		}
		tJob.Start();
		job();
		tJob.Stop();
		cummulativeTime += tJob.USecs();
	}
	Quit:
	t.Stop();
	activeTime = t.USecs();
	std::stringstream ss;
	ss 	<< "thread pool thread "<< threadNumber << " was active for " << cummulativeTime << "/" << "microseconds."
		<< 100 * double(cummulativeTime) /double(activeTime) << " busy\n";
	
	std::cout << ss.str();
}