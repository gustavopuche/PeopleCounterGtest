// ---------------------------------------------------------------------------
// @file    PeopleCounter.cpp
//
// @brieg   Detect people and count them using a camera or video
//
// @date    May 20, 2020
//
// @author  Gustavo Puche
// ---------------------------------------------------------------------------
#include "PeopleCounter.h"
#include <stdio.h>
#include <iostream>


using namespace std;
using namespace cv;

// Distance between 2 given points
double distance(int x0, int y0, int x1, int y1)
{
  return sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
}

//  Bubble sort Descending order
void bubbleSort(vector<Rect>& v)
{
  bool flag = true; // Flag to stop when vector is sorted

  // bubble sort on vector A by area
  const size_t size = v.size();
  for (size_t i = 0; (i < size) && flag; ++i)
	{
		flag = false; // Stop sorting if there is no changes

		for (size_t j = 0; j < size - i - 1; j++)
		{
			if (v[j + 1].area() > v[j].area()) // ascending order simply changes to <
			{
				Rect tmp;
				tmp = v[j];
				v[j] = v[j + 1];
				v[j + 1] = tmp;

				flag = true; // Continue sorting
			}
		}
	}
}


// Remove elements than are small than a given area
void removeSmall(vector<Rect> vIn, vector<Rect> vOut, int area)
{
	const size_t size = vIn.size();
	for (size_t i = 0; i < size; i++)
	{
		if (vIn[i].area() > area)
		{
			vOut.push_back(vIn[i]);
		}
	}
}



// Add new object
void CounterUpDown::add(Rect box,int x,int y)
{
	Detection tmp;
	
	tmp.bound = box;
	tmp.x =  x;
	tmp.y = y;
	tmp.walkState = 0;
	tmp.fps = 0;
	tmp.visited = true;
	tmp.direction = 0;  //  No direction. Error
		
	// If starts down, walks up direction
	if ((y > yLower))
	{
		tmp.direction = 1;  // Up direction
		tmp.walkState = 1;  // Down Screen
	}
	
	// If starts up, walks down direction
	if (y < yUpper)
	{
		tmp.direction = -1; // Down direction
		tmp.walkState = 3;  // UP screen
	}
	
	
	tmp.added = false;
	tmp.distance = -1;
	
	objects.push_back(tmp);
}

// Update existing object
void CounterUpDown::update(Rect box,int x,int y,int idx)
{
	// If not counted yet
	if (!objects[idx].added)
	{
		if (objects[idx].direction == 1)
		{// Walks up direction
			switch (objects[idx].walkState) {
			case 1: //  1st part
				if ((y < yLower)&&(y > yUpper))
				{// Reach middle
					objects[idx].walkState = 2;
				}
				break;
			case 2: //  Middle part
				if ((y < yUpper)||(box.y < yUpper))
				{// Reach Upper Line. Reach 3rd  part.
					objects[idx].walkState = 3;
					upCounter++;
					objects[idx].added=true;
				}
				break;
			case 3: //  3rd part
				break;
			}
		}
		else if (objects[idx].direction == -1)
		{// Walks down direction
			switch (objects[idx].walkState) {
			case 1: //  1st part
				break;
			case 2: //  Middle part
				if ((y > yLower)||(box.br().y > yLower))
				{
					objects[idx].walkState = 1;
					downCounter++;
					objects[idx].added=true;
				}
				break;
			case 3: //  3rd part
				if ((y < yLower)&&(y > yUpper))
				{// Reach middle
					objects[idx].walkState = 2;
				}
				break;
			}
		}

		objects[idx].fps      = 0;  //  Init fps

	}
	else
	{
		cout<<"(NO) Update Y: "<<y<<endl;
	}

	if (!objects[idx].added)
	{
		//  Debug
		cout <<"(Yes) Update Y(old): "<<objects[idx].y<<" with Y(new): "<<y<<endl;

		objects[idx].bound    = box;  // Bounding box of the object
		objects[idx].x        = x;
		objects[idx].y        = y;

	}

	objects[idx].bound    = box;  // Bounding box of the object
	objects[idx].x        = x;
	objects[idx].y        = y;
	objects[idx].visited  = true; // Mark visited object
}

