#include "ros/ros.h"
#include "std_msgs/String.h"
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <sensor_msgs/LaserScan.h>
#include "se306_project1/ResidentMsg.h"
#include "se306_project1/DoctorMsg.h"
#include "se306_project1/AssistantMsg.h"

#include <sstream>
#include "math.h"
#include <cmath>
#include <stdlib.h>
#include "time_conversion.hpp"
#include "Resident.h"	
#include "boost/graph/adjacency_list.hpp"

//velocity of the robot
double linear_x;
double angular_z;

//goal pose and orientation
double goal_x;
double goal_y;
double goal_angle;
bool isSet = false;
//current pose and orientation of the robot
double px;
double py;
double cur_angle;
		int checkpoints[3][2] = {  
			{47, 43},
			{47, 20},
			{34, 20}
		};



int cc = 1; //current_checkpoint = 0;
				
std::pair<double, double> move(double goal_x, double goal_y, double cur_angle, double goal_angle, double px, double py);
double calc_goal_angle(double goal_x, double goal_y, double cur_angle, double px, double py); 
void StageOdom_callback(nav_msgs::Odometry msg); 

// Change these names (cp0, cp1, etc.) to be more descriptive - e.g. kitchen, bedroom, etc.
std::string nameArr[6] = {
"cp0","cp1", "cp2", "cp3","cp4","cp5"
};
std::vector<std::string> checkpointNames(&nameArr[0], &nameArr[0]+2);

// Build a hashmap that corresponds names with checkpoint co-ordinates.
typedef std::string CheckpointName; // Key
typedef std::pair<int, int> Checkpoint; // Value
typedef std::map<CheckpointName, Checkpoint> CheckpointMap;
CheckpointMap c;

std::pair<double, double> Resident::movePath(int path[][2], int pathLength) {
	std::pair<double, double> ret;	
	ret = std::make_pair(0, 0); //initialize pair. Used to get return.
	//When goal reached
	
	if ((px <= goal_x + 0.5) && (px >= goal_x - 0.5) && (py <= goal_y + 0.5) && (py >= goal_y - 0.5)) {
		isSet = false;
		if (cc == pathLength) { //If at last checkpoint
			linear_x = 0;
		} else {
			cc++; //Increment checkpoint index
		}
		if (cc == pathLength) {
			linear_x = 0;
			goal_x = path[cc-1][0];
			goal_y = path[cc-1][1];
		} else {
			goal_x = path[cc][0];
			goal_y = path[cc][1];
		}
		//Account for delay by subtracting delay values from current pose and orientation
		goal_angle = calc_goal_angle(goal_x, goal_y, cur_angle - M_PI/20, px - 0.1, py - 0.1); 
		//goal_angle = calc_goal_angle(goal_x, goal_y, cur_angle, px, py);

	} else { //Do this until goal is reached
		ret = move(goal_x, goal_y, cur_angle, goal_angle, px, py);	
	}
		return ret;
}


/*
	Associates checkpoint names with checkpoint co-ordinates
*/
void checkpointMap() {
	// Number of checkpoints
	int checkpointNum = checkpointNames.size();

	// Convert array to pairs
	std::vector<std::pair<int,int> > vec;
	for (int i=0; i<checkpointNum; i++) {
		std::pair<int, int> p = std::make_pair(checkpoints[i][0],checkpoints[i][1]);
		vec.push_back(p);
	}

	// Add checkpoint name and checkpoint co-ordinates to the map
	for (int i=0; i<checkpointNum; i++) {
		c.insert(std::make_pair(CheckpointName(checkpointNames[i]), Checkpoint(vec[i])));
	}
}

struct VertexProperties {
    std::string cName;
};

/*
	Creates graph of checkpoints. Having to add the boost namespace drags the code out *a lot*, but it's nice to know where
	everything comes form. Use 'using namespace boost' if it bothers you. The idea is to have the names as a graph, then
	look them up in the hashmap to get the corresponding co-ordinates.
*/
void makeGraph() {

	typedef boost::adjacency_list <boost::listS, boost::listS, boost::undirectedS, VertexProperties> Graph;
	int cNum = checkpointNames.size(); // field for the number of checkpoints might be wise in the future. Lots of duplication.
	Graph cGraph(cNum); 

	boost::property_map<Graph, std::string VertexProperties::*>::type 
    cName = get(&VertexProperties::cName, cGraph);

    // // Some way to automate this would be good with higher numbers of checkpoints.
    // boost::add_edge(boost::vertex("cp0", cGraph), boost::vertex("cp1",cGraph), cGraph);
    // boost::add_edge(boost::vertex("cp1", cGraph), boost::vertex("cp2",cGraph), cGraph);
    // // etc. (adding more edges)
}
	
