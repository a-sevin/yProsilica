//
// C++ Implementation: prosilica
//
// Description:
//
//
// Author: Arnaud Sevin <Arnaud.Sevin@obspm.fr>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "prosilica.h"

void __nsleep(const struct timespec *req, struct timespec *rem) {
  struct timespec temp_rem;
  if (nanosleep(req, rem) == -1)
    __nsleep(rem, &temp_rem);
}

int msleep(unsigned long milisec) {
  struct timespec req = { 0 }, rem = { 0 };
  time_t sec = (int) (milisec / 1000);
  milisec = milisec - (sec * 1000);
  req.tv_sec = sec;
  req.tv_nsec = milisec * 1000000L;
  __nsleep(&req, &rem);
  return 1;
}

// wait for a camera to be plugged
void waitForCamera() {
  cout << "waiting for a camera (10s)";
  int index = 0;
  while (!PvCameraCount() && index < 40) {
    cout << "." << flush;
    msleep(250);
    index++;
  }
  cout << endl;
  if (index == 40)
    throw "failed to find a camera\n";
}

vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while(getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    return split(s, delim, elems);
}

// get the first camera found
void cameraGet(tCamera* camera) {
  tPvUint32 count, connected;
  tPvCameraInfo list;

  count = PvCameraList(&list, 1, &connected);
  if (count == 1) {
    camera->UID = list.UniqueId;
    cout << "got camera " << list.SerialString << endl;
  } else
    throw "failed to find a camera\n";
}

// open the camera
void cameraSetup(tCamera* camera) {
  tPvErr err = PvCameraOpen(camera->UID, ePvAccessMaster, &(camera->Handle));
  if (err != ePvErrSuccess){
    stringstream buf;
    buf << "failed to setup the camera : " << cameraGetError(err) << endl;
    throw buf.str();
  }
}

// define pixel format
void cameraSetPixelFormat(tCamera* camera, string format) {
  tPvErr err = PvAttrEnumSet(camera->Handle, "PixelFormat", format.c_str());
  if (err != ePvErrSuccess){
    stringstream buf;
    buf << "failed to define pixel format : " << cameraGetError(err) << endl;
    throw buf.str();
  }
}

// get pixel format
void cameraGetPixelFormat(tCamera* camera, char *format, const int formatSize) {
  long unsigned int resSize;
  tPvErr err = PvAttrEnumGet(camera->Handle, "PixelFormat", format, formatSize, &resSize);
  if (err != ePvErrSuccess){
    stringstream buf;
    buf << "failed to get pixel format : " << cameraGetError(err) << endl;
    throw buf.str();
  }
}

// setup and start streaming
void cameraStart(tCamera* camera, tPvUint32 packetSize) {
  cameraSetPixelFormat(camera, "Mono16");
  cameraSetExpo(camera, 1000);

  //tPvUint32 ROI[4]={0,0,493,659};
  //cameraSetROI(camera,ROI);

  // Auto adjust the packet size to max supported by the network, up to a max of 8228.
  // NOTE: In Vista, if the packet size on the network card is set lower than 8228,
  //       this call may break the network card's driver. See release notes.
  //

  tPvErr err = PvCaptureAdjustPacketSize(camera->Handle,packetSize);
  //err = PvAttrUint32Set(camera->Handle, "PacketSize", 1500);
  if (err != ePvErrSuccess){
    stringstream buf;
    buf << "failed to adjust packet size : " << cameraGetError(err) << endl;
    throw buf.str();
  }

  unsigned long FrameSize = 0;
  err = PvAttrUint32Get(camera->Handle, "TotalBytesPerFrame", &FrameSize);
  if (err != ePvErrSuccess){
    stringstream buf;
    buf << "failed to get TotalBytesPerFrame : " << cameraGetError(err) << endl;
    throw buf.str();
  }

  // allocate the buffer for the single frame we need
  camera->Frame.Context[0] = camera;
  camera->Frame.ImageBuffer = new char[FrameSize];
  if (camera->Frame.ImageBuffer)
    camera->Frame.ImageBufferSize = FrameSize;
  else
    throw "failed allocate Frame.ImageBuffer";


  // how big should the frame buffers be?
  if (!err) {
    // set the camera is capture mode
    if (!PvCaptureStart(camera->Handle)) {
      // set the camera in continuous acquisition mode
      if (!PvAttrEnumSet(camera->Handle, "FrameStartTriggerMode",
			 "Freerun")) { //"FixedRate" / "Freerun"
	// and set the acquisition mode into continuous
	if (PvCommandRun(camera->Handle, "AcquisitionStart")) {
	  // if that fail, we reset the camera to non capture mode
	  PvCaptureEnd(camera->Handle);
	  throw "failed to set the acquisition mode into continuous";
	}
      } else
	throw "failed to set the camera in continuous acquisition mode";
    } else
      throw "failed to set the camera is capture mode";
  } else
    throw "failed to get TotalBytesPerFrame parameter";
}

