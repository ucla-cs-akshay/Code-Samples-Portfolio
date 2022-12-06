#include "Actor.h"
#include "StudentWorld.h"
#include <queue>

// obj constructor
obj::obj(int hp, int imageID, int startX, int startY, StudentWorld* worldIn, Direction dir, double size, unsigned int depth)
    : GraphObject(imageID, startX, startY, dir, size, depth)
{
    world = worldIn; // saves a pointer to the StudentWorld it belongs to

    health = hp; // either ticks left for object to stay on field OR hit points left for TunnelMan/Protesters
}

// destructor
obj::~obj()
{
}

// returns true if object should still stay on field, false otherwise
// overloaded only in the case of Boulder
bool obj::getStatus()
{
    return health > 0;
}

// returns StudentWorld pointer so that derived classes can call functions from it
StudentWorld* obj::getWorld()
{
    return world;
}

// alter the ticks / hp that an object/TunnelMan/Protester has left
void obj::changeHitPoints(int change)
{
    health += change;
}

// returns ticks/hit points
int obj::getHitPoints()
{
    return health;
}

// creates a new Earth object at (x, y). health is set to -1 because Earth is handled differently than the other objects
// in StudentWorld
Earth::Earth(int x, int y, StudentWorld* worldIn)
    : obj(-1, TID_EARTH, x, y, worldIn, GraphObject::right, 0.25, 3)
{
    setVisible(true); // make the Earth visible on spawning in
}

// destructor
Earth::~Earth()
{
}

// mandatory definition since is pure function in base class
// does not do anything
void Earth::doSomething()
{
}

// creates a nwe Boulder object at position (x, y)
Boulder::Boulder(int x, int y, StudentWorld* worldIn)
    : obj(30, TID_BOULDER, x, y, worldIn, GraphObject::down, 1, 1)
{
    setVisible(true); // spawns in as visible

    status = "stable"; // spawns in as stable, since there is dirt underneath it
}

// destructor
Boulder::~Boulder()
{
}

// called every tick during the game
// tells Boulder object what to do
void Boulder::doSomething()
{
    // if the Boulder object is already dead, do nothing
    if (!getStatus())
        return;

    // else if the Boulder is marked stable but there is no longer any dirt under it
    // mark it as waiting to fall
    if (!dirtUnder() && status == "stable")
        status = "waiting";

    // if the Boulder is waiting to fall
    if (status == "waiting") {
        // and if the Boulder has already waited the 30 ticks before falling
        // move the Boulder down one space per tick until it hits another Boulder, Earth, or the bottom of the game
        if (getHitPoints() == 0) {
            // get the Boulders current coordinates
            int x = getX();
            int y = getY();
            // get a pointer to the StudentWorld object to use its methods
            StudentWorld* temp = getWorld();

            // if there is Earth or a Boulder under the Boulder or if it is at the bottom of the screen
            if (dirtUnder() || y - 1 == -1) {
                status = "dead"; // set the Boulder as dead so it can be removed from the game

                // return immediately since the Boulder is dead
                return;
            }

            // at this point, the Boulder must be falling

            // move the Boulder down one square
            moveTo(x, y - 1);

            // check if the Boulder collides with anything whe falling and react accordingly
            checkMoveCollisions();
        }

        // else if the Boulder is still waiting to fall
        else {
            changeHitPoints(-1); // decrement the wait time for falling by one

            // if the Boulder is about to fall
            if (getHitPoints() == 0) {
                StudentWorld* temp = getWorld(); // get a pointer to StudentWorld

                // get the Boulder's current coordinates
                int x = getX();
                int y = getY();

                // remove the Boulder from the hash table
                for (int i = x; i < x + SPRITE_WIDTH; i++)
                    for (int j = y; j < y + SPRITE_HEIGHT; j++)
                        temp->changePixelArrID(i, j, -1);

                // play the Boulder falling sound
                getWorld()->playSound(SOUND_FALLING_ROCK);
            }
        }
    }
}

// returns if the Boulder should be removed from the game or not
bool Boulder::getStatus()
{
    return !(status == "dead"); // return if the status is not dead, aka if the Boulder is "alive"
}

// returns true if there is a dirt or boulder underneath this Boulder, else return false
bool Boulder::dirtUnder()
{
    // get Boulder's current x-coord and the y-coord directly below it
    int x = getX();
    int y = getY() - 1;

    // get pointer to the StudentWorld the Boulder belongs to
    StudentWorld* temp = getWorld();

    // check the hash table in StudentWorld if there are any Boulders or Earth directly below this Boulder
    for (int i = x; i < x + SPRITE_WIDTH; i++)
        // if there is at least one pixel of Earth/Boulder underneat, return true
        if (temp->getPixelArrID(i, y) == TID_EARTH || temp->getPixelArrID(i, y) == TID_BOULDER)
            return true;

    // if there is no Earth or Boulder under this Boulder, return false
    return false;
}

// while Boulder is moving, check if it hits any TunnelMan/Protesters and deplete their hp
void Boulder::checkMoveCollisions()
{
    std::list<obj*> actors = getWorld()->getActors(); // get list of actors in StudentWorld

    // iterate through each obj in actors
    std::list<obj*>::iterator it = actors.begin();
    while (it != actors.end()) {
        int itID = (*it)->getID(); // get the ID of the current actor

        // if the obj is a player or protester
        if (itID == TID_PLAYER || itID == TID_PROTESTER || itID == TID_HARD_CORE_PROTESTER) {
            // calculate the distance between this Boulder and the actor
            double dist = getWorld()->calcDist(getX(), getY(), (*it)->getX(), (*it)->getY());

            // if the distance is less than three
            if (dist <= 3.0) {
                // if the actor is not TunnelMan, then they must be a protester
                if (itID != TID_PLAYER) {
                    // downcast to use protester methods
                    ProtesterTemplate* temp = dynamic_cast<ProtesterTemplate*>(*it);

                    // if the protester is stunned, skip then
                    if (temp->getStunned()) {
                        it++;
                        continue;
                    }

                    // else if they are not stunned, increase the game's score
                    getWorld()->increaseScore(500);
                }

                (*it)->changeHitPoints(-100); // deplete the actor's hit points
            }
        }

        it++; // move onto the next obj in actors
    }
}

// abstract base class for Goods type objects that TunnelMan can pick up
// creates a derived Good object at (x, y) with the specificed hp lifetime with ID
// these objects always face right, havea size of 1, and a depth of 2
Goods::Goods(int x, int y, int hp, int ID, StudentWorld* worldIn)
    : obj(hp, ID, x, y, worldIn, right, 1, 2)
{
}

// destructor
Goods::~Goods()
{
}

// constructor for Barrel, a derived class from Goods
// creates a Barrel object at (x, y). hp/ticks on field is set to 1, but the class will not decrement it
// until TunnelMan picks up the barrel, so it stays indefinitely
// Barrel starts off as invisible
Barrel::Barrel(int x, int y, StudentWorld* worldIn)
    : Goods(x, y, 1, TID_BARREL, worldIn)
{
}

