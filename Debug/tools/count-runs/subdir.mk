################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tools/count-runs/count-runs.cpp 

OBJS += \
./tools/count-runs/count-runs.o 

CPP_DEPS += \
./tools/count-runs/count-runs.d 


# Each subdirectory must supply rules for building sources it contributes
tools/count-runs/%.o: ../tools/count-runs/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


