#ifndef STRATEGYCONTROLLER_H
#define STRATEGYCONTROLLER_H

#include <iostream>
#include "strategy/strategy.h"
#include "model/gamemodel.h"
#include "strategy/stopstrategy.h"
#include "model/robot.h"
#include "behavior/behavior.h"
#include "behavior/stopbehavior.h"
#include "behavior/penaltybehavior.h"
#include "strategy/penaltystrategy.h"

class GameModel;
class Behavior;

class StrategyController
{
public:
    StrategyController();
    void gameModelUpdated();
    void setGameModel(GameModel *);

private:
    strategy *activeStrategy;
    GameModel * model;
};

#endif // STRATEGYCONTROLLER_H
