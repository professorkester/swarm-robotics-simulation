#include "ros/ros.h"
#include "std_msgs/String.h"
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <sensor_msgs/LaserScan.h>
#include <stdlib.h>
#include <sstream>
#include "math.h"
#include "time_conversion.hpp"
#include "Assistant.h"
#include <cmath>
#include "se306_project1/AssistantMsg.h"
#include "se306_project1/ResidentMsg.h"

using namespace std;

/**
*	@brief Periodic callback for the provision of medication.
*	Called by the ros::Timer in the run() function. Can specify start time, end time, and period.
*	@note The callback is called at the end of the duration specified for the timer.
*	@param TimerEvent& TimerEvent generated by a ros::Timer.
*/
void Assistant::medicate(se306_project1::ResidentMsg msg) {

	double lastCheckpointX = shortestPath.at(shortestPath.size()-1).first;
	double lastCheckpointY = shortestPath.at(shortestPath.size()-1).second;

	double distanceFromCheckpoint = sqrt(pow((lastCheckpointX - px),2) + pow((lastCheckpointY - py),2));

	if (!isMedicated) {
		//move(ResidentCheckpoint);
		if (distanceFromCheckpoint < 0.5) {
			isMedicated == true;

			se306_project1::AssistantMsg amsg;
			amsg.ResidentMedicated = true;
			Assistant_state_pub.publish(amsg);
		}
	}else if (isMedicated) {
		move(msg.currentCheckpoint);
		if (distanceFromCheckpoint < 0.5) {
			isMedicated == false;
		}
	}

}

/**
*	@brief Causes assistant to cook for resident and return to them with the food, feeding them.
*/
void Assistant::cook(se306_project1::ResidentMsg msg) {

	double lastCheckpointX = shortestPath.at(shortestPath.size()-1).first;
	double lastCheckpointY = shortestPath.at(shortestPath.size()-1).second;


	double distanceFromCheckpoint = sqrt(pow((lastCheckpointX - px),2) + pow((lastCheckpointY - py),2));

	if (!atKitchen && !finishedCooking) {
		move("KitchenNorthWest");
		if (distanceFromCheckpoint < 0.5) {
			atKitchen = true;
			pair<double, double> p1 = make_pair(6,-24);
			pair<double, double> p2 = make_pair(24,-24);
			pair<double, double> p3 = make_pair(24,-30);
			pair<double, double> p4 = make_pair(20,-30);
			pair<double, double> p5 = make_pair(20,-28);
			pair<double, double> p6 = make_pair(6,-28);

			shortestPath.clear();
			shortestPath.push_back(p1);
			shortestPath.push_back(p2);
			shortestPath.push_back(p3);
			shortestPath.push_back(p4);
			shortestPath.push_back(p5);
			shortestPath.push_back(p6);
			isMoving = true;
		}


	} else if (atKitchen && !finishedCooking) {

		// The path to simulate the cooking behaviour in the kitchen
		if ((distanceFromCheckpoint < 0.5) ) { // Final kitchen points (refer to p6)
			finishedCooking = true;

		}

	} else if (atKitchen && finishedCooking && !foodDelivered) {

		ROS_INFO("delivering food");
		move(msg.currentCheckpoint);
		lastCheckpointX = shortestPath.at(shortestPath.size()-1).first;
		lastCheckpointY = shortestPath.at(shortestPath.size()-1).second;
		distanceFromCheckpoint = sqrt(pow((lastCheckpointX - px),2) + pow((lastCheckpointY - py),2));
		ROS_INFO("distance: %f",distanceFromCheckpoint);
		ROS_INFO("chec x %f :",lastCheckpointX);
		ROS_INFO("chec y %f ",lastCheckpointY);
		if (distanceFromCheckpoint < 5) {
			isMoving = false;
			ROS_INFO("#########################################################################################################################################################################################################################################################################################################################");
			se306_project1::AssistantMsg amsg;
			foodDelivered = true;
			amsg.FoodDelivered = true;
			Assistant_state_pub.publish(amsg);
			move("HouseCentre");

		}
	} else if (atKitchen && finishedCooking && foodDelivered) {
		ROS_INFO("in");
		move("HouseCentre");
		if (distanceFromCheckpoint < 0.5) {
			ROS_INFO("HOME!");
			atKitchen = false;
			finishedCooking = false;
			foodDelivered = false;
		}
	}
}


