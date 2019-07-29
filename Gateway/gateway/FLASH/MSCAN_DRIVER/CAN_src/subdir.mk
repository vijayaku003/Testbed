################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../MSCAN_DRIVER/CAN_src/msCANID.c" \
"../MSCAN_DRIVER/CAN_src/msCANdrv.c" \
"../MSCAN_DRIVER/CAN_src/msCANgv.c" \

C_SRCS += \
../MSCAN_DRIVER/CAN_src/msCANID.c \
../MSCAN_DRIVER/CAN_src/msCANdrv.c \
../MSCAN_DRIVER/CAN_src/msCANgv.c \

OBJS += \
./MSCAN_DRIVER/CAN_src/msCANID.o \
./MSCAN_DRIVER/CAN_src/msCANdrv.o \
./MSCAN_DRIVER/CAN_src/msCANgv.o \

C_DEPS += \
./MSCAN_DRIVER/CAN_src/msCANID.d \
./MSCAN_DRIVER/CAN_src/msCANdrv.d \
./MSCAN_DRIVER/CAN_src/msCANgv.d \

OBJS_QUOTED += \
"./MSCAN_DRIVER/CAN_src/msCANID.o" \
"./MSCAN_DRIVER/CAN_src/msCANdrv.o" \
"./MSCAN_DRIVER/CAN_src/msCANgv.o" \

C_DEPS_QUOTED += \
"./MSCAN_DRIVER/CAN_src/msCANID.d" \
"./MSCAN_DRIVER/CAN_src/msCANdrv.d" \
"./MSCAN_DRIVER/CAN_src/msCANgv.d" \

OBJS_OS_FORMAT += \
./MSCAN_DRIVER/CAN_src/msCANID.o \
./MSCAN_DRIVER/CAN_src/msCANdrv.o \
./MSCAN_DRIVER/CAN_src/msCANgv.o \


# Each subdirectory must supply rules for building sources it contributes
MSCAN_DRIVER/CAN_src/msCANID.o: ../MSCAN_DRIVER/CAN_src/msCANID.c
	@echo 'Building file: $<'
	@echo 'Executing target #8 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"MSCAN_DRIVER/CAN_src/msCANID.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"MSCAN_DRIVER/CAN_src/msCANID.o"
	@echo 'Finished building: $<'
	@echo ' '

MSCAN_DRIVER/CAN_src/msCANdrv.o: ../MSCAN_DRIVER/CAN_src/msCANdrv.c
	@echo 'Building file: $<'
	@echo 'Executing target #9 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"MSCAN_DRIVER/CAN_src/msCANdrv.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"MSCAN_DRIVER/CAN_src/msCANdrv.o"
	@echo 'Finished building: $<'
	@echo ' '

MSCAN_DRIVER/CAN_src/msCANgv.o: ../MSCAN_DRIVER/CAN_src/msCANgv.c
	@echo 'Building file: $<'
	@echo 'Executing target #10 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"MSCAN_DRIVER/CAN_src/msCANgv.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"MSCAN_DRIVER/CAN_src/msCANgv.o"
	@echo 'Finished building: $<'
	@echo ' '


