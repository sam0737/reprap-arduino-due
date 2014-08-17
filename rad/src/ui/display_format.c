/*
    RAD - Copyright (C) 2013 Sam Wong

    This file is part of RAD project.

    RAD is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    RAD is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "ch.h"
#include "hal.h"
#include "rad.h"

static char conv[8];

char *itostr3(const int x)
{
  if (x >= 100)
    conv[0]=(x/100)%10+'0';
  else
    conv[0]=' ';
  if (x >= 10)
    conv[1]=(x/10)%10+'0';
  else
    conv[1]=' ';
  conv[2]=(x)%10+'0';
  conv[3]=0;
  return conv;
}

char *itostr3left(const int xx)
{
  if (xx >= 100)
  {
    conv[0]=(xx/100)%10+'0';
    conv[1]=(xx/10)%10+'0';
    conv[2]=(xx)%10+'0';
    conv[3]=0;
  }
  else if (xx >= 10)
  {
    conv[0]=(xx/10)%10+'0';
    conv[1]=(xx)%10+'0';
    conv[2]=0;
  }
  else
  {
    conv[0]=(xx)%10+'0';
    conv[1]=0;
  }
  return conv;
}

char *itostr2(const int xx)
{
  conv[0]=(xx/10)%10+'0';
  conv[1]=(xx)%10+'0';
  conv[2]=0;
  return conv;
}

char *ftostr42(const float x)
{
  int32_t xx=x*100;
  if (xx >= 0)
    conv[0]=(xx/100000)%10+'0';
  else
    conv[0]='-';
  xx=fabs(xx);
  conv[1]=(xx/10000)%10+'0';
  conv[2]=(xx/1000)%10+'0';
  conv[3]=(xx/100)%10+'0';
  conv[4]='.';
  conv[5]=(xx/10)%10+'0';
  conv[6]=(xx)%10+'0';
  conv[7]=0;
  return conv;
}

char *ftostr42best(const float x)
{
  uint8_t pos = 0;
  int32_t xx=x*100;
  if (xx < 0)
    conv[pos++]='-';
  xx=fabs(xx);
  if (xx > 100000)
    conv[pos++]=(xx/100000)%10+'0';
  if (xx > 10000)
    conv[pos++]=(xx/10000)%10+'0';
  if (xx > 1000)
    conv[pos++]=(xx/1000)%10+'0';
  conv[pos++]=(xx/100)%10+'0';
  conv[pos++]='.';
  conv[pos++]=(xx/10)%10+'0';
  conv[pos++]=(xx)%10+'0';
  conv[pos++]=0;
  return conv;
}

char *itospace(uint8_t space, int xx)
{
  space--;
  while (xx >= 10 && space)
  {
    space--;
    xx /= 10;
  }
  uint8_t i;
  for (i = 0; i < space; i++)
  {
    conv[i] = ' ';
  }
  conv[i] = 0;
  return conv;
}