void Resident::StageOdom_callback(nav_msgs::Odometry msg)
{
	/*ret = std::make_pair(0, 0); //initialize pair. Used to get return.
	*/
	//Converting from quaternion to radians
	cur_angle = acos(msg.pose.pose.orientation.w) * 2;
	if (msg.pose.pose.orientation.z > 0) {
		cur_angle = (2*M_PI)-acos(msg.pose.pose.orientation.w) * 2;
	}

	//Rounding to 3 decimal places
	cur_angle = ((int)(cur_angle * 1000 + .5) / 1000.0);
	
	//Update the current position
<<<<<<< HEAD
	px = msg.pose.pose.position.x + 34;//checkpoints[0][0];
	py = msg.pose.pose.position.y + 20; //checkpoints[0][1];
=======
	px = msg.pose.pose.position.x + checkpoints[0][0];
	py = msg.pose.pose.position.y + checkpoints[0][1];
>>>>>>> 6f4325dfcc39ce8764a381daccd2cd95122ba38d
	

}

void Resident::StageLaser_callback(sensor_msgs::LaserScan msg)
{
	//This is the callback function to process laser scan messages
	//you can access the range data from msg.ranges[i]. i = sample number
	
}

//doctor will heal resident when they are next to each other
void Resident::doctor_callback(se306_project1::DoctorMsg msg)
{
}

void Resident::assistant_callback(se306_project1::AssistantMsg msg)
{
	if (msg.FoodDelivered == 1)
	{
		hunger = 100;
		ROS_INFO("Resident has received food");
		ROS_INFO("Resident hunger: %d",hunger);
	}
	
}

//Keeps robot moving by changing linear_x and angular_z
std::pair<double, double> Resident::move(double goal_x, double goal_y, double cur_angle, double goal_angle, double px, double py) 
{	
	std::pair<double,double>_ret = std::make_pair(0, 0); //initialize pair. Used to get return.
	double moveSpeed = M_PI/2;
	moveSpeed = ((int)(moveSpeed * 1000 + .5) / 1000.0);

	//When the robot is facing the correct direction, start moving
	double threshold = cur_angle;//-moveSpeed/10;
	//threshold = ((int)(threshold * 1000 + .5) / 1000.0);
/*	ROS_INFO("##################");
	ROS_INFO("goal_y: %f",goal_y);
	ROS_INFO("py: %f",py);
	ROS_INFO("goal_x: %f",goal_x);
	ROS_INFO("px: %f",px);
//	ROS_INFO("angle Vel1: %f", _ret.second);
//	ROS_INFO("threshold: %f",threshold);
//	ROS_INFO("goal_angle: %f",goal_angle);
//	ROS_INFO("cur_angle: %f",cur_angle);
	ROS_INFO("##################");
*/

	if ((goal_angle  == threshold) || isSet) {
		_ret.first = 5; //linear_x
		_ret.second = 0; //angular_z
		isSet = true;
	} else if ((goal_angle <= cur_angle + 0.6) && (goal_angle >= cur_angle - 0.6) )  {
		_ret.first = 0; //linear_x
		_ret.second = fabs(goal_angle - cur_angle); //angular_z
		if (goal_angle == cur_angle) {
			isSet = true;		
		}

	} else {
		_ret.first = 0; //linear_x
		_ret.second = moveSpeed; //angular_z
		isSet = false;
	}


	if ((px-0.1 <= goal_x + 0.5) && (px-0.1 >= goal_x - 0.5) && (py-0.1 <= goal_y + 0.5) && (py-0.1 >= goal_y - 0.5)) {	
			_ret.first = 0; 
			isSet = false;
	}

	return _ret; 
}


double Resident::calc_goal_angle(double goal_x, double goal_y, double cur_angle, double px, double py) 
{

	//Initial and goal vectors used to calculate goal theta
	double init_vector_x;
	double init_vector_y;
	double goal_vector_x;
	double goal_vector_y;
	double goal_angle;
	
	//Finding the vector that the robot is facing and the goal vector
	init_vector_x = cos(cur_angle);
	init_vector_y = sin(cur_angle);
	goal_vector_x = goal_x - px;
	goal_vector_y = goal_y - py;
	
	goal_angle = atan2(goal_vector_y, goal_vector_x); //pi <= goal_angle < -pi
	if (goal_angle < 0) {
		goal_angle = 2 * M_PI + goal_angle; //Remove sign, then add to pi
	} else if (goal_angle == 2 * M_PI) { //New goal angle =>   >0 to 6.283
		goal_angle = 0;
	}
	goal_angle = (2* M_PI) - goal_angle;

	//rounding goal_angle to three decimal places
	goal_angle = ((int)(goal_angle * 1000 + .5) / 1000.0);
	
	return goal_angle;
}