// stop streaming
void cameraStop(tCamera* camera) {
  PvCommandRun(camera->Handle, "AcquisitionStop");
  PvCaptureEnd(camera->Handle);
  // and free the image buffer of the frame
  delete[] (char*) camera->Frame.ImageBuffer;
}

// snap and save a frame from the camera
void cameraSnap(tCamera* camera) {

  if (!PvCaptureQueueFrame(camera->Handle, &(camera->Frame), NULL)) {
    int index = 0;
    while ((PvCaptureWaitForFrameDone(camera->Handle, &(camera->Frame), 100) == ePvErrTimeout)
	   && (index < 10)){
      index++;
    }
    if (camera->Frame.Status != ePvErrSuccess)
      cout << "the frame failed to be captured : "<< cameraGetError(camera->Frame.Status) << endl;
  } else
    cout << "failed to enqueue the frame" << endl;
}

// snap and save a frame from the camera
void cameraFreeBuf(tPvFrame* buff, int nbSnap) {
  for(int i=0; i<nbSnap; i++) {
    delete[] (char*) buff[i].ImageBuffer;
  }
  delete buff;
}

long my_difftime (struct timeval * start, struct timeval * end)
{
  
  if (start->tv_sec == end->tv_sec) {
    return end->tv_usec - start->tv_usec;
  }
  else {
    long usecs = 1000000 - start->tv_usec;
    long secs = end->tv_sec - (start->tv_sec + 1);
    usecs += end->tv_usec;
    if (usecs >= 1000000) {
      usecs -= 1000000;
      secs += 1;
    }
    return usecs+secs*1000000;
  }
}


// snap and save a frame from the camera
tPvFrame *cameraSnap(tCamera* camera, int nbSnap) {
  unsigned long FrameSize = 0;
  PvAttrUint32Get(camera->Handle, "TotalBytesPerFrame", &FrameSize);

  tPvFrame *frames = new tPvFrame[nbSnap];
  for(int i=0; i<nbSnap; i++) {
    // allocate the buffer for the single frame we need
    frames[i].Context[0] = camera;
    frames[i].ImageBuffer = new char[FrameSize];
    if (frames[i].ImageBuffer)
      frames[i].ImageBufferSize = FrameSize;
    else
      throw "failed allocate Frame.ImageBuffer";
    
  }
  /*
  cerr<<"Warming up !\n";
  PvCaptureQueueFrame(camera->Handle, &(camera->Frame), NULL);
  while(camera->Frame.Status != ePvErrSuccess)
    PvCaptureQueueFrame(camera->Handle, &(camera->Frame), NULL);
  */

  struct timeval myTVstart, myTVend;
  gettimeofday (&myTVstart, NULL);
  for(int i=0; i<nbSnap; i++) {
    if (!PvCaptureQueueFrame(camera->Handle, &(frames[i]), NULL)) {
      int index = 0;
      while ((PvCaptureWaitForFrameDone(camera->Handle, &(frames[i]), 10) == ePvErrTimeout)
	     && (index < 100)){
	index++;
      }
      if (frames[i].Status != ePvErrSuccess)
	cout << "the frame failed to be captured : "<< cameraGetError(frames[i].Status) << endl;
    } else
      cout << "failed to enqueue the frame" << endl;
  }
  gettimeofday (&myTVend, NULL);
  double duration = my_difftime (&myTVstart, &myTVend)/1000000.;

  cout << "Acuisition tooks " << duration*1000 << " mseconds, (" << (double)nbSnap/duration << " Hz)" << endl;
  return frames;
}

// unsetup the camera
void cameraUnsetup(tCamera* camera) {
  // dequeue all the frame still queued (this will block until they all have been dequeued)
  PvCaptureQueueClear(camera->Handle);
  // then close the camera
  PvCameraClose(camera->Handle);
}

void cameraSetExpo(tCamera* camera, tPvUint32 expo) {
  if (PvAttrUint32Set(camera->Handle, "ExposureValue", expo))
    throw "failed to set ExposureValue parameter";
}

void cameraGetExpo(tCamera* camera, tPvUint32 *expo) {
  if (PvAttrUint32Get(camera->Handle, "ExposureValue", expo)) 
    throw "failed to get ExposureValue parameter";
}

