#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "GameConstants.h"
#include <map>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stack>
#include <utility>

class StudentWorld;

// constant to show that something is out of bounds
// returned by getPixelArr in StudentWorld
const int OUT_OF_BOUNDS = 27;

// base class for all actors
class obj : public GraphObject {
public:
    // constructor
    obj(int hp, int imageID, int startX, int startY, StudentWorld* worldIn, Direction dir, double size, unsigned int depth);

    // virtual destructor
    virtual ~obj();

    // pure virtual function to tell objects what to do
    virtual void doSomething() = 0; // virtual function telling child classes how to act

    // returns if obj should stay on field
    virtual bool getStatus();

    StudentWorld* getWorld(); // returns StudentWorld pointer for use by child classes

    // increments hit points / ticks
    void changeHitPoints(int change);

    // returns hit points
    int getHitPoints();

private:
    StudentWorld* world; // contains pointer to StudentWorld that object belongs to
    int health; // hit points OR ticks left
};

// class for Earth to fill map
class Earth : public obj {
public:
    // constructs a visible Earth object at (x, y)
    // -1 for ticks since ticks are not decremented during game
    Earth(int x, int y, StudentWorld* worldIn);

    // virtual destructor
    virtual ~Earth();

    // empty method, but mandatory since is derived from obj
    virtual void doSomething();
};

// class for boulder type objects in game
class Boulder : public obj {
public:
    // constructor that accepts coordinates and pointer to StudentWorld boulder belongs to
    Boulder(int x, int y, StudentWorld* worldIn);

    // virtual destructor
    virtual ~Boulder();

    // tells Boulder what to do every tick (do nothing, fall, or wait)
    virtual void doSomething();

    // returns if Boulder should stay on map during game
    virtual bool getStatus();

private:
    std::string status; // keeps track of whether Boulder is falling, waiting, or dead
    bool dirtUnder(); // checks if there is Earth or a Boulder beneath this Boulder
    void checkMoveCollisions(); // check if the Boulder hits a Protester or TunnelMan
};

// base class for items that can be picked up
class Goods : public obj {
public:
    // constructor
    Goods(int x, int y, int hp, int ID, StudentWorld* worldIn);

    // virtual destructor
    virtual ~Goods();
};

// class for oil barrel items on screen
class Barrel : public Goods {
public:
    // constructor
    Barrel(int x, int y, StudentWorld* worldIn);

    // virtual destructor
    virtual ~Barrel();

    // tells Barrel if should be visible or picked up per tick
    virtual void doSomething();
};

// class for sonar items on screen
class Sonar : public Goods {
public:
    // constructor
    Sonar(int x, int y, StudentWorld* worldIn);

    // virtual destructor
    virtual ~Sonar();

    // tells sonar what to do every tick
    virtual void doSomething();

private:
    int calcTicks(); // calculates amount of ticks for sonar to stay in game
};

// class for gold nuggets on screen
class GoldNugget : public Goods {
public:
    // constructor
    GoldNugget(int x, int y, int ticks, bool prot, StudentWorld* worldIn);

    // destructor
    virtual ~GoldNugget();

    // tells GoldNugget what to do every tick
    virtual void doSomething();

private:
    bool protestersSee; // true if protesters can pick up the gold, false if TunnelMan can
};

// class for water pools on screen
class WaterPool : public Goods {
public:
    // constructor
    WaterPool(int x, int y, StudentWorld* worldIn);

    // virtual destructor
    virtual ~WaterPool();

    // tells WaterPool what to do every tick
    virtual void doSomething();

private:
    int calcTicks(); // calculates amount of ticks for WaterPool to stay in game
};

// class for squirts that TunnelMan fires
class Squirt : public obj {
public:
    // constructor
    Squirt(int x, int y, GraphObject::Direction dir, StudentWorld* worldIn);

    // virtual destructor
    virtual ~Squirt();

    // tells Squirt what to do every tick
    virtual void doSomething();

private:
    bool checkMoveCollisions(); // check if Squirt hits a protester
};

// base class for TunnelMan and Protestors
class Actor : public obj {
public:
    // constructor
    Actor(StudentWorld* worldIn, int id, int x, int y, Direction dir, int hp);

    // virtual destructor
    virtual ~Actor();

    // returns alive
    virtual bool getStatus();

    // sets alive to false
    virtual void setDead();

    // attempt to move actor in direction dir
    // digEarth is whether actor can dig Earth (TunnelMan) or not (Protesters)
    void moveDir(int dir, bool digEarth);

private:
    bool alive; // holds whether actor should still be in play or not
};

// class for player character / TunnelMan
class TunnelMan : public Actor {
public:
    // constructor
    TunnelMan(StudentWorld* worldIn, int numBarrels);

    // virtual destructor
    virtual ~TunnelMan();

    // tells TunnelMan what to do every tick
    virtual void doSomething();

    // returns barrels
    int getBarrels();

    // decrements barrels
    void decBarrels();

