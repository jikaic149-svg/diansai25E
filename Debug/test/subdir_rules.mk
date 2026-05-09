################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-640746479: ../test/empty.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"F:/ti/ccstheia151/sysconfig_1.26.2/sysconfig_cli.bat" --script "C:/Users/26755/workspace_ccstheia/test/test/empty.syscfg" -o "." -s "F:/ti/ccstheia151/mspm0_sdk_2_10_00_04/.metadata/product.json" --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-640746479 ../test/empty.syscfg
device.opt: build-640746479
device.cmd.genlibs: build-640746479
ti_msp_dl_config.c: build-640746479
ti_msp_dl_config.h: build-640746479
Event.dot: build-640746479

test/startup_mspm0g350x_ticlang.o: F:/ti/ccstheia151/mspm0_sdk_2_10_00_04/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"F:/ti/ccstheia151/ccs/tools/compiler/ti-cgt-armllvm_4.0.0.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/26755/workspace_ccstheia/test" -I"C:/Users/26755/workspace_ccstheia/test/Debug" -I"F:/ti/ccstheia151/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"F:/ti/ccstheia151/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

test/%.o: ../test/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"F:/ti/ccstheia151/ccs/tools/compiler/ti-cgt-armllvm_4.0.0.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/26755/workspace_ccstheia/test" -I"C:/Users/26755/workspace_ccstheia/test/Debug" -I"F:/ti/ccstheia151/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"F:/ti/ccstheia151/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"test/$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


