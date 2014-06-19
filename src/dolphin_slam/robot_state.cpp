#include "robot_state.h"

const float DVL_ERROR = 0.1;

namespace dolphin_slam
{

RobotState::RobotState(): normal_(0,DVL_ERROR), var_nor(rng_, normal_)
{

    has_ground_truth_ = has_dvl_ = has_imu_ = false;
    robot_vel_ = cv::Point3f(0.0,0.0,0.0);
    robot_yaw_ = 0;

    delta_pc_ = delta_em_ = cv::Point3f(0.0,0.0,0.0);
}

void RobotState::loadConfig()
{

    node_handle_.param<std::string>("dvl_topic", dvl_topic_, "/g500/dvl");
    node_handle_.param<std::string>("imu_topic", imu_topic_, "/g500/imu");
    node_handle_.param<std::string>("gt_topic", gt_topic_, "/ground_truth");

}


void RobotState::groundTruthCallback(const nav_msgs::OdometryConstPtr &message)
{
    if(!has_ground_truth_)
    {
        first_ground_truth_.x = message->pose.pose.position.x;
        first_ground_truth_.y = message->pose.pose.position.y;
        first_ground_truth_.z = message->pose.pose.position.z;
    }

    ground_truth_.x = message->pose.pose.position.x - first_ground_truth_.x;
    ground_truth_.y = message->pose.pose.position.y - first_ground_truth_.y;
    ground_truth_.z = message->pose.pose.position.z - first_ground_truth_.z;

    has_ground_truth_ = true;
}

void RobotState::computeTraveledDistances(float elapsed_time)
{
    delta_pc_ += robot_vel_*elapsed_time;
    delta_em_ += robot_vel_*elapsed_time;

    //! Atualiza as novas velocidades e a nova orientação do robo
    robot_vel_.x = vel_dvl_.x * cos(robot_yaw_) - vel_dvl_.y * sin(robot_yaw_);
    robot_vel_.y = vel_dvl_.x * sin(robot_yaw_) + vel_dvl_.y * cos(robot_yaw_);
    robot_vel_.z = vel_dvl_.z;

    robot_yaw_ = yaw_imu_;
}
void RobotState::connectROSTopics()
{
    DVL_subscriber_ = node_handle_.subscribe(dvl_topic_,1,&RobotState::DVLCallback,this);
    IMU_subscriber_ = node_handle_.subscribe(imu_topic_,1,&RobotState::IMUCallback,this);
    ground_truth_subscriber_ = node_handle_.subscribe(gt_topic_,1,&RobotState::groundTruthCallback,this);
    


}


void RobotState::createROSServices()
{
    pc_service__ = node_handle_.advertiseService("robot_state_pc", &RobotState::pcService,this);
    em_service_= node_handle_.advertiseService("robot_state_em", &RobotState::emService,this);
}


bool RobotState::pcService(RobotPose::Request &req, RobotPose::Response &res)
{
    res.traveled_distance_.x = delta_pc_.x;
    res.traveled_distance_.y = delta_pc_.y;
    res.traveled_distance_.z = delta_pc_.z;

    res.ground_truth_.x = ground_truth_.x;
    res.ground_truth_.y = ground_truth_.y;
    res.ground_truth_.z = ground_truth_.z;

    if(req.reset)
    {
        delta_pc_ = cv::Point3f(0.0,0.0,0.0);
    }

    return true;
}

bool RobotState::emService(RobotPose::Request &req, RobotPose::Response &res)
{

    res.traveled_distance_.x = delta_em_.x;
    res.traveled_distance_.y = delta_em_.y;
    res.traveled_distance_.z = delta_em_.z;

    res.ground_truth_.x = ground_truth_.x;
    res.ground_truth_.y = ground_truth_.y;
    res.ground_truth_.z = ground_truth_.z;

    if(req.reset)
    {
        delta_em_ = cv::Point3f(0.0,0.0,0.0);
    }

    return true;
}


void RobotState::DVLCallback(const underwater_sensor_msgs::DVLConstPtr &message)
{
    cv::Point3f white_noise(var_nor(),var_nor(),var_nor());
    float elapsed_time;

    if(message->bi_error < 1)
    {
        vel_dvl_.x = message->bi_x_axis;
        vel_dvl_.y = message->bi_y_axis;
        vel_dvl_.z = message->bi_z_axis;

        //vel_dvl_ += white_noise;

        //ROS_DEBUG_STREAM_NAMED("rs","DVL velocity =  " << vel_dvl_);


        if(has_dvl_ && has_imu_)
        {
            elapsed_time = (message->header.stamp - timestamp_).toSec();

            computeTraveledDistances(elapsed_time);
        }

        timestamp_ = message->header.stamp;
        has_dvl_ = true;
    }
    else
    {
        ROS_DEBUG_STREAM_NAMED("rs","O erro da DVL eh " << message->bi_error);
    }


}

void RobotState::IMUCallback(const sensor_msgs::ImuConstPtr &message)
{


    yaw_imu_ = tf::getYaw(message->orientation);

    //ROS_DEBUG_STREAM_NAMED("rs","message->orientation =  " << message->orientation);

    has_imu_ = true;
}


}   //namespace

