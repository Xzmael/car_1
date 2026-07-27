################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := E:/ccs_app/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS

GEN_OPTS__FLAG := @"./device.opt" 
GEN_CMDS__FLAG := -Wl,-l"./device_linker.cmd" 

ORDERED_OBJS += \
"./empty.o" \
"./ti_msp_dl_config.o" \
"./startup_mspm0g350x_ticlang.o" \
"./Hardware/IMU660RB/Fusion/FusionAhrs.o" \
"./Hardware/IMU660RB/Fusion/FusionCompass.o" \
"./Hardware/IMU660RB/Fusion/FusionOffset.o" \
"./Hardware/IMU660RB/imu660rb.o" \
"./Hardware/IMU660RB/lsm6dsr_reg.o" \
"./Hardware/buzzer.o" \
"./Hardware/gray.o" \
"./Hardware/key.o" \
"./Hardware/led.o" \
"./Hardware/motor.o" \
"./Hardware/oled.o" \
"./Hardware/tft.o" \
"./Hardware/vision_uart.o" \
"./Task/task1.o" \
"./Task/task2.o" \
"./Task/task3.o" \
"./Task/task4.o" \
"./libruary/line_control.o" \
"./libruary/task_manager.o" \
$(GEN_CMDS__FLAG) \
-Wl,-ldevice.cmd.genlibs \
-Wl,-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include Hardware/IMU660RB/Fusion/subdir_vars.mk
-include Hardware/IMU660RB/subdir_vars.mk
-include Hardware/subdir_vars.mk
-include Task/subdir_vars.mk
-include libruary/subdir_vars.mk
-include subdir_rules.mk
-include Hardware/IMU660RB/Fusion/subdir_rules.mk
-include Hardware/IMU660RB/subdir_rules.mk
-include Hardware/subdir_rules.mk
-include Task/subdir_rules.mk
-include libruary/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
car_1_1.out 

EXE_OUTPUTS__QUOTED += \
"car_1_1.out" 

BIN_OUTPUTS += \
car_1_1.hex 

BIN_OUTPUTS__QUOTED += \
"car_1_1.hex" 


# All Target
all: $(OBJS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "car_1_1.out" secondary-outputs

# Tool invocations
car_1_1.out: $(OBJS) $(GEN_CMDS)
	@echo 'Arm Linker - building target: "$@"'
	"E:/ccs_app/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/bin/tiarmclang.exe" @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -g -Wall -Wl,-m"car_1_1.map" -Wl,-i"E:/ccs_app/mspm0_sdk_2_11_00_07/source" -Wl,-i"C:/Users/Luxury/workspace_ccstheia/car_1_1" -Wl,-i"C:/Users/Luxury/workspace_ccstheia/car_1_1/Debug/syscfg" -Wl,-i"E:/ccs_app/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/lib" -Wl,--diag_wrap=off -Wl,--display_error_number -Wl,--warn_sections -Wl,--xml_link_info="car_1_1_linkInfo.xml" -Wl,--rom_model -o "car_1_1.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

car_1_1.hex: $(EXE_OUTPUTS)
	@echo 'Arm Hex Utility - building secondary target: "$@"'
	"E:/ccs_app/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/bin/tiarmhex.exe" --diag_wrap=off -o "car_1_1.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building secondary target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(GEN_MISC_FILES__QUOTED)$(BIN_OUTPUTS__QUOTED)$(GEN_FILES__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "empty.o" "ti_msp_dl_config.o" "startup_mspm0g350x_ticlang.o" "Hardware\IMU660RB\Fusion\FusionAhrs.o" "Hardware\IMU660RB\Fusion\FusionCompass.o" "Hardware\IMU660RB\Fusion\FusionOffset.o" "Hardware\IMU660RB\imu660rb.o" "Hardware\IMU660RB\lsm6dsr_reg.o" "Hardware\buzzer.o" "Hardware\gray.o" "Hardware\key.o" "Hardware\led.o" "Hardware\motor.o" "Hardware\oled.o" "Hardware\tft.o" "Hardware\vision_uart.o" "Task\task1.o" "Task\task2.o" "Task\task3.o" "Task\task4.o" "libruary\line_control.o" "libruary\task_manager.o" 
	-$(RM) "empty.d" "ti_msp_dl_config.d" "startup_mspm0g350x_ticlang.d" "Hardware\IMU660RB\Fusion\FusionAhrs.d" "Hardware\IMU660RB\Fusion\FusionCompass.d" "Hardware\IMU660RB\Fusion\FusionOffset.d" "Hardware\IMU660RB\imu660rb.d" "Hardware\IMU660RB\lsm6dsr_reg.d" "Hardware\buzzer.d" "Hardware\gray.d" "Hardware\key.d" "Hardware\led.d" "Hardware\motor.d" "Hardware\oled.d" "Hardware\tft.d" "Hardware\vision_uart.d" "Task\task1.d" "Task\task2.d" "Task\task3.d" "Task\task4.d" "libruary\line_control.d" "libruary\task_manager.d" 
	-@echo ' '

secondary-outputs: $(BIN_OUTPUTS)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

