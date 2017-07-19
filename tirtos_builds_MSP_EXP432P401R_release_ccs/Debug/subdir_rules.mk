################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-223847199:
	@$(MAKE) -Onone -f subdir_rules.mk build-223847199-inproc

build-223847199-inproc: ../release.cfg
	@echo 'Building file: $<'
	@echo 'Invoking: XDCtools'
	"C:/ti/ccs7.1.0.00016/xdctools_3_50_01_12_core/xs" --xdcpath="C:/ti/simplelink_msp432_sdk_1_40_00_27/source;C:/ti/simplelink_msp432_sdk_1_40_00_27/kernel/tirtos/packages;C:/ti/ccs7.1.0.00016/ccsv7/ccs_base;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M4F -p ti.platforms.msp432:MSP432P401R -r release -c "C:/ti/ccs7.1.0.00016/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS" "$<"
	@echo 'Finished building: $<'
	@echo ' '

configPkg/linker.cmd: build-223847199 ../release.cfg
configPkg/compiler.opt: build-223847199
configPkg/: build-223847199


