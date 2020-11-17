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
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)../Common $(IncludeSwitch)../paho.mqtt.c/src $(IncludeSwitch)../ArduinoJson/src 
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
Objects0=$(IntermediateDirectory)/Main.cpp$(ObjectSuffix) $(IntermediateDirectory)/Timer.cpp$(ObjectSuffix) $(IntermediateDirectory)/Sys.cpp$(ObjectSuffix) $(IntermediateDirectory)/Joystick2Mqtt.cpp$(ObjectSuffix) 



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
$(IntermediateDirectory)/Main.cpp$(ObjectSuffix): Main.cpp $(IntermediateDirectory)/Main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/joystick2mqtt/Main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Main.cpp$(DependSuffix): Main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Main.cpp$(DependSuffix) -MM Main.cpp

$(IntermediateDirectory)/Main.cpp$(PreprocessSuffix): Main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Main.cpp$(PreprocessSuffix) Main.cpp

$(IntermediateDirectory)/Timer.cpp$(ObjectSuffix): Timer.cpp $(IntermediateDirectory)/Timer.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/joystick2mqtt/Timer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Timer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Timer.cpp$(DependSuffix): Timer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Timer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Timer.cpp$(DependSuffix) -MM Timer.cpp

$(IntermediateDirectory)/Timer.cpp$(PreprocessSuffix): Timer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Timer.cpp$(PreprocessSuffix) Timer.cpp

$(IntermediateDirectory)/Sys.cpp$(ObjectSuffix): Sys.cpp $(IntermediateDirectory)/Sys.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/joystick2mqtt/Sys.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Sys.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Sys.cpp$(DependSuffix): Sys.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Sys.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Sys.cpp$(DependSuffix) -MM Sys.cpp

$(IntermediateDirectory)/Sys.cpp$(PreprocessSuffix): Sys.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Sys.cpp$(PreprocessSuffix) Sys.cpp

$(IntermediateDirectory)/Joystick2Mqtt.cpp$(ObjectSuffix): Joystick2Mqtt.cpp $(IntermediateDirectory)/Joystick2Mqtt.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/joystick2mqtt/Joystick2Mqtt.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Joystick2Mqtt.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Joystick2Mqtt.cpp$(DependSuffix): Joystick2Mqtt.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Joystick2Mqtt.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Joystick2Mqtt.cpp$(DependSuffix) -MM Joystick2Mqtt.cpp

$(IntermediateDirectory)/Joystick2Mqtt.cpp$(PreprocessSuffix): Joystick2Mqtt.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Joystick2Mqtt.cpp$(PreprocessSuffix) Joystick2Mqtt.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


