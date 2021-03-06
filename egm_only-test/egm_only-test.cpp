/////////////////////////////////////////////////////////////////////////
// Sample using Google protocol buffers C++
//
#include "stdafx.h"
#include <WinSock2.h>
#include <iostream>
#include <fstream>
#include <conio.h>  // _khbit

#include "egm.pb.h" // generated by Google protoc.exe

#pragma comment(lib, "Ws2_32.lib")      // socket lib
#ifdef _DEBUG
	#pragma comment(lib, "libprotobufd.lib")
#else
	#pragma comment(lib, "libprotobuf.lib") // protobuf lib
#endif

static int portNumber = 6510;
static unsigned int sequenceNumber = 0;

using namespace std;
using namespace abb::egm;

class Sensor;
float XY_Values_Square(Sensor& sensor); // function declare
void DisplayRobotMessage(EgmRobot *pRobotMessage, Sensor& sensor);
void CreateSensorMessage(EgmSensor* pSensorMessage, Sensor& sensor);


// Protobuf-C++ is supported by Google and no other third party libraries needed for the protobuf part. 
// It can be a bit tricky to build the Google tools in Windows but here is a guide on how to build 
// protobuf for Windows (http://eli.thegreenplace.net/2011/03/04/building-protobuf-examples-on-windows-with-msvc).
//
// When you have built libprotobuf.lib and protoc.exe:
//	 Run Google protoc to generate access classes, protoc --cpp_out=. egm.proto
//	 Create a win32 console application
//	 Add protobuf source as include directory
//	 Add the generated egm.pb.cc to the project (exclude the file from precompile headers)
//	 Copy the file below
//	 Compile and run
// Copyright (c) 2014, ABB
// All rights reserved.


//////////////////////////////////////////////////////////////////////////
#if 0
double _x = 0;
double _y = 0;
double q0 = 0;
double q3 = 0;
double _xStartPoint = 28 - _x; // Bunlar asagidakilarla eyni olsa yaxsidir
double _yStartPoint = -400 + _y;
double _robotX = 0; // RobotStudiodan gelenleri bunlara kocureceyik
double _robotY = 0;
double _robotZ = 0;
#endif

class Sensor {
public:
	Sensor() {
		_point_index = 0;
		_x = 0;
		_y = 0;
		q0 = 0;
		q3 = 0;
		_xStartPoint = 28 - _x; // Bunlar asagidakilarla eyni olsa yaxsidir
		_yStartPoint = -400 + _y;
		_robotX = 0; // RobotStudiodan gelenleri bunlara kocureceyik
		_robotY = 0;
		_robotZ = 0;
	}

	double& x() { // direct reference, https://en.wikipedia.org/wiki/Reference_(C%2B%2B)
		return _x;
	}
	double& y() {
		return _y;
	}
	double& x_robot_onWorldCoordinate() {
		return _robotX;
	}
	double& y_robot_onWorldCoordinate() {
		return _robotY;
	}
	double& z_robot_onWorldCoordinate() {
		return _robotZ;
	}
	double& xStartPoint() {
		return _xStartPoint;
	}
	double& yStartPoint() {
		return _yStartPoint;
	}
	// Sensor sensor;
	// sensor.x() = 1.234;


	int& point_index() {
		return _point_index;
	}

private:
	int _point_index;
	double _x;
	double _y;
	double q0;
	double q3;
	double _xStartPoint; // Bunlar asagidakilarla eyni olsa yaxsidir
	double _yStartPoint;
	double _robotX; // RobotStudiodan gelenleri bunlara kocureceyik
	double _robotY;
	double _robotZ;

};


void transformFromRobot2Controller(float rx, float ry, float& wx, float& wy) {
	wx = 28 - rx;
	wy = ry + 400;
}

// Create a simple sensor message
void CreateSensorMessage(EgmSensor* pSensorMessage, Sensor& sensor)
{

	EgmHeader* header = new EgmHeader();
	header->set_mtype(EgmHeader_MessageType_MSGTYPE_CORRECTION);
	header->set_seqno(sequenceNumber++);
	header->set_tm(GetTickCount());

	pSensorMessage->set_allocated_header(header);

	///	sensor.x() = X_Values_Square(sensor); // _x yazdigim algorithme gore yeni deyer alir
	///	sensor.y() = Y_Values_Square(sensor); // _y yazdigim algorithme gore yeni deyer alir
	XY_Values_Square(sensor);

	float x_robot_onControllerCoordinate;
	float y_robot_onControllerCoordinate;
	transformFromRobot2Controller(sensor.x_robot_onWorldCoordinate(), sensor.y_robot_onWorldCoordinate(),
		x_robot_onControllerCoordinate, y_robot_onControllerCoordinate);

	EgmCartesian *pc = new EgmCartesian();
	printf("idx[%d] : (%f, %f) -> (%f,%f)\n",
		sensor.point_index(), x_robot_onControllerCoordinate, y_robot_onControllerCoordinate,
		sensor.x(), sensor.y());

	pc->set_x(sensor.x());
	pc->set_y(sensor.y());
	pc->set_z(0.0);

	EgmQuaternion *pq = new EgmQuaternion();
	pq->set_u0(1.0);
	pq->set_u1(0.0);
	pq->set_u2(0.0);
	pq->set_u3(0.0);

	EgmPose *pcartesian = new EgmPose();
	pcartesian->set_allocated_orient(pq);
	pcartesian->set_allocated_pos(pc);

	EgmPlanned *planned = new EgmPlanned();
	planned->set_allocated_cartesian(pcartesian);

	pSensorMessage->set_allocated_planned(planned);
}

