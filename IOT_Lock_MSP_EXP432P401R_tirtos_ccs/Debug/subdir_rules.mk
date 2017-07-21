################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
MSP_EXP432P401R.obj: ../MSP_EXP432P401R.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccs7.1.0.00016/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/a0232360/DIY2017/IOT_Lock_MSP_EXP432P401R_tirtos_ccs" --include_path="C:/ti/simplelink_msp432_sdk_wifi_plugin_1_40_00_01/source" --include_path="C:/ti/simplelink_msp432_sdk_1_40_01_00/source/third_party/CMSIS/Include" --include_path="C:/ti/simplelink_msp432_sdk_1_40_01_00/kernel/tirtos/packages/ti/sysbios/posix" --include_path="C:/ti/ccs7.1.0.00016/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/include" --advice:power=none --define=__MSP432P401R__ --define=DeviceFamily_MSP432P401x -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="MSP_EXP432P401R.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

client_cbs.obj: ../client_cbs.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccs7.1.0.00016/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/a0232360/DIY2017/IOT_Lock_MSP_EXP432P401R_tirtos_ccs" --include_path="C:/ti/simplelink_msp432_sdk_wifi_plugin_1_40_00_01/source" --include_path="C:/ti/simplelink_msp432_sdk_1_40_01_00/source/third_party/CMSIS/Include" --include_path="C:/ti/simplelink_msp432_sdk_1_40_01_00/kernel/tirtos/packages/ti/sysbios/posix" --include_path="C:/ti/ccs7.1.0.00016/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/include" --advice:power=none --define=__MSP432P401R__ --define=DeviceFamily_MSP432P401x -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="client_cbs.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main_tirtos.obj: ../main_tirtos.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccs7.1.0.00016/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/a0232360/DIY2017/IOT_Lock_MSP_EXP432P401R_tirtos_ccs" --include_path="C:/ti/simplelink_msp432_sdk_wifi_plugin_1_40_00_01/source" --include_path="C:/ti/simplelink_msp432_sdk_1_40_01_00/source/third_party/CMSIS/Include" --include_path="C:/ti/simplelink_msp432_sdk_1_40_01_00/kernel/tirtos/packages/ti/sysbios/posix" --include_path="C:/ti/ccs7.1.0.00016/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/include" --advice:power=none --define=__MSP432P401R__ --define=DeviceFamily_MSP432P401x -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="main_tirtos.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

mqtt_client_app.obj: ../mqtt_client_app.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccs7.1.0.00016/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/a0232360/DIY2017/IOT_Lock_MSP_EXP432P401R_tirtos_ccs" --include_path="C:/ti/simplelink_msp432_sdk_wifi_plugin_1_40_00_01/source" --include_path="C:/ti/simplelink_msp432_sdk_1_40_01_00/source/third_party/CMSIS/Include" --include_path="C:/ti/simplelink_msp432_sdk_1_40_01_00/kernel/tirtos/packages/ti/sysbios/posix" --include_path="C:/ti/ccs7.1.0.00016/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/include" --advice:power=none --define=__MSP432P401R__ --define=DeviceFamily_MSP432P401x -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="mqtt_client_app.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

network_if.obj: ../network_if.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccs7.1.0.00016/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/a0232360/DIY2017/IOT_Lock_MSP_EXP432P401R_tirtos_ccs" --include_path="C:/ti/simplelink_msp432_sdk_wifi_plugin_1_40_00_01/source" --include_path="C:/ti/simplelink_msp432_sdk_1_40_01_00/source/third_party/CMSIS/Include" --include_path="C:/ti/simplelink_msp432_sdk_1_40_01_00/kernel/tirtos/packages/ti/sysbios/posix" --include_path="C:/ti/ccs7.1.0.00016/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/include" --advice:power=none --define=__MSP432P401R__ --define=DeviceFamily_MSP432P401x -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="network_if.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

uart_term.obj: ../uart_term.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccs7.1.0.00016/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/a0232360/DIY2017/IOT_Lock_MSP_EXP432P401R_tirtos_ccs" --include_path="C:/ti/simplelink_msp432_sdk_wifi_plugin_1_40_00_01/source" --include_path="C:/ti/simplelink_msp432_sdk_1_40_01_00/source/third_party/CMSIS/Include" --include_path="C:/ti/simplelink_msp432_sdk_1_40_01_00/kernel/tirtos/packages/ti/sysbios/posix" --include_path="C:/ti/ccs7.1.0.00016/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/include" --advice:power=none --define=__MSP432P401R__ --define=DeviceFamily_MSP432P401x -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="uart_term.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


