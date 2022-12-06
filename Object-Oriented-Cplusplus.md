### C++ (Object Oriented) code sample notebook
---




```cpp

/** Actor.cpp */

#include "Actor.h"
#include "StudentWorld.h"


Actor::Actor(int imageID, int x, int y, Direction dir, int depth, bool visible,
             StudentWorld* world)
  : GraphObject(imageID, x, y, dir, 1, depth)
{
  m_alive = true;
  setVisible(visible);
  m_world = world;
}

Actor::~Actor() {}

bool Actor::isAlive() { return m_alive; }

void Actor::makeDead() { m_alive = false; }

StudentWorld* Actor::getWorld() { return m_world; }

bool Actor::touchingEarth(Direction dir,
                          int dist) {  // return true if actor is going to touch
                                       // earth if they move dist spaces in
                                       // direction specified
  switch (dir) {
    case left:
      for (int x = 1; x <= dist; x++) {
        for (int y = 0; y <= TUNNELMAN_SIZE; y++) {
          if (getWorld()->isThereEarth(getX() - x, getY() + y) ||
              getWorld()->outOfBounds(getX() - x, getY() + y)) {
            return true;
          }
        }
      }
      break;
    case right:
      for (int x = 1; x <= dist; x++) {
        for (int y = 0; y <= TUNNELMAN_SIZE; y++) {
          if (getWorld()->isThereEarth(getX() + x + TUNNELMAN_SIZE,
                                       getY() + y) ||
              getWorld()->outOfBounds(getX() + x + TUNNELMAN_SIZE,
                                      getY() + y)) {
            return true;
          }
        }
      }
      break;
    case up:
      for (int y = 1; y <= dist; y++) {
        for (int x = 0; x <= TUNNELMAN_SIZE; x++) {
          if (getWorld()->isThereEarth(getX() + x,
                                       getY() + y + TUNNELMAN_SIZE) ||
              getWorld()->outOfBounds(getX() + x,
                                      getY() + y + TUNNELMAN_SIZE)) {
            return true;
          }
        }
      }
      break;
    case down:
      for (int y = 1; y <= dist; y++) {
        for (int x = 0; x <= TUNNELMAN_SIZE; x++) {
          if (getWorld()->isThereEarth(getX() + x, getY() - y) ||
              getWorld()->outOfBounds(getX() + x, getY() - y)) {
            return true;
          }
        }
      }
      break;
  }
  return false;
}

Earth::Earth(int x, int y) : GraphObject(TID_EARTH, x, y, right, 0.25, 3) {
  setVisible(true);
}

Earth::~Earth() {}

Human::Human(int imageID, int x, int y, Direction dir, int hp,
             StudentWorld* world)
    : Actor(imageID, x, y, dir, 0, true, world) {
  m_hp = hp;
}

Human::~Human() {}

int Human::getHp() { return m_hp; }

void Human::annoy(int hpLoss) { m_hp -= hpLoss; }

TunnelMan::TunnelMan(StudentWorld* world)
    : Human(TID_PLAYER, 30, 60, right, 10, world) {
  m_water = 5;
  m_sonar = 1;
  m_gold = 0;
}

TunnelMan::~TunnelMan() {}

void TunnelMan::doSomething() {
  if (!isAlive()) {
    return;
  }
  bool dug = false;
  for (int y = 0; y <= TUNNELMAN_SIZE; y++) {
    for (int x = 0; x <= TUNNELMAN_SIZE; x++) {
      if (getWorld()->isThereEarth(getX() + x, getY() + y)) {
        getWorld()->removeEarth(getX() + x, getY() + y);
        dug = true;
      }
    }
  }
  if (dug) {
    getWorld()->playSound(SOUND_DIG);
  } else {
    int ch;
    if (getWorld()->getKey(ch) == true) {
      switch (ch) {
        case KEY_PRESS_ESCAPE:
          makeDead();
          break;
        case KEY_PRESS_SPACE:
          if (getWater() > 0) {
            if (getDirection() == left) {
              if (!touchingEarth(left, 4) &&
                  !getWorld()->touchingBoulder(getX() - 4, getY())) {
                getWorld()->addWaterSpurt(getX() - 4, getY());
              }
            } else if (getDirection() == right) {
              if (!touchingEarth(right, 4) &&
                  !getWorld()->touchingBoulder(getX() + 4, getY())) {
                getWorld()->addWaterSpurt(getX() + 4, getY());
              }
            } else if (getDirection() == up) {
              if (!touchingEarth(up, 4) &&
                  !getWorld()->touchingBoulder(getX(), getY() + 4)) {
                getWorld()->addWaterSpurt(getX(), getY() + 4);
              }
            } else if (getDirection() == down) {
              if (!touchingEarth(down, 4) &&
                  !getWorld()->touchingBoulder(getX(), getY() - 4)) {
                getWorld()->addWaterSpurt(getX(), getY() - 4);
              }
            }
            getWorld()->playSound(SOUND_PLAYER_SQUIRT);
            m_water--;
          }
          break;
        case KEY_PRESS_LEFT:
          if (getDirection() == left && getX() - 1 >= TUNNELMAN_MIN_CORD &&
              !getWorld()->touchingBoulder(getX() - 1, getY())) {
            moveTo(getX() - 1, getY());
          } else {
            setDirection(left);
          }
          break;
        case KEY_PRESS_RIGHT:
          if (getDirection() == right && getX() + 1 <= TUNNELMAN_MAX_CORD &&
              !getWorld()->touchingBoulder(getX() + 1, getY())) {
            moveTo(getX() + 1, getY());
          } else {
            setDirection(right);
          }
          break;
        case KEY_PRESS_UP:
          if (getDirection() == up && getY() + 1 <= TUNNELMAN_MAX_CORD &&
              !getWorld()->touchingBoulder(getX(), getY() + 1)) {
            moveTo(getX(), getY() + 1);
          } else {
            setDirection(up);
          }
          break;
        case KEY_PRESS_DOWN:
          if (getDirection() == down && getY() - 1 >= TUNNELMAN_MIN_CORD &&
              !getWorld()->touchingBoulder(getX(), getY() - 1)) {
            moveTo(getX(), getY() - 1);
          } else {
            setDirection(down);
          }
          break;
        case 'z':
        case 'Z':
          if (getSonar() > 0) {
            getWorld()->sonarScan(getX(), getY());
            m_sonar--;
          }
          break;
        case KEY_PRESS_TAB:
          if (getGold() > 0) {
            getWorld()->dropGold(getX(), getY());
            m_gold--;
          }
      }
    }
    if (getHp() <= 0) {
      makeDead();
      getWorld()->playSound(SOUND_PLAYER_GIVE_UP);
    }
  }
}

int TunnelMan::getWater() { return m_water; }

void TunnelMan::incWater() { m_water += 5; }

int TunnelMan::getSonar() { return m_sonar; }

void TunnelMan::incSonar() { m_sonar++; }

int TunnelMan::getGold() { return m_gold; }

void TunnelMan::incGold() { m_gold++; }

Protester::Protester(int numSquares, StudentWorld* world, int imageID, int hp)
    : Human(imageID, 60, 60, left, hp, world) {
  m_numSquares = numSquares;
  m_leaveField = false;
  m_ticksSinceLastMove = 0;
  m_ticksSinceLastShout = 0;
  m_ticksSinceLastTurn = 0;
  m_hardcore = false;
}

Protester::~Protester() {}

void Protester::doSomething() { protMove(false); }

void Protester::makeHardcore() { m_hardcore = true; }

bool Protester::getHardcore() { return m_hardcore; }

void Protester::protMove(bool hardcore) {
  if (!isAlive()) {
    return;
  }
  int ticksToWait = std::max(PROT_WAIT, 3 - getWorld()->getLevel() / 4);
  if (m_ticksSinceLastMove < ticksToWait) {
    m_ticksSinceLastMove++;
    return;
  }
  m_ticksSinceLastMove = 0;
  m_ticksSinceLastShout++;
  m_ticksSinceLastTurn++;
  if (m_leaveField) {
    if (getX() == 60 && getY() == 60) {
      makeDead();
      getWorld()->decNumProts();
      return;
    } else {
      getWorld()->moveTowardExit(getX(), getY(), this);
      return;
    }
  } else {
    Direction dirOfTm;
    if (m_ticksSinceLastShout >= 15 && facingTm(getDirection()) &&
        getWorld()->touchingTm(getX(), getY(), 2, 4)) {
      getWorld()->playSound(SOUND_PROTESTER_YELL);
      m_ticksSinceLastShout = 0;
      return;
    } else if (hardcore && !getWorld()->touchingTm(getX(), getY(), 0, 4) &&
               getWorld()->moveTowardTm(getX(), getY(), this)) {
      return;
    } else if (tmInSight(dirOfTm) &&
               !getWorld()->touchingTm(getX(), getY(), 0, 4)) {
      setDirection(dirOfTm);
      switch (dirOfTm) {
        case left:
          moveTo(getX() - 1, getY());
          break;
        case right:
          moveTo(getX() + 1, getY());
          break;
        case up:
          moveTo(getX(), getY() + 1);
          break;
        case down:
          moveTo(getX(), getY() - 1);
          break;
      }
      m_numSquares = 0;
      return;
    } else {
      m_numSquares--;
      if (m_numSquares <= 0) {
        Direction newDir;
        bool cannotMove = true;
        while (cannotMove) {
          int d = rand() % 4;  // btw 0 and 3
          switch (d) {
            case 0:
              newDir = left;
              if (!touchingEarth(left, 1) &&
                  !getWorld()->touchingBoulder(getX() - 1, getY())) {
                cannotMove = false;
              }
              break;
            case 1:
              newDir = right;
              if (!touchingEarth(right, 1) &&
                  !getWorld()->touchingBoulder(getX() + 1, getY())) {
                cannotMove = false;
              }
              break;
            case 2:
              newDir = up;
              if (!touchingEarth(up, 1) &&
                  !getWorld()->touchingBoulder(getX(), getY() + 1)) {
                cannotMove = false;
              }
              break;
            case 3:
              newDir = down;
              if (!touchingEarth(down, 1) &&
                  !getWorld()->touchingBoulder(getX(), getY() - 1)) {
                cannotMove = false;
              }
              break;
          }
        }
        setDirection(newDir);
        m_numSquares = rand() % 53 + 8;  // btw 8 and 60
      } else {
        if (m_ticksSinceLastTurn >= 200) {
          int d = rand() % 2;  // 0 or 1
          if (getDirection() == left || getDirection() == right) {
            if (d == 0) {
              if (!touchingEarth(up, 1) &&
                  !getWorld()->touchingBoulder(getX(), getY() + 1)) {
                setDirection(up);
                m_ticksSinceLastTurn = 0;
              } else if (!touchingEarth(down, 1) &&
                         !getWorld()->touchingBoulder(getX(), getY() - 1)) {
                setDirection(down);
                m_ticksSinceLastTurn = 0;
              }
            } else {
              if (!touchingEarth(down, 1) &&
                  !getWorld()->touchingBoulder(getX(), getY() - 1)) {
                setDirection(down);
                m_ticksSinceLastTurn = 0;
              } else if (!touchingEarth(up, 1) &&
                         !getWorld()->touchingBoulder(getX(), getY() + 1)) {
                setDirection(up);
                m_ticksSinceLastTurn = 0;
              }
            }
          } else if (getDirection() == up || getDirection() == down) {
            if (d == 0) {
              if (!touchingEarth(right, 1) &&
                  !getWorld()->touchingBoulder(getX() + 1, getY())) {
                setDirection(right);
                m_ticksSinceLastTurn = 0;
              } else if (!touchingEarth(left, 1) &&
                         !getWorld()->touchingBoulder(getX() - 1, getY())) {
                setDirection(left);
                m_ticksSinceLastTurn = 0;
              }
            } else {
              if (!touchingEarth(left, 1) &&
                  !getWorld()->touchingBoulder(getX() - 1, getY())) {
                setDirection(left);
                m_ticksSinceLastTurn = 0;
              } else if (!touchingEarth(right, 1) &&
                         !getWorld()->touchingBoulder(getX() + 1, getY())) {
                setDirection(right);
                m_ticksSinceLastTurn = 0;
              }
            }
          }
          m_numSquares = rand() % 53 + 8;  // btw 8 and 60
        }
      }
      if (!touchingEarth(getDirection(), 1)) {
        switch (getDirection()) {
          case left:
            if (!getWorld()->touchingBoulder(getX() - 1, getY())) {
              moveTo(getX() - 1, getY());
            } else {
              m_numSquares = 0;
            }
            break;
          case right:
            if (!getWorld()->touchingBoulder(getX() + 1, getY())) {
              moveTo(getX() + 1, getY());
            } else {
              m_numSquares = 0;
            }
            break;
          case up:
            if (!getWorld()->touchingBoulder(getX(), getY() + 1)) {
              moveTo(getX(), getY() + 1);
            } else {
              m_numSquares = 0;
            }
            break;
          case down:
            if (!getWorld()->touchingBoulder(getX(), getY() - 1)) {
              moveTo(getX(), getY() - 1);
            } else {
              m_numSquares = 0;
            }
            break;
        }
      } else {
        m_numSquares = 0;
      }
    }
  }
}

void Protester::bribed() {
  m_leaveField = true;
  m_ticksSinceLastMove = 51;
}

bool Protester::facingTm(Direction dir) {
  int x = getWorld()->getTm()->getX();
  int y = getWorld()->getTm()->getY();
  switch (dir) {
    case left:
      if (getX() >= x && getY() == y) {
        return true;
      }
      break;
    case right:
      if (getX() <= x && getY() == y) {
        return true;
      }
      break;
    case up:
      if (getX() == x && getY() <= y) {
        return true;
      }
      break;
    case down:
      if (getX() == x && getY() >= y) {
        return true;
      }
      break;
  }
  return false;
}

bool Protester::tmInSight(Direction& dir) {
  int x = getWorld()->getTm()->getX();
  int y = getWorld()->getTm()->getY();
  if (getY() == y) {
    if (getX() >= x) {
      dir = left;
      int i = 0;
      while (getX() - i >= x) {
        for (int yy = 0; yy <= TUNNELMAN_SIZE; yy++) {
          if (getWorld()->isThereEarth(getX() - i, getY() + yy) ||
              getWorld()->touchingBoulder(getX() - i, getY() + yy)) {
            return false;
          }
        }
        i++;
      }
    } else if (getX() < x) {
      dir = right;
      int i = 0;
      while (getX() + i <= x) {
        for (int yy = 0; yy <= TUNNELMAN_SIZE; yy++) {
          if (getWorld()->isThereEarth(getX() + i, getY() + yy) ||
              getWorld()->touchingBoulder(getX() + i, getY() + yy)) {
            return false;
          }
        }
        i++;
      }
    }
  } else if (getX() == x) {
    if (getY() >= y) {
      dir = down;
      int i = 0;
      while (getY() - i >= y) {
        for (int xx = 0; xx <= TUNNELMAN_SIZE; xx++) {
          if (getWorld()->isThereEarth(getX() + xx, getY() - i) ||
              getWorld()->touchingBoulder(getX() + xx, getY() - i)) {
            return false;
          }
        }
        i++;
      }
    } else if (getY() < y) {
      dir = up;
      int i = 0;
      while (getY() + i <= y) {
        for (int xx = 0; xx <= TUNNELMAN_SIZE; xx++) {
          if (getWorld()->isThereEarth(getX() + xx, getY() + i) ||
              getWorld()->touchingBoulder(getX() + xx, getY() + i)) {
            return false;
          }
        }
        i++;
      }
    }
  } else {
    return false;
  }
  return true;
}

void Protester::stun(unsigned int length) {
  m_ticksSinceLastMove = length * -1;
}

void Protester::leaving(int dir) {
  switch (dir) {
    case LEFT:
      setDirection(left);
      moveTo(getX() - 1, getY());
      break;
    case RIGHT:
      setDirection(right);
      moveTo(getX() + 1, getY());
      break;
    case UP:
      setDirection(up);
      moveTo(getX(), getY() + 1);
      break;
    case DOWN:
      setDirection(down);
      moveTo(getX(), getY() - 1);
      break;
  }
}

HardcoreProtester::HardcoreProtester(int numSquares, StudentWorld* world)
    : Protester(numSquares, world, TID_HARD_CORE_PROTESTER, 20) {
  makeHardcore();
}

HardcoreProtester::~HardcoreProtester() {}

void HardcoreProtester::doSomething() { protMove(true); }

Object::Object(int imageID, int x, int y, Direction dir, int depth,
               bool visible, StudentWorld* world)
    : Actor(imageID, x, y, dir, depth, visible, world) {}

Object::~Object() {}

PickupableObject::PickupableObject(int imageID, int x, int y, bool visible,
                                   int state, StudentWorld* world)
    : Object(imageID, x, y, right, 2, visible, world) {
  m_state = state;
  m_pickupableByTm = true;
  m_ticksSinceActivated = 0;
}

PickupableObject::~PickupableObject() {}

void PickupableObject::setPickupable(bool pickupable) {
  m_pickupableByTm = pickupable;
}

bool PickupableObject::pickupableByTm() { return m_pickupableByTm; }

int PickupableObject::getState() { return m_state; }

unsigned int PickupableObject::getTicks() { return m_ticksSinceActivated; }

void PickupableObject::incTicks() { m_ticksSinceActivated++; }

Barrel::Barrel(int x, int y, StudentWorld* world)
    : PickupableObject(TID_BARREL, x, y, false, PERMANENT, world) {}

Barrel::~Barrel() {}

void Barrel::doSomething() {
  if (!isAlive()) {
    return;
  }
  if (!isVisible() && getWorld()->touchingTm(getX(), getY(), 0, 4.0)) {
    setVisible(true);
    return;
  }
  if (getWorld()->touchingTm(getX(), getY(), 0, 3.0)) {
    makeDead();
    getWorld()->playSound(SOUND_FOUND_OIL);
    getWorld()->increaseScore(1000);
    getWorld()->decBarrels();
  }
}

Gold::Gold(int x, int y, bool visible, int state, StudentWorld* world)
    : PickupableObject(TID_GOLD, x, y, visible, state, world) {}

Gold::~Gold() {}

void Gold::doSomething() {
  if (!isAlive()) {
    return;
  }
  if (!isVisible() && getWorld()->touchingTm(getX(), getY(), 0, 4)) {
    setVisible(true);
    return;
  }
  if (pickupableByTm() && getWorld()->touchingTm(getX(), getY(), 0, 3)) {
    makeDead();
    getWorld()->playSound(SOUND_GOT_GOODIE);
    getWorld()->increaseScore(10);
    getWorld()->getTm()->incGold();
  } else if (!pickupableByTm() &&
             getWorld()->touchingProt(getX(), getY(), 0, false, true)) {
    makeDead();
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
  } else if (getState() == TEMPORARY) {
    incTicks();
    if (getTicks() >= GOODIE_LIFE) {
      makeDead();
    }
  }
}

Sonar::Sonar(int x, int y, StudentWorld* world)
    : PickupableObject(TID_SONAR, x, y, true, TEMPORARY, world) {}

Sonar::~Sonar() {}

void Sonar::doSomething() {
  if (!isAlive()) {
    return;
  }
  incTicks();
  if (getWorld()->touchingTm(getX(), getY(), 0, 3)) {
    makeDead();
    getWorld()->playSound(SOUND_GOT_GOODIE);
    getWorld()->getTm()->incSonar();
    getWorld()->increaseScore(75);
  } else if (getTicks() >=
             std::max(GOODIE_LIFE, 300 - 10 * getWorld()->getLevel())) {
    makeDead();
  }
}

WaterPool::WaterPool(int x, int y, StudentWorld* world)
    : PickupableObject(TID_WATER_POOL, x, y, true, TEMPORARY, world) {}

WaterPool::~WaterPool() {}

void WaterPool::doSomething() {
  if (!isAlive()) {
    return;
  }
  incTicks();
  if (getWorld()->touchingTm(getX(), getY(), 0, 3)) {
    makeDead();
    getWorld()->playSound(SOUND_GOT_GOODIE);
    getWorld()->getTm()->incWater();
    getWorld()->increaseScore(100);
  } else if (getTicks() >=
             std::max(GOODIE_LIFE, 300 - 10 * getWorld()->getLevel())) {
    makeDead();
  }
}

AnnoyingObject::AnnoyingObject(int imageID, int x, int y, Direction dir,
                               StudentWorld* world)
    : Object(imageID, x, y, dir, 1, true, world) {}

AnnoyingObject::~AnnoyingObject() {}

Boulder::Boulder(int x, int y, StudentWorld* world)
    : AnnoyingObject(TID_BOULDER, x, y, down, world) {
  m_stable = STABLE;
  m_ticksWaiting = 0;
}

Boulder::~Boulder() {}

void Boulder::doSomething() {
  if (!isAlive()) {
    return;
  }
  if (getStability() == STABLE) {
    bool earth = false;
    for (int i = 0; i <= OBJECT_SIZE; i++) {
      if (getWorld()->isThereEarth(getX() + i, getY() - 1)) {
        earth = true;
        break;
      }
    }
    if (!earth) {
      changeStability(WAITING);
    }
  } else if (getStability() == WAITING) {
    m_ticksWaiting++;
    if (m_ticksWaiting >= 30) {
      changeStability(FALLING);
      getWorld()->playSound(SOUND_FALLING_ROCK);
      getWorld()->removeBoulder();
    }
  } else if (getStability() == FALLING) {
    bool earth = false;
    for (int i = 0; i <= OBJECT_SIZE; i++) {
      if (getWorld()->isThereEarth(getX() + i, getY() - 1)) {
        earth = true;
        break;
      }
    }
    if (getY() == 0) {
      earth = true;
    }
    if (!earth && !getWorld()->touchingBoulder(getX(), getY() - 1)) {
      moveTo(getX(), getY() - 1);
      getWorld()->touchingProt(getX(), getY(), 100, true, false);
      getWorld()->touchingTm(getX(), getY(), 100, 3.0);
    } else {
      makeDead();
    }
  }
}

int Boulder::getStability() { return m_stable; }

void Boulder::changeStability(int stability) { m_stable = stability; }

WaterSpurt::WaterSpurt(int x, int y, Direction dir, StudentWorld* world)
    : AnnoyingObject(TID_WATER_SPURT, x, y, dir, world) {
  m_travelDist = 4;
}

WaterSpurt::~WaterSpurt() {}

void WaterSpurt::doSomething() {
  if (getWorld()->touchingProt(getX(), getY(), 2, false, false) ||
      m_travelDist <= 0) {
    makeDead();
  } else {
    Direction dir = getDirection();
    switch (dir) {
      case left:
        if (touchingEarth(left, 1) ||
            getWorld()->touchingBoulder(getX() - 1, getY()) ||
            getWorld()->outOfBounds(getX() - 1, getY())) {
          makeDead();
        } else {
          moveTo(getX() - 1, getY());
          m_travelDist--;
        }
        break;
      case right:
        if (touchingEarth(right, 1) ||
            getWorld()->touchingBoulder(getX() + 1, getY()) ||
            getWorld()->outOfBounds(getX() + 1, getY())) {
          makeDead();
        } else {
          moveTo(getX() + 1, getY());
          m_travelDist--;
        }
        break;
      case up:
        if (touchingEarth(up, 1) ||
            getWorld()->touchingBoulder(getX(), getY() + 1) ||
            getWorld()->outOfBounds(getX(), getY() + 1)) {
          makeDead();
        } else {
          moveTo(getX(), getY() + 1);
          m_travelDist--;
        }
        break;
      case down:
        if (touchingEarth(down, 1) ||
            getWorld()->touchingBoulder(getX(), getY() - 1) ||
            getWorld()->outOfBounds(getX(), getY() - 1)) {
          makeDead();
        } else {
          moveTo(getX(), getY() - 1);
          m_travelDist--;
        }
        break;
    }
  }
}
```