// destructor
Barrel::~Barrel()
{
}

// called every tick to tell the Barrel what to do
void Barrel::doSomething()
{
    // if the Barrel is dead, do nothing
    if (!getStatus())
        return;

    // get a pointer to the TunnelMan object
    TunnelMan* temp = getWorld()->getPlayer();

    // calculate the distance from the TunnelMan to this oil
    double dist = getWorld()->calcDist(getX(), getY(), temp->getX(), temp->getY());

    // if the oil is invisible and within 4 units of TunnelMan, set it to visible and immediately return
    if (!isVisible() && dist <= 4.0) {
        setVisible(true);
        return;
    }

    // if the oil is visible and is within 3 units of TunnelMan
    if (dist <= 3.0) {
        temp->decBarrels(); // tell TunnelMan it picked up a barrel

        getWorld()->playSound(SOUND_FOUND_OIL); // play a sound to indicated a barrel was found

        getWorld()->increaseScore(1000); // increase the game score by 1000

        changeHitPoints(-1); // make the Barrel's hp zero so it is deleted
    }
}

// constructor for Sonar objects that spawn in
// creates a Sonar object at (x, y)
// in StudentWorld, (x, y) is always (0, 60)
Sonar::Sonar(int x, int y, StudentWorld* worldIn)
    : Goods(x, y, 1, TID_SONAR, worldIn)
{
    // set the amount of ticks left on field to amount of ticks based on level
    int increaseAmount = calcTicks() - getHitPoints();
    changeHitPoints(increaseAmount);

    setVisible(true); // the Sonar spawns in as visible
}

// destructor
Sonar::~Sonar()
{
}

// tells the Sonar object what to do every tick
// either the health of the Sonar object is depleted until it reaches 0 and is removed from field
// or the TunnelMan picks it up
void Sonar::doSomething()
{
    // if the Sonar object is dead, immediately return
    if (!getStatus())
        return;

    // get a pointer to the TunnelMan object in the game
    TunnelMan* temp = getWorld()->getPlayer();

    // calculate the distance from Sonar to the TunnelMan
    double dist = getWorld()->calcDist(getX(), getY(), temp->getX(), temp->getY());

    // if the Sonar is within three units of TunnelMan
    if (dist <= 3.0) {
        changeHitPoints(-1 * calcTicks()); // set the Sonar's health to 0 so it is deleted from the field

        temp->changeSonar(1); // increase the amount of sonar charges held by TunnelMan

        getWorld()->playSound(SOUND_GOT_GOODIE); // play the found goodie sound

        getWorld()->increaseScore(75); // increase the game's score by 75
    }

    // else if the Sonar was not picked up, decrement its health by 1
    changeHitPoints(-1);
}

// calculates the ticks that Sonar should stay on screen based on the formula given in the specs
int Sonar::calcTicks()
{
    return ((100 > 300 - 10 * getWorld()->getLevel()) ? 100 : 300 - 10 * getWorld()->getLevel());
}

// constructor for GoldNuggets
// creates a GoldNugget object at (x, y) with a lifetime of ticks
// if prot is true, then:
//		protesters can pick up the object and TunnelMan cannot
//		the lifetime of the object is decremented per tick until a protester picks it up or it reaches 0 and is deleted
// else if prot is false, then:
//		protesters cannot pick up the object and TunnelMan can
//		the lifetime of the object is set to 1, but is not decremented per tick.
//		the object stays on screen until the level ends or until TunnelMan picks it up
GoldNugget::GoldNugget(int x, int y, int ticks, bool prot, StudentWorld* worldIn)
    : Goods(x, y, ticks, TID_GOLD, worldIn)
{
    protestersSee = prot;

    setVisible(protestersSee); // if protesters see it, is visible. else, is not visible until player is close enough
}

// destructor
GoldNugget::~GoldNugget()
{
}

// tells GoldNugget what to do every tick, and depending on if Protesters or TunnelMan can pick it up
void GoldNugget::doSomething()
{
    // if the object is dead, then immediately return
    if (!getStatus())
        return;

    // if the TunnelMan can pick up the GoldNugget
    if (!protestersSee) {
        // get a pointer to the TunnelMan
        TunnelMan* temp = getWorld()->getPlayer();

        // calculate the distance from the TunnelMan to the nugget
        double dist = getWorld()->calcDist(getX(), getY(), temp->getX(), temp->getY());

        // if the nugget is invisible and is <= 4 units away from TunnelMan, make it visible and immediately return
        if (!isVisible() && dist <= 4.0) {
            setVisible(true);
            return;
        }

        // if the nugget is visible and within three of from TunnelMan
        if (dist <= 3.0) {
            temp->changeNuggets(1); // increment the nuggets held by TunnelMan

            getWorld()->increaseScore(10); // increase the score by 10

            getWorld()->playSound(SOUND_GOT_GOODIE); // play the found goodie sound

            changeHitPoints(-1); // set the GoldNugget's health to 0 so it is deleted from the field

            return;
        }
    }

    // else the protesters can pick up the nugget
    else {
        changeHitPoints(-1); // decrement health per tick

        // get the coordinates of this object
        int x = getX();
        int y = getY();

        // get a list of objs in the game
        std::list<obj*> actors = getWorld()->getActors();

        // iterates through each obj in actors
        std::list<obj*>::iterator it = actors.begin();
        while (it != actors.end()) {
            // if the current obj is a protester
            if ((*it)->getID() == TID_PROTESTER || (*it)->getID() == TID_HARD_CORE_PROTESTER) {
                // calculate the distance from the protester to this object
                double dist = getWorld()->calcDist(x, y, (*it)->getX(), (*it)->getY());

                // if the object is within 3 units of the current obj
                if (dist <= 3.0) {
                    // the current obj must be a protester, so downcast it to get the protester to react accordingly
                    ProtesterTemplate* temp = dynamic_cast<ProtesterTemplate*>(*it);

                    // if the protester is already stunned, do nothing
                    if (temp->getStunned()) {
                        it++;
                        continue;
                    }

                    changeHitPoints(-100); // zero out this object's health to remove it from the field

                    temp->gotGold();

                    temp->changeStunned(true);

                    return;
                }
            }

            it++; // move onto the next obj in actors
        }
    }
}

// creates a new WaterPool object at (x, y) with the calculated lifetime
// always spawns in as visible, and (because of StudentWorld) only spawns in on empty space
WaterPool::WaterPool(int x, int y, StudentWorld* worldIn)
    : Goods(x, y, 1, TID_WATER_POOL, worldIn)
{
    // increase the lifetime of the object to the appropriate amount based on level
    int increaseAmount = calcTicks() - getHitPoints();
    changeHitPoints(increaseAmount);

    setVisible(true); // makes the object visible
}