    // return sonar
    int getSonar();

    // increment sonar by change
    void changeSonar(int change);

    // return nuggets
    int getNuggets();

    // increment nuggets by change
    void changeNuggets(int change);

    // return squirts
    int getSquirts();

    // increase squirts by 5
    void increaseSquirts();

    // bool returns status of in play
    virtual bool getStatus();

private:
    int barrels; // number of barrels left to find
    int sonar; // number of sonar charges held
    int nuggets; // number of gold held
    int squirts; // number of water held

    void getKey(int& x); // get last key pressed
    void useSonar(); // attempt to use 1 sonar charge
    void dropNugget(); // attempt to drop a nugget
    bool nuggetDroppedHere(); // check if a nugget was dropped at this location already
    void fireSquirt(); // attempt to fire a squirt
};

// base class for both kinds of protestors
class ProtesterTemplate : public Actor {
public:
    // constructor that takes pointer to StudentWorld protestor belongs to, protestor type, and hit points
    ProtesterTemplate(StudentWorld* worldIn, int id, int hp, bool reg);

    // virtual destructor
    virtual ~ProtesterTemplate();

    virtual void doSomething(); // tells protesters what to do every tick

    // randomly generates number of steps for protester to walk
    int calcSteps();

    // returns number of ticks to wait before acting
    int getTicks();

    // changes number of ticks to wait before acting
    void changeTicks(int change);

    // returns if protester is stunned or not
    bool getStunned();

    // changes stun status
    void changeStunned(bool change);

    // returns a stack with directions from the current coordinatse to (targetX, targetY)
    std::stack<std::pair<int, int> > makePathTo(int targetX, int TargetY);

    // play an annoyed sound, used by other classes when damage is dealt
    void playAnnoyed();

    // virtual function
    // used when protesters pick up gold nuggets
    virtual void gotGold() = 0;

private:
    // struct to use for generating paths
    struct coord {
        // constructor
        coord(std::pair<int, int> m_xy, int dist)
        {
            xy = m_xy;
            num = dist;
        }

        std::pair<int, int> xy; // contains coordinates to move to
        int num; // contains number of steps away from start of path
    };

    // returns number of ticks to wait between moves for protester
    int calcTicks();

    // gets number of squares to move before stopping
    int getSquares();

    // changes number of squares to move before stopping
    void changeSquares(int change);

    // returns numbre of nonresting ticks to wait before shouting at player
    int getShoutCount();

    // changes numbre of nonresting ticks to wait before shouting at player
    void changeShoutCount(int change);

    // returns true if able to shout at player
    bool shoutPlayer();

    // get ticks to wait before turning at an intersection
    int getTurn();

    // changes ticks to wait before turning at an intersection
    void changeTurn(int change);

    // creates a path to the exit (60, 60) and places it into exitPath
    void makeExitPath();

    // returns pointer to exitPath
    std::stack<std::pair<int, int> >* getExitPath();

    // sets the direction of protesters in the direction of coord
    void setCoorDir(std::pair<int, int> coord);

    // returns the direction that the TunnelMan is in relative to the protester
    Direction getTunnelManDir();

    // return true if there is no dirt between the player and the protester
    // else return false
    bool noDirtBetweenPlayer();

    // return a random direction of the four existing
    Direction genDir();

    // returns true if it is possible to move in the direction dir
    // else return false
    bool checkDirMove(Direction dir);

    bool isReg; // holds if protester is a regular protester or not
    int ticksToWaitBetweenMoves; // ticks between moves, depending on level
    int numSquaresToMoveInCurrentDirection; // number of squares for protestor to move
    int shoutCount; // number of nonresting ticks before protester is allowed to shout
    int perpTurn; // number of nonresting ticks before protester is forced to turn at intersection
    bool isStunned; // if the protester is stunned or not
    int map[64][64]; // map to use when generating a path
    std::stack<std::pair<int, int> > exitPath; // will hold path to exit
};

// class for regular protestors
class RegularProtester : public ProtesterTemplate {
public:
    // constructor that accepts a pointer to the StudentWorld protestor belongs to
    RegularProtester(StudentWorld* worldIn);

    // virtual destructor
    virtual ~RegularProtester();

    // tells protester what to do when picking up gold
    // used by GoldNugget class
    virtual void gotGold();
};

// class for hardcore protesters
class HardProtester : public ProtesterTemplate {
public:
    // constructor that accepts a pointer to the StudentWorld protestor belongs to
    HardProtester(StudentWorld* worldIn);

    // virtual destructor
    virtual ~HardProtester();

    // tells protester what to do when picking up gold
    // used by GoldNugget class
    virtual void gotGold();

    // creates a path from current coordinates to TunnelMan
    void makePlayerPath();

    // returns pointer to path to player
    std::stack<std::pair<int, int> >* getPlayerPath();

private:
    std::stack<std::pair<int, int> > playerPath; // contains path to player
};

#endif // ACTOR_H_
