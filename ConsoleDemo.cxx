// SoftKinetic DepthSense SDK
//
// COPYRIGHT AND CONFIDENTIALITY NOTICE - SOFTKINETIC CONFIDENTIAL
// INFORMATION
//
// All rights reserved to SOFTKINETIC SENSORS NV (a
// company incorporated and existing under the laws of Belgium, with
// its principal place of business at Boulevard de la Plainelaan 15,
// 1050 Brussels (Belgium), registered with the Crossroads bank for
// enterprises under company number 0811 341 454 - "Softkinetic
// Sensors").
//
// The source code of the SoftKinetic DepthSense Camera Drivers is
// proprietary and confidential information of Softkinetic Sensors NV.
//
// For any question about terms and conditions, please contact:
// info@softkinetic.com Copyright (c) 2002-2013 Softkinetic Sensors NV

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>

#include <DepthSense.hxx>
// C:\Program Files(x86)\SoftKinetic\DepthSenseSDK\include\

using namespace std;
using namespace DepthSense;

Context g_context;
uint32_t g_cFrames = 0;
uint32_t g_dFrames = 0;
const int frame_NUM = 1;

const int height = 240;
const int width = 320;

const int color_height = 240*2;
const int color_width = 320*2;

int color_data[color_height*color_width * 3][frame_NUM];
float depth_data[height*width][frame_NUM];

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
	cout << "New color sample received (timeOfCapture=" << data.timeOfCapture << ")" << endl;
	for (int i = 0; i < color_height*color_width * 2; i++){
		//cout << data.depthMap[1] << endl;
		color_data[i][g_dFrames] = data.colorMap[i];
	}
	//cout << data.captureConfiguration.compression << endl;
   //do not compress!	So shoul be YUY2 data cout 
	//<< data.compressedData[0] << endl;
	g_cFrames++;
	// Quit the main loop after 200 depth frames received
	if (g_cFrames >= frame_NUM)
    	g_context.quit();

}


static void onNewDepthSample(DepthNode obj, DepthNode::NewSampleReceivedData data)
{
	cout << "New depth sample received (timeOfCapture=" << data.timeOfCapture << ")" << endl; 
	for (int i = 0; i < height*width; i++){
		//cout << data.depthMap[1] << endl;
		depth_data[i][g_dFrames] = data.depthMap[i];
	}
	g_dFrames++;
	// Quit the main loop after 200 depth frames received
	//if (g_dFrames >= frame_NUM)
		//g_context.quit();	
}

static void writeDepthTxt(){
	ofstream fl("depths.txt");
	if (!fl)
	{		cout << "file could not be open for writing ! " << endl;
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

	// start streaming
	g_context.startNodes();

	// start the DepthSense main event loop
	g_context.run();
	g_context.stopNodes();
	//writeDepthTxt();
	writeColorTxt();
	return 0;
	}