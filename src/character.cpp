#include "character.hpp"

Character2D::Character2D(const char* bitmap, char sizeX, char sizeY)
: d_bitmap(bitmap), d_sizeX(sizeX), d_sizeY(sizeY),
  d_posX(0), d_posY(0), d_velX(0), d_velY(0) {}

char Character2D::getSizeX() const
{
  return d_sizeX;
}

char Character2D::getSizeY() const
{
  return d_sizeY;
}

void Character2D::getSizeXY(char *x, char *y) const
{
  *x = d_sizeX;
  *y = d_sizeY;
}

int Character2D::getPosX() const
{
  return d_posX;
}

int Character2D::getPosY() const
{
  return d_posY;
}

void Character2D::getPosXY(int *x, int *y) const
{
  *x = d_posX;
  *y = d_posY;
}

int Character2D::getVelX() const
{
  return d_velX;
}

int Character2D::getVelY() const
{
  return d_velY;
}

void Character2D::getVelXY(int *x, int *y) const
{
  *x = d_velX;
  *y = d_velY;
}

const char* Character2D::getBitmap() const
{
  return d_bitmap;
}

void Character2D::setPosX(int x)
{
  d_posX = x;
}

void Character2D::setPosY(int y)
{
  d_posY = y;
}

void Character2D::setPosXY(int x, int y)
{
  d_posX = x;
  d_posY = y;
}

void Character2D::setVelX(int x)
{
  d_velX = x;
}

void Character2D::setVelY(int y)
{
  d_velY = y;
}

void Character2D::setVelXY(int x, int y)
{
  d_velX = x;
  d_velY = y;
}