// destructor
WaterPool::~WaterPool()
{
}

// tells the WaterPool object what to do every tick
void WaterPool::doSomething()
{
    // if the WaterPool is dead, immediately return
    if (!getStatus())
        return;

    // get a pointer to the TunnelMan in game
    TunnelMan* temp = getWorld()->getPlayer();

    // calculate the distance from TunnelMan to this WaterPool
    double dist = getWorld()->calcDist(getX(), getY(), temp->getX(), temp->getY());

    // if the distance is <= three units
    if (dist <= 3.0) {
        changeHitPoints(-1 * calcTicks()); // zero out this WaterPool's health to remove it from the field

        temp->increaseSquirts(); // increase the amount of water held by TunnelMan by 5

        getWorld()->playSound(SOUND_GOT_GOODIE); // play the got goodie sound

        getWorld()->increaseScore(100); // increase the score by 100

        return;
    }

    // else if the object was not picked up, decrement its health by 1
    changeHitPoints(-1);
}

// calculate the number of ticks for the WaterPool to stay on field according to the formula in the spec
int WaterPool::calcTicks()
{
    return ((100 > 300 - 10 * getWorld()->getLevel()) ? 100 : 300 - 10 * getWorld()->getLevel());
}

// creates a new Squirt object at (x, y) facing dir
Squirt::Squirt(int x, int y, GraphObject::Direction dir, StudentWorld* worldIn)
    : obj(5, TID_WATER_SPURT, x, y, worldIn, dir, 1, 1)
{
    setVisible(true); // spawns in as visible
}

// destructor
Squirt::~Squirt()
{
}

// tells Squirt what to do every tick
void Squirt::doSomething()
{
    // if the squirt is dead, return immediately
    if (!getStatus())
        return;

    // get the direction the Squirt is supposed to be moving in
    Direction move = getDirection();

    // sets x and y to the new coordinates that the Squirt is about to move to based on direction
    int x = getX();
    int y = getY();
    switch (move) {
    case left: {
        x += -1;
        break;
    }
    case right: {
        x += 1;
        break;
    }
    case up: {
        y += 1;
        break;
    }
    case down: {
        y += -1;
        break;
    }
    }

    // get a pointer to StudentWorld
    StudentWorld* temp = getWorld();

    // if the area the Squirt is about to occupy has Earth or Boulder, zero out the Squirt's health to remove it from the field
    // if the Squirt hits a Protester, tell the Protester to react accordingly using checkCollisions
    // in both cases, immediately return
    int tempX = x;
    int tempY = y;
    switch (move) {
    case up:
        tempY += 3;
    case down: {
        for (int i = tempX; i < tempX + SPRITE_WIDTH; i++) {
            int TID = temp->getPixelArrID(i, tempY);
            if (checkMoveCollisions() || y > 60 || y < 0 || TID == TID_BOULDER || TID == TID_EARTH) {
                changeHitPoints(-5);
                return;
            }
        }
        break;
    }
    case right:
        tempX += 3;
    case left: {
        for (int i = tempY; i < tempY + SPRITE_HEIGHT; i++) {
            int TID = temp->getPixelArrID(tempX, i);
            if (checkMoveCollisions() || x > 60 || x < 0 || TID == TID_BOULDER || TID == TID_EARTH) {
                changeHitPoints(-5);
                return;
            }
        }
        break;
    }
    }

    // if the code reaches this point, the Squirt is still in play and moves one unit in the correct direction
    moveTo(x, y);

    changeHitPoints(-1); // decrement the lifetime of the Squirt by 1
}

// check if the Squirt collides with any Protesters
bool Squirt::checkMoveCollisions()
{
    // get a list of objs in the game
    std::list<obj*> actors = getWorld()->getActors();

    // iterate through each obj in the game
    std::list<obj*>::iterator it = actors.begin();
    while (it != actors.end()) {
        int itID = (*it)->getID(); // get the ID of the current obj

        // if the ID is a Protester
        if (itID == TID_PROTESTER || itID == TID_HARD_CORE_PROTESTER) {
            // downcast the Protester obj
            ProtesterTemplate* temp = dynamic_cast<ProtesterTemplate*>(*it);

            // if this protester is already stunned, move onto the next item in the list
            if (temp->getStunned()) {
                it++;
                continue;
            }

            // calculate the distance between the Squirt and the Protester
            double dist = getWorld()->calcDist(getX(), getY(), (*it)->getX(), (*it)->getY());

            // if the distance is <= 3
            if (dist <= 3.0) {
                // retrieve the obj's health before and after getting squirted
                int currStat = (*it)->getHitPoints();
                (*it)->changeHitPoints(-2);
                int newStat = (*it)->getHitPoints();

                // increase the score accordingly if the Protester's health reaches 0
                if (currStat > 0 && newStat <= 0) {
                    if (itID == TID_PROTESTER)
                        getWorld()->increaseScore(100);
                    else
                        getWorld()->increaseScore(250);
                }

                // if the Protester is not dead, then stun it and make it sound annoyed
                if (newStat > 0) {
                    // tell the protester to get stunned
                    // if the protester is aleady stunned, reset their stun duration
                    int stunTime = (50 > 100 - getWorld()->getLevel() * 10) ? 50 : 100 - getWorld()->getLevel() * 10;
                    temp->changeTicks(stunTime - temp->getTicks());

                    // tell the protester to sound annoyed
                    temp->playAnnoyed();

                    temp->changeStunned(true); // set the protester to stunned
                }

                return true; // return true since a Protester was hit
            }
        }

        it++; // move onto the next obj in the list
    }

    return false; // return false since a Protester was not hit
}

// base class for TunnelMan and Protesters
Actor::Actor(StudentWorld* worldIn, int id, int x, int y, Direction dir, int hp)
    : obj(hp, id, x, y, worldIn, dir, 1, 0)
{
    setVisible(true); // always visible
    alive = true; // keeps track of whether actor is alive
}

// destructor
Actor::~Actor()
{
}

// return whether actor is alive
bool Actor::getStatus()
{
    return alive;
}

// mark that the actor is dead
void Actor::setDead()
{
    alive = false;
}