/**
*	@brief Causes assistant to clean the house.
*/
void Assistant::clean() {

}	

/**
*	@brief Causes assistant to entertain the resident.
*	@returns true if behaviour was successful, false otherwise
*/
void Assistant::entertain(se306_project1::ResidentMsg msg) {

	double lastCheckpointX = shortestPath.at(shortestPath.size()-1).first;
	double lastCheckpointY = shortestPath.at(shortestPath.size()-1).second;

	double distanceFromCheckpoint = sqrt(pow((lastCheckpointX - px),2) + pow((lastCheckpointY - py),2));
	if (!atBedroom && !residentEntertained) {
		entertainmentCounter = 0;
		move(msg.currentCheckpoint);
		if (distanceFromCheckpoint < 0.5) {
			atBedroom = true;
		}
	} else if (atBedroom && !residentEntertained) {
		angular_z = 2;
		entertainmentCounter++;
		if (entertainmentCounter > 150) {
			angular_z = 0;
			residentEntertained = true;

			se306_project1::AssistantMsg amsg;
			amsg.ResidentEntertained = residentEntertained;
			Assistant_state_pub.publish(amsg);

		}
	} else if (atBedroom && residentEntertained) {
		move("HouseCentre");
		if (distanceFromCheckpoint < 0.5) {
			atBedroom = false;
			residentEntertained = false;
		}
	}
}


/*	@brief Callback function that unpacks and processes resident status messages.
* 	Calls other callback (do) functions.
*	Assistant should subscribe to the ResidentMsg topic in order for this callback to be called. ResidentMsg is published by the Resident.
*	@param msg A custom ResidentMsg message that contains information about the resident's current status.
*/
void Assistant::delegate(se306_project1::ResidentMsg msg) {
	// Resident status will be a string - one among SILL, ILL, HUNGRY, TIRED BORED, and HEALTHCARE - see Mustafa's pq.
	// alternatively we could send the status in another format.
	// enum residentStates {hunger,healthLow,bored,emergency,tired,caregiver,friends,medication,idle};

	if (msg.state != "emergency") {
		// check msg if cook do cooking e.t.c
		if (msg.state == "hunger") {
			cook(msg);
		}

		if (msg.state == "bored") {
			entertain(msg);
		}

		if (msg.state == "medication") {
			medicate(msg);
		}
	}

}

/**
*	@brief Main function for the Assistant process.
*	Controls node setup and periodic events.
*/
int Assistant::run(int argc, char **argv)
{
	//You must call ros::init() first of all. ros::init() function needs to see argc and argv. The third argument is the name of the node
	ros::init(argc, argv, "Assistant");

	//NodeHandle is the main access point to communicate with ros.
	ros::NodeHandle n;

	ros::Rate loop_rate(10);

	px = 17;
	py = 17;

	/* -- Publish / Subscribe -- */

	//advertise() function will tell ROS that you want to publish on a given topic_
	//to stage
	ros::Publisher RobotNode_stage_pub = n.advertise<geometry_msgs::Twist>("robot_1/cmd_vel",1000);
	Assistant_state_pub = n.advertise<se306_project1::ResidentMsg>("assistantStatus", 1000);

	//subscribe to listen to messages coming from stage
	ros::Subscriber StageOdo_sub = n.subscribe("robot_1/odom",1000, &Assistant::StageOdom_callback, dynamic_cast<Agent*>(this));
	ros::Subscriber residentSub = n.subscribe("residentStatus",1000, &Assistant::delegate, this);

	////messages
	//velocity of this RobotNode

	geometry_msgs::Twist RobotNode_cmdvel;

	while (ros::ok())
	{
		//messages to stage
		RobotNode_cmdvel.linear.x = linear_x;
		RobotNode_cmdvel.angular.z = angular_z;

		//publish the message
		RobotNode_stage_pub.publish(RobotNode_cmdvel);
		
		ros::spinOnce();

		loop_rate.sleep();
	}

	return 0;

}

/**
*	@brief Redirects to main function (run()) of the node.
*/
int main(int argc, char *argv[]) {
	Assistant *a = new Assistant();
	//setOriginName(argc, argv[0]); // Set the name of the starting checkpoint
	a->Assistant::run(argc, argv);
}