//  Cleans all expired objects
void CounterUpDown::clean()
{
	vector<Detection> tmp;

	size_t size = objects.size();

	for (size_t i = 0; i < size; i++)
	{
		Detection obj = objects[i];

		bool visited = obj.visited;
		bool added  = obj.added;
		int fps = obj.fps;

		if (fps == maxFPS)
		{// If reach max FPS without visit
			cout<<"(-) Delete object: "<<i<<" X: "<<obj.x
					<<" Y: "<<obj.y<<" direction: "<<obj.direction
					<<" Walk State: "<<obj.walkState<<" fps: "<<obj.fps
					<<" Visited: "<<obj.visited
					<<" Added: "<<obj.added<<endl;
		}
		else
		{// If fps less than maxFPS
			if ((!visited||added))
			{// If not visited or added
				//  Increment fps;
				obj.fps++;
			}

			if (visited)
			{// Clears visited flag
				obj.visited = false;
			}

			tmp.push_back(obj);
		}

	}// End for

	// Keep only objects alive
	objects = tmp;
}

// Find closer object in new objects. Return the object index closest to the given object
int	CounterUpDown::find(int prev_idx,vector<Rect> news,vector<bool> updated)
{
	vector<double> distances;

	int x0 = objects[prev_idx].x;
	int y0 = objects[prev_idx].y;

	objects[prev_idx].distance = -1; // Init distance

	size_t size = news.size();

	if (size == 0)
	{
		return -1;  //  There are no object
	}

	for (size_t i = 0; i < size; i++)
	{// Distances between previous object and news objects
		double dist;
		int x1  = news[i].x +(news[i].width/2);   //  Center x
		int y1  = news[i].y + (news[i].height/2); //  Center y

		dist = distance(x0, y0, x1, y1);  // Store last distances

		distances.push_back(dist);
	}

	int min = -1;
	double minDist = distances[0];

	for (size_t i = 0; i < size; i++)
	{
		int x1  = news[i].x +(news[i].width/2);   //  Center x
		int y1  = news[i].y + (news[i].height/2); //  Center y
		Detection prev_obj = objects[prev_idx];

		if ((prev_obj.added)&&(prev_obj.direction == 1))
		{// Added and walking up
			if (y1 > yLower)
			{// In 3rd section
				cout<<"find:: Skip upWalking Objects["<<i<<"] X0: "<<x0<<" Y0: "<<y0<<" X1: "<<x1<<" Y1: "<<y1
						<<" distance: "<<distances[i]<<endl;
				continue;
			}
		}

		if ((prev_obj.added)&&(prev_obj.direction == -1))
		{// Added and walking down
			if (y1 < yUpper)
			{// In 1st section
				cout<<"find:: Skip downWalking Objects["<<i<<"] X0: "<<x0<<" Y0: "<<y0<<" X1: "<<x1<<" Y1: "<<y1
						<<" distance: "<<distances[i]<<endl;
				continue;
			}
		}

		if (/*(distances[i]<(yLower-yUpper))&&*/(distances[i] <= minDist)&&(!updated[i]))
		{// If the object is allreade finded. Search another

			// Debug
			cout<<"find:: Objects["<<i<<"] X0: "<<x0<<" Y0: "<<y0<<" X1: "<<x1<<" Y1: "<<y1
					<<" distance: "<<distances[i]<<endl;

			min = i;
			minDist = distances[i];
			objects[prev_idx].distance = distances[i]; // Store distances
		}
	}

	//  Debug
	cout<<"find:: X: "<<x0<<" Y: "<<y0<<" return: "<<min<<endl;

	return min;  // Return the closest point
}

//  New version of compute.
void CounterUpDown::compute(vector<Rect> new_objects)
{
	size_t size_news = new_objects.size();
	size_t size_prev = objects.size();

	vector<bool> updated;

	for (size_t i = 0; i < size_news; i++)
	{// Init all to not updated
		updated.push_back(false);
	}

	for (size_t i=0; i<size_prev; i++)
	{// For each previous objects
		int news_idx;
		news_idx = find(i,new_objects,updated);

		if (news_idx > -1)
		{
			Rect box = new_objects[news_idx];

			//  Obtain the center of the object
			int x = box.x+(box.width/2);
			int y = box.y+(box.height/2);

			updated[news_idx] = true;
			update(box,x,y,i);
		}
	}

	for (size_t i = 0;i < size_news; i++)
	{// Add all objects not updated
		Rect box = new_objects[i];

		//  Obtain the center of the object
		int x = box.x+(box.width/2);
		int y = box.y+(box.height/2);

		if(!updated[i])
		{// If not updated add new objects to list
			if ((y > yLower)||(y < yUpper))
			{// 1st part of screen or 3rd part to add a new object
				cout<<"compute: add object  +++++++++++  Y: "<<y<<endl;
				add(box,x,y);
			}
		}
	}
}

