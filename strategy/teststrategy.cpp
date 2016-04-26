#include "teststrategy.h"
#include "skill/kicktopointomni.h"
#include "behavior/genericmovementbehavior.h"
#include "model/gamemodel.h"
#include "utilities/comparisons.h"
#include "behavior/defendbehavior.h"
#include "behavior/goaliebehavior.h"
#include "behavior/rotateonpoint.h"
#include "ctime"

/************************************************************************/
//Test Behaviors Section

// Test behavior to repeatedly kick the ball to the center
// Waits 6 seconds between kicks
class KickBeh : public Behavior
{
    Skill::KickToPointOmni* ktpo;
    Movement::GoToPosition move_skill;
    std::clock_t start;
    bool wait4recharge;
public:
    KickBeh() {
        ktpo = new Skill::KickToPointOmni(Point(0,0),-1,-1,true);
        wait4recharge = false;
    }
    ~KickBeh() {
        delete ktpo;
    }
    void perform(Robot * robot) override {
        bool finished = false;

        // Perform the kick skill as long as we are not recharging
        if(!wait4recharge)
            finished = ktpo->perform(robot);

        // If finished with kicking, we must recharge for 6 seconds
        // So do nothing
        if(finished)
        {
            wait4recharge = true;
            start = std::clock();
            move_skill.recreate(robot->getPosition());
            move_skill.perform(robot);

        }

        // If 6 seconds have passed since starting recharge,
        // We can kick again
        double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
        if(duration >= 6)
            wait4recharge = false;
    }


};

// Test behavior to rotate to the ball
class RotBeh : public GenericMovementBehavior
{
    void perform(Robot * robot) override
    {
        float ang = Measurements::angleBetween(robot, gameModel->getBallPoint());
        setMovementTargets(robot->getPosition(), ang);
        GenericMovementBehavior::perform(robot);
    }
};

// Test behavior to dribble when ball is close
class DribbleBeh : public Behavior
{
    void perform(Robot* robot) override
    {
        Point bp = gameModel->getBallPoint();
        bool b = Measurements::distance(robot,bp) < 500;
        robot->setDrible(b);
    }
};

// Test behavior to move to a point and print distance and angle errors
class MotionTestBeh : public GenericMovementBehavior
{
public:
    void perform(Robot * robot) override
    {
        Point target = Point(0,0);
        setMovementTargets(target,0, false, false);

        std::cout << "Distance Error: " << Measurements::distance(robot->getPosition(),target) << std::endl;
        std::cout << "Angle Error in Degrees: " << robot->getOrientation()*180/M_PI << std::endl;
        GenericMovementBehavior::perform(robot,Movement::Type::Default);
    }
};

// Test behavior to move the robot from (-2000,0) to (2000,0)
// while facing the ball
class ShamsiStrafe : public GenericMovementBehavior
{
public:
    enum {pos_one,pos_two} state = pos_one;
    void perform(Robot *robot) override
    {
        GameModel * gm = GameModel::getModel();
        Point rp = robot->getPosition();
        Point bp = gm->getBallPoint();
        Point target_one = Point(-2000,0);
        Point target_two = Point(2000,0);
        double ori = Measurements::angleBetween(rp,bp);
        switch(state)
        {
            case pos_one:
                setMovementTargets(target_one,ori);
                if (Measurements::isClose(rp,target_one,100))
                state = pos_two;
                break;
            case pos_two:
                setMovementTargets(target_two,ori);
                if (Measurements::isClose(rp,target_two,100))
                state = pos_one;
        }
        GenericMovementBehavior::perform(robot, Movement::Type::Default);
    }
};


// Moves robot to the given position
class GoToBehavior : public GenericMovementBehavior
{
public:
    GoToBehavior(Point& p):p(p){}

    void perform(Robot *robot) override
    {
        setMovementTargets(p);
        GenericMovementBehavior::perform(robot);
    }
private:
    Point p;
};

bool TestStrategy::update()
{
    //Change IDs and behaviors to be assigned here.
    //All robots must exists before any action is taken.
    Robot* r0  = gameModel->findMyTeam(3);
    Point p = Point(-1000,0);
    if(r0)
        r0->assignBeh<GoalieBehavior>();
    return false;
}

void TestStrategy::assignBeh()
{

}
