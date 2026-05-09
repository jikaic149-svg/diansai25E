################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
SYSCFG_SRCS += \
../test/empty.syscfg 

C_SRCS += \
../test/isr.c \
../test/main.c \
../test/schedule.c 

C_DEPS += \
./ti_msp_dl_config.d \
./startup_mspm0g350x_ticlang.d \
./test/isr.d \
./test/main.d \
./test/schedule.d 

OBJS += \
./test/startup_mspm0g350x_ticlang.o \
./test/isr.o \
./test/main.o \
./test/schedule.o 

GEN_MISC_FILES += \
./device.cmd.genlibs \
./ti_msp_dl_config.h \
./Event.dot 

OBJS__QUOTED += \
"test\startup_mspm0g350x_ticlang.o" \
"test\isr.o" \
"test\main.o" \
"test\schedule.o" 

GEN_MISC_FILES__QUOTED += \
"device.cmd.genlibs" \
"ti_msp_dl_config.h" \
"Event.dot" 

C_DEPS__QUOTED += \
"ti_msp_dl_config.d" \
"startup_mspm0g350x_ticlang.d" \
"test\isr.d" \
"test\main.d" \
"test\schedule.d" 

SYSCFG_SRCS__QUOTED += \
"../test/empty.syscfg" 

C_SRCS__QUOTED += \
"../test/isr.c" \
"../test/main.c" \
"../test/schedule.c" 


