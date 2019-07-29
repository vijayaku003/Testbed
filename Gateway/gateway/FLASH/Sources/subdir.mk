################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../Sources/CLK.c" \
"../Sources/MSCAN_Module.c" \
"../Sources/UART.c" \
"../Sources/main.c" \

C_SRCS += \
../Sources/CLK.c \
../Sources/MSCAN_Module.c \
../Sources/UART.c \
../Sources/main.c \

OBJS += \
./Sources/CLK.o \
./Sources/MSCAN_Module.o \
./Sources/UART.o \
./Sources/main.o \

C_DEPS += \
./Sources/CLK.d \
./Sources/MSCAN_Module.d \
./Sources/UART.d \
./Sources/main.d \

OBJS_QUOTED += \
"./Sources/CLK.o" \
"./Sources/MSCAN_Module.o" \
"./Sources/UART.o" \
"./Sources/main.o" \

C_DEPS_QUOTED += \
"./Sources/CLK.d" \
"./Sources/MSCAN_Module.d" \
"./Sources/UART.d" \
"./Sources/main.d" \

OBJS_OS_FORMAT += \
./Sources/CLK.o \
./Sources/MSCAN_Module.o \
./Sources/UART.o \
./Sources/main.o \


# Each subdirectory must supply rules for building sources it contributes
Sources/CLK.o: ../Sources/CLK.c
	@echo 'Building file: $<'
	@echo 'Executing target #1 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/CLK.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/CLK.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/MSCAN_Module.o: ../Sources/MSCAN_Module.c
	@echo 'Building file: $<'
	@echo 'Executing target #2 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/MSCAN_Module.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/MSCAN_Module.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/UART.o: ../Sources/UART.c
	@echo 'Building file: $<'
	@echo 'Executing target #3 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/UART.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/UART.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/main.o: ../Sources/main.c
	@echo 'Building file: $<'
	@echo 'Executing target #4 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/main.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/main.o"
	@echo 'Finished building: $<'
	@echo ' '