// if the actor is not facing dir, turn the actor to that direction
// if the actor is facing dir, movethe actor one unit in that direction if possible
// digEarth indicates if the actor can dig Earth or not
//		TunnelMan can dig Earth, so he is allowed to move through Earth
//		Protesters cannot, so they are not allowed to move through Earth
void Actor::moveDir(int dir, bool digEarth)
{
    // create a Direction variable and set it to none
    Direction move = none;

    // get the current actor's coordinates
    int x = getX();
    int y = getY();

    // change the coordinates and move accordingly depending on the inputted dir
    switch (dir) {
    case KEY_PRESS_LEFT: {
        move = left;
        x += -1;
        break;
    }
    case KEY_PRESS_RIGHT: {
        move = right;
        x += 1;
        break;
    }
    case KEY_PRESS_UP: {
        move = up;
        y += 1;
        break;
    }
    case KEY_PRESS_DOWN: {
        move = down;
        y += -1;
        break;
    }
    }

    // if the direction the actor is facing in is equal to the direction to be moved in
    if (getDirection() == move) {
        // if the area is out of bounds, immediately return
        if (x < 0 || x > 60 || y < 0 || y > 60)
            return;

        // get a pointer to StudentWorld
        StudentWorld* temp = getWorld();

        // create copies of the object's new coordinates
        int tempX = x;
        int tempY = y;

        // defaulted to false, used to check whether to play dig sound or not. only becomes true when Earth is dug
        bool pSound = false;

        // depending on the direction to be moved in
        // change tempX and tempY to check if the new intended actor position is occupied by Boulders or Earth
        // if the new position overlaps with a Boulder, return immediately
        // if the new positoin overlaps with Earth, if digEarth is true, then make the Earth invisible and
        // set pSound to true to play the dig sound. if digEarth is false, return immediately
        switch (move) {
        case up:
            tempY += 3;
        case down: {
            for (int i = tempX; i < tempX + SPRITE_WIDTH; i++) {
                if (temp->getPixelArrID(i, tempY) == TID_BOULDER || (!digEarth && temp->getPixelArrID(i, tempY) == TID_EARTH))
                    return;
                if (temp->getPixelArrID(i, tempY) == TID_EARTH) {
                    temp->setEarthInvis(i, tempY);
                    pSound = true;
                }
            }
            break;
        }
        case right:
            tempX += 3;
        case left: {
            for (int i = tempY; i < tempY + SPRITE_HEIGHT; i++) {
                if (temp->getPixelArrID(tempX, i) == TID_BOULDER || (!digEarth && temp->getPixelArrID(tempX, i) == TID_EARTH))
                    return;
                if (temp->getPixelArrID(tempX, i) == TID_EARTH) {
                    temp->setEarthInvis(tempX, i);
                    pSound = true;
                }
            }
            break;
        }
        }

        // if the code reaches this point, then the actor is able to move in this direction

        // play the dig sound depending on pSound
        if (pSound)
            getWorld()->playSound(SOUND_DIG);

        // move the actor to its new coordinates
        moveTo(x, y);
    }

    // else if the current direction is not equal to the intended direction and the intended direction is not none
    // set the current direction to move
    else if (move != none)
        setDirection(move);

    // if the code reaches this point, then move == none, so nothing should happen
}

// creates a new TunnelMan object, which always spawns in at (30, 60) with 5 water, 0 nuggets, and 1 sonar charge
// the barrels holds the amount of barrels to be collected, not the amount of barrels the player has
TunnelMan::TunnelMan(StudentWorld* worldIn, int numBarrels)
    : Actor(worldIn, TID_PLAYER, 30, 60, Direction::right, 10)
{
    // initialize TunnelMan's private variables to their default values
    squirts = 5;
    barrels = numBarrels;
    nuggets = 0;
    sonar = 1;
}

// destructor
TunnelMan::~TunnelMan()
{
}

// tells the TunnelMan what to do every tick
void TunnelMan::doSomething()
{
    // place the last pressed key into int x
    int x;
    getKey(x);

    // if player is dead or esc was pressed, set the player to dead, play the give up sound, then return
    if (!getStatus() || x == KEY_PRESS_ESCAPE) {
        getWorld()->playSound(SOUND_PLAYER_GIVE_UP);
        setDead();
        return;
    }

    // if a direction key was pressed, attempt to move TunnelMan in that direction using moveDir.
    // true is an input because TunnelMan is allowed to dig up earth
    if (x == KEY_PRESS_DOWN || x == KEY_PRESS_LEFT || x == KEY_PRESS_UP || x == KEY_PRESS_RIGHT)
        moveDir(x, true);

    // if space was pressed, attempt to fire a squirt in the direction TunnelMan is facing
    if (x == KEY_PRESS_SPACE)
        fireSquirt();

    // if tab was pressed, attempt to drop a nugget at TunnelMan's current location
    if (x == KEY_PRESS_TAB)
        dropNugget();

    // if z was pressed, attempt to use one sonar charge
    if (x == 'z' || x == 'Z')
        useSonar();
}

// return the number of barrels left for TunnelMan to collect
int TunnelMan::getBarrels()
{
    return barrels;
}

// decrease the amount of Barrels for TunnelMan to collect by 1
void TunnelMan::decBarrels()
{
    barrels--;
}

// return the number of sonar charges TunnelMan has
int TunnelMan::getSonar()
{
    return sonar;
}

// change the number of sonar charges TunnelMan has
void TunnelMan::changeSonar(int change)
{
    sonar += change;
}

// return the number of gold TunnelMan has
int TunnelMan::getNuggets()
{
    return nuggets;
}

// change the number of gold TunnelMan has
void TunnelMan::changeNuggets(int change)
{
    nuggets += change;
}

// return the number of water TunnelMan has
int TunnelMan::getSquirts()
{
    return squirts;
}

// increase the amount of water TunnelMan has by 5
void TunnelMan::increaseSquirts()
{
    squirts += 5;
}

// return whether TunnelMan is alive
bool TunnelMan::getStatus()
{
    return Actor::getStatus() && obj::getStatus();
}

// gets last pressed key
void TunnelMan::getKey(int& x)
{
    getWorld()->getKey(x);
}

// attempt to use one sonar charge
void TunnelMan::useSonar()
{
    // if the TunnelMan has no sonar charges, immediately return
    if (sonar <= 0)
        return;

    sonar--; // decrement the numbder of sonar charges by 1

    // get the TunnelMan's current coordinates
    int x1 = getX();
    int y1 = getY();

    // get the list of objs in play
    std::list<obj*> actors = getWorld()->getActors();

    // iterate through each obj in the list
    std::list<obj*>::iterator it = actors.begin();
    while (it != actors.end()) {
        // if the obj is not visible
        if (!(*it)->isVisible()) {
            // get the obj's coordinates
            int x2 = (*it)->getX();
            int y2 = (*it)->getY();

            // calculate the distance between the obj and TunnelMan
            double dist = getWorld()->calcDist(x1, y1, x2, y2);

            // if the obj is within 12 units of TunnelMan, make it visble
            if (dist <= 12.0)
                (*it)->setVisible(true);
        }

        it++; // move onto the next obj in the list
    }
}

// attempt to drop a nugget
void TunnelMan::dropNugget()
{
    // if TunnelMan has no nuggets or if a nugget was already dropped here, immediately return
    if (nuggets <= 0 || nuggetDroppedHere())
        return;

    nuggets--; // decrement the amount of nuggets held by TunnelMan

    // create a new GoldNugget object that is able to be picked up by protesters at TunnelMan's location
    // and add it to the list of actors
    GoldNugget* temp = new GoldNugget(getX(), getY(), 100, true, getWorld());
    getWorld()->getActors().push_back(temp);
}

