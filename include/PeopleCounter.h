#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <vector>

using namespace std;
using namespace cv;

double distance(int x0, int y0, int x1, int y1);

// Detection data.
class Detection
{
public:
	Rect bound; // Bounding Box of the object

	//  Center of the object
	int x;
	int y;

	int walkState;  // 0 false
	// 1 1º part
	// 2 Middle
	// 3 3º part
	int fps;  //  Number of frames the object is active.
	bool visited;
	int direction;  //  Walk direction.
	//  -1  Down
	//   0  No direction (Error)
	//   1  Up
	bool added;
	double distance; // Las distance
};

// Present objects during filmation
class CounterUpDown
{
public:
	int maxFPS; // Max frame rate to delete phantom.
	int yLower; // Number of y coordinate of lower line
	int yUpper; // Numner of y coordinate of upper line.
	int upCounter;
	int downCounter;
	int limit; // Limit to detect new object
	
	vector<Detection> objects;
	
	CounterUpDown()
	{
		// Debug
		cout<<"CounterUpDown::CounterUpDown default constructor called"<<endl<<endl;
			
		maxFPS=15;
		upCounter=0;
		downCounter=0;
		yLower = 90;
		yUpper = 30;
		limit = 10;
	}
	
	void setLower(int y)
	{
		yLower=y;
	}
	int getLower()
	{
		return yLower;
	}

	void setUpper(int y)
	{
		yUpper=y;
	}

	int getUpper()
	{
		return yUpper;
	}
	void setFPS(int fps)
	{
		cout << "Set max FPS: " << fps<<endl;
		maxFPS = fps;
	}
	int getFPS()
	{
		return maxFPS;
	}

	// Add new object
	void add(Rect box,int x,int y);

	// Update existing object
	void update(Rect box,int x,int y,int idx);

	//  Cleans all expired objects
	void clean();

	// Find closer object in new objects. Return the object index closest to the given object
	int	find(int prev_idx,vector<Rect> news,vector<bool> updated);

	//  New version of compute.
	void compute(vector<Rect> new_objects);

	//  Print debub mesages
	void debug();
};

// People Counter main process.
int peoplecount(int argc, char *argv[]);
