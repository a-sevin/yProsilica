plug_dir, _("./",plug_dir());
plug_in,"libprosilica";
yoga_version = "0.1";

extern camera_obj;

/*
  cam = camera_obj();
  cam("start", 1500);
  cam("stop");

  pli, cam("snap",1);
  for(i=0; i<1000; i++){pli, cam("snap",1);pause,10;}

  cube= cam("fastsnap",100); info, cube;
  fits_write, "~/Desktop/cube.fits", cube

  //// Taille max
  orig_x = 0;
  orig_y = 0;
  dx = 493;
  dy = 659;
  cam("setROI",[orig_x,orig_y,dx,dy]);
  ////

  //// Taille 50x200
  orig_x = 250;
  orig_y = 0;
  dx = 100;
  dy = 650;
  cam("setROI",[orig_x,orig_y,dx,dy]);
  ////

  cam("getROI");

  cam("getExpo");
  cam("setExpo", 1000); // 0.1ms exposure 

*/
