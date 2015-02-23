################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tools/sa-to-bwt/sa-to-bwt.cpp 

OBJS += \
./tools/sa-to-bwt/sa-to-bwt.o 

CPP_DEPS += \
./tools/sa-to-bwt/sa-to-bwt.d 


# Each subdirectory must supply rules for building sources it contributes
tools/sa-to-bwt/%.o: ../tools/sa-to-bwt/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


