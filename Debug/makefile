################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include tools/test/subdir.mk
-include tools/sa-to-bwt/subdir.mk
-include tools/sFM-index/subdir.mk
-include tools/lz77/subdir.mk
-include tools/dB-hash/subdir.mk
-include tools/cw-bwt/subdir.mk
-include tools/count-runs/subdir.mk
-include tools/bwt-to-sa/subdir.mk
-include tools/bwt-invert/subdir.mk
-include tools/bwt-check/subdir.mk
-include extern/bitvector/test/subdir.mk
-include bin/CMakeFiles/test.dir/tools/test/subdir.mk
-include bin/CMakeFiles/2.8.12.2/CompilerIdCXX/subdir.mk
-include bin/CMakeFiles/2.8.12.2/CompilerIdC/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: BWTIL

# Tool invocations
BWTIL: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross G++ Linker'
	g++  -o "BWTIL" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(C++_DEPS)$(OBJS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) BWTIL
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
