################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
test/M0G3507_PID_310s/%.o: ../test/M0G3507_PID_310s/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"F:/ti/ccstheia151/ccs/tools/compiler/ti-cgt-armllvm_4.0.0.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/26755/workspace_ccstheia/test" -I"C:/Users/26755/workspace_ccstheia/test/Debug" -I"F:/ti/ccstheia151/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"F:/ti/ccstheia151/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"test/M0G3507_PID_310s/$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-2057871423: ../test/M0G3507_PID_310s/main.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"F:/ti/ccstheia151/sysconfig_1.26.2/sysconfig_cli.bat" --script "C:/Users/26755/workspace_ccstheia/test/test/M0G3507_PID_310s/main.syscfg" -o "." -s "F:/ti/ccstheia151/mspm0_sdk_2_10_00_04/.metadata/product.json" --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-2057871423 ../test/M0G3507_PID_310s/main.syscfg
device.opt: build-2057871423
device.cmd.genlibs: build-2057871423
ti_msp_dl_config.c: build-2057871423
ti_msp_dl_config.h: build-2057871423
Event.dot: build-2057871423

test/M0G3507_PID_310s/startup_mspm0g350x_ticlang.o: F:/ti/ccstheia151/mspm0_sdk_2_10_00_04/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"F:/ti/ccstheia151/ccs/tools/compiler/ti-cgt-armllvm_4.0.0.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/26755/workspace_ccstheia/test" -I"C:/Users/26755/workspace_ccstheia/test/Debug" -I"F:/ti/ccstheia151/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"F:/ti/ccstheia151/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


