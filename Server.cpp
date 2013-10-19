#define BOOST_ALL_NO_LIB

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

using namespace std;
using namespace boost::interprocess;

struct sharedImage
{
	enum { width = 320 };
	enum { height = 240 };
	enum { dataLength = width*height*sizeof(unsigned short) };

	sharedImage(){}
	interprocess_mutex mutex;
	unsigned short  data[dataLength];
};

shared_memory_object shm;
sharedImage * sIm;
mapped_region region;

int setupSharedMemory(){
	// Clear the object if it exists
	shared_memory_object::remove("ImageMem");

	shm = shared_memory_object(create_only  /*only create*/, "ImageMem" /*name*/, read_write/*read-write mode*/);

	printf("Size:%i\n", sizeof(sharedImage));
	//Set size
	shm.truncate(sizeof(sharedImage));

	//Map the whole shared memory in this process
	region = mapped_region(shm, read_write);

	//Get the address of the mapped region
	void * addr = region.get_address();

	//Construct the shared structure in the preallocated memory of shm
	sIm = new (addr)sharedImage;

	return 0;
}


int shutdownSharedMemory(){
	shared_memory_object::remove("ImageMem");
	return 0;
}