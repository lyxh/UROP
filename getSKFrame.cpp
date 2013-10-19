#include <math.h>
#include <windows.h>
#include "mex.h"
//ADDED
#include <DepthSense.hxx>
#define BOOST_ALL_NO_LIB
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <iostream>
#include <cstdio>
#include <cstdlib>

using namespace boost::interprocess;

struct sharedImage
{
	enum { width = 320 };
	enum { height = 240 };
	enum { dataLength = width*height*sizeof(short) };

	sharedImage() : dirty(true){}
	interprocess_mutex mutex;
	uint8_t  data[dataLength];
	bool  dirty;
};

void getFrame(unsigned short *D)
{
	//Open the shared memory object.
	shared_memory_object shm(open_only, "ImageMem", read_write);

	//Map the whole shared memory in this process
	mapped_region region(shm, read_write);

	//Get the address of the mapped region
	void * addr = region.get_address();

	//Construct the shared structure in memory
	sharedImage * sIm = static_cast<sharedImage*>(addr);

	//scoped_lock<interprocess_mutex> lock(sIm->mutex);
	memcpy((char*)D, (char*)sIm->data, sIm->dataLength);
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	// Build outputs
	mwSize dims[2] = { 320, 240 };
	plhs[0] = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
	unsigned short *D = (unsigned short*)mxGetData(plhs[0]);
	try
	{
		getFrame(D);
	}
	catch (interprocess_exception &ex)
	{
		mexPrintf("getFrame:%s\n", ex.what());
	}
}