// checks if a nugget was already dropped in this location
bool TunnelMan::nuggetDroppedHere()
{
    // gets list of obj in the game
    std::list<obj*> actors = getWorld()->getActors();

    // gets current coordinates
    int x = getX();
    int y = getY();

    // iterate through obj in game
    std::list<obj*>::iterator it = actors.begin();
    while (it != actors.end()) {
        // if an object is gold
        if ((*it)->getID() == TID_GOLD) {
            // get the coordinates of the gold
            int x1 = (*it)->getX();
            int y1 = (*it)->getY();

            // if there is any overlap with the bootom left corner, return true
            if (x1 >= x && x1 <= x + 3 && y1 >= y && y1 <= y + 3)
                return true;

            // if there is any overlap with the bottom right corner, return true
            x1 += 3;
            if (x1 >= x && x1 <= x + 3 && y1 >= y && y1 <= y + 3)
                return true;

            // if there is any overlap with the upper right corner, return true
            y1 += 3;
            if (x1 >= x && x1 <= x + 3 && y1 >= y && y1 <= y + 3)
                return true;

            // if there is any overlap with the upper left corner, return true
            x1 -= 3;
            if (x1 >= x && x1 <= x + 3 && y1 >= y && y1 <= y + 3)
                return true;
        }

        it++; // move onto the next obj
    }

    return false; // there is no overlap, so return false
}

// attempt to fire a squirt
void TunnelMan::fireSquirt()
{
    // if the TunnelMan has no water, return immediately
    if (squirts <= 0)
        return;

    squirts--; // decrement the amount of water that TunnelMan has

    // get the direction that the Squirt should face
    Direction move = getDirection();

    // get the coordinates of the TunnelMan
    int x = getX();
    int y = getY();

    // alter x and y based on move to get the coordinates that the Squirt should spawn onto
    switch (move) {
    case left: {
        x += -3;
        break;
    }
    case right: {
        x += 3;
        break;
    }
    case up: {
        y += 3;
        break;
    }
    case down: {
        y += -3;
        break;
    }
    }

    // create a new squirt object at (x, y) facing direction move and add it to the list of objs active in game
    Squirt* temp = new Squirt(x, y, move, getWorld());
    getWorld()->getActors().push_back(temp);

    getWorld()->playSound(SOUND_PLAYER_SQUIRT); // play the sound to signify that a squirt was used
}

// base class for Protesters to be derived from
// all Protesters start at (60, 60) facing left
ProtesterTemplate::ProtesterTemplate(StudentWorld* worldIn, int id, int hp, bool reg)
    : Actor(worldIn, id, 60, 60, Direction::left, hp)
{
    numSquaresToMoveInCurrentDirection = calcSteps(); // calculate number of squares to move

    // calculate ticks to wait in between moves based on formula in spec
    ticksToWaitBetweenMoves = 0;
    shoutCount = 0;
    perpTurn = 0;

    isStunned = false; // the protestesr does not begin stunned

    isReg = reg; // mark this protester depending on its type
}

// destructor
ProtesterTemplate::~ProtesterTemplate()
{
}