//  Print debub mesages
void CounterUpDown::debug()
{
	size_t size = objects.size();

	cout<<endl<<"Total objects: "<<size<<endl;

	for (size_t i=0;i<size;i++)
	{
		Detection obj = objects[i];

		cout << "Object: "<<i<<" X: "<<obj.x<<" Y: "<<obj.y
				 <<" Direction: "<<obj.direction<<" Walk State: "<<obj.walkState
				 <<" fps: "<<obj.fps<<" Distance: "<<obj.distance
				 <<" Visited: "<<obj.visited<<" Added: "<<obj.added<<endl;
	}

	clog<<"Up Counter: "<<upCounter<<" Down Counter: "<<downCounter<<endl<<endl;
}

void usage(std::string name) {
  std::cerr << "Usage: " << name << " [-h][-t type][-i input_file][-o output_file]"
            << "Options:\n"
            << "\t-h, --help"<<endl<<"\t\tShow this help message"<<endl
            << "\t-t=TYPE"<<endl
            << "\t\thog People detector"<<endl
            << "\t\tmog2 Background subtractor MOG2"<<endl
            << "\t-i=FILENAME"<<endl
            << "\t\tInput video filename"<<endl
            << "\t-o=FILENAME"<<endl
            << "\t\tOupt video filename"<<endl
            << std::endl;
  exit(1);
}

