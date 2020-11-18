##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=joystick2mqtt
ConfigurationName      :=Debug
WorkspacePath          :=/home/lieven/workspace/ws1
ProjectPath            :=/home/lieven/workspace/joystick2mqtt
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Lieven
Date                   :=18/11/20
CodeLitePath           :=/home/lieven/.codelite
LinkerName             :=/usr/bin/g++
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="joystick2mqtt.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  -l:libpaho-mqtt3c.a
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)src $(IncludeSwitch)../Common $(IncludeSwitch)../paho.mqtt.c/src $(IncludeSwitch)../ArduinoJson/src 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)Common $(LibrarySwitch)pthread 
ArLibs                 :=  "Common" "pthread" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)../Common/Debug $(LibraryPathSwitch)../paho.mqtt.c/build/output 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=  -g -O0 -Wall $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/src_Joystick2Mqtt.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Timer.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Main.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Sys.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@test -d ./Debug || $(MakeDirCommand) ./Debug


$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/src_Joystick2Mqtt.cpp$(ObjectSuffix): src/Joystick2Mqtt.cpp $(IntermediateDirectory)/src_Joystick2Mqtt.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/joystick2mqtt/src/Joystick2Mqtt.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Joystick2Mqtt.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Joystick2Mqtt.cpp$(DependSuffix): src/Joystick2Mqtt.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Joystick2Mqtt.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Joystick2Mqtt.cpp$(DependSuffix) -MM src/Joystick2Mqtt.cpp

$(IntermediateDirectory)/src_Joystick2Mqtt.cpp$(PreprocessSuffix): src/Joystick2Mqtt.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Joystick2Mqtt.cpp$(PreprocessSuffix) src/Joystick2Mqtt.cpp

$(IntermediateDirectory)/src_Timer.cpp$(ObjectSuffix): src/Timer.cpp $(IntermediateDirectory)/src_Timer.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/joystick2mqtt/src/Timer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Timer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Timer.cpp$(DependSuffix): src/Timer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Timer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Timer.cpp$(DependSuffix) -MM src/Timer.cpp

$(IntermediateDirectory)/src_Timer.cpp$(PreprocessSuffix): src/Timer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Timer.cpp$(PreprocessSuffix) src/Timer.cpp

$(IntermediateDirectory)/src_Main.cpp$(ObjectSuffix): src/Main.cpp $(IntermediateDirectory)/src_Main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/joystick2mqtt/src/Main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Main.cpp$(DependSuffix): src/Main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Main.cpp$(DependSuffix) -MM src/Main.cpp

$(IntermediateDirectory)/src_Main.cpp$(PreprocessSuffix): src/Main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Main.cpp$(PreprocessSuffix) src/Main.cpp

$(IntermediateDirectory)/src_Sys.cpp$(ObjectSuffix): src/Sys.cpp $(IntermediateDirectory)/src_Sys.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/joystick2mqtt/src/Sys.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Sys.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Sys.cpp$(DependSuffix): src/Sys.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Sys.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Sys.cpp$(DependSuffix) -MM src/Sys.cpp

$(IntermediateDirectory)/src_Sys.cpp$(PreprocessSuffix): src/Sys.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Sys.cpp$(PreprocessSuffix) src/Sys.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


