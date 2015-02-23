################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tools/bwt-invert/bwt-invert.cpp 

OBJS += \
./tools/bwt-invert/bwt-invert.o 

CPP_DEPS += \
./tools/bwt-invert/bwt-invert.d 


# Each subdirectory must supply rules for building sources it contributes
tools/bwt-invert/%.o: ../tools/bwt-invert/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


