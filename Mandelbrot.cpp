/*
 	OOP345 Final Project.
 	Mandelbrot.cpp
 	Name 			: 	Arpit Parasana
 	Student Number 	: 	012565149

 	If run with....
 	g++ -std=c++0x -o mandelbrot -pthread Mandelbrot.cpp ThreadPool.cpp		-- Compile
 	mandelbrot mandel 1024 1024 200 -2 1 -2 2								-- Run
	It gives following output and three images.
	
	Run: 4021031 usecs.4.02103 secs.
	Running with 4 threads
	doWork: start,end=0/256,calc time 245964 usecs, 0.245964 secs.
	doWork: start,end=768/1024,calc time 641342 usecs, 0.641342 secs.
	doWork: start,end=256/512,calc time 1212346 usecs, 1.21235 secs.
	doWork: start,end=512/768,calc time 2638918 usecs, 2.63892 secs.
	RunThreaded: 2639168 usecs.2.63917 secs.
	thread pool thread 0 was active for 1703643/microseconds.99.5502 busy
	thread pool thread 2 was active for 1699171/microseconds.99.2894 busy
	thread pool thread 1 was active for 1704638/microseconds.99.5813 busy
	thread pool thread 3 was active for 1704558/microseconds.99.5743 busy
	RunThreadPool: 1712006 usecs.1.71201 secs.
*/

#include <complex>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <sstream>
#include <chrono>
#include <vector>
#include <thread>
#include <functional>

#include "ThreadPool.h"

using namespace std;


class Pixel{
	uint8_t R, G, B;
	public:
	Pixel(){}
	Pixel(uint8_t r,uint8_t g,uint8_t b) : R(r),G(g),B(b) {}
	uint8_t getR() { return R; }
	uint8_t getG() { return G; }
	uint8_t getB() { return B; }
};

class Image{
	size_t W;
	size_t H;
	Pixel* data;
	public:
	Image() : W(0), H(0), data(nullptr) {}
	Image(size_t w,size_t h) :	W(w), H(h), data(new Pixel[W*H]) {}
	~Image() { if(data) delete [] data; }
	size_t getW()	{ return W; }
	size_t getH()	{ return H; }
	void setPix(size_t x, size_t y, Pixel& pix){ data[y*W + x] = pix; }
	Pixel& getPix(size_t x, size_t y){ return data[y*W + x];}
	

	
	void Write(string filename){
		fstream os(filename.c_str(),ios::out);
		os 	<< "P3\n" << W << " " << H << "\n" << "255\n";
		
		for(int y = 0; y < H; y++ ){
			for(int x = 0; x < H; x++){
				Pixel pix = getPix(x,y);
				os << (int)pix.getR() << " " << (int)pix.getG() << " " << (int)pix.getB() << " ";
			}
			os << "\n";
		}
			
		if(os.is_open()){
			os.close();
		}
	}
	
};

class MandelBrot{
	int maxIter;
	double xmin,xmax,ymin,ymax;
	uint64_t iterTotal = 0LL;
	public:
	MandelBrot(int _maxIter,double _xmin, double _xmax, double _ymin, double _ymax):
	maxIter(_maxIter),
	xmin(_xmin), xmax(_xmax),
	ymin(_ymin), ymax(_ymax)
	{}
	
