
#include <stdlib.h>
#include <iostream>

#include <DepthSense.hxx>
// C:\Program Files(x86)\SoftKinetic\DepthSenseSDK\include\

using namespace std;
using namespace DepthSense;

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
	//cout << data.colorMap[13232];


	//yuy2rgb((unsigned char *)a, data.colorMap, 320, 270);

	const int height = 240;
	const int width = 320;
	int x, y;
	const int width2 = width * 2;
	const int width4 = width * 3;
	int answer[height*width * 3 / 2];

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x += 2) {
			int x2 = x * 2;
			int y1 = data.colorMap[x2];
			int y2 = data.colorMap[x2 + 2];
			int u = data.colorMap[x2 + 1] - 128;
			int v = data.colorMap[x2 + 3] - 128;
			int uvr = (15748 * v) / 10000;
			int uvg = (-1873 * u - 4681 * v) / 10000;
			int uvb = (18556 * u) / 10000;

			int x4 = x * 3;
			int r1 = y1 + uvr;
			int r2 = y2 + uvr;
			int g1 = y1 + uvg;
			int g2 = y2 + uvg;
			int b1 = y1 + uvb;
			int b2 = y2 + uvb;

			answer[x4 + 0] = (b1 > 255) ? 255 : ((b1 < 0) ? 0 : b1);
			answer[x4 + 1] = (g1 > 255) ? 255 : ((g1 < 0) ? 0 : g1);
			answer[x4 + 2] = (r1 > 255) ? 255 : ((r1 < 0) ? 0 : r1);
			//dst1[x4+3] = 255;

			answer[x4 + 3] = (b2 > 255) ? 255 : ((b2 < 0) ? 0 : b2);
			answer[x4 + 4] = (g2 > 255) ? 255 : ((g2 < 0) ? 0 : g2);
			answer[x4 + 5] = (r2 > 255) ? 255 : ((r2 < 0) ? 0 : r2);
			cout << answer[x4 + 3] << " " << answer[x4 + 4] << " " << answer[x4 + 5];
		}
		//data.colorMap += width2;
		//dst1 += width4;
	}



}


static void onNewDepthSample(DepthNode obj, DepthNode::NewSampleReceivedData data)
{
	cout << "New color sample received (timeOfCapture=" << data.timeOfCapture << ")" << endl;
	//int32_t w, h;
	const int height = 240;
	const int width = 320;

	float answer[height][width]; //the depth data 
	int count = 0;
	for (int i = 0; i < height; i++){
		for (int j = 0; j < width; j++) {
			if (count < 76800){
				//cout << data.depthMap[1] << endl;
				answer[i][j] = data.depthMap[count];
				cout << answer[i][j] << " ";
				count += 1;
			}
		}
	}
	//cout << data.depthMapFloatingPoint << endl;
	//std::istream::readsome();

}

int main(int argc, char** argv)
{
	// create a connection to the DepthSense server at localhost
	Context context = Context::create();


	DepthNode depthNode = getFirstAvailableDepthNode(context);
	if (!depthNode.isSet())
		depthNode.setEnableDepthMap(true);
	depthNode.newSampleReceivedEvent().connect(onNewDepthSample);
	context.registerNode(depthNode);


	// get the first available color sensor
	ColorNode colorNode = getFirstAvailableColorNode(context);

	// if no color node was found, fail
	if (!colorNode.isSet())
		error("no color node found");

	// enable the capture of the color map
	colorNode.setEnableColorMap(true);

	//DepthSense::FrameFormat  frameFormat = colorNode.getConfiguration.frameFormat;
	//int32_t framerate = colorNode.getConfiguration.framerate;
	//DepthSense::PowerLineFrequency  frequency = colorNode.getConfiguration.powerLineFrequency;
	//DepthSense::ColorNode::Configuration config = DepthSense::ColorNode::Configuration(frameFormat, framerate, frequency, COMPRESSION_TYPE_MJPEG);
	//colorNode.setConfiguration(config);

	// connect a callback to the newSampleReceived event of the color node
	colorNode.newSampleReceivedEvent().connect(onNewColorSample);

	// add the color node to the list of nodes that will be streamed
	context.registerNode(colorNode);
	// start streaming
	context.startNodes();

	// start the DepthSense main event loop
	context.run();
}