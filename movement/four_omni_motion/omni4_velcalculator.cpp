#include "movement/four_omni_motion/omni4_velcalculator.h"
#include "utilities/debug.h"
#include "include/config/tolerances.h"
using std::abs;

namespace Movement
{

#define FOUR_WHEEL_DEBUG 0

//Multiplier for theta_vel in defaultCalc
float THETA_MULT = 3;

//Multiplier for theta_vel in facePointCalc
float THETA_MULT2 = 1;

//Multiplier for x_vel and y_vel in defaultCalc
float XY_MULT = 1;

//Error for Proportional XY
float xy_prop_mult = 0.1;

//Multiplier for integral XY
float xy_int_mult = 0.0005;

//Multiplier for theta proportional
float theta_prop_mult = 1;

//Multiplier for theta integral error
float theta_int_mult = 0.001;

FourWheelCalculator::FourWheelCalculator()
{
    debug::registerVariable("xyp", &xy_prop_mult);
    debug::registerVariable("xyi", &xy_int_mult);
    debug::registerVariable("thp", &theta_prop_mult);
    debug::registerVariable("thi", &theta_int_mult);
}

fourWheelVels FourWheelCalculator::calculateVels
    (Robot* rob, Point goalPoint, float theta_goal, Type moveType)
{
	return calculateVels(rob, goalPoint.x, goalPoint.y, theta_goal, moveType);
}

fourWheelVels FourWheelCalculator::calculateVels
    (Robot* rob, float x_goal, float y_goal, float theta_goal, Type moveType)
{
    switch (moveType)
    {
    case Type::facePoint:
        return facePointCalc(rob,x_goal,y_goal,theta_goal);
        break;
    default:
        return defaultCalc(rob,x_goal,y_goal,theta_goal);
    }
}

fourWheelVels FourWheelCalculator::defaultCalc
    (Robot* rob, float x_goal, float y_goal, float theta_goal)
{
    //Current Position
    double x_current = rob->getRobotPosition().x;
    double y_current = rob->getRobotPosition().y;
    double theta_current = rob->getOrientation();
    last_goal_target = Point(x_goal, y_goal);

    Point rp = Point(x_current,y_current);
    Point gp = Point(x_goal,y_goal);
    distance_to_goal = Measurments::distance(rp,gp);
    float angle_to_goal = Measurments::angleBetween(rp, gp);
    angle_error = Measurments::angleDiff(rob->getOrientation(), theta_goal);

    //Calulate error integral component
    calc_error(x_goal, y_goal);

    //Inertial Frame Velocities
    double x_vel =
        (xy_prop_mult * distance_to_goal +
         xy_int_mult  * dist_error_integral)*cos(angle_to_goal);
    double y_vel =
        (xy_prop_mult * distance_to_goal +
         xy_int_mult  * dist_error_integral)*sin(angle_to_goal);
    double theta_vel =
         theta_prop_mult * angle_error
       + theta_int_mult  * angle_error_integral;

    if (abs(Measurments::angleDiff(theta_goal,theta_current))<
        abs(Measurments::angleDiff(theta_goal,theta_current+theta_vel)))
        theta_vel=-theta_vel;

    // Reduce speed near target
    if (distance_to_goal < 700)
    {
        x_vel *= XY_MULT;
        y_vel *= XY_MULT;
        theta_vel *= THETA_MULT;
    }

    // Robot Frame Velocities
    double y_vel_robot = cos(theta_current)*x_vel+sin(theta_current)*y_vel;
    double x_vel_robot = sin(theta_current)*x_vel-cos(theta_current)*y_vel;

    //Wheel Velocity Calculations
    double RF =  (-sin(RF_offset) * x_vel_robot + cos(RF_offset)*y_vel_robot + wheel_radius*theta_vel);
    double LF = -(-sin(LF_offset) * x_vel_robot + cos(LF_offset)*y_vel_robot + wheel_radius*theta_vel);
    double LB = -(-sin(LB_offset) * x_vel_robot + cos(LB_offset)*y_vel_robot + wheel_radius*theta_vel);
    double RB =  (-sin(RB_offset) * x_vel_robot + cos(RB_offset)*y_vel_robot + wheel_radius*theta_vel);

    //Normalize wheel velocities
    unsigned int max_mtr_spd = 100;
    if (abs(LF)>max_mtr_spd)
    {
        LB=(max_mtr_spd/abs(LF))*LB;
        RF=(max_mtr_spd/abs(LF))*RF;
        RB=(max_mtr_spd/abs(LF))*RB;
        LF=(max_mtr_spd/abs(LF))*LF;
    }
    if (abs(LB)>max_mtr_spd)
    {
        LF=(max_mtr_spd/abs(LB))*LF;
        RF=(max_mtr_spd/abs(LB))*RF;
        RB=(max_mtr_spd/abs(LB))*RB;
        LB=(max_mtr_spd/abs(LB))*LB;
    }
    if (abs(RF)>max_mtr_spd)
    {
        LF=(max_mtr_spd/abs(RF))*LF;
        LB=(max_mtr_spd/abs(RF))*LB;
        RB=(max_mtr_spd/abs(RF))*RB;
        RF=(max_mtr_spd/abs(RF))*RF;
    }
    if (abs(RB)>max_mtr_spd)
    {
        LF=(max_mtr_spd/abs(RB))*LF;
        LB=(max_mtr_spd/abs(RB))*LB;
        RF=(max_mtr_spd/abs(RB))*RF;
        RB=(max_mtr_spd/abs(RB))*RB;
    }

    //Create and return result container
    fourWheelVels vels;
    vels.LB = LB;
    vels.LF = LF;
    vels.RB = RB;
    vels.RF = RF;
    return vels;
}

fourWheelVels FourWheelCalculator::facePointCalc
    (Robot* rob, float x_goal, float y_goal, float theta_goal)
{
    //Current Position
    double x_current = rob->getRobotPosition().x;
    double y_current = rob->getRobotPosition().y;
    double theta_current = rob->getOrientation();
    last_goal_target = Point(x_goal, y_goal);

    Point rp = Point(x_current,y_current);
    Point gp = Point(x_goal,y_goal);
    distance_to_goal = Measurments::distance(rp,gp);
    float angle_to_goal = Measurments::angleBetween(rp, gp);
    angle_error = Measurments::angleDiff(rob->getOrientation(), theta_goal);

    //PID
    calc_error(x_goal, y_goal);

    //Interial Frame Velocities
    double x_vel =
        (xy_prop_mult * distance_to_goal +
         xy_int_mult  * dist_error_integral)*cos(angle_to_goal);
    double y_vel =
        (xy_prop_mult * distance_to_goal +
         xy_int_mult  * dist_error_integral)*sin(angle_to_goal);
    double theta_vel =
         theta_prop_mult * angle_error
       + theta_int_mult  * angle_error_integral;

    if (abs(Measurments::angleDiff(theta_goal,theta_current))<
        abs(Measurments::angleDiff(theta_goal,theta_current+theta_vel)))
        theta_vel=-theta_vel;

    // Reduce speed near target
    if (distance_to_goal < 300)
    {
        x_vel *= 0.7;
        y_vel *= 0.7;
    }

    // Focus on rotation
    double vel = sqrt(x_vel*x_vel+y_vel*y_vel);
    if (abs(Measurments::angleDiff(theta_goal,theta_current))>ROT_TOLERANCE*0.5 && vel > 40)
    {
        x_vel = 90*cos(angle_to_goal);
        y_vel = 90*sin(angle_to_goal);
        theta_vel *= THETA_MULT2;
    }

    // Robot Frame Velocities
    double y_vel_robot = cos(theta_current)*x_vel+sin(theta_current)*y_vel;
    double x_vel_robot = sin(theta_current)*x_vel-cos(theta_current)*y_vel;

    //Wheel Velocity Calculations
    double RF =  (-sin(RF_offset) * x_vel_robot + cos(RF_offset)*y_vel_robot + wheel_radius*theta_vel);
    double LF = -(-sin(LF_offset) * x_vel_robot + cos(LF_offset)*y_vel_robot + wheel_radius*theta_vel);
    double LB = -(-sin(LB_offset) * x_vel_robot + cos(LB_offset)*y_vel_robot + wheel_radius*theta_vel);
    double RB =  (-sin(RB_offset) * x_vel_robot + cos(RB_offset)*y_vel_robot + wheel_radius*theta_vel);

    //Normalize wheel velocities
    unsigned int max_mtr_spd = 100;
    if (abs(LF)>max_mtr_spd)
    {
        LB=(max_mtr_spd/abs(LF))*LB;
        RF=(max_mtr_spd/abs(LF))*RF;
        RB=(max_mtr_spd/abs(LF))*RB;
        LF=(max_mtr_spd/abs(LF))*LF;
    }
    if (abs(LB)>max_mtr_spd)
    {
        LF=(max_mtr_spd/abs(LB))*LF;
        RF=(max_mtr_spd/abs(LB))*RF;
        RB=(max_mtr_spd/abs(LB))*RB;
        LB=(max_mtr_spd/abs(LB))*LB;
    }
    if (abs(RF)>max_mtr_spd)
    {
        LF=(max_mtr_spd/abs(RF))*LF;
        LB=(max_mtr_spd/abs(RF))*LB;
        RB=(max_mtr_spd/abs(RF))*RB;
        RF=(max_mtr_spd/abs(RF))*RF;
    }
    if (abs(RB)>max_mtr_spd)
    {
        LF=(max_mtr_spd/abs(RB))*LF;
        LB=(max_mtr_spd/abs(RB))*LB;
        RF=(max_mtr_spd/abs(RB))*RF;
        RB=(max_mtr_spd/abs(RB))*RB;
    }

    //Create and return result container
    fourWheelVels vels;
    vels.LB = LB;
    vels.LF = LF;
    vels.RB = RB;
    vels.RF = RF;
    return vels;
}

void FourWheelCalculator::calc_error(float x_goal, float y_goal)
{
    //Reset queues if the target has moved
    if(Measurments::distance(Point(x_goal,y_goal), last_goal_target) > 50)
        clear_errors();

    //Integral Error for distance
    if (dist_error_deque.size() == dist_error_maxsize) {
        dist_error_integral -= dist_error_deque.front();
        dist_error_deque.pop_front();
    }
    dist_error_integral += distance_to_goal;
    dist_error_deque.push_back(distance_to_goal);

    //Integral Error for orientation
    if (angle_error_deque.size() == angle_error_maxsize) {
        angle_error_integral -= angle_error_deque.front();
        angle_error_deque.pop_front();
    }
    angle_error_integral += angle_error;
    angle_error_deque.push_back(angle_error);
}

void FourWheelCalculator::clear_errors()
{
    dist_error_deque.clear();
    angle_error_deque.clear();
    dist_error_integral = angle_error_integral = 0;
}

}