	void doWork(Image& image, size_t start,size_t end){
		size_t const ixsize = image.getW();
		size_t const iysize = image.getH();
		//uit64_t iterCount = 
		for(size_t ix = start; ix < end; ++ix){
			for(size_t iy = 0; iy < iysize; ++iy){
				complex<double> c(xmin + ix/(ixsize - 1.0 )*(xmax - xmin),ymin + iy/(iysize - 1.0 )*(ymax - ymin));
				complex<double> z = 0;
				unsigned iterations;
				
				for(iterations = 0 ; iterations < maxIter && norm(z) < 4.0; ++iterations){
					z = z*z + c;			
				}
				iterTotal += iterations;
				
				uint32_t color = double(iterations) / double(maxIter) * (1<<24);
				
				uint8_t r = color			& 0xff;
				uint8_t g = (color >> 8) 	& 0xff;
				uint8_t b = (color >> 16)	& 0xff;	
				
				Pixel pixcolor(r, g, b);
				image.setPix(ix, iy, pixcolor);
				
			}
		}
		//cout << "Total iterations " << iterTotal << "\n";
	}
	void doWorkTimed(Image& image, size_t start, size_t end){
		Timer t;
		uint64_t usecs;
		t.Start();
		
		doWork(image,start,end);
		
		t.Stop();
		usecs = t.USecs();
		stringstream ss;
		ss 	<< "doWork: start,end=" << start << "/" << end
			<< ",calc time " << usecs << " usecs, " << usecs *1e-6 << " secs."
			<< "\n";
		cout << ss.str();
		
		
	}
	
	void Run(Image& image){
		size_t const H = image.getH();
		doWork(image,0,H);
	}
	
	void RunThreaded(Image& image){
		const int NUM_THREADS = std::thread::hardware_concurrency();
		std::cout<< "Running with "<< NUM_THREADS << " threads\n";
		
		std::vector<std::thread> t(NUM_THREADS);
		
		size_t const H = image.getH();
		size_t start = 0;
		size_t end = H;
		
		size_t chunk = (end-start +(NUM_THREADS-1)) / NUM_THREADS;
		
		for(int i=0;i<NUM_THREADS;i++) {
			size_t s = start + i * chunk;
			size_t e = s + chunk;
			if(i == NUM_THREADS - 1) e = end;
			
			#if 0
				t[i] = thread (&MandelBrot::doWorkTimed,this,ref(image),s,e);
			#else	
				auto b = std::bind(&MandelBrot::doWorkTimed,this,ref(image),s,e);
				t[i]= std::thread (b);
			#endif
		}
		for(auto& e : t)	e.join();
	}
	
	void RunThreadPool(Image& image){
		size_t const H = image.getH();
        int threads = std::thread::hardware_concurrency();
		ThreadPool tp(threads);
        
		for(int j = 0; j < H; j++){
			auto job = std::bind(&MandelBrot::doWork,this,ref(image),j,j+1);
			tp.AddJob(job);
		}
	}
};



int main(int argc, char** argv){
	Timer t;
	uint64_t usecs;
	string filename("mandelbrot.ppm");
	size_t W = 1080;
	size_t H = 1080;
	int maxIter = 200;
	
	double xmin = -2;
	double xmax = 1;
	double ymin = -2;
	double ymax = 2;
	
	if(argc ==9){
		filename = argv[1];
		W = atoi(argv[2]);
		H = atoi(argv[3]);
		maxIter = atoi(argv[4]);

		xmin = atof(argv[5]);
		xmax = atof(argv[6]);
		ymin = atof(argv[7]);
		ymax = atof(argv[8]);
	}
	
	Image img(W,H);
	MandelBrot m(maxIter,xmin,xmax,ymin,ymax);
	t.Start();
	m.Run(img);
	t.Stop();
	usecs = t.USecs();
	std::cout << "Run: " << usecs << " usecs." << usecs * 1e-6 << " secs.\n";
	img.Write(filename+"1.ppm");
	
	t.Start();
	m.RunThreaded(img);
	t.Stop();
	usecs = t.USecs();
	img.Write(filename+"2.ppm");
	std::cout << "RunThreaded: " << usecs << " usecs." << usecs * 1e-6 << " secs.\n";
	
	t.Start();
	m.RunThreadPool(img);
	t.Stop();
	usecs = t.USecs();
	img.Write(filename+"3.ppm");
	std::cout << "RunThreadPool: " << usecs << " usecs." << usecs * 1e-6 << " secs.\n";
}