void cameraSetROI(tCamera* camera, tPvUint32 *ROI) {
  if(PvAttrUint32Set(camera->Handle, "RegionX", ROI[0]))
    throw "failed to set RegionX parameter";
  if(PvAttrUint32Set(camera->Handle, "RegionY", ROI[1]))
    throw "failed to set RegionY parameter";
  if(PvAttrUint32Set(camera->Handle, "Width",   ROI[2]))
    throw "failed to set Width parameter";
  if(PvAttrUint32Set(camera->Handle, "Height",  ROI[3]))
    throw "failed to set Height parameter";
}

void cameraGetROI(tCamera* camera, tPvUint32 *ROI) {
  if(PvAttrUint32Get(camera->Handle, "RegionX", &ROI[0]))
    throw "failed to get RegionX parameter";
  if(PvAttrUint32Get(camera->Handle, "RegionY", &ROI[1]))
    throw "failed to get RegionY parameter";
  if(PvAttrUint32Get(camera->Handle, "Width",   &ROI[2]))
    throw "failed to get Width parameter";
  if(PvAttrUint32Get(camera->Handle, "Height",  &ROI[3]))
    throw "failed to get Height parameter";
}

string cameraGetError(tPvErr Err) {
  switch (Err) {
  case ePvErrSuccess:
    return "Success\n";
  case ePvErrCameraFault:
    return "Unexpected camera fault\n";
  case ePvErrInternalFault:
    return "Unexpected fault in PvApi or driver\n";
  case ePvErrBadHandle:
    return "Camera handle is invalid\n";
  case ePvErrBadParameter:
    return "Bad parameter to API call\n";
  case ePvErrBadSequence:
    return "Sequence of API calls is incorrect\n";
  case ePvErrNotFound:
    return "Camera or attribute not found\n";
  case ePvErrAccessDenied:
    return "Camera cannot be opened in the specified mode\n";
  case ePvErrUnplugged:
    return "Camera was unplugged\n";
  case ePvErrInvalidSetup:
    return "Setup is invalid (an attribute is invalid)\n";
  case ePvErrResources:
    return "System/network resources or memory not available\n";
  case ePvErrBandwidth:
    return "1394 bandwidth not available\n";
  case ePvErrQueueFull:
    return "Too many frames on queue\n";
  case ePvErrBufferTooSmall:
    return "Frame buffer is too small\n";
  case ePvErrCancelled:
    return "Frame cancelled by user\n";
  case ePvErrDataLost:
    return "The data for the frame was lost\n";
  case ePvErrDataMissing:
    return "Some data in the frame is missing\n";
  case ePvErrTimeout:
    return "Timeout during wait\n";
  case ePvErrOutOfRange:
    return "Attribute value is out of the expected range\n";
  case ePvErrWrongType:
    return "Attribute is not this type (wrong access function)\n";
  case ePvErrForbidden:
    return "Attribute write forbidden at this time\n";
  case ePvErrUnavailable:
    return "Attribute is not available at this time\n";
  default:
    return "sorry, an error occured\n";
  }
}