// People Counter Main process.
int peoplecount(int argc, char *argv[])
{
  VideoCapture cap;

  std::string input_file;
  std::string output_file("out.avi");
  bool useMOG2    = true; //  Default value is background subtracto MOG2
  bool useCamera  = false;

  // cv::HOGDescriptor hog(cv::Size(48, 96),
  //                       cv::Size(16, 16),
  //                       cv::Size(8, 8),
  //                       cv::Size(8, 8), 9, 1,-1,
  //                       cv::HOGDescriptor::L2Hys, 0.2, true,
  //                       cv::HOGDescriptor::DEFAULT_NLEVELS);

	cv::HOGDescriptor hog(cv::Size(48, 96),
                        cv::Size(16, 16),
                        cv::Size(8, 8),
                        cv::Size(8, 8), 9, 1,-1,
                        cv::HOGDescriptor::L2Hys, 0.2, true,
                        cv::HOGDescriptor::DEFAULT_NLEVELS);

  hog.setSVMDetector(cv::HOGDescriptor::getDaimlerPeopleDetector());

  for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if ((arg == "-h") || (arg == "--help"))
		{
			usage(argv[0]);
			return 0;
		}
		else if ((arg == "-t"))
		{
			if (i + 1 < argc)
			{ // Make sure we aren't at the end of argv!
				std::string type = argv[i+1];

				cout << "type: "<<type<<endl;
				if (type == "hog")
				{
					useMOG2 = false;
				}
				else if (type == "mog2")
				{
					useMOG2 = true;
				}
			}
			else
			{ // Uh-oh, there was no argument to the destination option.
				std::cerr << "-t option requires one argument." << std::endl;
				usage(argv[0]);
				return 1;
			}
		}
		else if (arg == "-i")
		{
			if (i + 1 < argc)
			{ // Make sure we aren't at the end of argv!
				input_file = argv[i+1];
			}
			else
			{
				std::cerr << "-i option requires one argument." << std::endl;
				usage(argv[0]);
				return 1;
			}
		}
		else if (arg == "-o")
		{
			if (i + 1 < argc)
			{ // Make sure we aren't at the end of argv!
				input_file = argv[i+1];
			}
			else
			{
				std::cerr << "-o option requires one argument." << std::endl;
				usage(argv[0]);
				return 1;
			}
		}

	}

  if (!input_file.empty())
	{
		cout<<"Open file: "<<input_file<<endl;
		cap.open(input_file);
	}
  else
	{
		cout<<"Using camera....."<<endl;

		useCamera = true;

		if(!cap.open(CAP_ANY))
		{
			usage(argv[0]);
			return -1;
		}
	}

  if (useMOG2)
	{
		cout<<"Using Background Subtractor MOG2....."<<endl;
	}
  else
	{
		cout<<"Using People Detect HOG..."<<endl;
	}

  if( !cap.isOpened() )
	{
		cout<<"can not open camera or video file"<<endl;
		return -1;
	}

  cv::Mat frame;
  cv::Mat back;
  cv::Mat fore;


  Ptr<BackgroundSubtractorMOG2> bg = createBackgroundSubtractorMOG2();

  //  Debug
  cout << "(I) History: " << bg->getHistory() << endl;
  cout << "(I) NMixtures: " << bg->getNMixtures() << endl;
  cout << "(I) Background Ratio: " << bg->getBackgroundRatio() << endl;
  cout << "(I) Variance scale factor: " << bg->getVarThresholdGen() << endl;
  cout << "(I) Variance of each gaussian component: " << bg->getVarInit() << endl;
  cout << "(I) Complexity reduction threshold: " << bg->getComplexityReductionThreshold() << endl;
  cout << "(I) Is shadow Enabled? " << bg->getDetectShadows() << endl;
  cout << "(I) Shadow Value: " << bg->getShadowValue() << endl;
  cout << "(I) Shadow Threshold: " << bg->getShadowThreshold() << endl << endl;

  //    Set MOG2 parameters
  bg->setHistory(50);                         // Nº last frames affect background (500)
  bg->setNMixtures(5);                        // Nº gaussian components (5)
  bg->setBackgroundRatio(0.9);                // Background ration (0.9)
  bg->setVarThresholdGen(9);                  // Variance scale factor (9)
  bg->setVarInit(15);                         // Initial variance (15)
  bg->setComplexityReductionThreshold(0.05);  // Nº samples needed for accept exists component exists (0.05)
  bg->setDetectShadows(true);                 // Detect or no detect shadows
  bg->setShadowValue(255);                    // Value to mark shadows 0-255 (127)
  bg->setShadowThreshold(0.5);                // Value determine is is shadow or not

  //  Debug
  cout << "(S) History: " << bg->getHistory() << endl;
  cout << "(S) NMixtures: " << bg->getNMixtures() << endl;
  cout << "(S) Background Ratio: " << bg->getBackgroundRatio() << endl;
  cout << "(S) Variance scale factor: " << bg->getVarThresholdGen() << endl;
  cout << "(S) Variance of each gaussian component: " << bg->getVarInit() << endl;
  cout << "(S) Complexity reduction threshold: " << bg->getComplexityReductionThreshold() << endl;
  cout << "(S) Is shadow Enabled? " << bg->getDetectShadows() << endl;
  cout << "(S) Shadow Value: " << bg->getShadowValue() << endl;
  cout << "(S) Shadow Threshold: " << bg->getShadowThreshold() << endl << endl;

  // Write video output
  cv::VideoWriter output;

  if (!useCamera)
	{
		int ex = static_cast<int>(cap.get(CAP_PROP_FOURCC));
		//  int ex = FOURCC('P', 'I', 'M', '1');
		cv::Size size = cv::Size((int) cap.get(CAP_PROP_FRAME_WIDTH),
														 (int) cap.get(CAP_PROP_FRAME_HEIGHT));

		if(!output.open(output_file, ex, cap.get(CAP_PROP_FPS), size, true))
		{
			cerr<< "Failed to open the output video: "<<output_file<< endl;
			exit(6);
		}
		else
		{
			cerr << "saving to " << output_file << std::endl;
		}

	}

  vector<vector<Point> > contours;

  cv::namedWindow("Frame", WINDOW_NORMAL);

  if (useMOG2)
	{
		cv::namedWindow("Background", WINDOW_NORMAL);
	}

  //	Debub
  cout << "Image Resolution: " << cap.get(CAP_PROP_FRAME_WIDTH) << "x" << cap.get(
		CAP_PROP_FRAME_HEIGHT) << endl;

  // Varialbles to draw lines
  int lowerX0 = 0;
  int lowerY0 = 2 * cap.get(CAP_PROP_FRAME_HEIGHT) / 3;
  int lowerX1 = cap.get(CAP_PROP_FRAME_WIDTH);
  int lowerY1 = lowerY0;

  int upperX0 = 0;
  int upperY0 = cap.get(CAP_PROP_FRAME_HEIGHT) / 3;
  int upperX1 = cap.get(CAP_PROP_FRAME_WIDTH);
  int upperY1 = upperY0;

  //	Counter variables
  int upCont = 0;
  int downCont = 0;
  int yLower = lowerY0;
  int yUpper = upperY0;

	//  int minObjectArea = 800; // Min value to detect object. Rest is noise
  double cArea = 350; // Min contours area
  bool reachLower = false;
  bool reachUpper = false;


  // Initialize counter
  CounterUpDown counters;
  counters.setFPS(cap.get(CAP_PROP_FPS)/3);
  counters.setLower(yLower);
  counters.setUpper(yUpper);


  //  For each frame
  for (;cap.read(frame);)
	{
		//    cap >> frame;

		// Array of bounding box objects detected
		vector<Rect> vObjects;

		if (useMOG2)
		{// Use background subtractor
			bg->apply(frame, fore);

			bg->getBackgroundImage(back);

			cv::erode(fore, fore, cv::Mat());
			cv::dilate(fore, fore, cv::Mat());

			cv::findContours(fore, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
			cv::drawContours(frame, contours, -1, cv::Scalar(0, 0, 255), 2);

			//  Filter contours by area
			for (int i = 0; i < contours.size(); i++)
			{
				if (contourArea(contours[i]) > cArea)
				{
					vObjects.push_back(boundingRect(contours[i]));
				}
			}
		}
		else
		{// Use People Detect
			hog.detectMultiScale(frame, vObjects,0.2);
		}

		// Sort in descending order
		bubbleSort(vObjects);

		// Execute detected objects
		counters.compute(vObjects);


		// Iterate for all detected objects
		for (size_t i = 0; i < vObjects.size(); i++)
		{
			cv::Rect box = vObjects[i];

			Point center;
			center.x = box.x + (box.width/2);
			center.y = box.y + (box.height/2);

			double radius = box.width/2;
			if (box.height < box.width)
			{
				radius = box.height/2;
			}

			Point pt1, pt2;
			pt1.x = box.x;
			pt1.y = box.y;
			pt2.x = box.br().x;
			pt2.y = box.br().y;

			cout<<"O("<<i<<"): X: "<<center.x<<" Y:"<<center.y<<endl;

			//	Paint Rectangle
			cv::rectangle(frame, box, CV_RGB(0,255,0), 2);

			circle(frame, center, (int) radius, CV_RGB(0,0,255),2, 8, 0);

			// Debug
			std::ostringstream ossYpoint;
			ossYpoint << " Y: " << center.y;

			//	Where is rect.x and rect.y
			cv::putText(frame, ossYpoint.str(), cv::Point(pt1.x, pt1.y), 3, 0.5,
									CV_RGB(255,0,255));

			// Debug
			std::ostringstream ossXpoint;
			ossXpoint << " X: " << center.x;

			//  Where is rect.x and rect.y
			cv::putText(frame, ossXpoint.str(), cv::Point(pt1.x, pt2.y), 3, 0.5,
									CV_RGB(255,0,255));

			//	First version of count
			if (!reachLower && (pt1.y < yLower) && (pt1.y > yUpper))
			{ // Reach lower line first time but not upper one
				reachLower = true;

			}
			else if ((pt1.y < yUpper) && (reachLower))
			{ // Reach read line after reach lower one by the first time
				reachLower = false;
				upCont++;
			}
		}// End for all detected object in frame

		// Debug
		if (vObjects.size()>0)
		{
			//Debug
			counters.debug();
		}

		// Delete expired objects
		counters.clean();

		// Draw a lower line from the previous point to the current point
		line(frame, cv::Point(lowerX0, lowerY0), cv::Point(lowerX1, lowerY1),
				 CV_RGB(255,255,0), 2);
		line(frame, cv::Point(upperX0, upperY0), cv::Point(upperX1, upperY1),
				 CV_RGB(255,0,0), 2);

		std::ostringstream ossCount;

		//	Concat Text with count
		ossCount << "Up: " << upCont;

		ostringstream ossCompute;

		ossCompute << "up="<<counters.upCounter<<" down="<<counters.downCounter;
		cv::putText(frame, ossCompute.str(), cv::Point(0,50), 1, 1,CV_RGB(0,255,255));

		if (!useCamera)
		{
			output << frame;
		}

		cv::imshow("Frame", frame);

		if (useMOG2)
		{
			cv::imshow("Background", back);
		}

		if (cv::waitKey(30) >= 0)
		{
			break;
		}

	}// End for each frame

  return 0;
}