//////////////////////////////////////////////////////////////////////////
// Display inbound robot message
void DisplayRobotMessage(EgmRobot *pRobotMessage, Sensor& sensor)
{

	if (pRobotMessage->has_header() && pRobotMessage->header().has_seqno() && pRobotMessage->header().has_tm() && pRobotMessage->header().has_mtype())
	{
		//printf("SeqNo=%d Tm=%u Type=%d\n", pRobotMessage->header().seqno(), pRobotMessage->header().tm(), pRobotMessage->header().mtype());
		//Display Robot x,y,z positions
		sensor.x_robot_onWorldCoordinate() = pRobotMessage->feedback().cartesian().pos().x();
		sensor.y_robot_onWorldCoordinate() = pRobotMessage->feedback().cartesian().pos().y();
		sensor.z_robot_onWorldCoordinate() = pRobotMessage->feedback().cartesian().pos().z();
		//printf("feedback pose robotX, robotY, robotY:(%f, %f, %f)\n", sensor.robotX(), sensor.robotY(), sensor.robotZ());
	}
	else
	{
		printf("No header\n");
	}
}



float XY_Values_Square(Sensor& sensor) // definition of function
{
	float x_goal_onControllerCoordinate[13] = { 0, 0, -5, -7, -10, -13, -15, -20, -25, -30, -35, -40, -45 };
	float y_goal_onControllerCoordinate[13] = { 0, -20, -30, -40, -50, -60, -70, -85, -95, -105, -110, -110, -110 };
	int idx = sensor.point_index();
	sensor.x() = x_goal_onControllerCoordinate[idx];
	sensor.y() = y_goal_onControllerCoordinate[idx];

	//float dist2 = pow(sensor.robotX() - sensor.xStartPoint(), 2)+ pow(sensor.robotY() - sensor.yStartPoint(), 2);
	float x_robot_onControllerCoordinate;
	float y_robot_onControllerCoordinate;
	transformFromRobot2Controller(sensor.x_robot_onWorldCoordinate(), sensor.y_robot_onWorldCoordinate(),
		x_robot_onControllerCoordinate, y_robot_onControllerCoordinate);

	float dist2 = pow(x_robot_onControllerCoordinate - sensor.x(), 2) + pow(y_robot_onControllerCoordinate - sensor.y(), 2);
	float dist = sqrt(dist2);
	if (dist < 1.0) {
		idx += 1;
		if (idx < 13) {
			sensor.point_index() = idx;
			sensor.x() = x_goal_onControllerCoordinate[idx];
			sensor.y() = y_goal_onControllerCoordinate[idx];
		}
		else {
			printf("goal position. end program\n");
			exit(1);
		}
	}
	return idx;
}

int SendSensorMessage(SOCKET sockfd, const sockaddr* clientAddr, int len, EgmSensor* pSensorMessage)
{
	string messageBuffer;
	int n;
	pSensorMessage->SerializeToString(&messageBuffer);

	// send a message to the robot
	n = sendto(sockfd, messageBuffer.c_str(), messageBuffer.length(), 0, clientAddr, len);
	if (n < 0)
	{
		printf("Error send message\n");

	}
	return n;
}

int RecieveRobotMessage(SOCKET sockfd, sockaddr* clientAddr, int* len, EgmRobot* last_message)
{
	int n;
	char protoMessage[1400];
	n = recvfrom(sockfd, protoMessage, 1400, 0, clientAddr, len);
	if (n < 0)
	{
		printf("Error receive message\n");
		return n;
	}
	// deserialize inbound message
	EgmRobot *pRobotMessage = new EgmRobot();
	pRobotMessage->ParseFromArray(protoMessage, n);
	// PrintVelocity(pRobotMessage, last_message, outfile, speed_pose);
	*last_message = *pRobotMessage;
	delete pRobotMessage;
	return n;
}

int main_old() {
	Sensor sensor; // TODO
	SOCKET sockfd;
	int n;
	struct sockaddr_in serverAddr, clientAddr;
	EgmRobot last_message;
	int flag = 0;
	char protoMessage[1400];/// bu yoxdur
							/* Init winsock */
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		fprintf(stderr, "Could not open Windows connection.\n");
		exit(0);
	}
	// create socket to listen on
	sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
	memset(&serverAddr, sizeof(serverAddr), 0);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(portNumber);

	// listen on all interfaces
	bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

	while (1) {
		int len = sizeof(clientAddr);
		int n = recvfrom(sockfd, protoMessage, 1400, 0, (struct sockaddr *)&clientAddr, &len);
		if (n < 0){
			printf("Error receive message\n");
			continue;
		}
		EgmRobot *pRobotMessage = new EgmRobot();
		pRobotMessage->ParseFromArray(protoMessage, n);
		DisplayRobotMessage(pRobotMessage, sensor);
		delete pRobotMessage;
#if 1
		// deserialize inbound message
		// create and send a sensor message
		EgmSensor *pSensorMessage = new EgmSensor();
		CreateSensorMessage(pSensorMessage, sensor);
		flag = SendSensorMessage(sockfd, (struct sockaddr *)&clientAddr, len, pSensorMessage);
		delete pSensorMessage;
#endif
		if (_kbhit())
			break;
	}

}

int _tmain(int argc, _TCHAR* argv[])
{
	main_old();
	return 0;
}