extern "C" {

  char g_ObjectName[32] = "Camera Procilica Object";
  static y_userobj_t camera_yObj = { g_ObjectName, &camera_free, 0, &camera_eval,&camera_extract, 0 };

  void Y_camera_obj(int argc) {
    tCamera *camera = (tCamera *) ypush_obj(&camera_yObj, sizeof(tCamera));
    //    memset(handle,0,sizeof(tCamera));

    try {
      // initialise the Prosilica API
      if (!PvInitialize()) {
	// wait for a camera to be plugged
	waitForCamera();

	// get a camera from the list
	cameraGet(camera);

	// setup the camera
	cameraSetup(camera);
      } else {
	throw "failed to initialise the API\n";
      }
    } catch ( string msg ) {
      y_error(msg.c_str());
    }
    catch ( char const * msg ) {
      y_error(msg);
    }

  }

  void camera_eval(void *obj, int argc) {
    tCamera *camera = (tCamera *)obj;
    try {
      ystring_t func = ygets_q(argc-1);
      if(strcmp(func, "start")==0){
	long packetSize=8228;
	if(argc >1)  packetSize = ygets_l(argc-2);
	cameraStart(camera, packetSize);
	cameraSnap(camera);
      } else if(strcmp(func, "stop")==0){
	cameraStop(camera);
      } else if(strcmp(func, "getROI")==0){
	long dims[2]={1, 4};
	long *result = ypush_l(dims);
	tPvUint32 ROI[4];
	cameraGetROI(camera, ROI);
	for(int i=0;i<4;i++)  result[i] = ROI[i];
      } else if(strcmp(func, "setROI")==0){
	long ntot, dims;
	long *result = ygeta_l(argc-2, &ntot, &dims);
	tPvUint32 ROI[4];
	for(int i=0;i<4;i++) ROI[i]=result[i];
	cameraSetROI(camera, ROI);	
      } else if(strcmp(func, "getExpo")==0){
	tPvUint32 expo;
	cameraGetExpo(camera, &expo);
	ypush_long(expo);
     } else if(strcmp(func, "setExpo")==0){
	long expo = ygets_l(argc-2);
	cameraSetExpo(camera, expo);
      } else if(strcmp(func, "fastsnap")==0){
	long param = ygets_l(argc-2);
	unsigned long width = camera->Frame.Width;              // Image width
	unsigned long height= camera->Frame.Height;             // Image height
	long dims[4]={3, width, height, param};
	if(param ==1) dims[0]=2;
	char resValue[16];
	cameraGetPixelFormat(camera, resValue, 16);
	tPvFrame *buff = cameraSnap(camera, param);

	if(strcmp(resValue, "Mono8")==0) {
	  short *frame = ypush_s(dims);
	  for(long pose=0; pose<param; pose++){
	    unsigned char *src = (unsigned char *)buff[pose].ImageBuffer;
	    short *dst = &frame[pose*height*width];
	    for(unsigned long i=0; i<width*height; i++) {
	      dst[i] = src[i];
	    }
	  }
	} else if(strcmp(resValue, "Mono16")==0) {
	  int *frame = ypush_i(dims);
	  for(long pose=0; pose<param; pose++){
	    unsigned short *src = (unsigned short *)buff[pose].ImageBuffer;
	    int *dst = &frame[pose*height*width];
	    for(unsigned long i=0; i<width*height; i++) {
	      dst[i] = src[i];
	    }
	  }
	} else cout << "bad ppixel size !" << endl;
      } else if(strcmp(func, "snap")==0){
	long param = ygets_l(argc-2);
	unsigned long width = camera->Frame.Width;              // Image width
	unsigned long height= camera->Frame.Height;             // Image height
	long dims[4]={3, width, height, param};
	if(param ==1) dims[0]=2;
	char resValue[16];
	cameraGetPixelFormat(camera, resValue, 16);
	
	if(strcmp(resValue, "Mono8")==0) {
	  short *frame = ypush_s(dims);
	  for(long pose=0; pose<param; pose++){
	    cameraSnap(camera);
	    unsigned char *src = (unsigned char *)camera->Frame.ImageBuffer;
	    short *dst = &frame[pose*height*width];
	    for(unsigned long i=0; i<width*height; i++) {
	      dst[i] = src[i];
	    }
	  }
	} else if(strcmp(resValue, "Mono16")==0) {
	  int *frame = ypush_i(dims);
	  for(long pose=0; pose<param; pose++){
	    cameraSnap(camera);
	    unsigned short *src = (unsigned short *)camera->Frame.ImageBuffer;
	    int *dst = &frame[pose*height*width];
	    for(unsigned long i=0; i<width*height; i++) {
	      dst[i] = src[i];
	    }
	  }
	} else cout << "bad ppixel size !" << endl;
      } else cout << "FUCK !" << endl;

    } catch ( string msg ) {
      y_error(msg.c_str());
    }
    catch ( char const * msg ) {
      y_error(msg);
    }
  }

  void camera_extract(void *obj, char *func){
    return;
    //tCamera *camera = (tCamera *)obj;
    try {
      cout << func << endl;
      vector<string> elems = split(func, '_');
      /*
      cout << "size : " << elems.size() << endl;
      vector<string>::iterator it;
      cout << "elems contains:";
      for ( it=elems.begin() ; it < elems.end(); it++ )
	cout << " " << *it;
      cout << endl;
      */
      if(elems[0] == "start") cout << "START !" << endl;
      else if(elems[0] == "stop") cout << "STOP !" << endl;
      else cout << "FUCK !" << endl;
      long toto = ygets_l(0);
      ypush_long(toto);
    } catch ( string msg ) {
      y_error(msg.c_str());
    }
    catch ( char const * msg ) {
      y_error(msg);
    }
  }


  void camera_free(void *obj) {
    tCamera *camera = (tCamera *) obj;
    try {
      // unsetup the camera
      cameraUnsetup(camera);

      // uninitialise the API
      PvUnInitialize();

    } catch ( string msg ) {
      y_error(msg.c_str());
    }
    catch ( char const * msg ) {
      y_error(msg);
    }
  }

}
