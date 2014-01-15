//
// C++ Interface: prosilica
//
// Description:
//
//
// Author: Arnaud Sevin <Arnaud.Sevin@obspm.fr>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PROSILICA_H
#define PROSILICA_H

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <vector>

#include <time.h>
#include <sys/time.h>		/* gettimeofday() */

#include <PvApi.h>
#include <ImageLib.h>

#include "yapi.h"

using namespace std;

/**
   @author Arnaud Sevin <Arnaud.Sevin@obspm.fr>
*/

typedef struct tCamera {
  unsigned long   UID;
  tPvHandle       Handle;
  tPvFrame        Frame;
} tCamera;


// wait for a camera to be plugged
void waitForCamera();

// get the first camera found
void cameraGet(tCamera* camera);

// open the camera
void cameraSetup(tCamera* camera);

// setup and start streaming
void cameraStart(tCamera* camera, tPvUint32 packetSize);

// stop streaming
void cameraStop(tCamera* camera);

// snap and save a frame from the camera
void cameraSnap(tCamera* camera);
tPvFrame *cameraSnap(tCamera* camera, int nbSnap);
void cameraFreeBuf(tPvFrame* buff, int nbSnap);

// unsetup the camera
void cameraUnsetup(tCamera* camera);

string cameraGetError(tPvErr Err);

void cameraSetPixelFormat(tCamera* camera, string format);
void cameraGetPixelFormat(tCamera* camera, char *format, const int formatSize);

void cameraSetExpo(tCamera* camera, tPvUint32 expo);
void cameraGetExpo(tCamera* camera, tPvUint32 *expo);

void cameraSetROI(tCamera* camera, tPvUint32 *ROI);
void cameraGetROI(tCamera* camera, tPvUint32 *ROI);

extern "C" {

  void Y_camera_obj(int argc);
  void camera_free(void *obj);

  void camera_eval(void *obj, int argc);
  void camera_extract(void *obj, char *func);

  void Y_camera_start(int argc);
  void Y_camera_stop(int argc);

  void Y_camera_setExpo(int argc);
  void Y_camera_getExpo(int argc);

  void Y_camera_setROI(int argc);
  void Y_camera_getROI(int argc);
};

#endif
