################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../extern/bitvector/test/main.cpp 

OBJS += \
./extern/bitvector/test/main.o 

CPP_DEPS += \
./extern/bitvector/test/main.d 


# Each subdirectory must supply rules for building sources it contributes
extern/bitvector/test/%.o: ../extern/bitvector/test/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


