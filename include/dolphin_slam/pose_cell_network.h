#ifndef POSE_CELL_NETWORK_H
#define POSE_CELL_NETWORK_H

//ROS
#include <ros/ros.h>

#include <numeric>

#include <robot_state.h>

// Messages
#include <sensor_msgs/Imu.h>
#include <visualization_msgs/Marker.h>
#include <underwater_sensor_msgs/DVL.h>
#include <dolphin_slam/ExperienceEvent.h>
#include <dolphin_slam/LocalViewNetwork.h>
#include <dolphin_slam/RobotPose.h>
#include <dolphin_slam/ExecutionTime.h>

#include <local_view_module.h>

// OpenCV
#include <opencv2/opencv.hpp>

#include <boost/foreach.hpp>

#include <cmath>

#include <color.h>
#include <time_monitor.h>
#include <angles/angles.h>

#include <boost/math/special_functions/modf.hpp>


#define foreach BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH

namespace dolphin_slam
{


class PoseCellNetwork
{
public:
    PoseCellNetwork();

    void loadConfig();
    void connectROSTopics();
    void createROSTimers();
    void connectROSServices();


    void rectifyIndeces(int a[]);
    int getWrapIndex(int index, int dimension);


    void callRobotStateServices();
private:
    float squaredDistance(int center[], int new_index[]);
    float euclideanDistance(int center[], int new_index[]);
    float calculateMaxDistance();

    //! CANN
    void updateNetwork(); //!< \todo Decidir como será o update da rede
    void exciteNetwork();
    void globallyInhibitsNetwork();
    void learnExternalConnections();
    void applyExternalInputOnNetwork();
    void applyPathIntegrationOnNetwork();
    void limitNetworkActivity();
    void normalizeNetworkActivity();
    void getActiveNeuron(std::vector<int> &active_neuron);

    void integrateX(float delta_x, float delta_y, float delta_z, float delta_o);
    void integrateY(float delta_x, float delta_y, float delta_z, float delta_o);
    void integrateZ(float delta_x, float delta_y, float delta_z, float delta_o);
    void integrateYaw(float delta_x, float delta_y, float delta_z, float delta_o);

    //! Função para alocar a matriz de neuronios
    void createNeurons();
    //! Função para alocar a matriz de pesos
    void createExcitatoryWeights();
    void initNeuronsActivity();
    void initRecurrentExcitatoryWeights();
    void normalizeRecurrentExcitatoryWeights();


    //! callbacks
    void viewTemplateCallback(const LocalViewNetworkConstPtr &message); //!< \todo trocar para a mensagem de view template

    void timerCallback(const ros::TimerEvent &event);

    void publishNetworkActivityMessage();
    void publishExperienceMapEvent();
    void publishExecutionTime();


    //! Local View
    bool has_new_local_view_cell_;
    int most_active_lv_cell_;
    float lambda_learning_;
    float max_energy_;
    std::vector <Cell> lv_cells_active_;
    std::vector<cv::Mat_<float> > local_view_synaptic_weights_;    //!< matriz de pesos das conexões de entrada
    int max_view_template_id_;
    int external_input_min_age_;


    //! CANN
    std::vector<int> number_of_neurons_; //!< guarda o tamanho de cada dimensão da matriz de neuronios
    std::vector<float> excitatory_variance;  //!< armazena os desvios padrões da matriz de excitação
    std::vector<int> number_of_recurrent_excitatory_weights_ ;  //!< armazena o tamanho de cada dimensão da matriz de excitação
    std::vector<float> distance_between_neurons_; //!< Armazena a distancia entre dois neuronios, em cada dimensão da rede
    cv::Mat_<float> neurons_;   //!< 4D Array of neurons
    cv::Mat_<float> aux_neurons_;  //!< Matriz auxiliar para atualização da rede

    cv::Mat_<float> recurrent_excitatory_weights_; //!< Matriz dos pesos de excitação.
    float global_inibition_;
    std::vector<int> active_index_;
    float k_external_activation_;
    float learning_constant_;


    //RobotState robot_state_;    //! Stores the travelled distance between updates

    bool experience_event_;     //! variable to indicate a event to the experience map

    //ROS Related Variable

    //! ROS Node Handle
    ros::NodeHandle node_handle_;

    //! ROS Topics
    ros::Subscriber view_template_subscriber_;
    ros::Subscriber DVL_subscriber_;
    ros::Subscriber IMU_subscriber_;

    ros::Publisher net_activity_publisher_;
    ros::Publisher net_activity_yaw_publisher_;
    ros::Publisher experience_event_publisher_;
    ros::Publisher execution_time_publisher_;


    ros::Timer timer_;

    ros::ServiceClient robot_state_pc_service;
    ros::ServiceClient robot_state_em_service;

    dolphin_slam::RobotPose robot_pose_pc_;
    dolphin_slam::RobotPose robot_pose_em_;

    LocalViewNetworkPtr local_view_network_msg_;

    TimeMonitor time_monitor_;

    bool multiple_local_view_active_;
    bool use_gaussian_weights_;

    friend class PoseCellMessages;

};


int sign(float value);
void get_integer_decimal_part(float value, int &integer,float &decimal);




} //namespace

#endif // POSE_CELL_NETWORK_H