```cpp
/**
  - this for that and that
  */
class SpriteManager {

private:
  bool m_mipMapped;  /** https://en.wikipedia.org/wiki/Mipmap */
  std::map<unsigned int, GLuint> m_imageMap;
  std::map<unsigned int, unsigned int> m_frameCountPerSprite;

  static const int INVALID_SPRITE_ID = -1;
  static const int MAX_IMAGES = 1000;
  static const int MAX_FRAMES_PER_SPRITE = 100;

  int getSpriteID(unsigned int imageID, unsigned int frame) const
  {
    if (imageID >= MAX_IMAGES || frame >= MAX_FRAMES_PER_SPRITE)
      return INVALID_SPRITE_ID;

    return imageID * MAX_FRAMES_PER_SPRITE + frame;
  }


public:
  SpriteManager() : m_mipMapped(true) {}

  void setMipMapping(bool status) { m_mipMapped = status; }

  bool loadSprite(std::string filename_tga, int imageID, int frameNum)
  {
    // Load Texture Data From TGA File
    unsigned int spriteID = getSpriteID(imageID, frameNum);
    if (INVALID_SPRITE_ID == spriteID) return false;

    // keep track of how many frames per sprite we loaded
    m_frameCountPerSprite[imageID]++;


    std::string line;
    std::string contents = "";
    std::ifstream tgaFile(filename_tga, std::ios::in | std::ios::binary);

    if (!tgaFile) return false;

    char type[3];
    char info[6];
    unsigned char byteCount;
    unsigned int textureWidth;
    unsigned int textureHeight;
    long imageSize;
    char* imageData = nullptr;

    // Read file header info
    tgaFile.read(type, 3);
    tgaFile.seekg(12);
    tgaFile.read(info, 6);
    textureWidth = static_cast<unsigned char>(info[0]) +
                   static_cast<unsigned char>(info[1]) * 256;
    textureHeight = static_cast<unsigned char>(info[2]) +
                    static_cast<unsigned char>(info[3]) * 256;
    byteCount = static_cast<unsigned char>(info[4]) / 8;
    imageSize = textureWidth * textureHeight * byteCount;
    imageData = new char[imageSize];
    tgaFile.seekg(18);
    // Read image data
    tgaFile.read(imageData, imageSize);
    if (!tgaFile) {
      delete[] imageData;
      return false;
    }

    // image type either 2 (color) or 3 (greyscale)
    if (type[1] != 0 || (type[2] != 2 && type[2] != 3)) return false;

    if (byteCount != 3 && byteCount != 4) return false;

    // Transfer Texture To OpenGL

    glEnable(GL_DEPTH_TEST);

    // allocate a texture handle
    GLuint glTextureID;
    glGenTextures(1, &glTextureID);

    // bind our new texture
    glBindTexture(GL_TEXTURE_2D, glTextureID);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    if (m_mipMapped) {
      // when texture area is small, bilinear filter the closest mipmap
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR_MIPMAP_LINEAR);
      // when texture area is large, bilinear filter the first mipmap
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                      GL_LINEAR_MIPMAP_LINEAR);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    // Have the texture wrap both vertically and horizontally.
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    static_cast<GLfloat>(GL_REPEAT));
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    static_cast<GLfloat>(GL_REPEAT));

    if (m_mipMapped) {
      // build our texture mipmaps
      // byteCount of 3 means that BGR data is being supplied. byteCount of 4
      // means that BGRA data is being supplied.
      if (3 == byteCount)
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, textureWidth, textureHeight, GL_BGR,
                          GL_UNSIGNED_BYTE, imageData);
      else if (4 == byteCount)
        gluBuild2DMipmaps(GL_TEXTURE_2D, 4, textureWidth, textureHeight,
                          GL_BGRA, GL_UNSIGNED_BYTE, imageData);
    } else {
      // byteCount of 3 means that BGR data is being supplied. byteCount of 4
      // means that BGRA data is being supplied.
      if (3 == byteCount)
        glTexImage2D(GL_TEXTURE_2D, 0, 3, textureWidth, textureHeight, 0,
                     GL_BGR, GL_UNSIGNED_BYTE, imageData);
      else if (4 == byteCount)
        glTexImage2D(GL_TEXTURE_2D, 0, 4, textureWidth, textureHeight, 0,
                     GL_BGRA, GL_UNSIGNED_BYTE, imageData);
    }

    delete[] imageData;

    m_imageMap[spriteID] = glTextureID;

    return true;
  }

  unsigned int getNumFrames(int imageID) const {
    auto it = m_frameCountPerSprite.find(imageID);
    if (it == m_frameCountPerSprite.end()) return 0;

    return it->second;
  }

  enum Angle {
    degrees_0 = 0,
    degrees_90 = 90,
    degrees_180 = 180,
    degrees_270 = 270,
    face_left = 1,
    face_right = 2,
    face_up = 3,
    face_down = 4
  };

  bool plotSprite(int imageID, int frame, double gx, double gy, double gz,
                  Angle angleDegrees, double size) {
    unsigned int spriteID = getSpriteID(imageID, frame);
    if (INVALID_SPRITE_ID == spriteID) return false;

    auto it = m_imageMap.find(spriteID);
    if (it == m_imageMap.end()) return false;

    glPushMatrix();

    double finalWidth, finalHeight;

    finalWidth = SPRITE_WIDTH_GL * size;
    finalHeight = SPRITE_HEIGHT_GL * size;

    // object's x/y location is center-based, but sprite plotting is
    // upper-left-corner based
    const double xoffset = finalWidth / 2;
    const double yoffset = finalHeight / 2;

    glTranslatef(static_cast<GLfloat>(gx - xoffset),
                 static_cast<GLfloat>(gy - yoffset), static_cast<GLfloat>(gz));
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, it->second);

    glColor3f(1.0, 1.0, 1.0);

    double cx1, cx2, cx3, cx4;
    double cy1, cy2, cy3, cy4;

    switch (angleDegrees) {
      default:
      case degrees_0:
      case face_right:
        cx1 = 0;
        cy1 = 0;
        cx2 = 1;
        cy2 = 0;
        cx3 = 1;
        cy3 = 1;
        cx4 = 0;
        cy4 = 1;
        break;
      case face_left:
        cx1 = 1;
        cy1 = 0;
        cx2 = 0;
        cy2 = 0;
        cx3 = 0;
        cy3 = 1;
        cx4 = 1;
        cy4 = 1;
        break;
      case degrees_90:
      case face_up:
        cx1 = 0;
        cy1 = 1;
        cx2 = 0;
        cy2 = 0;
        cx3 = 1;
        cy3 = 0;
        cx4 = 1;
        cy4 = 1;
        break;
      case degrees_180:
        cx1 = 1;
        cy1 = 1;
        cx2 = 0;
        cy2 = 1;
        cx3 = 0;
        cy3 = 0;
        cx4 = 1;
        cy4 = 0;
        break;
      case degrees_270:
      case face_down:
        cx1 = 1;
        cy1 = 0;
        cx2 = 1;
        cy2 = 1;
        cx3 = 0;
        cy3 = 1;
        cx4 = 0;
        cy4 = 0;
        break;
    }

    glBegin(GL_QUADS);
    glTexCoord2d(cx1, cy1);
    glVertex3f(0, 0, 0);
    glTexCoord2d(cx2, cy2);
    glVertex3f(static_cast<GLfloat>(finalWidth), 0, 0);
    glTexCoord2d(cx3, cy3);
    glVertex3f(static_cast<GLfloat>(finalWidth),
               static_cast<GLfloat>(finalHeight), 0);
    glTexCoord2d(cx4, cy4);
    glVertex3f(0, static_cast<GLfloat>(finalHeight), 0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glPopAttrib();
    glPopMatrix();

    return true;
  }

  ~SpriteManager() {
    for (auto it = m_imageMap.begin(); it != m_imageMap.end(); it++)
      glDeleteTextures(1, &it->second);
  }

};

```