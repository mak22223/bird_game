#pragma once

class Character2D
{
public:
  Character2D() = delete;
  Character2D(const char* bitmap, char sizeX, char sizeY);

  char getSizeX() const;
  char getSizeY() const;
  void getSizeXY(char *x, char *y) const;
  int getPosX() const;
  int getPosY() const;
  void getPosXY(int *x, int *y) const;
  int getVelX() const;
  int getVelY() const;
  void getVelXY(int *x, int *y) const;
  const char* getBitmap() const;

  void setPosX(int x);
  void setPosY(int y);
  void setPosXY(int x, int y);
  void setVelX(int x);
  void setVelY(int y);
  void setVelXY(int x, int y);

protected:
  const char* d_bitmap;
  const char d_sizeX;
  const char d_sizeY;
  int d_posX;
  int d_posY;
  int d_velX;
  int d_velY;

};
