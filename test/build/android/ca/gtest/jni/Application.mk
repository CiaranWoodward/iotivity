APP_STL := gnustl_shared
APP_PLATFORM = android-21
APP_CPPFLAGS += -fexceptions
APP_CPPFLAGS += -frtti
APP_ABI := $(DEVICE_ARCH)
APP_CFLAGS = -Wno-error=format-security