// tells protesters what to do every tick
// only one action is exclusive to hardcore protesters
void ProtesterTemplate::doSomething()
{
    // if protester is dead, immediately return
    if (!getStatus())
        return;

    // if protester's health is <= 0
    if (!obj::getStatus()) {
        // and if the protester is resting, decrement rest ticks and immediately return
        if (getTicks() > 0) {
            if (getTicks() > calcTicks())
                changeTicks(0 - getTicks());
            changeTicks(-1);
            return;
        }

        // if the protester is at the exit location, set it to dead
        // then immediately return so it can be deleted
        if (getX() == 60 && getY() == 60) {
            setDead();
            return;
        }

        // create a pointer to the exit path
        std::stack<std::pair<int, int> >* exit = getExitPath();

        // if the exit path is empty
        if (exit->empty()) {
            // then the protester's health just reached 0, so play its give up sound
            getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);

            // then create the path to the exit
            makeExitPath();
        }

        // once the exit path is created
        if (!exit->empty()) {
            // get the next coordinate in the path and delete it from the path
            std::pair<int, int> currCoord = exit->top();
            exit->pop();

            // make the protester face the new direction and move to the coordinates
            setCoorDir(currCoord);
            moveTo(currCoord.first, currCoord.second);
        }

        // the protester just acted, so reset their rest ticks
        changeTicks(calcTicks() - getTicks());

        return;
    }

    // if the protester is resting, decrement their rest ticks and immediately return
    if (getTicks() > 0) {
        changeTicks(-1);
        return;
    }

    Direction outdatedDir = getDirection(); // get current direction

    changeStunned(false); // if stunned, stun cooldow reached 0, so no longer stunned

    // if protester can shout at player
    if (shoutPlayer()) {
        // and the protester has not shouted in the last 15 ticks
        if (getShoutCount() <= 0) {
            // play the sound to indicate the protester yelling
            getWorld()->playSound(SOUND_PROTESTER_YELL);

            //decrement the TunnelMan's health by 2
            getWorld()->getPlayer()->changeHitPoints(-2);

            changeShoutCount(15 - getShoutCount()); // reset the shout tick count

            return;
        }

        // else the protester has shouted in the last 15 ticks

        // protester has attempted to act, so reset rest ticks
        changeTicks(calcTicks() - getTicks());

        // player was not able to shout, so decrement shout ticks and turn ticks, then immediately return
        changeShoutCount(-1);
        changeTurn(-1);

        return;
    }

    TunnelMan* player = getWorld()->getPlayer(); // create a pointer to the TunnelMan

    // section exclusive to hardcore protesters
    // action to take if protester is hardcore / not reg
    if (!isReg) {
        // is a hardcore protester, so downcast to use its methods
        HardProtester* temp = dynamic_cast<HardProtester*>(this);

        temp->makePlayerPath(); // create a path to the player

        std::stack<std::pair<int, int> >* playerPathTemp = temp->getPlayerPath();

        // if the path is less than a certain number of moves
        if (playerPathTemp->size() < (16 + getWorld()->getLevel() * 2) && !playerPathTemp->empty()) {
            Direction oldDir = getDirection(); // get soon to be old direction

            // tell the protester to move one step on the path towards the protester
            // if protester turns, reset the turn count (set to 201 because will be immediately decremented)
            setCoorDir(playerPathTemp->top());
            Direction dir = getDirection();
            int direction = 0;
            switch (dir) {
            case up:
                if (oldDir == right || oldDir == left)
                    changeTurn(201 - getTurn());
                direction = KEY_PRESS_UP;
                break;
            case down:
                if (oldDir == right || oldDir == left)
                    changeTurn(201 - getTurn());
                direction = KEY_PRESS_DOWN;
                break;
            case left:
                if (oldDir == up || oldDir == down)
                    changeTurn(201 - getTurn());
                direction = KEY_PRESS_LEFT;
                break;
            case right:
                if (oldDir == up || oldDir == down)
                    changeTurn(201 - getTurn());
                direction = KEY_PRESS_RIGHT;
                break;
            }
            moveDir(direction, false);

            changeSquares(-64); // reroll the travel direction on the next tick

            changeTicks(calcTicks() - getTicks()); // protester just acted, so reset rest ticks

            changeShoutCount(-1); // protester did not shout on a nonresting tick, so decrement shout count

            changeTurn(-1); // protester did not turn on a nonresting tick, so decrement turn count

            return;
        }
    }

    // if the protester can see the player and there is no dirt between them
    if ((player->getX() == getX() || player->getY() == getY()) && noDirtBetweenPlayer()) {
        Direction oldDir = getDirection(); // get soon to be old direction

        // get the direction the TunnelMan is in relative to the protester
        Direction dir = getTunnelManDir();
        setDirection(dir); // set the protester to face this direction

        // tell the protester to take a step in this direction
        // if direction is a perp turn, reset turn count
        // set to 201 because will be immediately decremented
        int direction = 0;
        switch (dir) {
        case up:
            if (oldDir == left || oldDir == right)
                changeTurn(201 - getTurn());
            direction = KEY_PRESS_UP;
            break;
        case down:
            if (oldDir == left || oldDir == right)
                changeTurn(201 - getTurn());
            direction = KEY_PRESS_DOWN;
            break;
        case left:
            if (oldDir == up || oldDir == down)
                changeTurn(201 - getTurn());
            direction = KEY_PRESS_LEFT;
            break;
        case right:
            if (oldDir == up || oldDir == down)
                changeTurn(201 - getTurn());
            direction = KEY_PRESS_RIGHT;
            break;
        }
        moveDir(direction, false);

        // reset the number of squares moved so the protester picks a new direction if player moves out of sight
        changeSquares(-64);

        // protester acted, so reset rest ticks
        changeTicks(calcTicks() - getTicks());

        // protester did not shout during a nonrest tick, so decrement shout ticks
        changeShoutCount(-1);

        // protester did not turn during a nonrest tick, so decrement turn ticks
        changeTurn(-1);

        return;
    }

    changeSquares(-1); // the protester will move a square, so decrement the squares to move

    // if the number of squares to move is <= 0
    if (getSquares() <= 0) {
        // randomly generate a new valid direction for the protester to face
        Direction dir = genDir();

        // make sure the protester can take at least one step in the new direction
        while (!checkDirMove(dir)) {
            dir = genDir();
        }

        setDirection(dir); // make the protester face this direction

        changeSquares(calcSteps() - getSquares()); // reset the squares to move in this direction
    }

    Direction facing = getDirection(); // get the direction that the protester is facing

    // if the protester has not turned in the last 200 nonresting ticks
    if (getTurn() <= 0) {
        // check the perpendicular directions to the direction the protester is facing
        switch (facing) {
        case right:
        case left: {
            // if the protester can turn in both perp directions, randomly choose one and make the TunnelMan face that dir
            if (checkDirMove(up) && checkDirMove(down)) {
                int newDir = getWorld()->RNG(1, 2);
                if (newDir == 1)
                    setDirection(up);
                else
                    setDirection(down);

                changeTurn(200 - getTurn()); // reset the turn ticks

                break;
            }

            // if the protester can turn in only one direction, set the protester to turn in that dir
            if (checkDirMove(up)) {
                setDirection(up);

                changeTurn(200 - getTurn()); // reset the turn ticks

                break;
            }

            // if the protester can turn in only one direction, set the protester to turn in that dir
            if (checkDirMove(down)) {
                setDirection(down);
                changeTurn(200 - getTurn()); // reset the turn ticks
                break;
            }
            break;
        }
        case up:
        case down: {
            // if the protester can turn in both perp directions, randomly choose one and make the TunnelMan face that dir
            if (checkDirMove(left) && checkDirMove(right)) {
                int newDir = getWorld()->RNG(1, 2);
                if (newDir == 1)
                    setDirection(left);
                else
                    setDirection(right);

                changeTurn(200 - getTurn()); // reset the turn ticks

                break;
            }

            // if the protester can turn in only one direction, set the protester to turn in that dir
            if (checkDirMove(left)) {
                setDirection(left);

                changeTurn(200 - getTurn()); // reset the turn ticks

                break;
            }

            // if the protester can turn in only one direction, set the protester to turn in that dir
            if (checkDirMove(right)) {
                setDirection(right);

                changeTurn(200 - getTurn()); // reset the turn ticks

                break;
            }
            break;
        }
        }
    }

    facing = getDirection(); // record the new direction that the protester is facing

    // if made a perpendicular turn, reset turn count
    // set to 201 because will be automatically decremented
    switch (outdatedDir) {
    case right:
    case left:
        if (facing == up || facing == down)
            changeTurn(201 - getTurn());
        break;
    case up:
    case down:
        if (facing == left || facing == right)
            changeTurn(201 - getTurn());
        break;
    }

    // get the protester's current coordinates
    int currX = getX();
    int currY = getY();

    // tell the protester to move one step in the direction it is facing
    switch (facing) {
    case up:
        moveDir(KEY_PRESS_UP, false);
        break;
    case down:
        moveDir(KEY_PRESS_DOWN, false);
        break;
    case left:
        moveDir(KEY_PRESS_LEFT, false);
        break;
    case right:
        moveDir(KEY_PRESS_RIGHT, false);
        break;
    }

    // get the protester's new coordinates
    int newX = getX();
    int newY = getY();

    // if for some reason the protester was not able to move, change its squares to move to 0
    // so it can choose a new direction next tic
    if (currX == newX && currY == newY)
        changeSquares(-64);

    changeTicks(calcTicks() - getTicks()); // protester just acted, so reset rest ticks

    changeShoutCount(-1); // protester did not shout on a nonresting tick, so decrement shout ticks

    changeTurn(-1); // protester did not turn on a nonresting tick, so decrement turn count
}

// calculates and returns the number of ticks to wait in between each move for protesters
int ProtesterTemplate::calcTicks()
{
    // formula based on the specs
    int level = getWorld()->getLevel();
    return (0 > 3 - level / 4 ? 0 : 3 - level / 4);
}

// calculate steps to move in current direction
int ProtesterTemplate::calcSteps()
{
    // randomized from 8 to 60
    return getWorld()->RNG(8, 60);
}

// returns number of ticks left to wait until moving
int ProtesterTemplate::getTicks()
{
    return ticksToWaitBetweenMoves;
}

// changes number of ticks to wait before moving
void ProtesterTemplate::changeTicks(int change)
{
    ticksToWaitBetweenMoves += change;
}

