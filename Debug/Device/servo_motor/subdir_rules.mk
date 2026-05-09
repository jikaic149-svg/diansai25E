################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
Device/servo_motor/%.o: ../Device/servo_motor/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"E:/ti/ccs/tools/compiler/ti-cgt-armllvm_4.0.0.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/Administrator/workspace_ccstheia/25E" -I"C:/Users/Administrator/workspace_ccstheia/25E/Debug" -I"E:/ti/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"E:/ti/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -MMD -MP -MF"Device/servo_motor/$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


