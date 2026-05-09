################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-972706993: ../empty.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"E:/ti/sysconfig_1.26.2/sysconfig_cli.bat" --script "C:/Users/Administrator/workspace_ccstheia/25E/empty.syscfg" -o "." -s "E:/ti/mspm0_sdk_2_10_00_04/.metadata/product.json" --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-972706993 ../empty.syscfg
device.opt: build-972706993
device.cmd.genlibs: build-972706993
ti_msp_dl_config.c: build-972706993
ti_msp_dl_config.h: build-972706993
Event.dot: build-972706993

%.o: ./%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"E:/ti/ccs/tools/compiler/ti-cgt-armllvm_4.0.0.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/Administrator/workspace_ccstheia/25E" -I"C:/Users/Administrator/workspace_ccstheia/25E/Debug" -I"E:/ti/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"E:/ti/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0g350x_ticlang.o: E:/ti/mspm0_sdk_2_10_00_04/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"E:/ti/ccs/tools/compiler/ti-cgt-armllvm_4.0.0.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/Administrator/workspace_ccstheia/25E" -I"C:/Users/Administrator/workspace_ccstheia/25E/Debug" -I"E:/ti/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"E:/ti/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"E:/ti/ccs/tools/compiler/ti-cgt-armllvm_4.0.0.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/Administrator/workspace_ccstheia/25E" -I"C:/Users/Administrator/workspace_ccstheia/25E/Debug" -I"E:/ti/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"E:/ti/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


