################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tools/bwt-to-sa/bwt-to-sa.cpp 

OBJS += \
./tools/bwt-to-sa/bwt-to-sa.o 

CPP_DEPS += \
./tools/bwt-to-sa/bwt-to-sa.d 


# Each subdirectory must supply rules for building sources it contributes
tools/bwt-to-sa/%.o: ../tools/bwt-to-sa/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


