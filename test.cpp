
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include "mex.h"

#include <C:\\Users\\LLP-admin\\Desktop\\DepthSenseSDK\\include\\DepthSense.hxx>

using namespace std;
using namespace DepthSense;

/* Input Arguments */
#define	T_IN	prhs[0]
#define	Y_IN	prhs[1]

/* Output Arguments */
#define	YP_OUT	plhs[0]

#if !defined(MAX)
#define	MAX(A, B)	((A) > (B) ? (A) : (B))
#endif
#if !defined(MIN)
#define	MIN(A, B)	((A) < (B) ? (A) : (B))
#endif
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
static	double	mu = 1 / 82.45;
static	double	mus = 1 - 1 / 82.45;


static void test(double	yp[],double	*t,double	y[])
{
	double	r1, r2;
	(void)t;     /* unused parameter */
	r1 = sqrt((y[0] + mu)*(y[0] + mu) + y[2] * y[2]);
	r2 = sqrt((y[0] - mus)*(y[0] - mus) + y[2] * y[2]);

	/* Print warning if dividing by zero. */
	if (r1 == 0.0 || r2 == 0.0){
		mexWarnMsgIdAndTxt("MATLAB:yprime:divideByZero",
			"Division by zero!\n");
	}

	yp[0] = y[1];
	yp[1] = 2 * y[3] + y[0] - mus*(y[0] + mu) / (r1*r1*r1) - mu*(y[0] - mus) / (r2*r2*r2);
	yp[2] = y[3];
	yp[3] = -2 * y[1] + y[2] - mus*y[2] / (r1*r1*r1) - mu*y[2] / (r2*r2*r2);
	return;
}

void mexFunction(int nlhs, mxArray *plhs[],
	int nrhs, const mxArray*prhs[])
{
	double *yp;
	double *t, *y;
	size_t m, n; 
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


	// connect a callback to the newSampleReceived event of the color node
	colorNode.newSampleReceivedEvent().connect(onNewColorSample);

	// add the color node to the list of nodes that will be streamed
	context.registerNode(colorNode);

	/* Create a matrix for the return argument */
	YP_OUT = mxCreateDoubleMatrix((mwSize)m, (mwSize)n, mxREAL);

	/* Assign pointers to the various parameters */
	yp = mxGetPr(YP_OUT);

	t = mxGetPr(T_IN);
	y = mxGetPr(Y_IN);

	/* Do the actual computations in a subroutine */
	test(yp, t, y);
	return;


	// start streaming
	//context.startNodes();

	// start the DepthSense main event loop
	//context.run();

}