# these values filled in by    yorick -batch make.i
Y_MAKEDIR=/home/sthijs/yorick.git/relocate
Y_EXE=/home/sthijs/yorick.git/relocate/bin/yorick
Y_EXE_PKGS=

Y_EXE_HOME=/home/sthijs/yorick.git/relocate
Y_EXE_SITE=/home/sthijs/yorick.git/relocate
Y_HOME_PKG=

# ----------------------------------------------------- optimization flags

# options for make command line, e.g.-   make COPT=-g TGT=exe
COPT=$(COPT_DEFAULT)
TGT=$(DEFAULT_TGT)
# ------------------------------------------------ macros for this package

SDK_PATH = AVT_GigE_SDK

# Target CPU
CPU     = x64

# Target OS
OS      = LINUX

# compiler version
CVER    = 4.5
# compiler
#GCC     = g++-$(CVER)

PKG_NAME=libprosilica
PKG_I=prosilica.i

OBJS=prosilica.o

# ========== specifies sources and build directory  =============

# change to give the executable a name other than yorick
PKG_EXENAME=yorick

# PKG_DEPLIBS=-Lsomedir -lsomelib   for dependencies of this package
PKG_CFLAGS= 	-Wall \
		-fno-strict-aliasing -fexceptions \
		-I/usr/include -I$(SDK_PATH)/inc-pc \
		-D_$(CPU) -D_$(OS) -D_REENTRANT

PKG_LDFLAGS= 	

PKG_DEPLIBS= 	-lpthread -lz -lm -lc -lstdc++ \
		-L$(SDK_PATH)/bin-pc/$(CPU) -lPvAPI
#		-Bstatic $(SDK_PATH)/lib-pc/$(CPU)/$(CVER)/libPvAPI.a

# list of additional package names you want in PKG_EXENAME
# (typically Y_EXE_PKGS should be first here)
EXTRA_PKGS=$(Y_EXE_PKGS)

# list of additional files for clean
PKG_CLEAN=

# autoload file for this package, if any
PKG_I_START=
# non-pkg.i include files for this package, if any
PKG_I_EXTRA= 

# -------------------------------- standard targets and rules (in Makepkg)

# set macros Makepkg uses in target and dependency names
# DLL_TARGETS, LIB_TARGETS, EXE_TARGETS
# are any additional targets (defined below) prerequisite to
# the plugin library, archive library, and executable, respectively
PKG_I_DEPS=$(PKG_I)
Y_DISTMAKE=distmake

include $(Y_MAKEDIR)/Make.cfg
include $(Y_MAKEDIR)/Makepkg
include $(Y_MAKEDIR)/Make$(TGT)

CPPFLAGS = $(CFLAGS)

# override macros Makepkg sets for rules and other macros
# Y_HOME and Y_SITE in Make.cfg may not be correct (e.g.- relocatable)
Y_HOME=$(Y_EXE_HOME)
Y_SITE=$(Y_EXE_SITE)

# reduce chance of yorick-1.5 corrupting this Makefile
MAKE_TEMPLATE = protect-against-1.5

# ------------------------------------- targets and rules for this package
# color for printf
#31=red, 32=green, 33=yellow,34=blue, 35=pink, 36=cyan, 37=white

# -------------------------------------------------------- end of Makefile
