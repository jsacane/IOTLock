#
_XDCBUILDCOUNT = 
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = C:/ti/simplelink_msp432_sdk_1_40_00_27/source;C:/ti/simplelink_msp432_sdk_1_40_00_27/kernel/tirtos/packages;C:/ti/ccs7.1.0.00016/ccsv7/ccs_base
override XDCROOT = C:/ti/ccs7.1.0.00016/xdctools_3_50_01_12_core
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = C:/ti/simplelink_msp432_sdk_1_40_00_27/source;C:/ti/simplelink_msp432_sdk_1_40_00_27/kernel/tirtos/packages;C:/ti/ccs7.1.0.00016/ccsv7/ccs_base;C:/ti/ccs7.1.0.00016/xdctools_3_50_01_12_core/packages;..
HOSTOS = Windows
endif
