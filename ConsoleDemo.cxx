#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>

#include <DepthSense.hxx>
// C:\Program Files(x86)\SoftKinetic\DepthSenseSDK\include\

using namespace std;
using namespace DepthSense;

//added
#define BOOST_ALL_NO_LIB

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

using namespace std;
using namespace boost::interprocess;
//added end

Context g_context;
uint32_t g_cFrames = 0;
uint32_t g_dFrames = 0;
const int frame_NUM = 1000;
const int height = 240;
const int width = 320;
const int color_height = 240 * 2;
const int color_width = 320 * 2;
//int color_data[color_height*color_width * 3][frame_NUM];
//float depth_data[height*width][frame_NUM];


//edded
struct sharedImage
{
	enum { width = 320 };
	enum { height = 240 };
	enum { dataLength = width*height*sizeof(unsigned short) };//sizeof(unsigned short)=4

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

	printf("Size of the shared image:%i\n", sizeof(sharedImage));
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

static void error(const char* message)
{
	cerr << message << endl;
	exit(1);
}

static ColorNode getFirstAvailableColorNode(Context context){
	// obtain the list of devices attached to the host
	vector<Device> devices = context.getDevices();

	for (vector<Device>::const_iterator iter = devices.begin(); iter != devices.end(); iter++)
	{
		Device device = *iter;
		// obtain the list of nodes of the current device
		vector<Node> nodes = device.getNodes();
		for (vector<Node>::const_iterator nodeIter = nodes.begin(); nodeIter != nodes.end(); nodeIter++)
		{
			Node node = *nodeIter;
			// if the node is a DepthSense::ColorNode, return it
			ColorNode colorNode = node.as<ColorNode>();
			if (colorNode.isSet())
				return colorNode;
		}
	}
	// return an unset color node
	return ColorNode();
}


static DepthNode getFirstAvailableDepthNode(Context context)
{
	// obtain the list of devices attached to the host
	vector<Device> devices = context.getDevices();
	for (vector<Device>::const_iterator iter = devices.begin(); iter != devices.end(); iter++)
	{
		Device device = *iter;
		// obtain the list of nodes of the current device
		vector<Node> nodes = device.getNodes();
		for (vector<Node>::const_iterator nodeIter = nodes.begin(); nodeIter != nodes.end(); nodeIter++)
		{
			Node node = *nodeIter;
			// if the node is a DepthSense::ColorNode, return it
			DepthNode colorNode = node.as<DepthNode>();
			if (colorNode.isSet())
				return colorNode;
		}
	}

	// return an unset color node
	return DepthNode();
}


static void onNewColorSample(ColorNode obj, ColorNode::NewSampleReceivedData data)
{
	//cout << "New color sample received (timeOfCapture=" << data.timeOfCapture << ")" << endl;
	//for (int i = 0; i < color_height*color_width * 2; i++){
		//color_data[i][g_dFrames] = data.colorMap[i];
	//}
	g_cFrames++;
	// Quit the main loop after 200 depth frames received
	//if (g_cFrames >= frame_NUM)
		//g_context.quit();
}


static void onNewDepthSample(DepthNode obj, DepthNode::NewSampleReceivedData data)
{
	//cout << "New depth sample received (timeOfCapture=" << data.timeOfCapture << ")" << endl;
	//for (int i = 0; i < height*width; i++){
	//	depth_data[i][g_dFrames] = data.depthMap[i];
	//}
	memcpy(sIm->data, data.depthMap, sIm->dataLength);
	g_dFrames++;
	// Quit the main loop after 200 depth frames received
	//if (g_dFrames >= frame_NUM)
	//g_context.quit();        
}
/*
static void writeDepthTxt(){
	ofstream fl("depths.txt");
	if (!fl)
	{
		cout << "file could not be open for writing ! " << endl;
	}
	for (int frame = 0; frame < g_dFrames; frame++){
		for (int i = 0; i < height*width; i++){
			fl << depth_data[i][frame] << endl;
		}
	}
	fl.close();
}

static void writeColorTxt(){
	ofstream fl("colors.txt");
	if (!fl)
	{
		cout << "file could not be open for writing ! " << endl;
	}
	for (int frame = 0; frame < g_cFrames; frame++){
		for (int i = 0; i < color_height*color_width * 2; i++){
			fl << color_data[i][frame] << endl;
		}
	}
	fl.close();
}
*/
int main(int argc, char** argv)
{
	// create a connection to the DepthSense server at localhost
	g_context = Context::create();

	DepthNode depthNode = getFirstAvailableDepthNode(g_context);
	if (!depthNode.isSet())
		error("no depth node found");
	depthNode.setEnableDepthMap(true);
	depthNode.newSampleReceivedEvent().connect(onNewDepthSample);
	g_context.registerNode(depthNode);

	// get the first available color sensor
	ColorNode colorNode = getFirstAvailableColorNode(g_context);
	if (!colorNode.isSet())
		error("no color node found");
	colorNode.setEnableColorMap(true);
	colorNode.newSampleReceivedEvent().connect(onNewColorSample);
	g_context.registerNode(colorNode);

	setupSharedMemory();
	// start streaming
	g_context.startNodes();
	// start the DepthSense main event loop
	g_context.run();
	g_context.stopNodes();
	shutdownSharedMemory();
	//writeDepthTxt();
	//writeColorTxt();
	return 0;
}