/*
	Simple model of resident - resident should move to a random checkpoint every once in a while. Will be replaced with a
	more complex and realistic model later.
*/
void Resident::randomCheckpointCallback(const ros::TimerEvent&) {
	//ROS_INFO("hello");
}

int Resident::run(int argc, char **argv)
{


	//Initial pose. This is the same as the pose used in the world file.
	px = checkpoints[cc-1][0];
	py = checkpoints[cc-1][1];
	cur_angle = 0;

	//Set goal pose 
	goal_x = checkpoints[cc][0];
	goal_y = checkpoints[cc][1];

	goal_angle = calc_goal_angle(goal_x, goal_y, cur_angle, px, py);

	//Initial velocities
	linear_x = 0;
	angular_z = 0;
	
	//Align local system to global coordinates

	health = 100;
	boredom = 100;
	hunger = 100;
	
	//You must call ros::init() first of all. ros::init() function needs to see argc and argv. The third argument is the name of the node
	ros::init(argc, argv, "Resident");
	//NodeHandle is the main access point to communicate with ros.
	ros::NodeHandle n;

	//advertise() function will tell ROS that you want to publish on a given topic_
	//to stage
	ros::Publisher RobotNode_stage_pub = n.advertise<geometry_msgs::Twist>("robot_1/cmd_vel",1000); 

	//custom message/topic publisher "resident/state" for now
	//ros::Publisher Resident_pub = n.advertise<std_msgs::String>("residentStatus",1000); 
	ros::Publisher resident_pub = n.advertise<se306_project1::ResidentMsg>("residentStatus",1000); 

	//subscribe to listen to messages coming from stage
	ros::Subscriber StageOdo_sub = n.subscribe<nav_msgs::Odometry>("robot_1/odom",1000, &Resident::StageOdom_callback, this);
	ros::Subscriber StageLaser_sub = n.subscribe<sensor_msgs::LaserScan>("robot_1/base_scan",1000,&Resident::StageLaser_callback, this);

	//subscribe to doctor messages
	ros::Subscriber doctor_sub = n.subscribe<se306_project1::DoctorMsg>("healResident",1000, &Resident::doctor_callback, this);

	ros::Subscriber assistant_sub = n.subscribe<se306_project1::AssistantMsg>("assistantStatus",1000, &Resident::assistant_callback, this);

	//ros::Rate loop_rate(10);
	ros::Rate loop_rate(1000);

	//a count of howmany messages we have sent
	int count = 0;

	////messages
	//velocity of this RobotNode
	geometry_msgs::Twist RobotNode_cmdvel;

	// Periodic callback
	int dur2 = time_conversion::simHoursToRealSecs(2); // Perform callback every 2 simulation hours
	ros::Timer medicationTimer = n.createTimer(ros::Duration(dur2), &Resident::randomCheckpointCallback, this); 

	int hungerReductionRate = 1; //1 hunger point reduction per second
	int healthReductionRate = 1; // 0.1 health point reduction per second

	while (ros::ok())
	{
		//messages to stage
		RobotNode_cmdvel.linear.x = linear_x;
		RobotNode_cmdvel.angular.z = angular_z;

		//publish the message
		RobotNode_stage_pub.publish(RobotNode_cmdvel);
		
		// Reduces hunger every second
		if (count % 100 == 0){
				hunger -= hungerReductionRate;
				health -= healthReductionRate;
		}
			std::pair<double, double> velocityValues;	
			velocityValues = std::make_pair(0, 0);
		if (hunger < 90) {
			//ROS_INFO("hunger");
<<<<<<< HEAD
			velocityValues = movePath(checkpoints, 	3);
			linear_x = velocityValues.first;
			angular_z = velocityValues.second;
=======
>>>>>>> 6f4325dfcc39ce8764a381daccd2cd95122ba38d
			
		}
		linear_x = 0;
		angular_z=0;
		//std_msgs::String msg;
		//std::stringstream ss;
		//ss << "Hello world" << hunger;
		//msg.data = ss.str();
		
		//custom resident message publisher
		
		se306_project1::ResidentMsg msg; 
		msg.health = health;
		msg.hunger = hunger;
		msg.x = px; 
		msg.y = py;

		resident_pub.publish(msg);
		
		ros::spinOnce();

		loop_rate.sleep();
		++count;
	}

	return 0;

}

/* 
	Redirects to main function (run()) of the node.
*/
int main(int argc, char **argv) {
	Resident *a = new Resident();
	a->Resident::run(argc, argv);
}