// get remaining number of squares to move in current direction
int ProtesterTemplate::getSquares()
{
    return numSquaresToMoveInCurrentDirection;
}

// changes number of squares to move in current direction
void ProtesterTemplate::changeSquares(int change)
{
    numSquaresToMoveInCurrentDirection += change;
}

// returns number of nonresting ticks to wait before shouting at player
int ProtesterTemplate::getShoutCount()
{
    return shoutCount;
}

// changes number of nonresting ticks to wait before shouting at player
void ProtesterTemplate::changeShoutCount(int change)
{
    shoutCount += change;
}

// if within 4 units of player facing player direction, return true
// else, cannot shout at plater and return false
bool ProtesterTemplate::shoutPlayer()
{
    TunnelMan* player = getWorld()->getPlayer();

    double dist = getWorld()->calcDist(getX(), getY(), player->getX(), player->getY());
    if (dist > 4.0)
        return false;

    Direction dir = getTunnelManDir();

    return getDirection() == dir || none == dir;
}

// get number of nonresting ticks before forcing a perpendicular turn
int ProtesterTemplate::getTurn()
{
    return perpTurn;
}

// changes number of nonresting ticks before forcing a perpendicular turn
void ProtesterTemplate::changeTurn(int change)
{
    perpTurn += change;
}

// returns stun status
bool ProtesterTemplate::getStunned()
{
    return isStunned;
}

// changes stun status
void ProtesterTemplate::changeStunned(bool change)
{
    isStunned = change;
}

// fills exitPath with coordinates from current location to exit point (60, 60)
void ProtesterTemplate::makeExitPath()
{
    exitPath = makePathTo(60, 60);
}

// returns pointer to exitPath
std::stack<std::pair<int, int> >* ProtesterTemplate::getExitPath()
{
    return &exitPath;
}

// returns a stack with coordinates showing the path from current location to (targetX, targetY)
std::stack<std::pair<int, int> > ProtesterTemplate::makePathTo(int targetX, int targetY)
{
    // creates a pointer to the StudentWorld this protester belongs to
    StudentWorld* temp = getWorld();

    // fills the map private member with a copy of the hash table of Earth and Boulders in StudentWorld
    // must be a copy because values will be changed when making path
    for (int i = 0; i < 64; i++)
        for (int j = 0; j < 64; j++)
            map[j][i] = temp->getPixelArrID(j, i);

    std::queue<coord> exit; // create a queue to use for BFS search

    // arbitrarily chosen, for use in creating a path for protesters
    const int FOUND = 500;

    exit.push(coord(std::pair<int, int>(getX(), getY()), FOUND)); // push the current coordinates into the queue

    int dist = FOUND; // pick an arbitrary variable to use in marking distance from current location

    map[getX()][getY()] = FOUND; // mark current location on map as found

    // while the queue is not empty
    while (!exit.empty()) {
        // get the front item of the queue
        coord currLoc = exit.front();

        // get the coordinates of the front item
        int currX = currLoc.xy.first;
        int currY = currLoc.xy.second;
        // get the distance of these coordinates from the current location
        dist = currLoc.num;

        // if target found, exit the loop
        if (currX == targetX && currY == targetY) {
            break;
        }

        exit.pop(); // remove front item from queue

        // if spot to right of coordinates is open and undiscovered
        if (currX + 1 >= 0 && currX + 1 <= 60 && currY >= 0 && currY <= 60 && map[currX + 1][currY] < FOUND && !getWorld()->dirtHere(currX + 1, currY)) {
            // push it into the queue and mark its location on the map with its distance from current location
            exit.push(coord(std::pair<int, int>(currX + 1, currY), dist + 1));
            map[currX + 1][currY] = dist + 1;
        }

        // if spot above coordinates is open and undiscovered
        if (currY + 1 >= 0 && currY + 1 <= 60 && currX >= 0 && currX <= 60 && map[currX][currY + 1] < FOUND && !getWorld()->dirtHere(currX, currY + 1)) {
            // push it into the queue and mark its location on the map with its distance from current location
            exit.push(coord(std::pair<int, int>(currX, currY + 1), dist + 1));
            map[currX][currY + 1] = dist + 1;
        }

        // if spot to left of coordinates is open and undiscovered
        if (currX - 1 >= 0 && currX - 1 <= 60 && currY >= 0 && currY <= 60 && map[currX - 1][currY] < FOUND && !getWorld()->dirtHere(currX - 1, currY)) {
            // push it into the queue and mark its location on the map with its distance from current location
            exit.push(coord(std::pair<int, int>(currX - 1, currY), dist + 1));
            map[currX - 1][currY] = dist + 1;
        }

        // if spot below  coordinates is open and undiscovered
        if (currY - 1 >= 0 && currY - 1 <= 60 && currX >= 0 && currX <= 60 && map[currX][currY - 1] < FOUND && !getWorld()->dirtHere(currX, currY - 1)) {
            // push it into the queue and mark its location on the map with its distance from current location
            exit.push(coord(std::pair<int, int>(currX, currY - 1), dist + 1));
            map[currX][currY - 1] = dist + 1;
        }
    }

    std::stack<std::pair<int, int> > returnQ; // create a stack to store the coordinates in the path

    // push target coordinates into stack
    returnQ.push(std::pair<int, int>(targetX, targetY));

    while (!(targetX == getX() && targetY == getY())) {
        dist--; // decrement dist to the dist that must be adjacent to (targetX, targetY) on map

        // if coordinate to the left on map contains dist
        targetX--;
        if (targetX >= 0 && targetX <= 60 && targetY >= 0 && targetY <= 60 && map[targetX][targetY] == dist) {
            //push this coordinate into the stack and continue finding the next coordinate
            returnQ.push(std::pair<int, int>(targetX, targetY));
            continue;
        }

        // if coordinate to the right on map contains dist
        targetX += 2;
        if (targetX >= 0 && targetX <= 60 && targetY >= 0 && targetY <= 60 && map[targetX][targetY] == dist) {
            //push this coordinate into the stack and continue finding the next coordinate
            returnQ.push(std::pair<int, int>(targetX, targetY));
            continue;
        }

        // if coordinate below on map contains dist
        targetX--;
        targetY--;
        if (targetX >= 0 && targetX <= 60 && targetY >= 0 && targetY <= 60 && map[targetX][targetY] == dist) {
            //push this coordinate into the stack and continue finding the next coordinate
            returnQ.push(std::pair<int, int>(targetX, targetY));
            continue;
        }

        // if coordinate above on map contains dist
        targetY += 2;
        if (targetX >= 0 && targetX <= 60 && targetY >= 0 && targetY <= 60 && map[targetX][targetY] == dist) {
            //push this coordinate into the stack and continue finding the next coordinate
            returnQ.push(std::pair<int, int>(targetX, targetY));
            continue;
        }
    }

    returnQ.pop(); // pop the top coordinate off of the stack, which is the current location of the protester

    return returnQ; // return the stack of coordinates to the location inputted
}

// get the direction the protester must face to face TunnelMan
GraphObject::Direction ProtesterTemplate::getTunnelManDir()
{
    // creates a pointer to TunnelMan and get its coordinates
    TunnelMan* player = getWorld()->getPlayer();
    int playerX = player->getX();
    int playerY = player->getY();

    // get coordinates of current location
    int x = getX();
    int y = getY();

    // if TunnelMan is to the right, return right
    if (playerX > x)
        return right;

    // if TunnelMan is to the left, return left
    else if (playerX < x)
        return left;

    // if TunnelMan is below, return down
    else if (playerY < y)
        return down;

    // if TunnelMan is above, return up
    else if (playerY > y)
        return up;
}

// set protester to face the direction of the entered coordinates
void ProtesterTemplate::setCoorDir(std::pair<int, int> coord)
{
    // get coordinates to face
    int newX = coord.first;
    int newY = coord.second;

    // get current coordinates
    int currX = getX();
    int currY = getY();

    // if curr coordinates is above, make protester face up
    if (newY > currY)
        setDirection(up);

    // if curr coordinates is below, make protester face down
    else if (newY < currY)
        setDirection(down);

    // if curr coordinates is right, make protester face right
    else if (newX > currX)
        setDirection(right);

    // if curr coordinates is left, make protester face left
    else if (newX < currX)
        setDirection(left);
}

// return true if there is a travelable straight line from curr coordinates to player
// else, return false
bool ProtesterTemplate::noDirtBetweenPlayer()
{
    // get the direction that the TunnelMan is facing
    Direction dir = getTunnelManDir();

    switch (dir) {
    // if the TunnelMan is above the protester
    case up: {
        // check if there is any Earth or Boulders in the coordinates from
        // current coordinates to TunnelMan's coordinates
        for (int i = getWorld()->getPlayer()->getY(); i >= getY(); i--)
            // if there is dirt at this location, return false
            if (getWorld()->dirtHere(getX(), i))
                return false;

        break;
    }

    // if TunnelMan is below the protester
    case down: {
        // check if there is any Earth or Boulders in the coordinates from
        // current coordinates to TunnelMan's coordinates
        for (int i = getWorld()->getPlayer()->getY(); i <= getY(); i++)
            // if there is dirt at this location, return false
            if (getWorld()->dirtHere(getX(), i))
                return false;

        break;
    }

    // if TunnelMan is to the left of the protester
    case left: {
        // check if there is any Earth or Boulders in the coordinates from
        // current coordinates to TunnelMan's coordinates
        for (int i = getWorld()->getPlayer()->getX(); i <= getX(); i++)
            // if there is dirt at this location, return false
            if (getWorld()->dirtHere(i, getY()))
                return false;

        break;
    }

    // if TunnelMan is to the right of the protester
    case right: {
        // check if there is any Earth or Boulders in the coordinates from
        // current coordinates to TunnelMan's coordinates
        for (int i = getWorld()->getPlayer()->getX(); i >= getX(); i--)
            // if there is dirt at this location, return false
            if (getWorld()->dirtHere(i, getY()))
                return false;

        break;
    }
    }

    return true; // no Earth of Boulders in between, so return true
}

// generate and return a random direction
GraphObject::Direction ProtesterTemplate::genDir()
{
    // generate a random number from 1 to 4
    int dir = getWorld()->RNG(1, 4);

    // return a direction depending on the number generating
    switch (dir) {
    case 1:
        return left;
    case 2:
        return right;
    case 3:
        return up;
    case 4:
        return down;
    }
}

// returns true if protester can take a step in direction dir
bool ProtesterTemplate::checkDirMove(Direction dir)
{
    // get current coordinates
    int x = getX();
    int y = getY();

    // adjust coordinates to the new pixel location protester is about to occupy
    switch (dir) {
    case right: {
        x += 4;
        break;
    }
    case left: {
        x -= 1;
        break;
    }
    case up: {
        y += 4;
        break;
    }
    case down: {
        y -= 1;
        break;
    }
    }

    // check if the new pixels being occupied are in bounds and not Earth or Boulders
    switch (dir) {
    // in the left and right directions
    case right:
    case left: {
        // for each new pixel occupised
        for (int i = y; i < y + SPRITE_HEIGHT; i++) {
            int ID = getWorld()->getPixelArrID(x, i); // get the pixel's ID

            // if the new pixel location is invalid, return false
            if (ID == TID_BOULDER || ID == TID_EARTH || ID == OUT_OF_BOUNDS)
                return false;
        }

        break;
    }

    // in the up and down directions
    case up:
    case down: {
        // for each new pixel occupied
        for (int i = x; i < x + SPRITE_WIDTH; i++) {
            int ID = getWorld()->getPixelArrID(i, y); // get the pixel's ID

            // if the new pixel location is invalid, return false
            if (ID == TID_BOULDER || ID == TID_EARTH || ID == OUT_OF_BOUNDS)
                return false;
        }

        break;
    }
    }

    return true; // if the code reaches this point, it is possible to move in dir, so return true
}

// play an annoyed sound
// used by other classes that deal dmg to protester
void ProtesterTemplate::playAnnoyed()
{
    getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
}

// normal protester constructor
RegularProtester::RegularProtester(StudentWorld* worldIn)
    : ProtesterTemplate(worldIn, TID_PROTESTER, 5, true)
{
}

// destructor
RegularProtester::~RegularProtester()
{
}

// tells regular protester what to do when it picks up a gold nugget
void RegularProtester::gotGold()
{
    getWorld()->increaseScore(25); // increase score for bribing protester

    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD); // play sound to indicate gold was picked up

    changeHitPoints(-5); // tell the protester to leave the field
}

// hardcore protester class constructor
HardProtester::HardProtester(StudentWorld* worldIn)
    : ProtesterTemplate(worldIn, TID_HARD_CORE_PROTESTER, 20, false)
{
}

// destructor
HardProtester::~HardProtester()
{
}

// tells HardProtester what to do when it picks up a gold nugget
void HardProtester::gotGold()
{
    getWorld()->increaseScore(50); // increase score for distracting protester

    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD); // play sound to indicate gold was picked up

    // increase duration that protester rests because it is busy looking at gold
    changeTicks(((50 > 100 - getWorld()->getLevel() * 10) ? 50 : 100 - getWorld()->getLevel() * 10) - getTicks());
}

// set playerPath to contain a path leading to the player
void HardProtester::makePlayerPath()
{
    TunnelMan* temp = getWorld()->getPlayer(); // create a pointer to TunnelMan

    // set playerPath to a stack containing a path to TunnelMan's coordinates
    playerPath = makePathTo(temp->getX(), temp->getY());
}

// returns pointer to path to TunnelMan
std::stack<std::pair<int, int> >* HardProtester::getPlayerPath()
{
    return &playerPath;
}